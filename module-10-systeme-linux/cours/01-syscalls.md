# Cours 01 : Appels Système Linux

## Qu'est-ce qu'un Syscall ?

Un syscall est une demande au noyau pour exécuter une opération privilégiée.

```
┌─────────────────────────────────────────────────────────────────┐
│  ESPACE UTILISATEUR                                             │
│  ─────────────────                                              │
│  Programme → libc wrapper (write()) → syscall instruction      │
│                                                                 │
│  ═══════════════════ Transition Ring 3 → Ring 0 ═══════════════│
│                                                                 │
│  ESPACE NOYAU                                                   │
│  ────────────                                                   │
│  sys_write() → Pilote → Matériel                               │
└─────────────────────────────────────────────────────────────────┘
```

## Syscalls Courants

```c
// Fichiers
int open(const char *path, int flags, mode_t mode);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);

// Processus
pid_t fork(void);
int execve(const char *path, char *const argv[], char *const envp[]);
pid_t getpid(void);
int kill(pid_t pid, int sig);

// Mémoire
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off);
int mprotect(void *addr, size_t len, int prot);

// Réseau
int socket(int domain, int type, int protocol);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

## Appel Direct (sans libc)

```c
#include <sys/syscall.h>
#include <unistd.h>

// Via syscall()
long result = syscall(SYS_write, 1, "Hello\n", 6);

// En assembleur inline (x86_64)
ssize_t my_write(int fd, const void *buf, size_t count) {
    ssize_t ret;
    asm volatile (
        "syscall"
        : "=a" (ret)
        : "0" (SYS_write), "D" (fd), "S" (buf), "d" (count)
        : "rcx", "r11", "memory"
    );
    return ret;
}
```

## Table des Syscalls x86_64

```
┌────────┬────────────┬─────────────────────────────────────────┐
│ Numéro │ Nom        │ Arguments                               │
├────────┼────────────┼─────────────────────────────────────────┤
│   0    │ read       │ fd, buf, count                          │
│   1    │ write      │ fd, buf, count                          │
│   2    │ open       │ filename, flags, mode                   │
│   3    │ close      │ fd                                      │
│  57    │ fork       │ -                                       │
│  59    │ execve     │ filename, argv, envp                    │
│  60    │ exit       │ status                                  │
│  62    │ kill       │ pid, sig                                │
│ 101    │ ptrace     │ request, pid, addr, data                │
└────────┴────────────┴─────────────────────────────────────────┘
```

## Conventions d'Appel x86_64

```
Arguments : rdi, rsi, rdx, r10, r8, r9
Numéro syscall : rax
Retour : rax (-errno si erreur)
```

## Shellcode : Write sans libc

```nasm
; write(1, "Hello\n", 6)
section .text
global _start

_start:
    mov rax, 1          ; syscall write
    mov rdi, 1          ; fd = stdout
    lea rsi, [rel msg]  ; buffer
    mov rdx, 6          ; count
    syscall

    mov rax, 60         ; syscall exit
    xor rdi, rdi        ; status = 0
    syscall

msg: db "Hello", 10
```

## Vérifier les Syscalls

```bash
# Tracer les syscalls d'un programme
strace ./programme

# Compter les syscalls
strace -c ./programme

# Filtrer
strace -e write ./programme
```

## Application Sécurité

```c
// Execve pour lancer un shell
char *argv[] = {"/bin/sh", NULL};
char *envp[] = {NULL};
execve("/bin/sh", argv, envp);

// Alternative sans chaîne visible
char path[] = "/bin/sh";
// XOR chaque caractère pour obfusquer
for (int i = 0; i < sizeof(path)-1; i++) {
    path[i] ^= 0x42;
}
// Déchiffrer avant execve
for (int i = 0; i < sizeof(path)-1; i++) {
    path[i] ^= 0x42;
}
execve(path, argv, envp);
```
