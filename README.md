# Formation C pour la Sécurité Offensive

```
   ██████╗    ███████╗███████╗ ██████╗██╗   ██╗██████╗ ██╗████████╗██╗   ██╗
  ██╔════╝    ██╔════╝██╔════╝██╔════╝██║   ██║██╔══██╗██║╚══██╔══╝╚██╗ ██╔╝
  ██║         ███████╗█████╗  ██║     ██║   ██║██████╔╝██║   ██║    ╚████╔╝
  ██║         ╚════██║██╔══╝  ██║     ██║   ██║██╔══██╗██║   ██║     ╚██╔╝
  ╚██████╗    ███████║███████╗╚██████╗╚██████╔╝██║  ██║██║   ██║      ██║
   ╚═════╝    ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚═╝   ╚═╝      ╚═╝

        De zéro absolu à développeur d'outils offensifs professionnel
```

## Avertissement Légal

Ce repository est destiné **exclusivement** à des fins éducatives et de recherche en sécurité. Les techniques enseignées doivent être utilisées uniquement dans un cadre légal :
- Tests de pénétration autorisés
- Recherche en sécurité
- Environnements de laboratoire contrôlés
- Compétitions CTF

L'utilisation malveillante de ces connaissances est illégale et contraire à l'éthique.

---

## La Vision

Ce repository est conçu pour emmener quelqu'un qui n'a **aucune connaissance préalable** jusqu'à la capacité d'écrire des outils offensifs de niveau professionnel. Chaque concept est expliqué en profondeur, avec la théorie qui précède toujours la pratique.

### Philosophie Pédagogique

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          PYRAMIDE D'APPRENTISSAGE                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│                              ┌───────────┐                                  │
│                              │  PROJETS  │  ← Outils offensifs complets     │
│                              │ INTÉGRÉS  │                                  │
│                            ┌─┴───────────┴─┐                                │
│                            │   ÉVASION &   │  ← Anti-détection, anti-debug  │
│                            │  ANTI-ANALYSE │                                │
│                          ┌─┴───────────────┴─┐                              │
│                          │    INJECTION &    │  ← Manipulation mémoire      │
│                          │   MANIPULATION    │                              │
│                        ┌─┴───────────────────┴─┐                            │
│                        │   PROGRAMMATION       │  ← Sockets, protocoles     │
│                        │       RÉSEAU          │                            │
│                      ┌─┴───────────────────────┴─┐                          │
│                      │  PROGRAMMATION SYSTÈME    │  ← Linux & Windows       │
│                      │    (Linux & Windows)      │                          │
│                    ┌─┴───────────────────────────┴─┐                        │
│                    │      MAÎTRISE DU C AVANCÉ     │  ← Fichiers, Préproc   │
│                    │                               │                        │
│                  ┌─┴───────────────────────────────┴─┐                      │
│                  │    MÉMOIRE & STRUCTURES DONNÉES   │  ← Heap, Listes      │
│                  │                                   │                      │
│                ┌─┴───────────────────────────────────┴─┐                    │
│                │           POINTEURS                    │  ← Le cœur du C   │
│                │                                        │                   │
│              ┌─┴────────────────────────────────────────┴─┐                 │
│              │          FONDAMENTAUX DU C                 │  ← Syntaxe      │
│              │                                            │                 │
│            ┌─┴──────────────────────────────────────────────┴─┐             │
│            │         FONDAMENTAUX INFORMATIQUES               │ ← Binaire   │
│            │         (Bits, Mémoire, CPU, OS)                 │             │
│            └──────────────────────────────────────────────────┘             │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Principes Directeurs

1. **Jamais de magie** - Tout est expliqué, rien n'est "juste comme ça"
2. **Théorie d'abord** - Comprendre le "pourquoi" avant le "comment"
3. **Progression logique** - Chaque concept s'appuie sur le précédent
4. **Pratique réelle** - Pas d'exemples jouets, tout est utilisable
5. **Autonomie complète** - Tout est inclus, aucune ressource externe nécessaire

---

## Structure du Repository

