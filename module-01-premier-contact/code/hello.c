/*
 * hello.c - Premier programme C
 *
 * Compilation : gcc -Wall -g hello.c -o hello
 * Exécution   : ./hello
 *
 * Ce programme affiche "Hello, World!" sur la sortie standard.
 */

#include <stdio.h>  // Bibliothèque standard pour les entrées/sorties

/*
 * Fonction principale - Point d'entrée du programme
 *
 * Retourne : 0 en cas de succès, autre valeur en cas d'erreur
 */
int main(void) {
    // printf() affiche du texte formaté
    // \n = nouvelle ligne (newline)
    printf("Hello, World!\n");

    // Retourner 0 indique que le programme s'est terminé avec succès
    // Cette valeur peut être vérifiée avec : echo $?
    return 0;
}

/*
 * Points clés à retenir :
 *
 * 1. #include <stdio.h> importe les déclarations de printf()
 *
 * 2. main() est le point d'entrée obligatoire
 *
 * 3. Chaque instruction se termine par un point-virgule (;)
 *
 * 4. Les commentaires peuvent être :
 *    - Sur une ligne : // commentaire
 *    - Sur plusieurs lignes : /* commentaire * /
 *
 * 5. Le code est sensible à la casse : Main != main
 */
