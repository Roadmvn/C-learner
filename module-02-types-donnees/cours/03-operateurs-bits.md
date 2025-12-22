# Cours 03 : Opérateurs Bit à Bit

## Opérateurs de Base

```
┌─────────────────────────────────────────────────────────────────┐
│  OPÉRATEUR    SYMBOLE    DESCRIPTION                            │
│  ───────────────────────────────────────────────────────        │
│  AND          &          1 si les deux bits sont 1             │
│  OR           |          1 si au moins un bit est 1            │
│  XOR          ^          1 si les bits sont différents         │
│  NOT          ~          Inverse tous les bits                 │
│  Left Shift   <<         Décale les bits vers la gauche        │
│  Right Shift  >>         Décale les bits vers la droite        │
└─────────────────────────────────────────────────────────────────┘
```

## AND (&) - Masquage

```c
    1010 1100
AND 0000 1111
────────────
    0000 1100   // Garde seulement les 4 bits de droite

// Usage : Extraire des bits
uint8_t valeur = 0xAC;
uint8_t bas = valeur & 0x0F;     // 0x0C (bits de poids faible)
uint8_t haut = (valeur >> 4) & 0x0F;  // 0x0A (bits de poids fort)
```

## OR (|) - Mise à 1

```c
    1010 0000
OR  0000 1111
────────────
    1010 1111   // Met les 4 bits de droite à 1

// Usage : Activer des flags
uint8_t flags = 0;
flags |= 0x01;  // Active le bit 0
flags |= 0x04;  // Active le bit 2
// flags = 0x05 = 00000101
```

## XOR (^) - Basculement et Crypto

```c
    1010 1100
XOR 1111 0000
────────────
    0101 1100   // Inverse les bits où le masque est 1

// Propriétés importantes :
// A ^ A = 0
// A ^ 0 = A
// A ^ B ^ B = A  (utilisé en crypto)

// Échange sans variable temporaire
int a = 5, b = 10;
a ^= b;  // a = 15
b ^= a;  // b = 5
a ^= b;  // a = 10

// Chiffrement XOR simple (démo seulement!)
char *message = "SECRET";
char key = 0x42;
for (int i = 0; message[i]; i++) {
    message[i] ^= key;  // Chiffre
}
for (int i = 0; message[i]; i++) {
    message[i] ^= key;  // Déchiffre
}
```

## NOT (~) - Inversion

```c
uint8_t x = 0x0F;   // 00001111
uint8_t y = ~x;     // 11110000 = 0xF0

// Attention avec les entiers signés !
int a = ~0;         // -1 (tous les bits à 1)
```

## Décalages (<< et >>)

```c
// Left Shift : multiplie par 2^n
uint8_t x = 1;       // 00000001
x << 1;              // 00000010 = 2
x << 4;              // 00010000 = 16

// Right Shift : divise par 2^n
uint8_t y = 16;      // 00010000
y >> 1;              // 00001000 = 8
y >> 4;              // 00000001 = 1

// Attention : pour les signés, >> peut être arithmétique ou logique
int neg = -8;        // 11111000
neg >> 2;            // 11111110 = -2 (shift arithmétique, signe préservé)
```

## Applications Pratiques

### Manipulation de Flags

```c
#define FLAG_READ    0x01  // 00000001
#define FLAG_WRITE   0x02  // 00000010
#define FLAG_EXEC    0x04  // 00000100
#define FLAG_HIDDEN  0x08  // 00001000

uint8_t permissions = 0;

// Activer
permissions |= FLAG_READ | FLAG_WRITE;

// Désactiver
permissions &= ~FLAG_EXEC;

// Vérifier
if (permissions & FLAG_READ) {
    printf("Lecture autorisée\n");
}

// Basculer
permissions ^= FLAG_HIDDEN;
```

### Extraction de Bytes

```c
uint32_t ip = 0xC0A80001;  // 192.168.0.1

uint8_t octet1 = (ip >> 24) & 0xFF;  // 192
uint8_t octet2 = (ip >> 16) & 0xFF;  // 168
uint8_t octet3 = (ip >> 8) & 0xFF;   // 0
uint8_t octet4 = ip & 0xFF;          // 1
```

### Création de Valeurs

```c
// Construire un int32 à partir de 4 bytes
uint8_t b0 = 0xEF, b1 = 0xBE, b2 = 0xAD, b3 = 0xDE;
uint32_t val = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
// val = 0xDEADBEEF
```

### Vérification de Bits

```c
// Vérifier si un bit spécifique est à 1
int bit_is_set(uint32_t value, int bit_position) {
    return (value >> bit_position) & 1;
}

// Compter les bits à 1
int popcount(uint32_t x) {
    int count = 0;
    while (x) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}
```

## Application en Sécurité

### Obfuscation Simple

```c
// XOR avec une clé pour masquer des chaînes
void decode_string(char *str, size_t len, uint8_t key) {
    for (size_t i = 0; i < len; i++) {
        str[i] ^= key;
    }
}

// Dans les malwares, les chaînes sensibles sont souvent XORées
char encoded[] = "\x2b\x2c\x2c\x31\x5c\x58\x58\x16\x26\x27\x2c\x2a"; // "http://evil"
decode_string(encoded, sizeof(encoded)-1, 0x42);
```

### Shellcode : Éviter les NULL bytes

```c
// Au lieu de : mov eax, 0  (contient des 0x00)
// Utiliser : xor eax, eax  (31 C0, pas de NULL)
```
