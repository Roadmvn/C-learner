# Cours 01 : Types de Données Fondamentaux

## Vue d'Ensemble

```
┌─────────────────────────────────────────────────────────────────┐
│                   TYPES DE DONNÉES EN C                         │
├─────────────────────────────────────────────────────────────────┤
│  TYPE           TAILLE      PLAGE (signé)        PLAGE (non signé) │
│  ──────────────────────────────────────────────────────────────────│
│  char           1 byte      -128 à 127           0 à 255            │
│  short          2 bytes     -32768 à 32767       0 à 65535          │
│  int            4 bytes     -2^31 à 2^31-1       0 à 2^32-1         │
│  long           4/8 bytes   dépend de l'arch.    dépend             │
│  long long      8 bytes     -2^63 à 2^63-1       0 à 2^64-1         │
│  float          4 bytes     ~1.2E-38 à 3.4E+38                      │
│  double         8 bytes     ~2.2E-308 à 1.8E+308                    │
│  void*          4/8 bytes   adresse mémoire                         │
└─────────────────────────────────────────────────────────────────┘
```

## Entiers

### Signés vs Non-signés

```c
int x = -1;              // Signé : peut être négatif
unsigned int y = 255;    // Non signé : 0 ou positif seulement

// Représentation de -1 en complément à deux (32 bits)
// 11111111 11111111 11111111 11111111 = 0xFFFFFFFF
// Comme unsigned : 4294967295
```

### Types à Taille Fixe (stdint.h)

```c
#include <stdint.h>

int8_t   a;    // Exactement 8 bits, signé
int16_t  b;    // Exactement 16 bits, signé
int32_t  c;    // Exactement 32 bits, signé
int64_t  d;    // Exactement 64 bits, signé

uint8_t  e;    // Exactement 8 bits, non signé
uint16_t f;    // Exactement 16 bits, non signé
uint32_t g;    // Exactement 32 bits, non signé
uint64_t h;    // Exactement 64 bits, non signé

size_t   s;    // Taille d'un objet (unsigned, dépend de l'arch.)
ssize_t  ss;   // Comme size_t mais signé
```

**Recommandation pour la sécurité** : Utilisez toujours les types de `stdint.h` pour éviter les surprises entre architectures.

## Caractères

```c
char c = 'A';           // 65 en ASCII
char c2 = 65;           // Même chose
char c3 = 0x41;         // Même chose en hexa

unsigned char byte = 255;  // Pour manipuler des bytes
```

## Nombres Flottants

```c
float f = 3.14f;        // Simple précision (suffixe f)
double d = 3.14159;     // Double précision (défaut)
long double ld = 3.14L; // Précision étendue

// Attention : Les flottants ont des erreurs d'arrondi
float a = 0.1f + 0.2f;  // Pas exactement 0.3!
```

## Limites (limits.h)

```c
#include <limits.h>
#include <stdint.h>

printf("int max: %d\n", INT_MAX);       // 2147483647
printf("int min: %d\n", INT_MIN);       // -2147483648
printf("uint max: %u\n", UINT_MAX);     // 4294967295
printf("char max: %d\n", CHAR_MAX);     // 127
printf("size_t max: %zu\n", SIZE_MAX);  // Dépend de l'arch.
```

## Overflow (Dépassement)

```c
// Dépassement entier : COMPORTEMENT INDÉFINI pour les signés!
int max = INT_MAX;      // 2147483647
int overflow = max + 1; // Comportement indéfini!

// Pour les non signés : wrapping prévisible
unsigned int u = UINT_MAX;  // 4294967295
unsigned int wrap = u + 1;  // 0 (retour à zéro)

// Exemple dangereux pour la sécurité
size_t taille = get_user_input();       // Ex: 0xFFFFFFFF
char *buf = malloc(taille + 1);         // Overflow! Allocation très petite
read(fd, buf, taille);                  // Buffer overflow!
```

## Casting (Conversion de Types)

```c
int i = 42;
float f = (float)i;           // Conversion explicite

// Attention aux conversions implicites!
unsigned int u = -1;          // Devient 4294967295!
int neg = (int)u;             // Redevient -1

// Troncature
long long big = 0x123456789AB;
int small = (int)big;         // Troncation à 0x56789AB
```

## sizeof

```c
printf("sizeof(char) = %zu\n", sizeof(char));        // 1
printf("sizeof(int) = %zu\n", sizeof(int));          // 4
printf("sizeof(long) = %zu\n", sizeof(long));        // 4 ou 8
printf("sizeof(void*) = %zu\n", sizeof(void*));      // 4 ou 8

int arr[10];
printf("sizeof(arr) = %zu\n", sizeof(arr));          // 40 (10 * 4)
printf("nb éléments = %zu\n", sizeof(arr)/sizeof(arr[0]));  // 10
```

## Application en Sécurité

### Integer Overflow Exploitation

```c
// Code vulnérable
void copy_data(char *input, size_t len) {
    if (len > 1024) return;  // Vérification

    size_t buf_size = len + 1;  // Si len = SIZE_MAX, overflow!
    char *buf = malloc(buf_size);  // Allocation minuscule

    memcpy(buf, input, len);  // Heap overflow!
}
```

### Type Confusion

```c
// Mélange signé/non signé dangereux
int check_bounds(int index, int size) {
    if (index < size) {
        return array[index];  // Si index négatif, pas de problème?
    }                         // FAUX! index négatif passe la vérif
}
```

## Résumé

- Utilisez `stdint.h` pour des tailles prévisibles
- Attention aux overflows (surtout dans les calculs de taille)
- Les conversions implicites peuvent être dangereuses
- Toujours vérifier les bornes avec le bon type
