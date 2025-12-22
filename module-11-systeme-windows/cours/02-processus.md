# Cours 02 : Processus Windows

## Énumérer les Processus

```c
#include <windows.h>
#include <tlhelp32.h>

void list_processes(void) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            printf("PID: %lu, Name: %s\n",
                   pe.th32ProcessID, pe.szExeFile);
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
}
```

## Ouvrir un Processus

```c
// Obtenir un handle vers un processus
HANDLE hProcess = OpenProcess(
    PROCESS_ALL_ACCESS,  // Droits demandés
    FALSE,               // Hériter le handle?
    targetPid            // PID cible
);

if (hProcess == NULL) {
    printf("Erreur: %lu\n", GetLastError());
    return;
}

// Utiliser le handle...

CloseHandle(hProcess);
```

## Créer un Processus

```c
STARTUPINFOA si = {0};
si.cb = sizeof(si);
PROCESS_INFORMATION pi = {0};

BOOL success = CreateProcessA(
    "C:\\Windows\\System32\\notepad.exe",  // Application
    NULL,                                   // Arguments
    NULL,                                   // Process security
    NULL,                                   // Thread security
    FALSE,                                  // Inherit handles
    0,                                      // Creation flags
    NULL,                                   // Environment
    NULL,                                   // Current directory
    &si,                                    // Startup info
    &pi                                     // Process info (sortie)
);

if (success) {
    printf("PID: %lu, TID: %lu\n",
           pi.dwProcessId, pi.dwThreadId);

    // Attendre la fin
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
```

## Créer un Processus Suspendu

```c
// Pour injection avant exécution
CreateProcessA(
    path,
    NULL,
    NULL,
    NULL,
    FALSE,
    CREATE_SUSPENDED,  // Le processus démarre suspendu
    NULL,
    NULL,
    &si,
    &pi
);

// Faire l'injection ici...

// Reprendre l'exécution
ResumeThread(pi.hThread);
```

## Terminer un Processus

```c
HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
TerminateProcess(hProcess, 0);
CloseHandle(hProcess);
```

## Mémoire d'un Autre Processus

```c
HANDLE hProcess = OpenProcess(
    PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION,
    FALSE, pid
);

// Allouer dans le processus cible
LPVOID remoteAddr = VirtualAllocEx(
    hProcess,
    NULL,
    4096,
    MEM_COMMIT | MEM_RESERVE,
    PAGE_EXECUTE_READWRITE
);

// Écrire dans le processus cible
SIZE_T written;
WriteProcessMemory(
    hProcess,
    remoteAddr,
    data,
    dataSize,
    &written
);

// Lire depuis le processus cible
SIZE_T read;
ReadProcessMemory(
    hProcess,
    remoteAddr,
    buffer,
    bufferSize,
    &read
);
```

## Application : Injection de DLL Classique

```c
// 1. Ouvrir le processus cible
HANDLE hProcess = OpenProcess(
    PROCESS_ALL_ACCESS, FALSE, targetPid
);

// 2. Allouer de la mémoire pour le chemin de la DLL
size_t pathLen = strlen(dllPath) + 1;
LPVOID remotePath = VirtualAllocEx(
    hProcess, NULL, pathLen,
    MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
);

// 3. Écrire le chemin de la DLL
WriteProcessMemory(hProcess, remotePath, dllPath, pathLen, NULL);

// 4. Obtenir l'adresse de LoadLibraryA
HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
LPVOID pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");

// 5. Créer un thread distant qui appelle LoadLibrary
HANDLE hThread = CreateRemoteThread(
    hProcess,
    NULL,
    0,
    (LPTHREAD_START_ROUTINE)pLoadLibrary,
    remotePath,
    0,
    NULL
);

// 6. Attendre et nettoyer
WaitForSingleObject(hThread, INFINITE);
VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
CloseHandle(hThread);
CloseHandle(hProcess);
```
