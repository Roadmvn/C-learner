# Cours 02 : Pointeurs et Tableaux

## Équivalence Pointeur-Tableau

En C, le nom d'un tableau **est** un pointeur vers son premier élément :

```c
int arr[5] = {10, 20, 30, 40, 50};
int *ptr = arr;       // Équivalent à : ptr = &arr[0]

printf("%d\n", arr[0]);   // 10
printf("%d\n", *ptr);     // 10
printf("%d\n", *arr);     // 10
printf("%d\n", ptr[0]);   // 10

// Tous équivalents!
```

## Arithmétique des Pointeurs

```c
int arr[5] = {10, 20, 30, 40, 50};
int *ptr = arr;

ptr + 1    // Pointe vers arr[1], pas ptr + 1 byte!
ptr + 2    // Pointe vers arr[2]

// L'incrément dépend du type
// Pour int*, +1 = +4 bytes (sizeof(int))
// Pour char*, +1 = +1 byte
```

```
┌─────────────────────────────────────────────────────────────────┐
│  ARITHMÉTIQUE DES POINTEURS                                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  int arr[5]                                                     │
│  ┌─────┬─────┬─────┬─────┬─────┐                                │
│  │ 10  │ 20  │ 30  │ 40  │ 50  │                                │
│  └─────┴─────┴─────┴─────┴─────┘                                │
│  0x100 0x104 0x108 0x10C 0x110                                  │
│    ↑                                                            │
│   ptr                                                           │
│                                                                 │
│  ptr     = 0x100, *ptr     = 10                                │
│  ptr + 1 = 0x104, *(ptr+1) = 20                                │
│  ptr + 2 = 0x108, *(ptr+2) = 30                                │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Notation Équivalente

```c
arr[i]  ≡  *(arr + i)
ptr[i]  ≡  *(ptr + i)

// Même chose, syntaxe différente
printf("%d\n", arr[2]);      // 30
printf("%d\n", *(arr + 2));  // 30
printf("%d\n", 2[arr]);      // 30 (oui, ça marche! car a[b] = *(a+b))
```

## Parcourir avec un Pointeur

```c
int arr[5] = {10, 20, 30, 40, 50};

// Méthode classique (index)
for (int i = 0; i < 5; i++) {
    printf("%d ", arr[i]);
}

// Méthode pointeur
for (int *ptr = arr; ptr < arr + 5; ptr++) {
    printf("%d ", *ptr);
}

// Ou avec pointeur de fin
int *end = arr + 5;
for (int *ptr = arr; ptr < end; ptr++) {
    printf("%d ", *ptr);
}
```

## Différence : Pointeur vs Tableau

```c
int arr[5];           // Tableau : taille fixe, alloué sur stack
int *ptr;             // Pointeur : peut pointer n'importe où

sizeof(arr);          // 20 (5 * 4 bytes)
sizeof(ptr);          // 4 ou 8 (taille d'un pointeur)

arr = ptr;            // ERREUR! arr n'est pas modifiable
ptr = arr;            // OK, ptr pointe vers arr
```

## Tableaux Multidimensionnels

```c
int matrix[3][4];     // Matrice 3x4

// En mémoire : layout linéaire
// [0][0], [0][1], [0][2], [0][3], [1][0], [1][1], ...

// Accès via pointeur
int *ptr = &matrix[0][0];
// matrix[i][j] == ptr[i * 4 + j]
// matrix[1][2] == ptr[1 * 4 + 2] == ptr[6]
```

## Chaînes de Caractères

Les chaînes en C sont des tableaux de `char` terminés par `\0` :

```c
char str[] = "Hello";    // Tableau de 6 chars (H,e,l,l,o,\0)
char *ptr = "Hello";     // Pointeur vers chaîne en lecture seule

// Parcourir une chaîne
char *s = "Hello";
while (*s != '\0') {
    printf("%c", *s);
    s++;
}
// Ou simplement : while (*s) { ... s++; }

// Longueur manuelle
size_t len = 0;
char *p = str;
while (*p++) len++;
```

## Application Sécurité

### Out-of-Bounds Access

```c
int arr[5] = {1, 2, 3, 4, 5};
int *ptr = arr;

// Accès valide
printf("%d\n", ptr[4]);   // 5

// HORS LIMITES (comportement indéfini!)
printf("%d\n", ptr[5]);   // ??? Lit la mémoire après le tableau
printf("%d\n", ptr[-1]);  // ??? Lit avant le tableau

// En sécurité : peut lire/écraser des données sensibles
ptr[10] = 0x41414141;     // Écrase la mémoire plus loin
```

### Buffer Overflow via Pointeur

```c
void vulnerable(char *src) {
    char dest[10];
    char *d = dest;
    char *s = src;

    // Copie sans vérification
    while (*s) {
        *d++ = *s++;  // Overflow si src > 10 chars!
    }
    *d = '\0';
}
```

### Lecture de Mémoire Arbitraire

```c
// Si un attaquant contrôle 'offset'
int leak_data(int *base, int offset) {
    return base[offset];  // Peut lire n'importe où!
}

// base[offset] = *(base + offset)
// Si offset = 1000, lit 4000 bytes plus loin
```
