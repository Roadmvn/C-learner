# PEB Walking - Résolution Dynamique des APIs

## Introduction

Le **PEB Walking** est la technique utilisée pour trouver les adresses des fonctions Windows **sans utiliser l'Import Address Table**. C'est essentiel pour le shellcode car on n'a pas accès aux imports.

---

## Le Process Environment Block (PEB)

Le PEB est une structure en mémoire qui contient des informations sur le processus en cours.

```
┌────────────────────────────────────────────────────────────────────┐
│                    ACCÈS AU PEB                                    │
├────────────────────────────────────────────────────────────────────┤
│                                                                    │
│   Le PEB est accessible via les registres de segment :             │
│                                                                    │
│   x86 (32-bit):  fs:[0x30]  →  PEB                                │
│   x64 (64-bit):  gs:[0x60]  →  PEB                                │
│                                                                    │
│   ┌─────────────────────────────────────────┐                     │
│   │  TEB (Thread Environment Block)         │                     │
│   │  ├─ fs:[0x00] : SEH Chain               │                     │
│   │  ├─ fs:[0x18] : TEB Self Reference      │                     │
│   │  └─ fs:[0x30] : PEB Pointer      ──────────► PEB              │
│   └─────────────────────────────────────────┘                     │
│                                                                    │
└────────────────────────────────────────────────────────────────────┘
```

---

## Structure du PEB

```c
// Structure simplifiée du PEB (offsets importants)
typedef struct _PEB {
    BYTE Reserved1[2];                  // 0x00
    BYTE BeingDebugged;                 // 0x02
    BYTE Reserved2[1];                  // 0x03
    PVOID Reserved3[2];                 // 0x04 / 0x08 (x64)
    PPEB_LDR_DATA Ldr;                  // 0x0C / 0x18 (x64) ← IMPORTANT
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    // ...
} PEB, *PPEB;

// PEB_LDR_DATA contient les listes de modules
typedef struct _PEB_LDR_DATA {
    ULONG Length;                       // 0x00
    BOOLEAN Initialized;                // 0x04
    PVOID SsHandle;                     // 0x08
    LIST_ENTRY InLoadOrderModuleList;   // 0x0C / 0x10 (x64)
    LIST_ENTRY InMemoryOrderModuleList; // 0x14 / 0x20 (x64) ← UTILISÉ
    LIST_ENTRY InInitOrderModuleList;   // 0x1C / 0x30 (x64)
} PEB_LDR_DATA, *PPEB_LDR_DATA;

// Chaque entrée de la liste
typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;          // 0x10 (x64)
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;                          // 0x30 (x64) ← BASE ADDRESS
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;             // 0x58 (x64) ← NOM DU MODULE
    // ...
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
```

---

## Parcourir la liste des modules

```
┌─────────────────────────────────────────────────────────────────────┐
│              CHAÎNE DE MODULES (InMemoryOrderModuleList)           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   PEB                                                               │
│    │                                                                │
│    ▼                                                                │
│   Ldr ──► InMemoryOrderModuleList                                   │
│            │                                                        │
│            ▼                                                        │
│   ┌────────────────┐    ┌────────────────┐    ┌────────────────┐   │
│   │   Program.exe  │───►│   ntdll.dll    │───►│ kernel32.dll   │──►...
│   │ (notre process)│    │                │    │                │   │
│   │ DllBase: 0x...│    │ DllBase: 0x... │    │ DllBase: 0x... │   │
│   └────────────────┘    └────────────────┘    └────────────────┘   │
│                                                                     │
│   On veut trouver kernel32.dll pour accéder à GetProcAddress       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Implémentation en ASM (x64)

```asm
; find_kernel32.asm - Trouve l'adresse de base de kernel32.dll

BITS 64

find_kernel32:
    xor rcx, rcx                ; RCX = 0
    mov rax, [gs:rcx + 0x60]    ; RAX = PEB
    mov rax, [rax + 0x18]       ; RAX = PEB->Ldr
    mov rsi, [rax + 0x20]       ; RSI = Ldr->InMemoryOrderModuleList.Flink

    ; Premier module = notre exécutable
    lodsq                        ; RAX = [RSI], RSI += 8
    xchg rax, rsi               ; Swap pour le prochain lodsq

    ; Deuxième module = ntdll.dll
    lodsq
    xchg rax, rsi

    ; Troisième module = kernel32.dll (ou kernelbase.dll sur Win7+)
    lodsq
    mov rbx, [rax + 0x20]       ; RBX = DllBase de kernel32.dll

    ; RBX contient maintenant l'adresse de base de kernel32.dll
    ret
```

---

## Implémentation en C

```c
#include <windows.h>
#include <winternl.h>

/**
 * GetKernel32Base - Trouve kernel32.dll via PEB walking
 *
 * @return Adresse de base de kernel32.dll
 */
