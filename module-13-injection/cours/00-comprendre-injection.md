# Comprendre les Techniques d'Injection - Le Contexte

## Pourquoi l'injection de code existe ?

Avant de voir le code, il faut comprendre **le problème que les attaquants cherchent à résoudre**.

---

## Le Modèle de Sécurité Windows

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    COMMENT WINDOWS VOIT LES PROCESSUS                   │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   Chaque processus est une "boîte isolée" :                            │
│                                                                         │
│   ┌─────────────┐    ┌─────────────┐    ┌─────────────┐                │
│   │ notepad.exe │    │ chrome.exe  │    │ svchost.exe │                │
│   │             │    │             │    │             │                │
│   │ Mémoire     │    │ Mémoire     │    │ Mémoire     │                │
│   │ privée      │    │ privée      │    │ privée      │                │
│   │             │    │             │    │             │                │
│   │ Ses DLLs    │    │ Ses DLLs    │    │ Ses DLLs    │                │
│   │ Ses threads │    │ Ses threads │    │ Ses threads │                │
│   └─────────────┘    └─────────────┘    └─────────────┘                │
│         │                  │                  │                         │
│         └──────────────────┼──────────────────┘                         │
│                            │                                            │
│                     ┌──────┴──────┐                                     │
│                     │   KERNEL    │  ← Seul le kernel peut              │
│                     │   WINDOWS   │    voir/modifier tous               │
│                     └─────────────┘    les processus                    │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### Pourquoi cette isolation ?
- **Stabilité** : Un crash de Chrome ne tue pas Word
- **Sécurité** : Un malware dans Chrome ne peut pas lire les mots de passe de Firefox
- **Permissions** : Chaque processus a ses propres droits

---

## Le Problème de l'Attaquant

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    LE DILEMME DU MALWARE                                │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   SCÉNARIO : Vous avez réussi à faire exécuter votre malware.exe       │
│                                                                         │
│   Problème 1 : VISIBILITÉ                                              │
│   ┌─────────────────────────────────────────────────────────────────┐  │
│   │ Gestionnaire des tâches :                                        │  │
│   │                                                                  │  │
│   │   Nom                    PID    CPU    Mémoire                   │  │
│   │   ─────────────────────────────────────────────────              │  │
│   │   System                 4      0%     0.1 MB                    │  │
│   │   svchost.exe           856     1%     15 MB                     │  │
│   │   explorer.exe          1234    2%     50 MB                     │  │
│   │   chrome.exe            5678    10%    200 MB                    │  │
│   │   ⚠️ malware.exe         9999    5%     10 MB  ← SUSPECT !       │  │
│   │                                                                  │  │
│   └─────────────────────────────────────────────────────────────────┘  │
│                                                                         │
│   → Un utilisateur ou un EDR voit "malware.exe" et s'inquiète          │
│   → Le nom du fichier, le chemin, la signature sont analysés           │
│                                                                         │
│   ─────────────────────────────────────────────────────────────────    │
│                                                                         │
│   Problème 2 : PERMISSIONS                                              │
│                                                                         │
│   Votre malware.exe veut :                                              │
│   • Lire les mots de passe de Chrome → Besoin d'accéder à Chrome       │
│   • Intercepter les frappes clavier → Besoin de hooks globaux          │
│   • Survivre au redémarrage → Besoin de droits admin                   │
│   • Se cacher des EDR → Besoin de manipuler la mémoire système         │
│                                                                         │
│   → Mais malware.exe est un processus SÉPARÉ                           │
│   → Il ne peut pas facilement accéder aux autres processus              │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## La Solution : L'Injection

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    L'IDÉE DE L'INJECTION                                │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   "Et si au lieu de créer mon propre processus visible,                │
│    je mettais mon code DANS un processus déjà existant ?"              │
│                                                                         │
│   AVANT (malware classique) :                                           │
│   ┌─────────────┐    ┌─────────────┐                                   │
│   │ notepad.exe │    │ malware.exe │ ← Visible, suspect                │
│   │  (légitime) │    │  (malware)  │                                   │
│   └─────────────┘    └─────────────┘                                   │
│                                                                         │
│   APRÈS (injection) :                                                   │
│   ┌─────────────────────────────┐                                      │
│   │       notepad.exe           │                                      │
│   │  ┌───────────────────────┐  │                                      │
│   │  │ Code légitime notepad │  │                                      │
│   │  ├───────────────────────┤  │                                      │
│   │  │ 💀 NOTRE CODE MALWARE │  │ ← Invisible ! Même PID, même nom    │
│   │  └───────────────────────┘  │                                      │
│   └─────────────────────────────┘                                      │
│                                                                         │
│   Avantages :                                                           │
│   ✓ Pas de nouveau processus visible                                   │
│   ✓ Hérite des permissions du processus hôte                           │
│   ✓ Plus difficile à détecter                                          │
│   ✓ Peut accéder aux données du processus hôte                         │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Pourquoi plusieurs techniques ?

