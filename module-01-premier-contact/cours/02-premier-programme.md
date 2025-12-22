# Cours 02 : Premier Programme C

## Le Programme Hello World

Commençons par le programme le plus classique en programmation :

```c
#include <stdio.h>

int main(void) {
    printf("Hello, World!\n");
    return 0;
}
```

Ce programme affiche "Hello, World!" puis se termine. Analysons chaque partie.

---

## Anatomie du Programme

```
┌─────────────────────────────────────────────────────────────────┐
│                   STRUCTURE D'UN PROGRAMME C                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  #include <stdio.h>                                             │
│  │                                                              │
│  └── DIRECTIVES PRÉPROCESSEUR                                  │
│      Inclut les déclarations pour printf()                     │
│                                                                 │
│  int main(void) {                                              │
│  │   │    │                                                     │
│  │   │    └── PARAMÈTRES (void = aucun)                        │
│  │   └─────── NOM DE LA FONCTION (point d'entrée)              │
│  └─────────── TYPE DE RETOUR (entier)                          │
│                                                                 │
│      printf("Hello, World!\n");                                │
│      │                     │                                    │
│      │                     └── ARGUMENT (chaîne à afficher)    │
│      └──────────────────────── APPEL DE FONCTION               │
│                                                                 │
│      return 0;                                                  │
│      │      │                                                   │
│      │      └── VALEUR RETOURNÉE (0 = succès)                  │
│      └───────── INSTRUCTION DE RETOUR                          │
│                                                                 │
│  }                                                              │
│  └── FIN DE LA FONCTION                                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## #include et les Headers

```c
#include <stdio.h>  // Header de la bibliothèque standard
```

### Pourquoi les includes ?

Le C sépare **déclaration** et **définition** :

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│  stdio.h (header)              libc.so (bibliothèque)          │
│  ┌─────────────────────┐       ┌─────────────────────┐         │
│  │ int printf(...);    │       │ [code machine de    │         │
│  │ int scanf(...);     │ ────► │  printf, scanf,     │         │
│  │ // déclarations     │       │  etc.]              │         │
│  └─────────────────────┘       └─────────────────────┘         │
│                                                                 │
│  Le compilateur a besoin       Le linker trouve le             │
│  de connaître la signature     vrai code ici                   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Syntaxe des includes

```c
#include <stdio.h>    // Cherche dans les dossiers système
#include "monheader.h" // Cherche d'abord dans le dossier courant
```

---

## La Fonction main()

La fonction `main` est le **point d'entrée** du programme.

### Signatures valides

```c
// Forme minimale
int main(void) {
    return 0;
}

// Avec arguments de ligne de commande
int main(int argc, char *argv[]) {
    // argc = nombre d'arguments
    // argv = tableau des arguments
    return 0;
}

// Forme équivalente
int main(int argc, char **argv) {
    return 0;
}
```

### Valeur de retour

```c
return 0;   // Succès
return 1;   // Erreur générique
return -1;  // Souvent utilisé pour erreur
```

Le shell récupère cette valeur :
```bash
./programme
echo $?  # Affiche le code de retour
```

---

## printf() - Affichage Formaté

`printf` affiche du texte formaté sur la sortie standard.

### Syntaxe

```c
printf("format", arg1, arg2, ...);
```

### Spécificateurs de format

```
┌──────────────────────────────────────────────────────────────┐
│                  SPÉCIFICATEURS PRINTF                       │
├──────────────────────────────────────────────────────────────┤
│  %d, %i    Entier signé décimal                             │
│  %u        Entier non signé décimal                          │
│  %x, %X    Entier en hexadécimal (minuscule/majuscule)      │
│  %o        Entier en octal                                   │
│  %c        Caractère                                         │
│  %s        Chaîne de caractères                              │
│  %p        Pointeur (adresse)                                │
│  %f        Nombre flottant                                   │
│  %e, %E    Notation scientifique                             │
│  %%        Affiche un %                                      │
│  %n        DANGEREUX - écrit le nombre de chars affichés    │
└──────────────────────────────────────────────────────────────┘
```

### Exemples

```c
int age = 25;
char initial = 'J';
float taille = 1.75;
char *nom = "Jean";

