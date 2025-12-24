# Techniques Anti-Analyse

## Introduction

Les techniques anti-analyse visent à rendre le reverse engineering et l'analyse dynamique plus difficiles. Elles ciblent :
- Les **débogueurs** (x64dbg, WinDbg, OllyDbg)
- Les **sandbox** (Any.run, Joe Sandbox, Cuckoo)
- Les **machines virtuelles** (VMware, VirtualBox, Hyper-V)

---

## 1. Détection de Débogueur

### IsDebuggerPresent (basique)

```c
#include <windows.h>

BOOL CheckDebugger_Basic() {
    return IsDebuggerPresent();
}
```

**Contournement** : Patcher le retour ou modifier `PEB.BeingDebugged`

### PEB.BeingDebugged (direct)

```c
/**
 * Accès direct au PEB - plus difficile à hook
 */
BOOL CheckDebugger_PEB() {
#ifdef _WIN64
    // En x64, le PEB est à gs:[0x60]
    PPEB pPeb = (PPEB)__readgsqword(0x60);
#else
    // En x86, le PEB est à fs:[0x30]
    PPEB pPeb = (PPEB)__readfsdword(0x30);
#endif

    return pPeb->BeingDebugged;
}
```

### NtGlobalFlag

```c
/**
 * Quand un débogueur est attaché, NtGlobalFlag contient des flags heap
 */
BOOL CheckDebugger_NtGlobalFlag() {
#ifdef _WIN64
    PPEB pPeb = (PPEB)__readgsqword(0x60);
    DWORD ntGlobalFlag = *(DWORD*)((BYTE*)pPeb + 0xBC);
#else
    PPEB pPeb = (PPEB)__readfsdword(0x30);
    DWORD ntGlobalFlag = *(DWORD*)((BYTE*)pPeb + 0x68);
#endif

    // FLG_HEAP_ENABLE_TAIL_CHECK | FLG_HEAP_ENABLE_FREE_CHECK | FLG_HEAP_VALIDATE_PARAMETERS
    return (ntGlobalFlag & 0x70) != 0;
}
```

### Heap Flags

```c
/**
 * Les flags du heap changent sous débogueur
 */
BOOL CheckDebugger_HeapFlags() {
#ifdef _WIN64
    PPEB pPeb = (PPEB)__readgsqword(0x60);
    PVOID pHeap = (PVOID)(*(DWORD_PTR*)((BYTE*)pPeb + 0x30));
    DWORD flags = *(DWORD*)((BYTE*)pHeap + 0x70);
    DWORD forceFlags = *(DWORD*)((BYTE*)pHeap + 0x74);
#else
    PPEB pPeb = (PPEB)__readfsdword(0x30);
    PVOID pHeap = (PVOID)(*(DWORD*)((BYTE*)pPeb + 0x18));
    DWORD flags = *(DWORD*)((BYTE*)pHeap + 0x40);
    DWORD forceFlags = *(DWORD*)((BYTE*)pHeap + 0x44);
#endif

    // Sous débogueur : HEAP_GROWABLE (0x2) n'est pas le seul flag
    return (flags & ~HEAP_GROWABLE) != 0 || forceFlags != 0;
}
```

### NtQueryInformationProcess

```c
#include <winternl.h>

typedef NTSTATUS (NTAPI* pNtQueryInformationProcess)(
    HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG
);

/**
 * ProcessDebugPort - Retourne le port de debug
 */
BOOL CheckDebugger_DebugPort() {
    pNtQueryInformationProcess NtQueryInformationProcess =
        (pNtQueryInformationProcess)GetProcAddress(
            GetModuleHandleA("ntdll.dll"),
            "NtQueryInformationProcess"
        );

    DWORD_PTR debugPort = 0;
    NTSTATUS status = NtQueryInformationProcess(
        GetCurrentProcess(),
        ProcessDebugPort,  // 7
        &debugPort,
        sizeof(debugPort),
        NULL
    );

    return debugPort != 0;
}

/**
 * ProcessDebugObjectHandle - Vérifie si un debug object existe
 */
BOOL CheckDebugger_DebugObject() {
    pNtQueryInformationProcess NtQueryInformationProcess =
        (pNtQueryInformationProcess)GetProcAddress(
            GetModuleHandleA("ntdll.dll"),
            "NtQueryInformationProcess"
        );

    HANDLE debugObject = NULL;
    NTSTATUS status = NtQueryInformationProcess(
        GetCurrentProcess(),
        ProcessDebugObjectHandle,  // 0x1E
        &debugObject,
        sizeof(debugObject),
        NULL
    );

    // STATUS_PORT_NOT_SET = pas de débogueur
    return status == 0;
}
```

### Timing Attacks

