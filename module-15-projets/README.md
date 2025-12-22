# Module 15 : Projets Intégrateurs

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│   Ce module final met en pratique tous les concepts appris.                │
│   Chaque projet est un outil complet que vous implémenterez.               │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Projets

### Projet 1 : Keylogger Simple
**Difficulté** : ★★☆☆☆

Implémenter un keylogger basique qui :
- Capture les frappes clavier (Linux ou Windows)
- Les écrit dans un fichier log
- Fonctionne en arrière-plan

**Concepts utilisés** : Fichiers, hooks clavier, processus

### Projet 2 : Reverse Shell
**Difficulté** : ★★★☆☆

Créer un reverse shell multi-plateforme :
- Connexion TCP vers un serveur d'écoute
- Exécution de commandes
- Transfert de fichiers basique

**Concepts utilisés** : Sockets, processus, redirection I/O

### Projet 3 : Injecteur de DLL
**Difficulté** : ★★★☆☆

Développer un injecteur qui :
- Énumère les processus
- Injecte une DLL choisie
- Gère les erreurs proprement

**Concepts utilisés** : API Windows, mémoire, threads

### Projet 4 : Scanner de Ports
**Difficulté** : ★★☆☆☆

Construire un scanner de ports TCP :
- Scan d'une plage de ports
- Multithreading pour la vitesse
- Affichage des services connus

**Concepts utilisés** : Sockets, threads, réseau

### Projet 5 : Crypter Simple
**Difficulté** : ★★★★☆

Créer un crypter/packer basique :
- Chiffre un exécutable avec XOR ou RC4
- Génère un stub qui déchiffre et exécute
- Évite les signatures basiques

**Concepts utilisés** : Crypto, manipulation binaire, PE format

### Projet 6 : Mini RAT
**Difficulté** : ★★★★★

Développer un outil d'administration distant minimal :
- Serveur C2 en Python ou C
- Agent en C
- Commandes : shell, upload, download, screenshot

**Concepts utilisés** : Tout le curriculum!

## Guide de Développement

1. **Planifier** : Dessiner l'architecture avant de coder
2. **Modulariser** : Séparer en fonctions/fichiers
3. **Tester** : Tester chaque composant
4. **Documenter** : Commenter le code
5. **Sécuriser** : Valider les entrées, gérer les erreurs

## Structure Recommandée

```
projet/
├── src/
│   ├── main.c
│   ├── network.c
│   └── utils.c
├── include/
│   ├── network.h
│   └── utils.h
├── Makefile
└── README.md
```

## Navigation
| Précédent | Accueil |
|-----------|---------|
| [Module 14](../module-14-evasion/) | [README Principal](../README.md) |
