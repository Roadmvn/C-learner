# Cours 01 : API Win32 Basics

## Types Windows

```c
#include <windows.h>

// Types de base
BYTE    b;      // unsigned char (8 bits)
WORD    w;      // unsigned short (16 bits)
DWORD   dw;     // unsigned long (32 bits)
QWORD   qw;     // unsigned __int64 (64 bits)

// Pointeurs
LPSTR   str;    // char*
LPCSTR  cstr;   // const char*
LPWSTR  wstr;   // wchar_t* (Unicode)
LPCWSTR wcstr;  // const wchar_t*

// Handles
HANDLE  h;      // Handle générique
HWND    hwnd;   // Handle de fenêtre
HMODULE hmod;   // Handle de module (DLL)

// Booléen Windows
BOOL    success; // TRUE (non-zero) ou FALSE (0)
```

## Handles

Un **handle** est un identifiant opaque vers une ressource système.

```c
HANDLE hFile = CreateFile(
    "test.txt",
    GENERIC_READ,
    0,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL
);

if (hFile == INVALID_HANDLE_VALUE) {
    // Erreur
}

// Toujours fermer les handles!
CloseHandle(hFile);
```

## Gestion des Erreurs

```c
DWORD error = GetLastError();

// Obtenir un message d'erreur
LPSTR message = NULL;
FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    error,
    0,
    (LPSTR)&message,
    0,
    NULL
);
printf("Erreur: %s\n", message);
LocalFree(message);
```

## Unicode vs ANSI

```c
// Fonctions ANSI (char)
CreateFileA("test.txt", ...);
MessageBoxA(NULL, "Hello", "Title", MB_OK);

// Fonctions Unicode (wchar_t)
CreateFileW(L"test.txt", ...);
MessageBoxW(NULL, L"Hello", L"Title", MB_OK);

// Macro générique (dépend de UNICODE)
CreateFile(TEXT("test.txt"), ...);
```

## Compilation Windows

```bash
# MinGW (depuis Linux)
x86_64-w64-mingw32-gcc prog.c -o prog.exe

# Visual Studio
cl /W4 prog.c

# Linker les bibliothèques
gcc prog.c -lkernel32 -luser32 -o prog.exe
```

## Structure d'un Programme Windows

```c
#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // Code console classique
    printf("Hello Windows!\n");

    // Utiliser l'API Win32
    MessageBoxA(NULL, "Hello!", "Test", MB_OK);

    return 0;
}
```

## Fonctions Courantes

```c
// Obtenir le PID courant
DWORD pid = GetCurrentProcessId();

// Obtenir un handle vers soi-même
HANDLE hSelf = GetCurrentProcess();

// Sleep
Sleep(1000);  // 1 seconde

// Obtenir un module
HMODULE hKernel = GetModuleHandleA("kernel32.dll");
HMODULE hNtdll = GetModuleHandleA("ntdll.dll");

// Obtenir l'adresse d'une fonction
FARPROC pFunc = GetProcAddress(hKernel, "VirtualAlloc");
```

## Application Sécurité : Résolution Dynamique

```c
// Éviter d'avoir les noms de fonctions dans l'import table
typedef LPVOID (WINAPI *pVirtualAlloc)(
    LPVOID, SIZE_T, DWORD, DWORD
);

HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
pVirtualAlloc myVirtualAlloc = (pVirtualAlloc)GetProcAddress(
    hKernel32,
    "VirtualAlloc"
);

LPVOID mem = myVirtualAlloc(
    NULL,
    4096,
    MEM_COMMIT | MEM_RESERVE,
    PAGE_EXECUTE_READWRITE
);
```
