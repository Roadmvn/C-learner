# Cours 01 : Environnement de Développement

## Introduction

Avant d'écrire du code, vous devez configurer votre environnement. Ce cours couvre l'installation des outils nécessaires sur Linux.

---

## Outils Essentiels

```
┌─────────────────────────────────────────────────────────────────┐
│                    TOOLCHAIN C                                  │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  GCC (GNU Compiler Collection)                                  │
│  └── Le compilateur C standard sur Linux                       │
│                                                                 │
│  GDB (GNU Debugger)                                             │
│  └── Débogueur pour analyser l'exécution                       │
│                                                                 │
│  Make                                                            │
│  └── Automatisation de la compilation                          │
│                                                                 │
│  Éditeur de texte                                               │
│  └── vim, nano, VSCode, ou autre                               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Installation sur Linux

### Debian/Ubuntu

```bash
# Installer les outils de développement
sudo apt update
sudo apt install build-essential gdb

# Vérifier l'installation
gcc --version
gdb --version
make --version
```

### Arch Linux

```bash
sudo pacman -S base-devel gdb
```

### Fedora

```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install gdb
```

---

## Vérification de l'Installation

```bash
# Créer un fichier test
echo '#include <stdio.h>
int main() {
    printf("Installation OK\\n");
    return 0;
}' > test.c

# Compiler
gcc test.c -o test

# Exécuter
./test

# Nettoyer
rm test test.c
```

Si "Installation OK" s'affiche, vous êtes prêt.

---

## Configuration Recommandée

### Fichier ~/.gdbinit

Créez ce fichier pour améliorer GDB :

```bash
cat > ~/.gdbinit << 'EOF'
# Désactiver la pagination
set pagination off

# Afficher le code source avec les instructions
set disassembly-flavor intel

# Afficher automatiquement les registres
# (décommentez si souhaité)
# display/i $pc

# Historique des commandes
set history save on
set history filename ~/.gdb_history
EOF
```

### Alias utiles (~/.bashrc)

```bash
# Ajouter à ~/.bashrc
alias gc='gcc -Wall -Wextra -g'
alias gcr='gcc -Wall -Wextra -O2'
alias gcs='gcc -Wall -Wextra -g -fno-stack-protector -z execstack -no-pie'
```

- `gc` : Compilation avec warnings et debug
- `gcr` : Compilation optimisée (release)
- `gcs` : Compilation sans protections (pour tests de sécurité)

---

## Structure d'un Projet C

```
projet/
├── src/           # Code source (.c)
├── include/       # Headers (.h)
├── obj/           # Fichiers objets (.o)
├── bin/           # Exécutables
├── Makefile       # Script de compilation
└── README.md      # Documentation
```

### Makefile Basique

```makefile
# Makefile simple
CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = programme
SRC = main.c utils.c
OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: clean
```

---

## Éditeurs de Texte

### Vim (recommandé pour la sécurité)

```vim
" Ajouter à ~/.vimrc
syntax on
set number
set tabstop=4
set shiftwidth=4
set expandtab
set autoindent
filetype plugin indent on
```

### VSCode

Extensions recommandées :
- C/C++ (Microsoft)
- CodeLLDB (debugging)
- Hex Editor

---

## Premier Test Complet

Créez ces fichiers pour vérifier que tout fonctionne :

**main.c**
```c
#include <stdio.h>

int main(void) {
    int x = 42;
    printf("La réponse est : %d\n", x);
    return 0;
}
```

**Compilation et debug**
```bash
# Compiler avec debug
gcc -g -Wall main.c -o main

# Lancer le débogueur
gdb ./main

# Dans GDB:
# (gdb) break main
# (gdb) run
# (gdb) print x
# (gdb) quit
```

---

## Résumé

```
┌─────────────────────────────────────────────────────────────────┐
│                        CHECKLIST                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  [ ] GCC installé et fonctionnel                               │
│  [ ] GDB installé                                               │
│  [ ] Make installé                                              │
│  [ ] Éditeur de texte configuré                                │
│  [ ] Premier programme compilé et exécuté                      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Navigation

| Précédent | Suivant |
|-----------|---------|
| [README du Module](../README.md) | [02 - Premier Programme](02-premier-programme.md) |