```c
/**
 * Les breakpoints ralentissent l'exécution
 */
BOOL CheckDebugger_Timing() {
    LARGE_INTEGER start, end, freq;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    // Opération rapide
    volatile int x = 0;
    for (int i = 0; i < 1000000; i++) {
        x += i;
    }

    QueryPerformanceCounter(&end);

    double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;

    // Si plus de 100ms pour cette opération = suspect
    return elapsed > 0.1;
}

/**
 * RDTSC - Instruction CPU pour le timing
 */
BOOL CheckDebugger_RDTSC() {
    UINT64 start, end;

    start = __rdtsc();

    // Opération simple
    volatile int x = 0;
    for (int i = 0; i < 1000; i++) x++;

    end = __rdtsc();

    // Environ 10000 cycles sans débogueur, beaucoup plus avec
    return (end - start) > 100000;
}
```

---

## 2. Détection de Sandbox

### Artefacts de sandbox

```c
/**
 * Vérifie les artefacts typiques des sandbox
 */
BOOL CheckSandbox_Artifacts() {
    // Fichiers typiques des sandbox
    const char* sandboxFiles[] = {
        "C:\\agent\\agent.pyw",
        "C:\\sandbox\\starter.exe",
        "C:\\analysis\\analysis.exe",
        NULL
    };

    for (int i = 0; sandboxFiles[i]; i++) {
        if (GetFileAttributesA(sandboxFiles[i]) != INVALID_FILE_ATTRIBUTES) {
            return TRUE;
        }
    }

    // Processus typiques
    const char* sandboxProcesses[] = {
        "vmsrvc.exe", "vmusrvc.exe", "vboxtray.exe",
        "vmtoolsd.exe", "vmwaretray.exe", "vmwareuser.exe",
        "VGAuthService.exe", "vmacthlp.exe",
        "sandboxie", "SbieSvc.exe",
        NULL
    };

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = { sizeof(pe) };

    if (Process32First(hSnapshot, &pe)) {
        do {
            for (int i = 0; sandboxProcesses[i]; i++) {
                if (_stricmp(pe.szExeFile, sandboxProcesses[i]) == 0) {
                    CloseHandle(hSnapshot);
                    return TRUE;
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return FALSE;
}
```

### Vérifications environnementales

```c
/**
 * Les sandbox ont souvent peu de ressources
 */
BOOL CheckSandbox_Resources() {
    // Peu de RAM
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatusEx(&memInfo);

    if (memInfo.ullTotalPhys < 2ULL * 1024 * 1024 * 1024) {  // < 2 GB
        return TRUE;
    }

    // Peu de CPU
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    if (sysInfo.dwNumberOfProcessors < 2) {
        return TRUE;
    }

    // Petit disque
    ULARGE_INTEGER freeBytes, totalBytes;
    GetDiskFreeSpaceExA("C:\\", &freeBytes, &totalBytes, NULL);

    if (totalBytes.QuadPart < 60ULL * 1024 * 1024 * 1024) {  // < 60 GB
        return TRUE;
    }

    return FALSE;
}

/**
 * Vérifie l'activité utilisateur (sandbox = pas d'activité)
 */
BOOL CheckSandbox_UserActivity() {
    // Vérifier le nombre de fichiers récents
    char recentPath[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_RECENT, NULL, 0, recentPath);

    WIN32_FIND_DATAA findData;
    char searchPath[MAX_PATH];
    sprintf(searchPath, "%s\\*", recentPath);

    HANDLE hFind = FindFirstFileA(searchPath, &findData);
    int fileCount = 0;

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                fileCount++;
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }

    // Peu de fichiers récents = sandbox
    if (fileCount < 10) {
        return TRUE;
    }

    // Vérifier l'uptime (sandbox = fraîchement démarré)
    if (GetTickCount64() < 10 * 60 * 1000) {  // < 10 minutes
        return TRUE;
    }

    return FALSE;
}
```

### Détection de comportement

```c
/**
 * Sleep acceleration - Les sandbox accélèrent les Sleep
 */
BOOL CheckSandbox_SleepAcceleration() {
    DWORD startTime = GetTickCount();

    Sleep(10000);  // Dormir 10 secondes

    DWORD elapsed = GetTickCount() - startTime;

    // Si moins de 9 secondes réelles = accéléré
    return elapsed < 9000;
}

/**
 * Mouse movement check
 */
BOOL CheckSandbox_MouseMovement() {
    POINT pt1, pt2;

    GetCursorPos(&pt1);
    Sleep(5000);  // Attendre 5 secondes
    GetCursorPos(&pt2);

    // Pas de mouvement = pas d'utilisateur réel
    return (pt1.x == pt2.x && pt1.y == pt2.y);
}
```

---

## 3. Détection de Machine Virtuelle

### CPUID

