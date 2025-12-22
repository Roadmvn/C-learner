# Cours 03 : Pointeurs Avancés

## Pointeurs de Pointeurs

```c
int x = 42;
int *ptr = &x;       // Pointeur vers int
int **pptr = &ptr;   // Pointeur vers pointeur vers int

printf("%d\n", x);       // 42
printf("%d\n", *ptr);    // 42
printf("%d\n", **pptr);  // 42

// Modification
**pptr = 100;
printf("%d\n", x);       // 100
```

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│   x          ptr          pptr                                  │
│  ┌────┐     ┌────┐       ┌────┐                                │
│  │ 42 │ ◄── │0x10│ ◄──── │0x14│                                │
│  └────┘     └────┘       └────┘                                │
│  0x10       0x14         0x18                                  │
│                                                                 │
│  **pptr = *(*pptr) = *(ptr) = x = 42                          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Usage : Tableau de Chaînes

```c
char *names[] = {"Alice", "Bob", "Charlie"};
char **ptr = names;

printf("%s\n", ptr[0]);   // Alice
printf("%s\n", *ptr);     // Alice
printf("%c\n", **ptr);    // A
printf("%c\n", *ptr[1]);  // B (premier char de "Bob")
```

### Usage : Modification de Pointeur

```c
void allouer(int **ptr) {
    *ptr = malloc(sizeof(int));
    **ptr = 42;
}

int main(void) {
    int *p = NULL;
    allouer(&p);          // Passe l'adresse du pointeur
    printf("%d\n", *p);   // 42
    free(p);
    return 0;
}
```

## void* - Pointeur Générique

```c
void *generic;   // Peut pointer vers n'importe quel type

int x = 42;
float f = 3.14f;

generic = &x;
printf("%d\n", *(int*)generic);    // Cast nécessaire

generic = &f;
printf("%f\n", *(float*)generic);

// Usage : malloc retourne void*
void *mem = malloc(100);
int *arr = (int*)mem;
```

### Usage : Fonction Générique

```c
// Copie de mémoire brute
void my_memcpy(void *dest, const void *src, size_t n) {
    char *d = (char*)dest;
    const char *s = (const char*)src;
    while (n--) {
        *d++ = *s++;
    }
}
```

## const et Pointeurs

```c
// 1. Pointeur vers const (ne peut pas modifier la valeur)
const int *ptr1;
int const *ptr1;    // Équivalent
*ptr1 = 10;         // ERREUR
ptr1 = &autre;      // OK

// 2. Pointeur const (ne peut pas modifier le pointeur)
int *const ptr2 = &x;
*ptr2 = 10;         // OK
ptr2 = &autre;      // ERREUR

// 3. Les deux
const int *const ptr3 = &x;
*ptr3 = 10;         // ERREUR
ptr3 = &autre;      // ERREUR

// Astuce : lire de droite à gauche
// "const int *ptr" = ptr is a pointer to const int
// "int *const ptr" = ptr is a const pointer to int
```

## restrict (C99)

```c
// Indique que les pointeurs ne se chevauchent pas
void copy(int *restrict dest, const int *restrict src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}
// Le compilateur peut mieux optimiser
```

## Tableaux de Pointeurs vs Pointeurs de Tableaux

```c
int *arr[10];         // Tableau de 10 pointeurs vers int
int (*ptr)[10];       // Pointeur vers tableau de 10 int

int matrix[3][10];
ptr = matrix;         // ptr pointe vers la première ligne
ptr++;                // Avance de 10 ints (une ligne)
```

## Application Sécurité : Write-What-Where

```c
// Si un attaquant contrôle 'where' et 'what'
void write_what_where(void **where, void *what) {
    *where = what;    // Écriture arbitraire!
}

// Exemple d'exploitation :
// where = adresse de la GOT (printf)
// what = adresse de notre shellcode
// → Le prochain appel à printf exécute notre code!
```

## Application Sécurité : Heap Corruption

```c
// Structure simplifiée d'un chunk malloc
struct chunk {
    size_t size;
    struct chunk *fd;    // Forward pointer
    struct chunk *bk;    // Backward pointer
};

// Si on peut corrompre fd/bk, on peut faire :
// bk->fd = fd   (unlink operation)
// → Écriture arbitraire!
```

## Résumé

```
┌─────────────────────────────────────────────────────────────────┐
│  POINTEURS - RÉSUMÉ                                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  int *p          Pointeur vers int                             │
│  int **pp        Pointeur vers pointeur vers int               │
│  void *v         Pointeur générique                            │
│  const int *p    Valeur non modifiable                         │
│  int *const p    Pointeur non modifiable                       │
│                                                                 │
│  &x              Adresse de x                                  │
│  *p              Valeur à l'adresse p                          │
│  p[i]            *(p + i)                                      │
│  p + n           p + n * sizeof(*p)                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```
