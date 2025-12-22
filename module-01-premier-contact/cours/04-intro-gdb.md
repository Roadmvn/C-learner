# Cours 04 : Introduction à GDB

## Qu'est-ce que GDB ?

GDB (GNU Debugger) est l'outil standard pour déboguer les programmes C/C++ sur Linux. Pour la sécurité offensive, c'est indispensable pour :

- Comprendre le comportement d'un programme
- Analyser les exploits
- Développer des shellcodes
- Reverse engineering

---

## Démarrage de Base

### Compiler pour GDB

```bash
# Toujours compiler avec -g pour avoir les symboles de debug
gcc -g -Wall programme.c -o programme
```

### Lancer GDB

```bash
gdb ./programme           # Lancer GDB avec le programme
gdb -q ./programme        # Mode silencieux (sans bannière)
gdb --args ./prog arg1    # Avec des arguments
```

---

## Commandes Essentielles

```
┌─────────────────────────────────────────────────────────────────┐
│                   COMMANDES GDB DE BASE                         │
├─────────────────────────────────────────────────────────────────┤
│  EXÉCUTION                                                      │
│  run (r)            Lancer le programme                        │
│  run arg1 arg2      Lancer avec arguments                      │
│  continue (c)       Continuer après un arrêt                   │
│  next (n)           Ligne suivante (pas rentrer dans fonction) │
│  step (s)           Ligne suivante (rentrer dans fonction)     │
│  finish             Finir la fonction courante                 │
│  quit (q)           Quitter GDB                                │
├─────────────────────────────────────────────────────────────────┤
│  BREAKPOINTS                                                    │
│  break main         Breakpoint au début de main                │
│  break 42           Breakpoint à la ligne 42                   │
│  break *0x401234    Breakpoint à une adresse                   │
│  info breakpoints   Lister les breakpoints                     │
│  delete 1           Supprimer le breakpoint #1                 │
│  disable 1          Désactiver le breakpoint #1                │
├─────────────────────────────────────────────────────────────────┤
│  AFFICHAGE                                                      │
│  print var          Afficher une variable                      │
│  print/x var        Afficher en hexadécimal                    │
│  print $eax         Afficher un registre                       │
│  info registers     Afficher tous les registres                │
│  info locals        Variables locales                          │
│  backtrace (bt)     Pile d'appels                              │
├─────────────────────────────────────────────────────────────────┤
│  MÉMOIRE                                                        │
│  x/10x $esp         10 mots en hexa depuis ESP                 │
│  x/s 0x401234       Afficher une chaîne                        │
│  x/i $eip           Afficher l'instruction courante            │
│  x/20i main         20 instructions depuis main                │
├─────────────────────────────────────────────────────────────────┤
│  CODE                                                           │
│  list               Afficher le code source                    │
│  disassemble        Désassembler la fonction courante          │
│  disas main         Désassembler main                          │
└─────────────────────────────────────────────────────────────────┘
```

---

## Session GDB Exemple

### Programme de test

```c
// debug_demo.c
#include <stdio.h>

int calculer(int a, int b) {
    int resultat = a + b;
    return resultat;
}

int main(void) {
    int x = 10;
    int y = 20;
    int somme = calculer(x, y);
    printf("Somme: %d\n", somme);
    return 0;
}
```

### Session de Debug

```bash
$ gcc -g debug_demo.c -o debug_demo
$ gdb -q ./debug_demo

(gdb) break main
Breakpoint 1 at 0x1169: file debug_demo.c, line 9.

(gdb) run
Starting program: ./debug_demo
Breakpoint 1, main () at debug_demo.c:9
9           int x = 10;

(gdb) next
10          int y = 20;

(gdb) print x
$1 = 10

(gdb) next
11          int somme = calculer(x, y);

(gdb) step
calculer (a=10, b=20) at debug_demo.c:4
4           int resultat = a + b;

(gdb) print a
$2 = 10

(gdb) print b
$3 = 20

(gdb) next
5           return resultat;

(gdb) print resultat
$4 = 30

(gdb) backtrace
#0  calculer (a=10, b=20) at debug_demo.c:5
#1  0x000055555555518e in main () at debug_demo.c:11

(gdb) continue
Continuing.
Somme: 30
[Inferior 1 (process 12345) exited normally]

(gdb) quit
```

---

## Examiner la Mémoire

### Syntaxe de x (examine)

```
x/NFU adresse

N = nombre d'unités
F = format (x=hex, d=décimal, s=string, i=instruction)
U = taille d'unité (b=byte, h=halfword, w=word, g=giant)
```

### Exemples