HMODULE GetKernel32Base() {
    PPEB pPeb;
    PPEB_LDR_DATA pLdr;
    PLIST_ENTRY pListHead, pListEntry;
    PLDR_DATA_TABLE_ENTRY pDataTableEntry;
    HMODULE hKernel32 = NULL;

    // Accéder au PEB
#ifdef _WIN64
    pPeb = (PPEB)__readgsqword(0x60);
#else
    pPeb = (PPEB)__readfsdword(0x30);
#endif

    // Accéder au Loader Data
    pLdr = pPeb->Ldr;

    // Parcourir InMemoryOrderModuleList
    pListHead = &pLdr->InMemoryOrderModuleList;
    pListEntry = pListHead->Flink;

    while (pListEntry != pListHead) {
        // Récupérer l'entrée courante
        // Note : InMemoryOrderLinks est à l'offset 0x10 dans LDR_DATA_TABLE_ENTRY
        pDataTableEntry = CONTAINING_RECORD(pListEntry,
            LDR_DATA_TABLE_ENTRY,
            InMemoryOrderLinks);

        // Vérifier le nom du module
        // BaseDllName est une UNICODE_STRING
        if (pDataTableEntry->BaseDllName.Buffer != NULL) {
            // Comparaison insensible à la casse
            // On cherche "kernel32.dll" ou "KERNEL32.DLL"
            WCHAR* dllName = pDataTableEntry->BaseDllName.Buffer;

            // Simple check des premiers caractères
            if ((dllName[0] == 'k' || dllName[0] == 'K') &&
                (dllName[1] == 'e' || dllName[1] == 'E') &&
                (dllName[2] == 'r' || dllName[2] == 'R') &&
                (dllName[3] == 'n' || dllName[3] == 'N') &&
                (dllName[4] == 'e' || dllName[4] == 'E') &&
                (dllName[5] == 'l' || dllName[5] == 'L') &&
                (dllName[6] == '3') &&
                (dllName[7] == '2')) {

                hKernel32 = (HMODULE)pDataTableEntry->DllBase;
                break;
            }
        }

        pListEntry = pListEntry->Flink;
    }

    return hKernel32;
}
```

---

## Trouver une fonction dans l'Export Table

Une fois kernel32.dll trouvée, on parse son Export Directory pour trouver `GetProcAddress`.

```c
/**
 * GetFunctionByHash - Trouve une fonction par son hash
 *
 * @param hModule : Base address du module
 * @param dwHash  : Hash du nom de la fonction
 *
 * @return Adresse de la fonction
 */
FARPROC GetFunctionByHash(HMODULE hModule, DWORD dwHash) {
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_NT_HEADERS pNtHeaders;
    PIMAGE_EXPORT_DIRECTORY pExportDir;
    PDWORD pAddressOfFunctions;
    PDWORD pAddressOfNames;
    PWORD pAddressOfNameOrdinals;
    DWORD i;

    // Parser les headers PE
    pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);

    // Récupérer l'Export Directory
    DWORD exportDirRVA = pNtHeaders->OptionalHeader
        .DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    if (exportDirRVA == 0) {
        return NULL;
    }

    pExportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule + exportDirRVA);

    // Tables d'export
    pAddressOfFunctions = (PDWORD)((BYTE*)hModule + pExportDir->AddressOfFunctions);
    pAddressOfNames = (PDWORD)((BYTE*)hModule + pExportDir->AddressOfNames);
    pAddressOfNameOrdinals = (PWORD)((BYTE*)hModule + pExportDir->AddressOfNameOrdinals);

    // Parcourir les noms exportés
    for (i = 0; i < pExportDir->NumberOfNames; i++) {
        char* szFunctionName = (char*)((BYTE*)hModule + pAddressOfNames[i]);

        // Calculer le hash du nom
        DWORD currentHash = HashString(szFunctionName);

        if (currentHash == dwHash) {
            // Trouvé ! Récupérer l'adresse
            WORD ordinal = pAddressOfNameOrdinals[i];
            DWORD functionRVA = pAddressOfFunctions[ordinal];

            return (FARPROC)((BYTE*)hModule + functionRVA);
        }
    }

    return NULL;
}

/**
 * HashString - Calcule un hash djb2 d'une chaîne
 */
DWORD HashString(const char* str) {
    DWORD hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }

    return hash;
}
```

---

## Version ASM complète (x64)

```asm
; peb_walk.asm - Trouve GetProcAddress via PEB walking
; Compile: nasm -f win64 peb_walk.asm -o peb_walk.obj

BITS 64

section .text
global find_function

; Hashes pré-calculés (djb2)
%define HASH_GETPROCADDRESS 0x7c0dfcaa
%define HASH_LOADLIBRARYA   0xec0e4e8e

