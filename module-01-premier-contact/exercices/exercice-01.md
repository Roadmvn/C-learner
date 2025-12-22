# Exercice 01 : Premier Programme

## Objectif
Écrire, compiler et exécuter vos premiers programmes C.

---

## Partie 1 : Hello World

1. Créez un fichier `hello.c` qui affiche "Hello, World!"
2. Compilez-le avec `gcc -Wall -g hello.c -o hello`
3. Exécutez-le avec `./hello`
4. Vérifiez le code de retour avec `echo $?`

---

## Partie 2 : Informations Personnelles

Écrivez un programme `info.c` qui affiche :
- Votre prénom
- Votre âge
- Votre langage de programmation préféré

Utilisez plusieurs appels à `printf()`.

---

## Partie 3 : Variables et Affichage

Écrivez un programme `variables.c` qui :
1. Déclare un entier `age` initialisé à 25
2. Déclare un caractère `initiale` initialisé à 'J'
3. Déclare un flottant `taille` initialisé à 1.75
4. Affiche ces valeurs avec printf en utilisant les bons spécificateurs

Sortie attendue :
```
Age: 25
Initiale: J
Taille: 1.75 m
```

---

## Partie 4 : Affichage Hexadécimal

Écrivez un programme `hexa.c` qui :
1. Déclare un entier `valeur` initialisé à 255
2. Affiche cette valeur en décimal, hexadécimal et binaire (simulé)

Sortie attendue :
```
Décimal: 255
Hexa: 0xFF
Hexa (majuscule): 0XFF
```

**Indice** : Utilisez `%d`, `%x`, et `%X`

---

## Partie 5 : Adresses Mémoire

Écrivez un programme `adresses.c` qui :
1. Déclare trois variables de types différents
2. Affiche l'adresse de chaque variable avec `%p`

**Indice** : Utilisez l'opérateur `&` pour obtenir l'adresse

---

## Partie 6 : Arguments de Ligne de Commande

Modifiez votre programme pour utiliser `int main(int argc, char *argv[])` :
1. Affichez le nombre d'arguments (`argc`)
2. Affichez chaque argument avec son index

Test :
```bash
./programme arg1 arg2 arg3
```

Sortie attendue :
```
Nombre d'arguments: 4
argv[0] = ./programme
argv[1] = arg1
argv[2] = arg2
argv[3] = arg3
```

---

## Partie 7 : Codes de Retour

Écrivez un programme `retour.c` qui :
1. Prend un argument en ligne de commande
2. Si l'argument est "ok", retourne 0
3. Si l'argument est "erreur", retourne 1
4. Si aucun argument, retourne 2

Testez avec :
```bash
./retour ok && echo "Succès"
./retour erreur || echo "Erreur"
```

---

## Critères de Validation

- [ ] Tous les programmes compilent sans warning avec `-Wall`
- [ ] Les sorties correspondent aux exemples
- [ ] Le code est proprement formaté et commenté
- [ ] Les codes de retour sont corrects

---

## Solutions

→ Voir [../solutions/](../solutions/)
