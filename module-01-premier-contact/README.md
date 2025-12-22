# Module 01 : Premier Contact avec le C

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│   "Le C est le langage de la machine. Le maîtriser,                        │
│    c'est maîtriser la machine elle-même."                                   │
│                                                                             │
│   Ce module vous fait écrire vos premiers programmes C                      │
│   et comprendre le processus de compilation.                                │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Objectifs du Module

À la fin de ce module, vous serez capable de :

- [ ] Configurer un environnement de développement C
- [ ] Comprendre la structure d'un programme C
- [ ] Compiler et exécuter un programme
- [ ] Utiliser les bases de printf et scanf
- [ ] Débugger avec GDB à un niveau basique

## Pourquoi le C pour la Sécurité Offensive ?

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│  Le C est choisi pour les outils offensifs car :               │
│                                                                 │
│  ✓ Contrôle total sur la mémoire                               │
│  ✓ Pas de runtime ou garbage collector                         │
│  ✓ Génère du code machine compact                              │
│  ✓ Accès direct aux appels système                             │
│  ✓ Portable entre systèmes                                      │
│  ✓ Lecture/écriture de malware existant                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Plan du Module

### Cours Théoriques

| # | Cours | Description |
|---|-------|-------------|
| 01 | [Environnement de Développement](cours/01-environnement.md) | Installation et configuration |
| 02 | [Premier Programme](cours/02-premier-programme.md) | Hello World disséqué |
| 03 | [Compilation en Détail](cours/03-compilation.md) | Du source à l'exécutable |
| 04 | [Introduction à GDB](cours/04-intro-gdb.md) | Débogage fondamental |

### Exemples de Code

| Fichier | Description |
|---------|-------------|
| [hello.c](code/hello.c) | Premier programme |
| [structure.c](code/structure.c) | Structure d'un programme |
| [compilation.c](code/compilation.c) | Exemple de compilation |

### Exercices

| # | Exercice | Difficulté |
|---|----------|------------|
| 01 | [Premier Programme](exercices/exercice-01.md) | ★☆☆ |
| 02 | [Compilation Manuelle](exercices/exercice-02.md) | ★★☆ |
| 03 | [Debug avec GDB](exercices/exercice-03.md) | ★★☆ |

## Prérequis

- Module 00 complété
- Accès à un système Linux (VM recommandée)

## Navigation

| Précédent | Suivant |
|-----------|---------|
| [Module 00 : Fondamentaux](../module-00-fondamentaux/) | [Module 02 : Types de Données](../module-02-types-donnees/) |
