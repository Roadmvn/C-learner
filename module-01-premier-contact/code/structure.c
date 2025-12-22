/*
 * structure.c - Structure complète d'un programme C
 *
 * Compilation : gcc -Wall -g structure.c -o structure
 * Exécution   : ./structure
 *
 * Ce fichier illustre l'organisation typique d'un programme C.
 */

/* ============================================================
 * SECTION 1 : INCLUDES
 * Les directives #include importent les déclarations nécessaires
 * ============================================================ */

#include <stdio.h>   // printf, scanf, etc.
#include <stdlib.h>  // malloc, free, exit, etc.
#include <string.h>  // strlen, strcpy, etc.

/* ============================================================
 * SECTION 2 : MACROS ET CONSTANTES
 * Définitions traitées par le préprocesseur
 * ============================================================ */

#define VERSION "1.0"
#define MAX_BUFFER 256
#define CARRE(x) ((x) * (x))  // Macro avec paramètre

/* ============================================================
 * SECTION 3 : DÉCLARATIONS DE TYPES
 * Structures, énumérations, typedefs
 * ============================================================ */

// Énumération
enum Couleur {
    ROUGE,
    VERT,
    BLEU
};

// Structure
struct Personne {
    char nom[50];
    int age;
};

// Typedef pour simplifier l'utilisation
typedef struct Personne Personne;

/* ============================================================
 * SECTION 4 : VARIABLES GLOBALES
 * À utiliser avec modération !
 * ============================================================ */

static int compteur_global = 0;  // static = visible uniquement dans ce fichier

/* ============================================================
 * SECTION 5 : PROTOTYPES DE FONCTIONS
 * Déclarations des fonctions définies plus bas
 * ============================================================ */

void afficher_banniere(void);
int additionner(int a, int b);
void saluer(const Personne *p);

/* ============================================================
 * SECTION 6 : FONCTION PRINCIPALE
 * Point d'entrée du programme
 * ============================================================ */

int main(int argc, char *argv[]) {
    // Afficher les arguments de ligne de commande
    printf("Nombre d'arguments: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("  argv[%d] = %s\n", i, argv[i]);
    }

    // Utiliser la bannière
    afficher_banniere();

    // Utiliser une fonction
    int resultat = additionner(5, 3);
    printf("5 + 3 = %d\n", resultat);

    // Utiliser une macro
    printf("Carré de 7: %d\n", CARRE(7));

    // Utiliser une structure
    Personne jean = {"Jean", 25};
    saluer(&jean);

    // Utiliser une énumération
    enum Couleur couleur_preferee = BLEU;
    printf("Couleur préférée (valeur): %d\n", couleur_preferee);

    return EXIT_SUCCESS;  // EXIT_SUCCESS = 0, défini dans stdlib.h
}

/* ============================================================
 * SECTION 7 : DÉFINITIONS DES FONCTIONS
 * Implémentation des fonctions déclarées plus haut
 * ============================================================ */

void afficher_banniere(void) {
    printf("\n");
    printf("=================================\n");
    printf("   Programme Structure v%s\n", VERSION);
    printf("=================================\n");
    printf("\n");
}

int additionner(int a, int b) {
    compteur_global++;  // Utilisation de la variable globale
    return a + b;
}

void saluer(const Personne *p) {
    // const = on ne modifie pas l'objet pointé
    printf("Bonjour %s, vous avez %d ans.\n", p->nom, p->age);
}

/*
 * Points clés :
 *
 * 1. Organisation logique du code en sections
 *
 * 2. Les prototypes permettent d'appeler les fonctions avant leur définition
 *
 * 3. Les macros sont des substitutions textuelles (attention aux effets de bord)
 *
 * 4. static sur une variable globale limite sa visibilité au fichier courant
 *
 * 5. const empêche la modification accidentelle
 */
