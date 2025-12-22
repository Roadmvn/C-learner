# Cours 02 : Pile d'Appels

## Stack Frame

Chaque appel de fonction crée une **stack frame** contenant :

```
┌─────────────────────────────────────────────────────────────────┐
│                    STACK FRAME                                  │
├─────────────────────────────────────────────────────────────────┤
│   Adresses hautes                                               │
│   ┌────────────────────────────────────────────┐                │
│   │  Arguments (empilés par l'appelant)        │                │
│   ├────────────────────────────────────────────┤                │
│   │  Adresse de retour (empilée par CALL)     │                │
│   ├────────────────────────────────────────────┤                │
│   │  EBP sauvegardé (ancien base pointer)     │ ← EBP          │
│   ├────────────────────────────────────────────┤                │
│   │  Variables locales                         │                │
│   │  (allouées par la fonction)               │                │
│   └────────────────────────────────────────────┘ ← ESP          │
│   Adresses basses                                               │
└─────────────────────────────────────────────────────────────────┘
```

## Prologue et Épilogue

```nasm
; PROLOGUE (début de fonction)
push ebp        ; Sauvegarder l'ancien EBP
mov ebp, esp    ; Nouveau EBP = ESP actuel
sub esp, N      ; Allouer N bytes pour les variables locales

; ... code de la fonction ...

; ÉPILOGUE (fin de fonction)
mov esp, ebp    ; Restaurer ESP
pop ebp         ; Restaurer l'ancien EBP
ret             ; Retourner (pop EIP)
```

## Exemple Pratique

```c
int calcul(int x, int y) {
    int local = x + y;
    return local * 2;
}

int main(void) {
    int resultat = calcul(5, 3);
    return 0;
}
```

État de la pile dans `calcul()` :
```
ESP →  │ local = 8            │ [EBP - 4]
       ├──────────────────────┤
EBP →  │ EBP de main          │
       ├──────────────────────┤
       │ Adresse retour       │ [EBP + 4]
       ├──────────────────────┤
       │ x = 5                │ [EBP + 8]
       ├──────────────────────┤
       │ y = 3                │ [EBP + 12]
```

## Récursion

```c
int factorielle(int n) {
    if (n <= 1) return 1;
    return n * factorielle(n - 1);
}

// factorielle(4) crée 4 stack frames:
// factorielle(4) → factorielle(3) → factorielle(2) → factorielle(1)
```

**Attention** : Récursion trop profonde → Stack Overflow !

```c
// Convertir en itératif si possible
int factorielle_iter(int n) {
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}
```

## Tail Call Optimization

```c
// Récursion terminale (peut être optimisée par le compilateur)
int factorielle_tail(int n, int acc) {
    if (n <= 1) return acc;
    return factorielle_tail(n - 1, n * acc);  // Le dernier appel
}

// Appel : factorielle_tail(5, 1)
```

Avec `-O2`, GCC peut transformer cela en boucle.

## Application Sécurité : Buffer Overflow

```c
void vulnerable(char *input) {
    char buffer[64];
    strcpy(buffer, input);  // Pas de vérification de taille!
}
```

Si `input` > 64 caractères :
1. Écrase les variables locales
2. Écrase l'EBP sauvegardé
3. Écrase l'adresse de retour ← **CONTRÔLE DU FLUX**
4. Quand `ret` est exécuté, saute à l'adresse écrasée

```
Avant:                      Après overflow:
├──────────────────┤        ├──────────────────┤
│ buffer[64]       │        │ AAAAAAAAAAAAAAA  │
├──────────────────┤        ├──────────────────┤
│ EBP sauvegardé   │        │ AAAAAAAAAAAAA    │
├──────────────────┤        ├──────────────────┤
│ Adresse retour   │   →    │ 0x41414141       │ ← Contrôlé!
├──────────────────┤        ├──────────────────┤
```
