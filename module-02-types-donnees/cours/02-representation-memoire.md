# Cours 02 : Représentation en Mémoire

## Complément à Deux (Entiers Signés)

Les nombres négatifs utilisent le **complément à deux** :

```
┌─────────────────────────────────────────────────────────────────┐
│  Valeur      Binaire (8 bits)    Hexa                          │
│  ─────────────────────────────────────────                      │
│   127        01111111            0x7F                           │
│     1        00000001            0x01                           │
│     0        00000000            0x00                           │
│    -1        11111111            0xFF                           │
│    -2        11111110            0xFE                           │
│  -128        10000000            0x80                           │
└─────────────────────────────────────────────────────────────────┘
```

Pour obtenir le complément à deux d'un nombre :
1. Inverser tous les bits (complément à un)
2. Ajouter 1

```c
int8_t n = -5;
// 5 = 00000101
// Inversion = 11111010
// +1 = 11111011 = -5 (0xFB)
```

## Endianness (Boutisme)

```
┌─────────────────────────────────────────────────────────────────┐
│  Valeur : 0x12345678                                           │
│                                                                 │
│  LITTLE-ENDIAN (x86, x64)         BIG-ENDIAN (Network)         │
│  Adresse   Valeur                  Adresse   Valeur             │
│  0x1000    0x78 (LSB)              0x1000    0x12 (MSB)         │
│  0x1001    0x56                    0x1001    0x34               │
│  0x1002    0x34                    0x1002    0x56               │
│  0x1003    0x12 (MSB)              0x1003    0x78 (LSB)         │
└─────────────────────────────────────────────────────────────────┘
```

```c
#include <stdint.h>

// Vérifier l'endianness
int is_little_endian(void) {
    uint32_t x = 1;
    return *((uint8_t*)&x) == 1;
}

// Conversion réseau (toujours big-endian)
#include <arpa/inet.h>
uint32_t host_to_net = htonl(0x12345678);  // Host to Network Long
uint32_t net_to_host = ntohl(host_to_net); // Network to Host Long
```

## Alignement Mémoire

```c
struct Exemple {
    char a;      // 1 byte  + 3 padding
    int b;       // 4 bytes (aligné sur 4)
    char c;      // 1 byte  + 3 padding
};
// sizeof = 12 bytes (pas 6!)

// Optimiser l'ordre
struct Optimise {
    int b;       // 4 bytes
    char a;      // 1 byte
    char c;      // 1 byte + 2 padding
};
// sizeof = 8 bytes

// Forcer pas de padding (attention à la performance!)
struct __attribute__((packed)) Packed {
    char a;
    int b;
    char c;
};
// sizeof = 6 bytes
```

## Représentation des Flottants (IEEE 754)

```
┌─────────────────────────────────────────────────────────────────┐
│  float (32 bits) : | Signe | Exposant | Mantisse |             │
│                    |  1 bit|  8 bits  | 23 bits  |             │
│                                                                 │
│  double (64 bits): | Signe | Exposant | Mantisse |             │
│                    |  1 bit| 11 bits  | 52 bits  |             │
│                                                                 │
│  Exemple : 3.14f                                               │
│  Binaire : 0 10000000 10010001111010111000011                  │
│           (+ 128     1.57...)                                  │
└─────────────────────────────────────────────────────────────────┘
```

## Visualiser en Mémoire

```c
#include <stdio.h>
#include <stdint.h>

void dump_memory(void *ptr, size_t size) {
    uint8_t *bytes = (uint8_t*)ptr;
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}

int main(void) {
    int32_t n = 0x12345678;
    printf("int 0x12345678 en mémoire: ");
    dump_memory(&n, sizeof(n));
    // Affiche: 78 56 34 12 (little-endian)

    float f = 3.14f;
    printf("float 3.14 en mémoire: ");
    dump_memory(&f, sizeof(f));

    return 0;
}
```

## Unions pour Inspecter les Bytes

```c
// Technique utile pour voir la représentation
union IntBytes {
    int32_t value;
    uint8_t bytes[4];
};

union IntBytes u;
u.value = 0xDEADBEEF;
printf("Byte 0: 0x%02X\n", u.bytes[0]);  // 0xEF (little-endian)
printf("Byte 3: 0x%02X\n", u.bytes[3]);  // 0xDE
```
