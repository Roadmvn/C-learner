# Cours 03 : Compilation en Détail

## Le Processus de Compilation

Quand vous tapez `gcc programme.c -o programme`, plusieurs étapes se produisent :

```
┌─────────────────────────────────────────────────────────────────┐
│                    ÉTAPES DE COMPILATION                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   programme.c                                                   │
│       │                                                         │
│       ▼  gcc -E (préprocesseur)                                │
│   programme.i  (code préprocessé)                              │
│       │                                                         │
│       ▼  gcc -S (compilateur)                                  │
│   programme.s  (assembleur)                                    │
│       │                                                         │
│       ▼  gcc -c (assembleur)                                   │
│   programme.o  (code objet)                                    │
│       │                                                         │
│       ▼  ld (éditeur de liens)                                 │
│   programme    (exécutable)                                    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Étape par Étape

### 1. Préprocesseur (-E)

```bash
gcc -E programme.c -o programme.i
```

Le préprocesseur :
- Traite les `#include` (copie le contenu des headers)
- Remplace les `#define` (macros)
- Gère `#ifdef`, `#endif` (compilation conditionnelle)
- Supprime les commentaires

**Exemple :**

```c
// programme.c
#include <stdio.h>
#define MESSAGE "Hello"

int main(void) {
    printf(MESSAGE);
    return 0;
}
```

Après préprocesseur :
```c
// Des milliers de lignes de stdio.h...
extern int printf(const char *, ...);
// ...

int main(void) {
    printf("Hello");
    return 0;
}
```

### 2. Compilation (-S)

```bash
gcc -S programme.c -o programme.s
```

Génère le code assembleur :

```nasm
    .file   "programme.c"
    .text
    .section    .rodata
.LC0:
    .string "Hello"
    .text
    .globl  main
    .type   main, @function
main:
    pushq   %rbp
    movq    %rsp, %rbp
    leaq    .LC0(%rip), %rdi
    movl    $0, %eax
    call    printf@PLT
    movl    $0, %eax
    popq    %rbp
    ret
```

### 3. Assemblage (-c)

```bash
gcc -c programme.c -o programme.o
```

Crée le fichier objet (code machine non lié).

```bash
# Examiner le contenu
hexdump -C programme.o | head -5
objdump -d programme.o
```

### 4. Édition de liens

```bash
gcc programme.o -o programme
# ou directement
ld programme.o -lc -o programme  # Plus complexe
```

Combine les fichiers objets et les bibliothèques.

---

## Options GCC Essentielles

### Compilation

```bash
gcc source.c -o executable    # Compilation complète
gcc -c source.c               # Jusqu'au fichier objet
gcc -S source.c               # Jusqu'à l'assembleur
gcc -E source.c               # Jusqu'au préprocesseur
```

### Warnings et Debug

```bash
gcc -Wall source.c            # Tous les warnings courants
gcc -Wextra source.c          # Warnings supplémentaires
gcc -Werror source.c          # Warnings → Erreurs
gcc -g source.c               # Informations de debug (pour GDB)
gcc -ggdb source.c            # Debug optimisé pour GDB
```

### Optimisation

```bash
gcc -O0 source.c              # Pas d'optimisation (défaut)
gcc -O1 source.c              # Optimisations basiques
gcc -O2 source.c              # Optimisations standard
gcc -O3 source.c              # Optimisations agressives
gcc -Os source.c              # Optimiser la taille
```

### Sécurité (Désactivation pour Tests)

```bash
# Désactiver les protections (LAB UNIQUEMENT!)
gcc -fno-stack-protector source.c   # Pas de canary
gcc -z execstack source.c           # Stack exécutable
gcc -no-pie source.c                # Pas de PIE/ASLR
gcc -D_FORTIFY_SOURCE=0 source.c    # Pas de fortification

# Combinaison complète pour tests d'exploitation
gcc -fno-stack-protector -z execstack -no-pie -g source.c -o vuln
```

### Architecture

```bash
gcc -m32 source.c             # Compiler en 32 bits
gcc -m64 source.c             # Compiler en 64 bits (défaut)
gcc -march=native source.c    # Optimiser pour le CPU local
```

