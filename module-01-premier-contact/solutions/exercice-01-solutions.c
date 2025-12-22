/*
 * Solutions de l'Exercice 01 - Premier Programme
 *
 * Ce fichier contient toutes les solutions des parties 1 à 7.
 * Chaque solution est dans une fonction séparée pour organisation.
 *
 * Pour compiler une partie spécifique, utilisez les #define au début.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
 * PARTIE 1 : Hello World
 * ================================================================ */

/*
 * hello.c
 *
 * #include <stdio.h>
 *
 * int main(void) {
 *     printf("Hello, World!\n");
 *     return 0;
 * }
 */

/* ================================================================
 * PARTIE 2 : Informations Personnelles
 * ================================================================ */

void partie2_info(void) {
    printf("=== Partie 2 : Informations ===\n");
    printf("Prénom: Jean\n");
    printf("Age: 25 ans\n");
    printf("Langage préféré: C\n");
    printf("\n");
}

/* ================================================================
 * PARTIE 3 : Variables et Affichage
 * ================================================================ */

void partie3_variables(void) {
    printf("=== Partie 3 : Variables ===\n");

    int age = 25;
    char initiale = 'J';
    float taille = 1.75f;

    printf("Age: %d\n", age);
    printf("Initiale: %c\n", initiale);
    printf("Taille: %.2f m\n", taille);
    printf("\n");
}

/* ================================================================
 * PARTIE 4 : Affichage Hexadécimal
 * ================================================================ */

void partie4_hexa(void) {
    printf("=== Partie 4 : Hexadécimal ===\n");

    int valeur = 255;

    printf("Décimal: %d\n", valeur);
    printf("Hexa: 0x%x\n", valeur);
    printf("Hexa (majuscule): 0x%X\n", valeur);

    // Bonus: Affichage binaire (simulation)
    printf("Binaire: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (valeur >> i) & 1);
    }
    printf("\n\n");
}

/* ================================================================
 * PARTIE 5 : Adresses Mémoire
 * ================================================================ */

void partie5_adresses(void) {
    printf("=== Partie 5 : Adresses ===\n");

    int entier = 42;
    char caractere = 'A';
    double decimal = 3.14159;

    printf("entier    : valeur = %d,    adresse = %p\n",
           entier, (void*)&entier);
    printf("caractere : valeur = %c,    adresse = %p\n",
           caractere, (void*)&caractere);
    printf("decimal   : valeur = %.5f, adresse = %p\n",
           decimal, (void*)&decimal);
    printf("\n");
}

/* ================================================================
 * PARTIE 6 : Arguments de Ligne de Commande
 * ================================================================ */

void partie6_arguments(int argc, char *argv[]) {
    printf("=== Partie 6 : Arguments ===\n");
    printf("Nombre d'arguments: %d\n", argc);

    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    printf("\n");
}

/* ================================================================
 * PARTIE 7 : Codes de Retour
 * ================================================================ */

/*
 * retour.c - Compiler séparément
 *
 * #include <stdio.h>
 * #include <string.h>
 *
 * int main(int argc, char *argv[]) {
 *     if (argc < 2) {
 *         printf("Usage: %s <ok|erreur>\n", argv[0]);
 *         return 2;
 *     }
 *
 *     if (strcmp(argv[1], "ok") == 0) {
 *         printf("Tout va bien!\n");
 *         return 0;
 *     } else if (strcmp(argv[1], "erreur") == 0) {
 *         printf("Une erreur s'est produite!\n");
 *         return 1;
 *     } else {
 *         printf("Argument non reconnu: %s\n", argv[1]);
 *         return 2;
 *     }
 * }
 */

/* ================================================================
 * FONCTION PRINCIPALE - Exécute toutes les parties
 * ================================================================ */

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("   Solutions Exercice 01               \n");
    printf("========================================\n\n");

    partie2_info();
    partie3_variables();
    partie4_hexa();
    partie5_adresses();
    partie6_arguments(argc, argv);

    printf("========================================\n");
    printf("   Fin des solutions                   \n");
    printf("========================================\n");

    return 0;
}

/*
 * Notes importantes :
 *
 * 1. Toujours compiler avec -Wall pour voir les warnings
 *
 * 2. Le cast (void*) pour %p est une bonne pratique
 *
 * 3. strcmp() retourne 0 si les chaînes sont égales
 *
 * 4. Les codes de retour standards :
 *    - 0 : succès
 *    - 1 : erreur générique
 *    - 2 : mauvaise utilisation
 *
 * 5. Testez avec :
 *    $ gcc -Wall solutions.c -o solutions
 *    $ ./solutions arg1 arg2
 *    $ echo $?
 */
