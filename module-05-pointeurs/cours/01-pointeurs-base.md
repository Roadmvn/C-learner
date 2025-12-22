# Cours 01 : Pointeurs de Base

## Qu'est-ce qu'un Pointeur ?

Un **pointeur** est une variable qui contient une **adresse mémoire**.

```
┌─────────────────────────────────────────────────────────────────┐
│                         MÉMOIRE                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   Adresse     Contenu         Variable                          │
│   ─────────────────────────────────────                         │
│   0x1000      42              int x = 42;                       │
│   0x1004      0x1000          int *ptr = &x;                    │
│                ↑                                                │
│                └── ptr contient l'ADRESSE de x                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Déclaration

```c
int *ptr;        // Pointeur vers un int
char *str;       // Pointeur vers un char
float *fptr;     // Pointeur vers un float
void *generic;   // Pointeur générique (vers n'importe quoi)
```

**Lecture** : `int *ptr` se lit "ptr est un pointeur vers un int"

## Opérateurs Fondamentaux

### & (Adresse de)

```c
int x = 42;
int *ptr = &x;   // ptr = adresse de x

printf("Valeur de x: %d\n", x);           // 42
printf("Adresse de x: %p\n", (void*)&x);  // 0x7ffd... (exemple)
printf("Valeur de ptr: %p\n", (void*)ptr); // Même adresse
```

### * (Déréférencement)

```c
int x = 42;
int *ptr = &x;

printf("Valeur pointée: %d\n", *ptr);  // 42

*ptr = 100;      // Modifier x via le pointeur
printf("x = %d\n", x);  // 100
```

## Schéma Mental

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│   int x = 42;              int *ptr = &x;                      │
│                                                                 │
│   ┌─────────┐              ┌─────────┐                          │
│   │   42    │ ◄────────────│ 0x1000  │                          │
│   └─────────┘              └─────────┘                          │
│   0x1000 (x)               0x1004 (ptr)                         │
│                                                                 │
│   x     → valeur directe (42)                                  │
│   &x    → adresse de x (0x1000)                                │
│   ptr   → contenu du pointeur (0x1000)                         │
│   *ptr  → valeur pointée (42)                                  │
│   &ptr  → adresse du pointeur (0x1004)                         │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Pointeur NULL

```c
int *ptr = NULL;  // Pointeur vers "rien"

// Toujours vérifier avant déréférencement!
if (ptr != NULL) {
    printf("%d\n", *ptr);
} else {
    printf("Pointeur NULL!\n");
}

// Déréférencer NULL = Segmentation Fault (crash)
```

## Passage par Pointeur

```c
// Sans pointeur : on modifie une copie
void increment_copie(int x) {
    x++;  // Modifie la copie locale
}

// Avec pointeur : on modifie l'original
void increment_pointeur(int *x) {
    (*x)++;  // Modifie la valeur à l'adresse pointée
}

int main(void) {
    int a = 5;

    increment_copie(a);
    printf("a = %d\n", a);  // 5 (pas changé)

    increment_pointeur(&a);
    printf("a = %d\n", a);  // 6 (changé!)

    return 0;
}
```

## Taille d'un Pointeur

```c
// Tous les pointeurs ont la même taille (dépend de l'architecture)
printf("sizeof(int*) = %zu\n", sizeof(int*));     // 4 (32-bit) ou 8 (64-bit)
printf("sizeof(char*) = %zu\n", sizeof(char*));   // Idem
printf("sizeof(void*) = %zu\n", sizeof(void*));   // Idem
```

## Erreurs Courantes

```c
// 1. Pointeur non initialisé (dangereux!)
int *ptr;          // Valeur indéfinie!
*ptr = 10;         // Comportement indéfini - crash probable

// 2. Confusion entre * dans déclaration et déréférencement
int *ptr;          // Déclaration
*ptr = 10;         // Déréférencement (différent!)

// 3. Oublier le parenthésage
int *ptr = &x;
*ptr++;            // Incrémente ptr, puis déréférence (mauvais)
(*ptr)++;          // Incrémente la valeur pointée (correct)
```

## Application Sécurité

```c
// Lecture/écriture arbitraire avec un pointeur
void write_anywhere(void *addr, uint32_t value) {
    *(uint32_t*)addr = value;
}

void read_anywhere(void *addr) {
    uint32_t value = *(uint32_t*)addr;
    printf("Valeur à %p: 0x%08X\n", addr, value);
}

// DANGER : Si un attaquant contrôle addr et value,
// il peut écrire n'importe où en mémoire!
```
