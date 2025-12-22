# Configuration de l'Environnement Linux

## Installation des Outils

### Debian/Ubuntu

```bash
# Mise à jour
sudo apt update && sudo apt upgrade -y

# Outils de développement
sudo apt install -y build-essential gdb git

# Outils de sécurité
sudo apt install -y nasm radare2 strace ltrace

# Pour la compilation Windows (cross-compile)
sudo apt install -y mingw-w64

# Python pour les scripts
sudo apt install -y python3 python3-pip
pip3 install pwntools
```

### Configuration GDB

```bash
# GEF (GDB Enhanced Features)
bash -c "$(curl -fsSL https://gef.blah.cat/sh)"

# Ou PEDA
git clone https://github.com/longld/peda.git ~/peda
echo "source ~/peda/peda.py" >> ~/.gdbinit
```

### Configuration Bash

Ajoutez à `~/.bashrc` :

```bash
# Aliases de compilation
alias gc='gcc -Wall -Wextra -g'
alias gc32='gcc -Wall -Wextra -g -m32'
alias gcvuln='gcc -fno-stack-protector -z execstack -no-pie -g'

# Désactiver ASLR temporairement
alias noaslr='echo 0 | sudo tee /proc/sys/kernel/randomize_va_space'
alias aslr='echo 2 | sudo tee /proc/sys/kernel/randomize_va_space'
```

## VM Recommandées

1. **Kali Linux** - Pré-installé avec outils de sécurité
2. **Ubuntu/Debian** - Pour le développement
3. **Windows 10** - Pour les tests Windows (isolé!)

## Réseau

```bash
# Configuration réseau lab
# Utilisez NAT ou Host-Only pour l'isolation
```

## Test de l'Installation

```bash
# Créer un programme test
cat > test.c << 'EOF'
#include <stdio.h>
int main(void) {
    printf("Installation OK!\n");
    return 0;
}
EOF

# Compiler et exécuter
gcc -Wall test.c -o test && ./test
rm test test.c
```
