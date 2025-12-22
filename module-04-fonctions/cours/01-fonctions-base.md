# Cours 01 : Fonctions de Base

## Déclaration et Définition

```c
// DÉCLARATION (prototype) - Dit au compilateur que la fonction existe
int addition(int a, int b);

// DÉFINITION - Le code de la fonction
int addition(int a, int b) {
    return a + b;
}

// APPEL
int resultat = addition(5, 3);  // resultat = 8
```

## Anatomie d'une Fonction

```c
type_retour nom_fonction(type1 param1, type2 param2) {
    // Corps de la fonction
    return valeur;  // Si type_retour n'est pas void
}
```

## Passage de Paramètres

### Par Valeur (Copie)

```c
void incrementer(int x) {
    x++;  // Modifie la COPIE, pas l'original
}

int main(void) {
    int a = 5;
    incrementer(a);
    printf("%d\n", a);  // Affiche 5 (pas 6!)
    return 0;
}
```

### Par Pointeur (Référence)

```c
void incrementer(int *x) {
    (*x)++;  // Modifie l'original via le pointeur
}

int main(void) {
    int a = 5;
    incrementer(&a);
    printf("%d\n", a);  // Affiche 6
    return 0;
}
```

## void - Pas de Retour

```c
void afficher_message(const char *msg) {
    printf("%s\n", msg);
    // Pas de return (ou return; sans valeur)
}
```

## Fonctions avec Nombre Variable d'Arguments

```c
#include <stdarg.h>

int somme(int count, ...) {
    va_list args;
    va_start(args, count);

    int total = 0;
    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);
    }

    va_end(args);
    return total;
}

int resultat = somme(4, 10, 20, 30, 40);  // 100
```

## static - Fonctions Locales au Fichier

```c
// Visible uniquement dans ce fichier .c
static int helper_function(int x) {
    return x * 2;
}

// Visible partout
int public_function(int x) {
    return helper_function(x) + 1;
}
```

## inline - Suggestion d'Optimisation

```c
// Suggère au compilateur d'inliner la fonction
static inline int carre(int x) {
    return x * x;
}
```

## Tableaux comme Paramètres

```c
// Ces trois déclarations sont équivalentes pour les paramètres
void traiter(int arr[], size_t len);
void traiter(int *arr, size_t len);
void traiter(int arr[10], size_t len);  // Le 10 est ignoré!

// Toujours passer la taille explicitement
void afficher_tableau(int *arr, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%d ", arr[i]);
    }
}
```

## Retourner Plusieurs Valeurs

```c
// Méthode 1 : Pointeurs (paramètres de sortie)
void divmod(int a, int b, int *quotient, int *reste) {
    *quotient = a / b;
    *reste = a % b;
}

int q, r;
divmod(17, 5, &q, &r);  // q=3, r=2

// Méthode 2 : Structure
typedef struct {
    int quotient;
    int reste;
} DivResult;

DivResult divmod2(int a, int b) {
    DivResult res = {a / b, a % b};
    return res;
}
```

## Application Sécurité

```c
// Validation des entrées
int secure_copy(char *dest, size_t dest_size,
                const char *src, size_t src_len) {
    if (dest == NULL || src == NULL) {
        return -1;  // Erreur : pointeur NULL
    }
    if (src_len >= dest_size) {
        return -2;  // Erreur : buffer trop petit
    }

    memcpy(dest, src, src_len);
    dest[src_len] = '\0';
    return 0;  // Succès
}
```