Chaque technique répond à un **besoin différent** :

### 1. DLL Injection (Classique)
**Quand l'utiliser ?**
- Vous voulez exécuter du code dans un processus existant
- Vous avez une DLL sur le disque
- Vous ne vous souciez pas trop de la furtivité

**Comment ça marche ?**
```
Votre malware → Demande au processus cible de charger votre DLL
            → LoadLibrary("malware.dll")
            → DllMain() s'exécute dans le contexte du processus
```

**Pourquoi pas toujours ?**
- Laisse une DLL sur le disque (détectable)
- Appelle LoadLibrary (monitoré par les EDR)
- La DLL apparaît dans la liste des modules du processus

---

### 2. Process Hollowing
**Quand l'utiliser ?**
- Vous voulez un contrôle TOTAL sur le processus
- Vous ne voulez pas que le code original s'exécute
- Vous voulez ressembler parfaitement à un processus légitime

**Comment ça marche ?**
```
Créer notepad.exe (suspendu)
    → Vider son contenu mémoire
    → Remplacer par votre malware
    → Reprendre l'exécution

Résultat : "notepad.exe" mais c'est votre code qui tourne
```

**Pourquoi pas toujours ?**
- Plus complexe à implémenter
- Le processus suspendu peut être détecté
- Les protections mémoire modernes compliquent les choses

---

### 3. APC Injection
**Quand l'utiliser ?**
- Vous ne voulez pas créer de nouveau thread (plus furtif)
- Le processus cible a des threads "alertable"

**Comment ça marche ?**
```
Les threads Windows ont une "file d'attente APC"
    → Vous y mettez votre code
    → Quand le thread se "réveille", il exécute votre code
    → Pas de CreateRemoteThread (moins surveillé)
```

**Pourquoi pas toujours ?**
- Nécessite un thread en état "alertable"
- Moins fiable que CreateRemoteThread

---

### 4. Shellcode Injection
**Quand l'utiliser ?**
- Vous n'avez pas de fichier sur le disque
- Vous voulez le maximum de furtivité
- Vous avez du code autonome (shellcode)

**Comment ça marche ?**
```
Allouer mémoire dans le processus cible
    → Écrire votre shellcode (pas de DLL, juste du code brut)
    → Exécuter ce code

Avantage : Rien sur le disque !
```

---

## Le Jeu du Chat et de la Souris

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    ÉVOLUTION ATTAQUE/DÉFENSE                            │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   1990s : Virus simples                                                 │
│   ────────────────────                                                  │
│   Attaque : Malware.exe s'exécute directement                          │
│   Défense : Antivirus scanne les fichiers sur disque                   │
│                                                                         │
│   2000s : DLL Injection                                                 │
│   ─────────────────────                                                 │
│   Attaque : Injecter dans les processus légitimes                      │
│   Défense : Surveiller CreateRemoteThread, LoadLibrary                 │
│                                                                         │
│   2010s : Process Hollowing, Reflective DLL                            │
│   ─────────────────────────────────────────────                        │
│   Attaque : Éviter le disque, modifier les processus                   │
│   Défense : Analyse mémoire, hooks ntdll, EDR                          │
│                                                                         │
│   2020s : Direct Syscalls, Unhooking                                   │
│   ───────────────────────────────────                                  │
│   Attaque : Contourner les hooks EDR                                   │
│   Défense : Kernel callbacks, ETW, hardware telemetry                  │
│                                                                         │
│   → C'est une course permanente !                                       │
│   → Comprendre les techniques = comprendre comment se défendre         │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Pourquoi apprendre ça ?

### Si tu veux faire du Red Team / Pentest
- Tu dois comprendre comment fonctionnent les vrais malwares
- Tu dois savoir contourner les protections pour tester leur efficacité
- Tu dois pouvoir écrire des outils personnalisés

### Si tu veux faire du Blue Team / Défense
- Tu dois comprendre ce que tu cherches à détecter
- Tu dois savoir quelles APIs surveiller
- Tu dois reconnaître les patterns d'attaque

### Si tu veux faire de la Recherche en Sécurité
- Tu dois pouvoir analyser les malwares
- Tu dois comprendre les techniques pour les documenter
- Tu dois pouvoir créer des PoC pour les vulnérabilités

---

## Prochaines étapes

Maintenant que tu comprends **pourquoi** ces techniques existent, on peut voir **comment** elles fonctionnent :

1. [Process Hollowing](./01-process-hollowing.md) - Prendre le contrôle total d'un processus
2. [DLL Injection](./02-dll-injection.md) - Charger notre code dans un processus
3. [Shellcode Injection](./03-shellcode-injection.md) - Injection sans fichier

Chaque technique sera expliquée avec :
- Le **problème** qu'elle résout
- Le **pourquoi** de chaque étape
- Le **code** commenté ligne par ligne
- Les **limites** et détections possibles