```c
/**
 * CPUID révèle l'hyperviseur
 */
BOOL CheckVM_CPUID() {
    int cpuInfo[4];

    // CPUID avec EAX=1 : ECX bit 31 = hypervisor present
    __cpuid(cpuInfo, 1);
    if (cpuInfo[2] & (1 << 31)) {
        return TRUE;
    }

    // CPUID avec EAX=0x40000000 : signature hyperviseur
    __cpuid(cpuInfo, 0x40000000);

    char vendor[13];
    memcpy(vendor, &cpuInfo[1], 4);
    memcpy(vendor + 4, &cpuInfo[2], 4);
    memcpy(vendor + 8, &cpuInfo[3], 4);
    vendor[12] = '\0';

    // Signatures connues
    if (strcmp(vendor, "VMwareVMware") == 0 ||
        strcmp(vendor, "Microsoft Hv") == 0 ||
        strcmp(vendor, "VBoxVBoxVBox") == 0 ||
        strcmp(vendor, "KVMKVMKVM") == 0) {
        return TRUE;
    }

    return FALSE;
}
```

### Registres et services

```c
/**
 * Vérifie les clés de registre des VM
 */
BOOL CheckVM_Registry() {
    HKEY hKey;

    // VMware
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\VMware, Inc.\\VMware Tools", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return TRUE;
    }

    // VirtualBox
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Oracle\\VirtualBox Guest Additions", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return TRUE;
    }

    // Hyper-V
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Virtual Machine\\Guest\\Parameters", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return TRUE;
    }

    return FALSE;
}

/**
 * Vérifie les services des VM
 */
BOOL CheckVM_Services() {
    SC_HANDLE hSCM = OpenSCManagerA(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCM) return FALSE;

    const char* vmServices[] = {
        "VMTools", "VMwarePhysicalDiskHelper",
        "VBoxService", "VBoxGuest",
        "vmicheartbeat", "vmicvss", "vmicshutdown",
        NULL
    };

    for (int i = 0; vmServices[i]; i++) {
        SC_HANDLE hService = OpenServiceA(hSCM, vmServices[i], SERVICE_QUERY_STATUS);
        if (hService) {
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            return TRUE;
        }
    }

    CloseServiceHandle(hSCM);
    return FALSE;
}
```

### Hardware fingerprinting

```c
/**
 * Vérifie les identifiants matériels
 */
BOOL CheckVM_Hardware() {
    // MAC address des VM
    // VMware: 00:0C:29, 00:50:56
    // VirtualBox: 08:00:27
    // Hyper-V: 00:15:5D

    IP_ADAPTER_INFO adapterInfo[16];
    DWORD bufLen = sizeof(adapterInfo);

    if (GetAdaptersInfo(adapterInfo, &bufLen) == ERROR_SUCCESS) {
        PIP_ADAPTER_INFO pAdapter = adapterInfo;

        while (pAdapter) {
            // VMware
            if (pAdapter->Address[0] == 0x00 &&
                pAdapter->Address[1] == 0x0C &&
                pAdapter->Address[2] == 0x29) {
                return TRUE;
            }

            // VirtualBox
            if (pAdapter->Address[0] == 0x08 &&
                pAdapter->Address[1] == 0x00 &&
                pAdapter->Address[2] == 0x27) {
                return TRUE;
            }

            pAdapter = pAdapter->Next;
        }
    }

    return FALSE;
}
```

---

## 4. Wrapper complet

```c
/**
 * AntiAnalysis - Vérifie toutes les conditions
 */
typedef enum {
    ANALYSIS_NONE       = 0,
    ANALYSIS_DEBUGGER   = 1 << 0,
    ANALYSIS_SANDBOX    = 1 << 1,
    ANALYSIS_VM         = 1 << 2
} ANALYSIS_TYPE;

DWORD CheckAnalysisEnvironment() {
    DWORD result = ANALYSIS_NONE;

    // Checks anti-debug
    if (IsDebuggerPresent() ||
        CheckDebugger_PEB() ||
        CheckDebugger_NtGlobalFlag() ||
        CheckDebugger_DebugPort()) {
        result |= ANALYSIS_DEBUGGER;
    }

    // Checks anti-sandbox
    if (CheckSandbox_Resources() ||
        CheckSandbox_UserActivity() ||
        CheckSandbox_Artifacts()) {
        result |= ANALYSIS_SANDBOX;
    }

    // Checks anti-VM
    if (CheckVM_CPUID() ||
        CheckVM_Registry() ||
        CheckVM_Hardware()) {
        result |= ANALYSIS_VM;
    }

    return result;
}

// Utilisation
int main() {
    DWORD analysisFlags = CheckAnalysisEnvironment();

    if (analysisFlags & ANALYSIS_DEBUGGER) {
        // Comportement bénin ou exit
        return 0;
    }

    if (analysisFlags & ANALYSIS_SANDBOX) {
        // Attendre ou exit
        Sleep(INFINITE);
    }

    // Code malveillant ici...

    return 0;
}
```

---

## Références

- [Anti-Debug Tricks](https://anti-debug.checkpoint.com/)
- [al-khaser](https://github.com/LordNoteworthy/al-khaser) - Outil de test anti-analyse
- [Unprotect Project](https://unprotect.it/)