```bash
(gdb) x/4xw $esp      # 4 mots (32b) en hexa depuis ESP
0xffffd100: 0x00000001 0x00000002 0xffffd200 0x08048456

(gdb) x/20xb $esp     # 20 bytes en hexa
0xffffd100: 0x01 0x00 0x00 0x00 0x02 0x00 0x00 0x00 ...

(gdb) x/s 0x08048500  # Chaîne à cette adresse
0x8048500: "Hello, World!"

(gdb) x/10i main      # 10 instructions depuis main
0x8048456 <main>:      push   %ebp
0x8048457 <main+1>:    mov    %esp,%ebp
...
```

---

## Manipulation des Registres

```bash
(gdb) info registers              # Tous les registres
(gdb) info registers eax ebx      # Registres spécifiques

(gdb) print $eax                  # Valeur de EAX
(gdb) print/x $eax                # En hexadécimal

(gdb) set $eax = 42               # Modifier EAX
(gdb) set $eip = 0x08048500       # Modifier EIP (saut!)
```

---

## Watchpoints

Les watchpoints arrêtent l'exécution quand une variable change :

```bash
(gdb) watch variable              # S'arrête si variable change
(gdb) rwatch variable             # S'arrête si variable est lue
(gdb) awatch variable             # S'arrête si lue ou écrite

(gdb) info watchpoints            # Lister les watchpoints
```

---

## Déboguer avec des Arguments

```bash
# Méthode 1: Depuis la ligne de commande
$ gdb --args ./programme arg1 arg2
(gdb) run

# Méthode 2: Dans GDB
(gdb) run arg1 arg2

# Méthode 3: Définir les arguments
(gdb) set args arg1 arg2
(gdb) run
```

---

## Déboguer un Core Dump

```bash
# Activer les core dumps
ulimit -c unlimited

# Si le programme crash, analyser le core
gdb ./programme core

(gdb) backtrace       # Voir où ça a crashé
(gdb) frame 0         # Aller à la frame du crash
(gdb) info locals     # Variables locales
```

---

## Attacher à un Processus

```bash
# Trouver le PID
ps aux | grep programme

# Attacher GDB
gdb -p 12345
# ou
(gdb) attach 12345

# Détacher
(gdb) detach
```

---

## GDB pour l'Exploitation

### Désactiver ASLR dans GDB

```bash
(gdb) set disable-randomization on   # Défaut
(gdb) set disable-randomization off  # Pour tests réalistes
```

### Analyser un Buffer Overflow

```bash
(gdb) run $(python -c 'print "A"*100')

# Quand ça crash:
(gdb) info registers
# EIP = 0x41414141 ? → Contrôle de EIP!

(gdb) x/100x $esp     # Examiner la pile
```

### Définir des Breakpoints Conditionnels

```bash
(gdb) break *0x08048500 if $eax == 0
(gdb) break fonction if argc > 1
```

---

## Configuration ~/.gdbinit

```bash
# ~/.gdbinit - Configuration recommandée

# Syntaxe Intel (plus lisible)
set disassembly-flavor intel

# Pas de pagination
set pagination off

# Historique
set history save on
set history size 10000
set history filename ~/.gdb_history

# Suivre les forks
set follow-fork-mode child

# Afficher les instructions après chaque step
display/i $pc
```

---

## Extensions Utiles

### GEF (GDB Enhanced Features)

```bash
# Installation
bash -c "$(curl -fsSL https://gef.blah.cat/sh)"

# Fonctionnalités:
# - Affichage amélioré des registres
# - Visualisation de la pile
# - Aide à l'exploitation
```

### PEDA

```bash
git clone https://github.com/longld/peda.git ~/peda
echo "source ~/peda/peda.py" >> ~/.gdbinit
```

### pwndbg

```bash
git clone https://github.com/pwndbg/pwndbg
cd pwndbg && ./setup.sh
```

---

## Résumé

```
┌─────────────────────────────────────────────────────────────────┐
│                   COMMANDES À MÉMORISER                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Démarrage:  gdb -q ./programme                                │
│                                                                 │
│  Exécution:  break main → run → next/step → continue          │
│                                                                 │
│  Inspection: print var, info registers, x/10x $esp             │
│                                                                 │
│  Code:       disassemble, list, backtrace                      │
│                                                                 │
│  Mémoire:    x/format adresse                                  │
│              x = hex, s = string, i = instruction              │
│                                                                 │
│  Modifier:   set $reg = valeur, set var = valeur              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Navigation

| Précédent | Suivant |
|-----------|---------|
| [03 - Compilation](03-compilation.md) | [Exercices](../exercices/exercice-01.md) |