---

## Compilation Multi-Fichiers

### Structure du projet

```
projet/
├── main.c
├── utils.c
└── utils.h
```

**utils.h**
```c
#ifndef UTILS_H
#define UTILS_H

int addition(int a, int b);
void afficher(int n);

#endif
```

**utils.c**
```c
#include <stdio.h>
#include "utils.h"

int addition(int a, int b) {
    return a + b;
}

void afficher(int n) {
    printf("Valeur: %d\n", n);
}
```

**main.c**
```c
#include "utils.h"

int main(void) {
    int resultat = addition(5, 3);
    afficher(resultat);
    return 0;
}
```

### Compilation

```bash
# Méthode 1: Tout d'un coup
gcc main.c utils.c -o programme

# Méthode 2: Fichiers objets séparés (recommandé)
gcc -c main.c -o main.o
gcc -c utils.c -o utils.o
gcc main.o utils.o -o programme

# Méthode 3: Makefile (voir cours précédent)
make
```

---

## Examiner les Binaires

### objdump - Désassemblage

```bash
# Désassembler le code
objdump -d programme

# Avec code source (si compilé avec -g)
objdump -S programme

# Voir les sections
objdump -h programme

# Voir les symboles
objdump -t programme
```

### readelf - Analyser ELF

```bash
# Header ELF
readelf -h programme

# Sections
readelf -S programme

# Symboles
readelf -s programme

# Segments (pour le chargement)
readelf -l programme
```

### nm - Table des symboles

```bash
nm programme
# T = fonction dans text
# D = donnée initialisée
# B = donnée non initialisée (BSS)
# U = undefined (externe)
```

### strings - Chaînes ASCII

```bash
strings programme
strings -n 10 programme  # Minimum 10 caractères
```

### file - Type de fichier

```bash
file programme
# programme: ELF 64-bit LSB pie executable, x86-64, ...
```

---

## Bibliothèques

### Statiques (.a)

```bash
# Créer une bibliothèque statique
gcc -c utils.c -o utils.o
ar rcs libutils.a utils.o

# Utiliser
gcc main.c -L. -lutils -o programme
```

### Dynamiques (.so)

```bash
# Créer une bibliothèque partagée
gcc -fPIC -c utils.c -o utils.o
gcc -shared utils.o -o libutils.so

# Utiliser
gcc main.c -L. -lutils -o programme
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./programme
```

---

## Variables d'Environnement

```bash
# Chemin des bibliothèques
export LD_LIBRARY_PATH=/chemin/libs:$LD_LIBRARY_PATH

# Précharger une bibliothèque
export LD_PRELOAD=/chemin/libhook.so

# Afficher les recherches de bibliothèques
LD_DEBUG=libs ./programme
```

---

## Application en Sécurité

### Vérifier les protections d'un binaire

```bash
# Avec checksec (outil de pwntools)
checksec --file=programme

# Manuellement
readelf -l programme | grep GNU_STACK  # Stack NX?
readelf -d programme | grep BIND_NOW   # Full RELRO?
```

### Compiler un shellcode

```bash
# Compiler en position-independant
gcc -fPIC -pie -nostdlib shellcode.c -o shellcode

# Extraire le code machine
objcopy -O binary -j .text shellcode shellcode.bin
xxd -i shellcode.bin
```

---

## Résumé

```
┌─────────────────────────────────────────────────────────────────┐
│                   COMMANDES À RETENIR                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Compilation de base :                                          │
│  gcc -Wall -g source.c -o programme                            │
│                                                                 │
│  Voir le code assembleur :                                      │
│  gcc -S source.c && cat source.s                               │
│                                                                 │
│  Désassembler un binaire :                                      │
│  objdump -d programme                                          │
│                                                                 │
│  Compilation sans protections (LAB) :                          │
│  gcc -fno-stack-protector -z execstack -no-pie source.c       │
│                                                                 │
│  Analyser un ELF :                                              │
│  readelf -a programme                                          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Navigation

| Précédent | Suivant |
|-----------|---------|
| [02 - Premier Programme](02-premier-programme.md) | [04 - Introduction à GDB](04-intro-gdb.md) |
