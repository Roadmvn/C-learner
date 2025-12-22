# Cours 02 : Boucles

## for

```c
// Syntaxe : for (initialisation; condition; incrémentation)
for (int i = 0; i < 10; i++) {
    printf("%d ", i);  // 0 1 2 3 4 5 6 7 8 9
}

// Parcourir un tableau
int arr[] = {10, 20, 30, 40, 50};
size_t len = sizeof(arr) / sizeof(arr[0]);
for (size_t i = 0; i < len; i++) {
    printf("%d ", arr[i]);
}

// Boucle infinie
for (;;) {
    // Boucle à l'infini jusqu'à break
}
```

## while

```c
// Condition testée AVANT chaque itération
int count = 0;
while (count < 5) {
    printf("%d ", count);
    count++;
}

// Lecture jusqu'à EOF
int c;
while ((c = getchar()) != EOF) {
    putchar(c);
}
```

## do-while

```c
// Condition testée APRÈS chaque itération (au moins une exécution)
int num;
do {
    printf("Entrez un nombre positif: ");
    scanf("%d", &num);
} while (num <= 0);
```

## break et continue

```c
// break : sort de la boucle
for (int i = 0; i < 100; i++) {
    if (arr[i] == target) {
        printf("Trouvé à l'index %d\n", i);
        break;  // Sort immédiatement
    }
}

// continue : passe à l'itération suivante
for (int i = 0; i < 10; i++) {
    if (i % 2 == 0) {
        continue;  // Saute les pairs
    }
    printf("%d ", i);  // Affiche 1 3 5 7 9
}
```

## Boucles Imbriquées

```c
// Matrice
for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
        printf("[%d][%d] ", i, j);
    }
    printf("\n");
}

// Break ne sort que de la boucle interne!
for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
        if (condition) {
            break;  // Sort de la boucle j, pas de i
        }
    }
}

// Solution : utiliser goto ou un flag
int found = 0;
for (int i = 0; i < 10 && !found; i++) {
    for (int j = 0; j < 10; j++) {
        if (arr[i][j] == target) {
            found = 1;
            break;
        }
    }
}
```

## Pièges Courants

```c
// Boucle infinie accidentelle
int i = 0;
while (i < 10) {
    printf("%d", i);
    // Oublié: i++;  → Boucle infinie!
}

// Off-by-one error
for (int i = 0; i <= 10; i++) {  // Devrait être < 10?
    arr[i] = 0;  // Accès arr[10] si arr a 10 éléments!
}

// Modification de l'itérateur
for (int i = 0; i < 10; i++) {
    if (condition) {
        i--;  // Dangereux! Peut causer boucle infinie
    }
}
```

## Application Sécurité : Bruteforce

```c
// Bruteforce simple (éducatif)
#include <string.h>

int bruteforce_pin(int target) {
    for (int pin = 0; pin <= 9999; pin++) {
        if (pin == target) {
            return pin;  // Trouvé!
        }
    }
    return -1;
}

// Génération de combinaisons
void generate_passwords(char *charset, int length) {
    int charset_len = strlen(charset);
    char password[length + 1];
    password[length] = '\0';

    // Compteur pour chaque position
    int counters[length];
    memset(counters, 0, sizeof(counters));

    while (1) {
        // Construire le mot de passe
        for (int i = 0; i < length; i++) {
            password[i] = charset[counters[i]];
        }
        printf("%s\n", password);

        // Incrémenter (comme un compteur)
        int pos = length - 1;
        while (pos >= 0) {
            counters[pos]++;
            if (counters[pos] < charset_len) {
                break;
            }
            counters[pos] = 0;
            pos--;
        }
        if (pos < 0) break;  // Overflow = terminé
    }
}
```
