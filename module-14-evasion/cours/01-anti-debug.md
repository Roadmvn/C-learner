# Cours 01 : Techniques Anti-Debug

## Pourquoi Anti-Debug ?

Les techniques anti-debug rendent l'analyse plus difficile pour les chercheurs.

## Windows : IsDebuggerPresent

```c
#include <windows.h>

if (IsDebuggerPresent()) {
    // Débogueur détecté
    ExitProcess(0);
}
```

## Windows : CheckRemoteDebuggerPresent

```c
BOOL debugged = FALSE;
CheckRemoteDebuggerPresent(GetCurrentProcess(), &debugged);
if (debugged) {
    ExitProcess(0);
}
```

## Windows : PEB.BeingDebugged

```c
// Accès direct au PEB (Process Environment Block)
#ifdef _WIN64
    PPEB peb = (PPEB)__readgsqword(0x60);
#else
    PPEB peb = (PPEB)__readfsdword(0x30);
#endif

if (peb->BeingDebugged) {
    ExitProcess(0);
}
```

## Windows : NtGlobalFlag

```c
// Si débogué, NtGlobalFlag contient des flags spécifiques
DWORD ntGlobalFlag = *(DWORD*)((char*)peb + 0x68);  // Offset x86
// 0x70 pour x64

if (ntGlobalFlag & 0x70) {  // FLG_HEAP_*
    ExitProcess(0);
}
```

## Windows : Timing Checks

```c
// Les breakpoints ralentissent l'exécution
LARGE_INTEGER start, end, freq;
QueryPerformanceFrequency(&freq);
QueryPerformanceCounter(&start);

// Code suspect
int x = 0;
for (int i = 0; i < 1000000; i++) x++;

QueryPerformanceCounter(&end);
double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;

if (elapsed > 0.5) {  // Trop lent = probablement débogué
    ExitProcess(0);
}
```

## Linux : ptrace

```c
#include <sys/ptrace.h>

// Un processus ne peut être tracé qu'une fois
if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
    // Déjà tracé (par un débogueur)
    exit(1);
}
```

## Linux : /proc/self/status

```c
#include <stdio.h>
#include <string.h>

int check_tracer(void) {
    FILE *f = fopen("/proc/self/status", "r");
    char line[256];

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "TracerPid:", 10) == 0) {
            int pid;
            sscanf(line, "TracerPid: %d", &pid);
            fclose(f);
            return pid != 0;
        }
    }
    fclose(f);
    return 0;
}
```

## Détection de Breakpoints

```c
// Vérifier l'intégrité du code (breakpoint = 0xCC)
unsigned char *func = (unsigned char*)&SensitiveFunction;
if (*func == 0xCC) {  // INT3
    ExitProcess(0);
}

// Checksum de fonction
unsigned int checksum = 0;
for (int i = 0; i < func_size; i++) {
    checksum += func[i];
}
if (checksum != EXPECTED_CHECKSUM) {
    ExitProcess(0);
}
```

## Contremesures (Pour Analystes)

```
Pour contourner ces checks :

• Patcher les appels à IsDebuggerPresent
• Modifier les valeurs retournées
• Utiliser des plugins anti-anti-debug (ScyllaHide)
• Hook les fonctions de timing
• Modifier le PEB manuellement
```
