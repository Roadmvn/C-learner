# Module 12 : Développement de Shellcode

## Objectifs

Apprendre à développer du shellcode from scratch :
- Comprendre les contraintes du shellcode
- Trouver les APIs Windows dynamiquement (PEB walking)
- Écrire du code position-independent
- Techniques d'encodage et d'obfuscation

## Cours

| # | Titre | Description |
|---|-------|-------------|
| 01 | [Fondamentaux du Shellcode](cours/01-fondamentaux.md) | Contraintes, structure, compilation |
| 02 | [PEB Walking](cours/02-peb-walking.md) | Résolution dynamique des APIs |
| 03 | [Shellcode Classiques](cours/03-shellcodes-classiques.md) | MessageBox, Reverse Shell, Download & Execute |

## Prérequis

- Module 05 : Pointeurs
- Module 10 : Programmation système Linux (optionnel)
- Module 11 : Programmation système Windows
- Bases en assembleur x86/x64

## Outils nécessaires

- NASM ou MASM (assembleur)
- MinGW ou Visual Studio
- x64dbg pour le debug
- Python pour l'extraction du shellcode
