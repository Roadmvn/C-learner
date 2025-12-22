# Cours 01 : Conditions

## if, else if, else

```c
int age = 18;

if (age < 18) {
    printf("Mineur\n");
} else if (age == 18) {
    printf("Tout juste majeur\n");
} else {
    printf("Majeur\n");
}
```

## Opérateurs de Comparaison

```c
==    // Égal
!=    // Différent
<     // Inférieur
>     // Supérieur
<=    // Inférieur ou égal
>=    // Supérieur ou égal
```

## Opérateurs Logiques

```c
&&    // ET logique
||    // OU logique
!     // NON logique

if (age >= 18 && age < 65) {
    printf("Adulte actif\n");
}

if (role == ADMIN || role == ROOT) {
    printf("Accès autorisé\n");
}

if (!is_blocked) {
    execute_action();
}
```

## Opérateur Ternaire

```c
// condition ? valeur_si_vrai : valeur_si_faux
int max = (a > b) ? a : b;
char *status = (active) ? "ON" : "OFF";
```

## switch

```c
int code = get_code();

switch (code) {
    case 0:
        printf("Succès\n");
        break;
    case 1:
        printf("Erreur mineure\n");
        break;
    case 2:
    case 3:  // Fall-through intentionnel
        printf("Erreur critique\n");
        break;
    default:
        printf("Code inconnu: %d\n", code);
        break;
}
```

## Piège Courant : = vs ==

```c
// ERREUR COURANTE (assigne au lieu de comparer)
if (x = 5) {  // Toujours vrai! x devient 5
    // ...
}

// CORRECT
if (x == 5) {
    // ...
}

// Technique défensive (Yoda conditions)
if (5 == x) {  // Erreur de compilation si on met =
    // ...
}
```

## Évaluation Court-Circuit

```c
// Le second terme n'est évalué que si nécessaire
if (ptr != NULL && ptr->value > 10) {
    // Si ptr est NULL, ptr->value n'est jamais évalué
    // Évite le crash!
}

// Utile pour les vérifications
if (len > 0 && buffer[0] == 'A') {
    // Sûr car len > 0 est vérifié d'abord
}
```

## Application Sécurité

```c
// Vérification d'authentification
int check_password(const char *input, const char *stored) {
    // VULNÉRABLE : timing attack possible
    if (strcmp(input, stored) == 0) {
        return 1;
    }
    return 0;
}

// Plus sûr : comparaison en temps constant
int secure_compare(const char *a, const char *b, size_t len) {
    int result = 0;
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];  // Accumule les différences
    }
    return result == 0;
}
```