```
C-learner/
│
├── module-00-fondamentaux/          # Bits, bytes, CPU, mémoire
├── module-01-premier-contact/       # Hello World et environnement
├── module-02-types-donnees/         # int, char, représentation
├── module-03-structures-controle/   # if, while, for
├── module-04-fonctions/             # Fonctions et stack
├── module-05-pointeurs/             # Adresses et manipulation
├── module-06-memoire-dynamique/     # malloc, heap, gestion
├── module-07-structures-donnees/    # struct, union, listes
├── module-08-fichiers-io/           # Fichiers et I/O
├── module-09-preprocesseur/         # #define, compilation
├── module-10-systeme-linux/         # Syscalls, processus Linux
├── module-11-systeme-windows/       # Win32 API, processus Windows
├── module-12-reseau/                # Sockets et protocoles
├── module-13-injection/             # Injection et manipulation
├── module-14-evasion/               # Techniques anti-détection
├── module-15-projets/               # Projets intégrateurs
│
└── ressources/
    ├── outils/                      # Scripts et utilitaires
    ├── references/                  # Documentation de référence
    └── environnement/               # Configuration des VMs
```

### Organisation de Chaque Module

```
module-XX-nom/
├── README.md           # Vue d'ensemble du module
├── cours/              # Leçons théoriques détaillées
│   ├── 01-introduction.md
│   ├── 02-concept.md
│   └── ...
├── code/               # Exemples de code commentés
│   ├── exemple-01.c
│   └── ...
├── exercices/          # Exercices pratiques
│   ├── exercice-01.md
│   └── ...
└── solutions/          # Solutions détaillées
    ├── solution-01.c
    └── ...
```

---

## Parcours d'Apprentissage

### Phase 1 : Les Fondations (Modules 00-04)
**Durée estimée : 2-3 semaines**

Objectif : Comprendre ce qu'est réellement un ordinateur et maîtriser les bases du C.

| Module | Contenu | Prérequis |
|--------|---------|-----------|
| 00 | Binaire, hexadécimal, mémoire, CPU | Aucun |
| 01 | Premier programme C, compilation | Module 00 |
| 02 | Types de données, représentation | Module 01 |
| 03 | Conditions, boucles, logique | Module 02 |
| 04 | Fonctions, pile d'appels | Module 03 |

### Phase 2 : Maîtrise de la Mémoire (Modules 05-07)
**Durée estimée : 3-4 semaines**

Objectif : Devenir maître de la mémoire - c'est la clé de la sécurité offensive.

| Module | Contenu | Prérequis |
|--------|---------|-----------|
| 05 | Pointeurs, adresses, arithmétique | Module 04 |
| 06 | Allocation dynamique, heap | Module 05 |
| 07 | Structures, unions, listes chaînées | Module 06 |

### Phase 3 : C Avancé (Modules 08-09)
**Durée estimée : 1-2 semaines**

Objectif : Maîtriser les aspects avancés du langage.

| Module | Contenu | Prérequis |
|--------|---------|-----------|
| 08 | Fichiers, I/O binaire, parsing | Module 07 |
| 09 | Préprocesseur, macros, compilation | Module 08 |

### Phase 4 : Programmation Système (Modules 10-12)
**Durée estimée : 4-5 semaines**

Objectif : Interagir directement avec le système d'exploitation.

| Module | Contenu | Prérequis |
|--------|---------|-----------|
| 10 | Linux : syscalls, processus, signaux | Module 09 |
| 11 | Windows : Win32, processus, registry | Module 09 |
| 12 | Sockets TCP/UDP, raw sockets | Modules 10-11 |

### Phase 5 : Techniques Offensives (Modules 13-15)
**Durée estimée : 4-6 semaines**

Objectif : Appliquer tout ce qui précède pour créer des outils offensifs.

| Module | Contenu | Prérequis |
|--------|---------|-----------|
| 13 | Injection de code, hooking, shellcode | Module 12 |
| 14 | Anti-debug, anti-VM, obfuscation | Module 13 |
| 15 | Projets complets : implants, C2, tools | Module 14 |

---

## Comment Utiliser ce Repository

### Pour Chaque Module