printf("Nom: %s, Initiale: %c\n", nom, initial);
printf("Age: %d ans\n", age);
printf("Taille: %.2f m\n", taille);
printf("Adresse de age: %p\n", (void*)&age);
printf("Age en hexa: 0x%x\n", age);
```

### Caractères d'échappement

```c
\n    // Nouvelle ligne
\t    // Tabulation
\r    // Retour chariot
\\    // Backslash
\"    // Guillemet
\0    // Caractère NULL (fin de chaîne)
\x41  // Caractère par code hexa (A)
```

---

## Variables et Types Basiques

### Déclaration

```c
type nom_variable;           // Déclaration
type nom_variable = valeur;  // Déclaration + initialisation
```

### Types fondamentaux

```c
int entier = 42;              // Entier (généralement 32 bits)
char caractere = 'A';         // Caractère (8 bits)
float decimal = 3.14f;        // Flottant simple précision
double precision = 3.14159;   // Flottant double précision
```

### Convention de nommage

```c
// snake_case (recommandé pour le C)
int nombre_utilisateurs;
char *nom_fichier;

// camelCase (acceptable)
int nombreUtilisateurs;
```

---

## Exemple Complet Commenté

```c
/*
 * Programme : demo.c
 * Description : Démonstration des bases du C
 * Usage : ./demo
 */

#include <stdio.h>  // Pour printf()

// Point d'entrée du programme
int main(void) {
    // Déclaration et initialisation de variables
    int annee = 2024;
    char lettre = 'X';
    float pi = 3.14159f;

    // Affichage formaté
    printf("=== Démonstration C ===\n");
    printf("Année: %d\n", annee);
    printf("Lettre: %c (ASCII: %d, Hexa: 0x%02X)\n",
           lettre, lettre, lettre);
    printf("Pi: %.5f\n", pi);

    // Affichage d'adresses mémoire
    printf("\nAdresses en mémoire:\n");
    printf("  annee:  %p\n", (void*)&annee);
    printf("  lettre: %p\n", (void*)&lettre);
    printf("  pi:     %p\n", (void*)&pi);

    // Retour avec succès
    return 0;
}
```

### Compilation et exécution

```bash
gcc -Wall -g demo.c -o demo
./demo
```

---

## Erreurs Courantes

### 1. Oublier le point-virgule

```c
printf("Hello")   // ERREUR : manque ;
printf("Hello");  // Correct
```

### 2. Mauvais type dans printf

```c
int x = 42;
printf("%s", x);  // ERREUR : %s attend une chaîne, pas un int
printf("%d", x);  // Correct
```

### 3. Oublier le \n

```c
printf("Ligne 1");
printf("Ligne 2");
// Affiche: Ligne 1Ligne 2 (sur la même ligne)

printf("Ligne 1\n");
printf("Ligne 2\n");
// Affiche correctement sur deux lignes
```

### 4. Oublier return dans main

```c
int main(void) {
    printf("Hello");
    // ATTENTION: pas de return, comportement indéfini
}
```

---

## Application en Sécurité

### Format String Vulnerability

```c
// Code VULNÉRABLE - NE JAMAIS FAIRE
char *input = get_user_input();
printf(input);  // DANGER: l'utilisateur contrôle le format

// Code SÉCURISÉ
printf("%s", input);  // L'input est traité comme une chaîne
```

Si l'utilisateur entre `%x %x %x`, le code vulnérable afficherait des valeurs de la pile !

---

## Résumé

```
┌─────────────────────────────────────────────────────────────────┐
│                        POINTS CLÉS                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  • #include pour importer les déclarations                     │
│                                                                 │
│  • main() est le point d'entrée                                │
│                                                                 │
│  • return 0 indique le succès                                  │
│                                                                 │
│  • printf() pour l'affichage formaté                           │
│                                                                 │
│  • Toujours utiliser printf("%s", var) pour les chaînes       │
│    utilisateur (sécurité)                                      │
│                                                                 │
│  • Compiler avec -Wall pour voir les warnings                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Navigation

| Précédent | Suivant |
|-----------|---------|
| [01 - Environnement](01-environnement.md) | [03 - Compilation](03-compilation.md) |