find_function:
    ; Sauvegarder les registres
    push rbx
    push rdi
    push rsi
    push rbp

    ; RCX = hash de la fonction à trouver
    mov r8, rcx                 ; Sauvegarder le hash cible

    ; === ÉTAPE 1 : Trouver kernel32.dll ===
    xor rcx, rcx
    mov rax, [gs:rcx + 0x60]    ; PEB
    mov rax, [rax + 0x18]       ; Ldr
    mov rsi, [rax + 0x20]       ; InMemoryOrderModuleList

    ; Parcourir les modules
.next_module:
    lodsq                        ; RAX = Flink
    mov rbx, [rax + 0x20]       ; RBX = DllBase
    mov rdi, [rax + 0x50]       ; RDI = BaseDllName.Buffer

    ; Vérifier si c'est kernel32.dll
    ; On check juste "k32" pour simplifier
    cmp word [rdi + 12], 0x0032 ; '2\0'
    jne .next_module

    ; === ÉTAPE 2 : Parser l'Export Directory ===
    mov ecx, [rbx + 0x3C]       ; e_lfanew
    add rcx, rbx                ; RCX = NT Headers
    mov ecx, [rcx + 0x88]       ; Export Directory RVA
    add rcx, rbx                ; RCX = Export Directory VA

    ; Tables
    mov r9d, [rcx + 0x20]       ; AddressOfNames RVA
    add r9, rbx                 ; R9 = AddressOfNames VA
    mov r10d, [rcx + 0x1C]      ; AddressOfFunctions RVA
    add r10, rbx                ; R10 = AddressOfFunctions VA
    mov r11d, [rcx + 0x24]      ; AddressOfNameOrdinals RVA
    add r11, rbx                ; R11 = AddressOfNameOrdinals VA
    mov ecx, [rcx + 0x18]       ; NumberOfNames

    xor rdi, rdi                ; Index = 0

.search_loop:
    ; Récupérer le nom de la fonction
    mov esi, [r9 + rdi * 4]     ; RVA du nom
    add rsi, rbx                ; RSI = VA du nom

    ; Calculer le hash
    xor rax, rax                ; hash = 0
    cdq                         ; EDX = 0
    mov eax, 5381               ; hash = 5381

.hash_loop:
    movzx edx, byte [rsi]
    test dl, dl
    jz .hash_done
    imul eax, eax, 33           ; hash * 33
    add eax, edx                ; hash + char
    inc rsi
    jmp .hash_loop

.hash_done:
    ; Comparer avec le hash cible
    cmp eax, r8d
    je .found

    ; Passer au nom suivant
    inc rdi
    cmp edi, ecx
    jl .search_loop

    ; Non trouvé
    xor rax, rax
    jmp .return

.found:
    ; Récupérer l'ordinal
    movzx eax, word [r11 + rdi * 2]
    ; Récupérer l'adresse de la fonction
    mov eax, [r10 + rax * 4]
    add rax, rbx                ; RAX = VA de la fonction

.return:
    pop rbp
    pop rsi
    pop rdi
    pop rbx
    ret
```

---

## Utilisation en shellcode complet

```c
// Shellcode qui affiche une MessageBox

// Hashes pré-calculés
#define HASH_LOADLIBRARYA   0xec0e4e8e
#define HASH_GETPROCADDRESS 0x7c0dfcaa
#define HASH_MESSAGEBOXA    0xbc4da2a8

void shellcode() {
    // 1. Trouver kernel32.dll
    HMODULE hKernel32 = GetKernel32Base();

    // 2. Trouver GetProcAddress et LoadLibraryA
    typedef FARPROC (WINAPI *pGetProcAddress)(HMODULE, LPCSTR);
    typedef HMODULE (WINAPI *pLoadLibraryA)(LPCSTR);

    pGetProcAddress GetProcAddress = (pGetProcAddress)
        GetFunctionByHash(hKernel32, HASH_GETPROCADDRESS);

    pLoadLibraryA LoadLibraryA = (pLoadLibraryA)
        GetFunctionByHash(hKernel32, HASH_LOADLIBRARYA);

    // 3. Charger user32.dll
    char user32[] = {'u','s','e','r','3','2','.','d','l','l',0};
    HMODULE hUser32 = LoadLibraryA(user32);

    // 4. Trouver MessageBoxA
    char msgbox[] = {'M','e','s','s','a','g','e','B','o','x','A',0};
    typedef int (WINAPI *pMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
    pMessageBoxA MessageBoxA = (pMessageBoxA)GetProcAddress(hUser32, msgbox);

    // 5. Afficher la MessageBox
    char title[] = {'P','W','N','E','D',0};
    char text[] = {'H','e','l','l','o',' ','W','o','r','l','d',0};
    MessageBoxA(NULL, text, title, MB_OK);
}
```

---

## Références

- [PEB Structure - MSDN](https://docs.microsoft.com/en-us/windows/win32/api/winternl/ns-winternl-peb)
- [Resolving APIs Dynamically](https://www.ired.team/offensive-security/code-injection-process-injection/finding-kernel32-base-and-function-addresses-in-shellcode)
