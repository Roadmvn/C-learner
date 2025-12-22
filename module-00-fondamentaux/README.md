# Module 00 : Fondamentaux Informatiques

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│   "Avant de programmer, il faut comprendre la machine."                     │
│                                                                             │
│   Ce module pose les bases absolues. Sans ces connaissances,                │
│   vous ne ferez que répéter des incantations magiques.                      │
│   Avec elles, vous comprendrez réellement ce qui se passe.                  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Objectifs du Module

À la fin de ce module, vous serez capable de :

- [ ] Convertir entre binaire, décimal et hexadécimal
- [ ] Expliquer comment un processeur exécute des instructions
- [ ] Décrire l'organisation de la mémoire (stack, heap, etc.)
- [ ] Comprendre le rôle du système d'exploitation
- [ ] Lire et interpréter des dumps mémoire en hexadécimal

## Pourquoi c'est Crucial pour la Sécurité Offensive

```
┌─────────────────────────────────────────────────────────────────┐
│                    APPLICATIONS DIRECTES                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  BINAIRE/HEXA        →   Lecture de shellcode, patches         │
│  CPU & REGISTRES     →   Reverse engineering, ROP chains       │
│  MÉMOIRE             →   Buffer overflow, injection            │
│  SYSTÈME D'EXPLOIT.  →   Élévation de privilèges              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

Chaque technique offensive repose sur une compréhension profonde de ces concepts.

## Plan du Module

### Cours Théoriques

| # | Cours | Durée | Description |
|---|-------|-------|-------------|
| 01 | [Systèmes de Numération](cours/01-systemes-numeration.md) | 45 min | Binaire, décimal, hexadécimal |
| 02 | [Le Processeur (CPU)](cours/02-processeur.md) | 60 min | Architecture, registres, cycle d'exécution |
| 03 | [La Mémoire](cours/03-memoire.md) | 60 min | RAM, stack, heap, segments |
| 04 | [Le Système d'Exploitation](cours/04-systeme-exploitation.md) | 45 min | Rôle, processus, appels système |
| 05 | [Du Code Source à l'Exécution](cours/05-code-a-execution.md) | 30 min | Compilation, chargement, exécution |

### Exercices Pratiques

| # | Exercice | Difficulté | Concepts |
|---|----------|------------|----------|
| 01 | [Conversions Numériques](exercices/exercice-01.md) | ★☆☆ | Binaire, hexa, décimal |
| 02 | [Lecture Hexadécimale](exercices/exercice-02.md) | ★★☆ | Interprétation de dumps |
| 03 | [Analyse de Stack](exercices/exercice-03.md) | ★★☆ | Comprendre la pile |
| 04 | [Quiz Complet](exercices/exercice-04.md) | ★★★ | Tous les concepts |

## Prérequis

**Aucun.** Ce module est le point de départ absolu.

## Temps Estimé

- Cours : ~4 heures
- Exercices : ~2 heures
- **Total : ~6 heures**

## Conseils

1. **Prenez des notes manuscrites** - Écrire aide à mémoriser
2. **Faites les conversions à la main** - Pas de calculatrice au début
3. **Dessinez les schémas vous-même** - La visualisation est clé
4. **Relisez si nécessaire** - Ces concepts sont fondamentaux

## Navigation

| Précédent | Suivant |
|-----------|---------|
| - | [Module 01 : Premier Contact](../module-01-premier-contact/) |

---

```
   ┌────────────────────────────────────────────┐
   │  Prêt ? Commençons par les nombres...     │
   │  → cours/01-systemes-numeration.md        │
   └────────────────────────────────────────────┘
```
