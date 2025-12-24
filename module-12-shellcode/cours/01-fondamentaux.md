# Fondamentaux du Shellcode

## Qu'est-ce qu'un Shellcode ?

Un **shellcode** est une séquence de bytes qui représente du code machine exécutable. Le nom vient historiquement du but de ce code : obtenir un **shell**.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    SHELLCODE vs PROGRAMME NORMAL                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   Programme normal (.exe)          Shellcode                        │
│   ┌──────────────────┐            ┌──────────────────┐             │
│   │ PE Headers       │            │                  │             │
│   │ - DOS Header     │            │  \x48\x31\xc0    │             │
│   │ - NT Headers     │            │  \x48\x89\xe5    │             │
│   │ - Section Table  │            │  \xb8\x01\x00    │             │
│   ├──────────────────┤            │  \x00\x00\x0f    │             │
│   │ .text (code)     │            │  \x05\xc3...     │             │
│   │ .data (données)  │            │                  │             │
│   │ .rdata (imports) │            │  (juste du code  │             │
│   │ .reloc           │            │   machine brut)  │             │
│   └──────────────────┘            └──────────────────┘             │
│                                                                     │
│   Dépend du loader Windows        Autonome, auto-suffisant          │
│   Adresses fixes                  Position-Independent Code         │
│   Imports résolus par l'OS        Doit résoudre ses propres APIs   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Contraintes du Shellcode

### 1. Position-Independent Code (PIC)

Le shellcode peut être chargé **n'importe où** en mémoire. Donc :
- ❌ Pas d'adresses absolues
- ✅ Utiliser des offsets relatifs

```asm
; MAUVAIS - Adresse absolue
mov rax, 0x7FF123456789    ; Cette adresse sera fausse

; BON - Offset relatif
lea rax, [rip + data]      ; Relatif à l'instruction courante
```

### 2. Auto-suffisant

Le shellcode n'a pas accès aux imports d'un PE. Il doit :
- Trouver `kernel32.dll` en mémoire
- Localiser `GetProcAddress`
- Résoudre toutes les APIs nécessaires

### 3. Pas de NULL bytes (souvent)

Si le shellcode est injecté via `strcpy()` ou similaire, les NULL bytes (`\x00`) terminent la chaîne.

```asm
; MAUVAIS - Contient des NULL
mov eax, 1                  ; B8 01 00 00 00

; BON - Évite les NULL
xor eax, eax                ; 31 C0
inc eax                     ; FF C0
```

### 4. Taille minimale

Plus le shellcode est petit, plus il est facile à injecter.

---

## Structure typique d'un Shellcode Windows

```
┌──────────────────────────────────────────────────────────────────┐
│                    STRUCTURE D'UN SHELLCODE                      │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  1. INITIALISATION                                               │
│     - Sauvegarder les registres si nécessaire                   │
│     - Aligner la stack (important en x64)                       │
│                                                                  │
│  2. TROUVER KERNEL32.DLL                                        │
│     - Accéder au PEB                                            │
│     - Parcourir la liste des modules chargés                    │
│     - Identifier kernel32.dll                                    │
│                                                                  │
│  3. RÉSOUDRE LES APIS                                           │
│     - Parser l'Export Directory de kernel32                     │
│     - Trouver GetProcAddress et LoadLibraryA                    │
│     - Charger d'autres DLLs si besoin (user32, ws2_32...)      │
│                                                                  │
│  4. EXÉCUTER LE PAYLOAD                                         │
│     - Appeler les fonctions résolues                            │
│     - MessageBox, reverse shell, download, etc.                 │
│                                                                  │
│  5. CLEANUP (optionnel)                                         │
│     - Restaurer les registres                                   │
│     - Retourner proprement                                      │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

---

## Premier Shellcode : Exit Process

Le shellcode le plus simple : terminer le processus.

### Version ASM (x64)

```asm
; exit_shellcode.asm
; Compile: nasm -f win64 exit_shellcode.asm -o exit_shellcode.obj

BITS 64

section .text
global _start

_start:
    ; Trouver kernel32.dll via PEB
    xor rcx, rcx                  ; RCX = 0
    mov rax, [gs:rcx + 0x60]      ; RAX = PEB
    mov rax, [rax + 0x18]         ; RAX = PEB->Ldr
    mov rsi, [rax + 0x20]         ; RSI = Ldr->InMemoryOrderModuleList
    lodsq                          ; RAX = premier module (exe)
    xchg rax, rsi
    lodsq                          ; RAX = deuxième module (ntdll)
    xchg rax, rsi
    lodsq                          ; RAX = troisième module (kernel32)
    mov rbx, [rax + 0x20]         ; RBX = kernel32 base address

    ; Parser l'Export Directory pour trouver ExitProcess
    ; (code simplifié - voir module PEB walking pour version complète)

    ; Appeler ExitProcess(0)
    xor rcx, rcx                  ; ExitCode = 0
    ; call ExitProcess            ; Adresse résolue dynamiquement
```

### Extraction du shellcode

```python
#!/usr/bin/env python3
# extract_shellcode.py

import sys

def extract_shellcode(obj_file):
    with open(obj_file, 'rb') as f:
        data = f.read()

    # Afficher en format C
    print("unsigned char shellcode[] = {")
    for i, byte in enumerate(data):
        if i % 12 == 0:
            print("    ", end="")
        print(f"0x{byte:02x}", end="")
        if i < len(data) - 1:
            print(", ", end="")
        if (i + 1) % 12 == 0:
            print()
    print("\n};")
    print(f"// Size: {len(data)} bytes")

