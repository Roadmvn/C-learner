# Module 05 : Pointeurs - Le Cœur du C

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│   "Les pointeurs sont la raison pour laquelle le C existe.                 │
│    Les maîtriser, c'est maîtriser la mémoire elle-même."                   │
│                                                                             │
│   Ce module est LE module le plus important.                                │
│   Sans lui, impossible de comprendre la sécurité offensive.                │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Objectifs

- [ ] Comprendre ce qu'est une adresse mémoire
- [ ] Maîtriser les opérateurs * et &
- [ ] Manipuler les tableaux via pointeurs
- [ ] Comprendre l'arithmétique des pointeurs
- [ ] Utiliser les pointeurs de pointeurs

## Cours

| # | Cours | Description |
|---|-------|-------------|
| 01 | [Pointeurs de Base](cours/01-pointeurs-base.md) | Adresses, déclaration, déréférencement |
| 02 | [Pointeurs et Tableaux](cours/02-pointeurs-tableaux.md) | Équivalence, arithmétique |
| 03 | [Pointeurs Avancés](cours/03-pointeurs-avances.md) | Double pointeurs, void*, const |

## Importance pour la Sécurité

```
Buffer Overflow      → Manipulation de pointeurs
Heap Exploitation    → Compréhension des allocations
Shellcode            → Pointeurs vers code
Injection            → Écriture via pointeurs
```

## Navigation
| Précédent | Suivant |
|-----------|---------|
| [Module 04](../module-04-fonctions/) | [Module 06](../module-06-memoire-dynamique/) |
