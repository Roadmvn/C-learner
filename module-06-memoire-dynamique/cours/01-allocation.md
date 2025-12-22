# Cours 01 : Allocation Dynamique

## Pourquoi l'Allocation Dynamique ?

- Taille inconnue à la compilation
- Durée de vie contrôlée
- Grandes quantités de mémoire

## malloc - Memory Allocation

```c
#include <stdlib.h>

// Alloue 'size' bytes, retourne pointeur ou NULL
void *malloc(size_t size);

// Exemple
int *ptr = (int*)malloc(sizeof(int) * 10);  // 10 entiers
if (ptr == NULL) {
    perror("malloc failed");
    exit(1);
}

// Utilisation
ptr[0] = 42;

// Libération obligatoire!
free(ptr);
ptr = NULL;  // Bonne pratique
```

## calloc - Cleared Allocation

```c
// Alloue ET initialise à zéro
void *calloc(size_t nmemb, size_t size);

int *arr = (int*)calloc(10, sizeof(int));  // 10 ints à 0
```

## realloc - Réallocation

```c
// Redimensionne un bloc existant
void *realloc(void *ptr, size_t size);

int *arr = malloc(10 * sizeof(int));
arr = realloc(arr, 20 * sizeof(int));  // Agrandir

// Attention : realloc peut déplacer le bloc!
// Toujours utiliser un pointeur temporaire
int *tmp = realloc(arr, 100 * sizeof(int));
if (tmp == NULL) {
    // arr est toujours valide!
    free(arr);
    return;
}
arr = tmp;
```

## free - Libération

```c
void free(void *ptr);

free(ptr);
ptr = NULL;  // Évite les dangling pointers

// free(NULL) est sûr (ne fait rien)
```

## Erreurs Communes

```c
// 1. Fuite mémoire (memory leak)
void leak(void) {
    int *ptr = malloc(100);
    // Oublié: free(ptr);
}  // ptr perdu, mémoire jamais libérée

// 2. Use After Free
int *ptr = malloc(sizeof(int));
free(ptr);
*ptr = 42;  // ERREUR! Mémoire libérée

// 3. Double Free
int *ptr = malloc(sizeof(int));
free(ptr);
free(ptr);  // ERREUR! Corruption du heap

// 4. Buffer overflow
int *arr = malloc(10 * sizeof(int));
arr[10] = 0;  // ERREUR! Index hors limites
```

## Application Sécurité : Use After Free

```c
struct Object {
    void (*handler)(void);
};

struct Object *obj = malloc(sizeof(struct Object));
obj->handler = safe_function;

free(obj);  // Libéré

// Attaquant alloue au même endroit
struct Object *evil = malloc(sizeof(struct Object));
evil->handler = shellcode;

// Le code utilise encore obj
obj->handler();  // Exécute shellcode!
```

## Application Sécurité : Heap Overflow

```c
char *buf1 = malloc(64);
char *buf2 = malloc(64);  // Adjacent en mémoire

strcpy(buf1, attacker_input);  // Overflow!
// Peut écraser les métadonnées de buf2
// Peut écraser le contenu de buf2
```