```
┌─────────────────────────────────────────────────────────────────┐
│                    CYCLE D'APPRENTISSAGE                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│     ┌──────────────┐                                            │
│     │   1. LIRE    │  Étudier les cours dans l'ordre           │
│     │   le cours   │  Prendre des notes                        │
│     └──────┬───────┘                                            │
│            │                                                    │
│            ▼                                                    │
│     ┌──────────────┐                                            │
│     │  2. ÉTUDIER  │  Lire et comprendre chaque ligne          │
│     │   le code    │  Modifier et expérimenter                 │
│     └──────┬───────┘                                            │
│            │                                                    │
│            ▼                                                    │
│     ┌──────────────┐                                            │
│     │  3. FAIRE    │  Sans regarder les solutions              │
│     │ les exercices│  Échouer est normal et utile              │
│     └──────┬───────┘                                            │
│            │                                                    │
│            ▼                                                    │
│     ┌──────────────┐                                            │
│     │ 4. COMPARER  │  Comprendre les différences               │
│     │ aux solutions│  Noter les améliorations                  │
│     └──────┬───────┘                                            │
│            │                                                    │
│            ▼                                                    │
│     ┌──────────────┐                                            │
│     │  5. PASSER   │  Seulement quand tout est clair           │
│     │   au suivant │  Pas de précipitation                     │
│     └──────────────┘                                            │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Règles d'Or

1. **Ne sautez pas de modules** - Même si vous pensez connaître, révisez
2. **Tapez le code vous-même** - Ne copiez-collez pas, même pour les exemples
3. **Échouez d'abord** - Essayez les exercices avant de voir les solutions
4. **Expérimentez** - Modifiez le code pour voir ce qui se passe
5. **Prenez votre temps** - La maîtrise vaut mieux que la vitesse

---

## Configuration de l'Environnement

Voir [ressources/environnement/](ressources/environnement/) pour les guides détaillés.

### Minimum Requis

**Linux** (recommandé pour débuter) :
```bash
# Debian/Ubuntu
sudo apt update
sudo apt install build-essential gdb git

# Arch Linux
sudo pacman -S base-devel gdb git
```

**Windows** (pour les modules Windows) :
- Visual Studio 2022 Community avec "Développement Desktop C++"
- Windows SDK
- Ou : MinGW-w64 + MSYS2

### Environnement de Lab Recommandé

```
┌─────────────────────────────────────────────────────────────┐
│                    SETUP DE LAB RECOMMANDÉ                  │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   Machine Hôte (Linux ou Windows)                           │
│   ├── VM Linux (Debian/Ubuntu)                              │
│   │   └── Développement et tests Linux                      │
│   │                                                         │
│   ├── VM Windows 10/11 (développement)                      │
│   │   └── Visual Studio, outils Windows                     │
│   │                                                         │
│   └── VM Windows 10/11 (cible isolée)                       │
│       └── Tests des outils offensifs                        │
│                                                             │
│   Réseau : NAT ou Host-Only pour isolation                  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## Validation des Compétences

À la fin de chaque phase, vous devriez pouvoir :

### Après Phase 1
- [ ] Expliquer comment un programme devient des instructions machine
- [ ] Lire et écrire des programmes C basiques
- [ ] Débugger avec GDB à un niveau basique

### Après Phase 2
- [ ] Manipuler la mémoire avec les pointeurs
- [ ] Créer des structures de données personnalisées
- [ ] Comprendre et prévenir les buffer overflows

### Après Phase 3
- [ ] Lire et écrire des fichiers binaires
- [ ] Utiliser le préprocesseur efficacement
- [ ] Compiler pour différentes plateformes

### Après Phase 4
- [ ] Écrire des programmes utilisant les syscalls
- [ ] Créer des processus et threads
- [ ] Programmer des communications réseau

### Après Phase 5
- [ ] Injecter du code dans des processus
- [ ] Implémenter des techniques d'évasion
- [ ] Développer des outils offensifs complets

---

## Ressources Complémentaires

Bien que ce repository soit autonome, voici des références pour approfondir :

### Livres
- "The C Programming Language" - Kernighan & Ritchie
- "Expert C Programming" - Peter van der Linden
- "Hacking: The Art of Exploitation" - Jon Erickson

### Documentation
- Man pages Linux (`man function_name`)
- MSDN pour l'API Windows
- Intel x86 Software Developer Manual

---

## Contribution

Ce repository est en développement actif. Les contributions sont bienvenues :
- Corrections et améliorations
- Nouveaux exercices
- Traductions

---

## Licence

Ce contenu est fourni à des fins éducatives uniquement. Utilisez-le de manière responsable et légale.

---

```
                    "Savoir c'est pouvoir, mais avec le pouvoir
                     vient la responsabilité de l'utiliser éthiquement."
```