if __name__ == "__main__":
    extract_shellcode(sys.argv[1])
```

---

## Développer en C puis convertir

Écrire directement en ASM est difficile. On peut écrire en C avec des contraintes.

### Règles pour du C "shellcode-friendly"

```c
/**
 * Règles pour écrire du shellcode en C :
 *
 * 1. Pas de variables globales (pas de .data/.bss)
 * 2. Pas de strings littérales (elles vont dans .rdata)
 * 3. Pas de fonctions de la CRT (printf, malloc...)
 * 4. Résoudre les APIs dynamiquement
 * 5. Compiler avec les bons flags
 */

#include <windows.h>

// Définir les strings sur la stack
void shellcode_entry() {
    // String sur la stack (pas de .rdata)
    char kernel32[] = { 'k','e','r','n','e','l','3','2','.','d','l','l',0 };
    char loadlib[]  = { 'L','o','a','d','L','i','b','r','a','r','y','A',0 };

    // Trouver kernel32 via PEB
    HMODULE hKernel32;
    __asm {
        mov rax, gs:[0x60]          // PEB
        mov rax, [rax + 0x18]       // Ldr
        mov rax, [rax + 0x20]       // InMemoryOrderModuleList
        mov rax, [rax]              // Flink (exe)
        mov rax, [rax]              // Flink (ntdll)
        mov rax, [rax + 0x20]       // Flink->DllBase (kernel32)
        mov hKernel32, rax
    }

    // Parser l'export table pour trouver GetProcAddress
    // ... (voir cours 02)
}
```

### Compilation pour shellcode

```bash
# MinGW - Compilation en position-independent
x86_64-w64-mingw32-gcc -c shellcode.c -o shellcode.o \
    -fno-stack-protector \
    -fno-asynchronous-unwind-tables \
    -fno-ident \
    -O2

# Extraire la section .text
objcopy -O binary -j .text shellcode.o shellcode.bin

# Visual Studio
cl.exe /c /O1 /GS- shellcode.c
```

---

## Tester son Shellcode

### Loader de test

```c
/**
 * shellcode_loader.c - Exécute un shellcode pour le tester
 */
#include <windows.h>
#include <stdio.h>

// Shellcode à tester
unsigned char shellcode[] = {
    0x48, 0x31, 0xc9,                   // xor rcx, rcx
    0x48, 0x81, 0xec, 0x00, 0x01, 0x00, 0x00, // sub rsp, 0x100
    // ... reste du shellcode
    0xc3                                 // ret
};

int main() {
    printf("[*] Shellcode size: %zu bytes\n", sizeof(shellcode));

    // Allouer mémoire exécutable
    LPVOID mem = VirtualAlloc(NULL, sizeof(shellcode),
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!mem) {
        printf("[-] VirtualAlloc failed\n");
        return 1;
    }

    printf("[*] Allocated at: 0x%p\n", mem);

    // Copier le shellcode
    memcpy(mem, shellcode, sizeof(shellcode));

    printf("[*] Executing shellcode...\n");

    // Exécuter
    ((void(*)())mem)();

    printf("[+] Shellcode returned\n");

    VirtualFree(mem, 0, MEM_RELEASE);
    return 0;
}
```

Compilation :
```bash
gcc -o loader.exe shellcode_loader.c
```

---

## Debugging du Shellcode

### Avec x64dbg

1. Charger le loader dans x64dbg
2. Mettre un breakpoint sur `VirtualAlloc`
3. Continuer jusqu'à l'allocation
4. Mettre un breakpoint sur l'adresse allouée
5. Step into le shellcode

### Avec WinDbg

```
; Charger le loader
windbg loader.exe

; Breakpoint sur l'exécution du shellcode
bp loader!main+0x50

; Une fois à l'adresse du shellcode
t   ; Step into
p   ; Step over
```

---

## Encodage du Shellcode

### Pourquoi encoder ?

1. Éviter les bad characters (NULL, CR, LF...)
2. Éviter les signatures d'antivirus
3. Obfuscation basique

### XOR Encoder simple

```python
#!/usr/bin/env python3
# xor_encoder.py

def xor_encode(shellcode, key):
    """Encode un shellcode avec XOR"""
    encoded = bytearray()
    for byte in shellcode:
        encoded.append(byte ^ key)
    return bytes(encoded)

def generate_decoder_stub(key, shellcode_len):
    """Génère le stub de décodage x64"""
    stub = f"""
    ; XOR Decoder stub (x64)
    jmp short get_shellcode
decode:
    pop rsi                     ; RSI = adresse du shellcode
    xor rcx, rcx
    mov cl, {shellcode_len}     ; Taille du shellcode
decode_loop:
    xor byte [rsi], {key}       ; XOR avec la clé
    inc rsi
    loop decode_loop
    jmp short encoded_shellcode
get_shellcode:
    call decode
encoded_shellcode:
    ; Shellcode encodé ici
    """
    return stub

# Exemple
original = b"\x48\x31\xc0\x48\x89\xe5"
key = 0x41
encoded = xor_encode(original, key)

print(f"Original : {original.hex()}")
print(f"Encoded  : {encoded.hex()}")
print(f"Key      : 0x{key:02x}")
```

---

## Références

- [Writing Shellcode in C - ired.team](https://www.ired.team/offensive-security/code-injection-process-injection/writing-and-compiling-shellcode-in-c)
- [Windows x64 Shellcode](https://www.tophertimzen.com/blog/windowsx64Shellcode/)
- [Shellcode Template](https://github.com/rainerzufalldererste/windows_x64_shellcode_template)
