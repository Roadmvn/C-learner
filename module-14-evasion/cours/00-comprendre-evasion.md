# Comprendre l'Évasion EDR - Le Contexte

## Pourquoi l'évasion existe ?

Avant de voir les syscalls et l'unhooking, il faut comprendre **contre quoi on se bat**.

---

## Comment un EDR fonctionne

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    ARCHITECTURE D'UN EDR                                │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   Quand votre malware veut faire quelque chose (allouer mémoire,       │
│   créer un thread, etc.), il doit passer par le système :              │
│                                                                         │
│   ┌─────────────────────────────────────────────────────────────────┐  │
│   │                     VOTRE MALWARE                                │  │
│   │                          │                                       │  │
│   │                          ▼                                       │  │
│   │                  VirtualAlloc()    ← API Win32 (kernel32.dll)    │  │
│   │                          │                                       │  │
│   │                          ▼                                       │  │
│   │            NtAllocateVirtualMemory() ← API native (ntdll.dll)    │  │
│   │                          │                                       │  │
│   │                    ┌─────┴─────┐                                 │  │
│   │                    │   HOOK    │  ← L'EDR s'insère ICI !         │  │
│   │                    │    EDR    │                                 │  │
│   │                    └─────┬─────┘                                 │  │
│   │                          │                                       │  │
│   │               ┌──────────┴──────────┐                            │  │
│   │               │  L'EDR inspecte :   │                            │  │
│   │               │  - Qui appelle ?    │                            │  │
│   │               │  - Quels arguments ?│                            │  │
│   │               │  - Est-ce suspect ? │                            │  │
│   │               └──────────┬──────────┘                            │  │
│   │                          │                                       │  │
│   │                          ▼                                       │  │
│   │                      syscall        ← Appel au kernel            │  │
│   │                          │                                       │  │
│   │                          ▼                                       │  │
│   │                   KERNEL WINDOWS                                 │  │
│   └─────────────────────────────────────────────────────────────────┘  │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### Que surveille un EDR ?

| Ce que fait votre code | Ce que l'EDR voit | Verdict |
|------------------------|-------------------|---------|
| `VirtualAlloc(PAGE_EXECUTE_READWRITE)` | Allocation mémoire exécutable | ⚠️ Suspect |
| `WriteProcessMemory(autre_processus)` | Écriture inter-processus | 🚨 Très suspect |
| `CreateRemoteThread()` | Création thread distant | 🚨 Alerte ! |
| `SetWindowsHookEx()` | Hook global | ⚠️ Suspect |
| Séquence : Alloc → Write → Execute | Pattern d'injection | 🚨 Blocage |

---

## Comment un Hook EDR fonctionne

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    ANATOMIE D'UN HOOK                                   │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   AVANT (ntdll.dll normale) :                                          │
│   ┌─────────────────────────────────────────────────────────────────┐  │
│   │  NtAllocateVirtualMemory:                                        │  │
│   │      4C 8B D1          mov r10, rcx      ; Setup syscall        │  │
│   │      B8 18 00 00 00    mov eax, 0x18     ; Syscall number       │  │
│   │      0F 05             syscall           ; Call kernel          │  │
│   │      C3                ret                                       │  │
│   └─────────────────────────────────────────────────────────────────┘  │
│                                                                         │
│   APRÈS (ntdll.dll hookée par EDR) :                                   │
│   ┌─────────────────────────────────────────────────────────────────┐  │
│   │  NtAllocateVirtualMemory:                                        │  │
│   │      E9 XX XX XX XX    jmp EDR_Hook      ; REDIRIGE vers EDR !  │  │
│   │      90 90 90 90       nop nop nop nop   ; Padding              │  │
│   │      0F 05             syscall                                   │  │
│   │      C3                ret                                       │  │
│   └─────────────────────────────────────────────────────────────────┘  │
│                                                                         │
│   Quand votre code appelle NtAllocateVirtualMemory :                   │
│                                                                         │
│   1. La première instruction est maintenant JMP EDR_Hook               │
│   2. Le code de l'EDR s'exécute                                        │
│   3. L'EDR analyse les arguments, le call stack, etc.                  │
│   4. Si OK → l'EDR exécute le vrai syscall et retourne                │
│   5. Si suspect → l'EDR bloque ou alerte                               │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Le Problème à Résoudre

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    LE DILEMME DE L'ATTAQUANT                            │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   Votre malware a besoin de :                                          │
│   • Allouer de la mémoire exécutable (pour le shellcode)               │
│   • Écrire dans d'autres processus (pour l'injection)                  │
│   • Créer des threads (pour exécuter le code)                          │
│                                                                         │
│   MAIS toutes ces opérations passent par ntdll.dll                     │
│   ET l'EDR a hooké ntdll.dll                                           │
│                                                                         │
│   ┌───────────────────────────────────────────────────────────────┐    │
│   │                                                               │    │
│   │   Votre code ──► ntdll.dll ──► EDR HOOK ──► Détection !      │    │
│   │                                                               │    │
│   └───────────────────────────────────────────────────────────────┘    │
│                                                                         │
│   Question : Comment atteindre le kernel SANS passer par les hooks ?   │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Les Solutions (et pourquoi elles marchent)

### Solution 1 : Direct Syscalls

**L'idée :** "Si l'EDR hook ntdll.dll, je n'utilise pas ntdll.dll !"

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    DIRECT SYSCALL                                       │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   Normalement :                                                         │
│   Malware → kernel32.dll → ntdll.dll → [HOOK] → syscall → Kernel       │
│                                                                         │
│   Avec Direct Syscall :                                                 │
│   Malware → syscall → Kernel                                            │
│                                                                         │
│   On écrit nous-mêmes l'instruction syscall dans notre code !          │
│                                                                         │
│   mov r10, rcx           ; Argument 1                                   │
│   mov eax, 0x18          ; Numéro du syscall                           │
│   syscall                ; Appel direct au kernel                       │
│   ret                    ; Retour                                       │
│                                                                         │
│   L'EDR ne voit RIEN car on ne passe jamais par ntdll.dll              │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

**Pourquoi ça marche ?**
- L'EDR ne peut hooker que du code qui existe
- Si on exécute notre propre instruction `syscall`, l'EDR ne peut pas l'intercepter en usermode
- Le kernel ne sait pas d'où vient l'appel

**Limite :** Les numéros de syscall changent selon les versions de Windows (0x18 sur Win10 peut être 0x15 sur Win7)

---

### Solution 2 : NTDLL Unhooking

**L'idée :** "Si l'EDR a modifié ntdll.dll, je restaure l'original !"

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    NTDLL UNHOOKING                                      │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   État initial après démarrage du processus :                          │
│                                                                         │
│   ntdll.dll en mémoire          ntdll.dll sur disque                   │
│   ┌────────────────────┐        ┌────────────────────┐                 │
│   │ JMP EDR_Hook       │        │ mov r10, rcx       │ ← Original !    │
│   │ nop nop nop        │   VS   │ mov eax, 0x18      │                 │
│   │ syscall            │        │ syscall            │                 │
│   └────────────────────┘        └────────────────────┘                 │
│           ▲                              │                              │
│           │                              │                              │
│           └──────── COPIER ◄─────────────┘                              │
│                                                                         │
│   On lit ntdll.dll depuis le disque (version non hookée)               │
│   On écrase la section .text en mémoire avec la version propre         │
│   Maintenant ntdll.dll fonctionne normalement, sans hooks !            │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

**Pourquoi ça marche ?**
- Le fichier sur disque n'est pas modifié par l'EDR
- L'EDR modifie uniquement la copie en mémoire au chargement du processus
- On peut "réparer" la mémoire en la remplaçant par l'original

**Limite :** L'EDR peut détecter la lecture de ntdll.dll depuis le disque

---

### Solution 3 : Indirect Syscalls

**L'idée :** "Les EDR modernes détectent les syscalls depuis du code suspect. Je vais exécuter le syscall depuis ntdll.dll elle-même !"

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    INDIRECT SYSCALL                                     │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   Problème des Direct Syscalls :                                        │
│   L'instruction "syscall" s'exécute depuis notre malware.exe           │
│   → L'EDR peut vérifier l'adresse de retour : "D'où vient ce syscall?"│
│   → Si ce n'est pas depuis ntdll.dll, c'est suspect !                  │
│                                                                         │
│   Solution :                                                            │
│   1. On prépare les arguments (mov r10, rcx; mov eax, SSN)             │
│   2. Au lieu d'exécuter syscall nous-mêmes...                          │
│   3. On SAUTE vers l'instruction syscall dans ntdll.dll !              │
│                                                                         │
│   Notre code:                                                           │
│       mov r10, rcx                                                      │
│       mov eax, 0x18                                                     │
│       jmp [adresse_syscall_dans_ntdll]  ← On saute dans ntdll !        │
│                                                                         │
│   ntdll.dll:                                                            │
│       ...                                                               │
│       syscall    ← Le syscall s'exécute depuis ntdll (légitime !)      │
│       ret                                                               │
│                                                                         │
│   L'adresse de retour pointe vers ntdll.dll → L'EDR est satisfait      │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Pourquoi les techniques évoluent ?

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    ÉVOLUTION DES TECHNIQUES                             │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   2015 : DLL Injection classique                                        │
│   ─────────────────────────────                                         │
│   Attaque : CreateRemoteThread + LoadLibrary                           │
│   Défense : Hook sur CreateRemoteThread                                │
│   Contournement : Utiliser NtCreateThreadEx                            │
│                                                                         │
│   2017 : Utilisation des API natives                                    │
│   ───────────────────────────────                                       │
│   Attaque : Appeler directement ntdll.dll                              │
│   Défense : Hook sur ntdll.dll (toutes les fonctions Nt*)              │
│   Contournement : Direct syscalls                                       │
│                                                                         │
│   2019 : Direct Syscalls (Hell's Gate)                                  │
│   ─────────────────────────────────────                                 │
│   Attaque : Exécuter syscall sans passer par ntdll                     │
│   Défense : Vérifier l'adresse de retour du syscall                    │
│   Contournement : Indirect syscalls                                     │
│                                                                         │
│   2021 : Indirect Syscalls + Unhooking                                  │
│   ──────────────────────────────────                                    │
│   Attaque : Combiner plusieurs techniques                              │
│   Défense : Kernel callbacks, ETW, hardware telemetry                  │
│   Contournement : ... la course continue !                             │
│                                                                         │
│   LEÇON : Chaque nouvelle défense crée une nouvelle technique d'attaque │
│           Comprendre les deux côtés est essentiel                       │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Ce que tu vas apprendre

| Cours | Technique | Pourquoi l'apprendre |
|-------|-----------|---------------------|
| 01 | Direct Syscalls | Contourner les hooks usermode |
| 02 | NTDLL Unhooking | Restaurer les fonctions originales |
| 03 | Anti-Analysis | Détecter les environnements d'analyse |

Chaque cours explique :
1. **Le problème** à résoudre
2. **Pourquoi** cette solution fonctionne
3. **Le code** avec explications détaillées
4. **Les limites** et comment les défenseurs détectent

---

## Pour les Défenseurs

Si tu travailles côté Blue Team, ce module t'apprend :

- **Quoi surveiller** : Lectures suspectes de ntdll.dll, syscalls depuis des régions inhabituelles
- **Quels indicateurs** : Modifications de la mémoire de ntdll, absence d'appels ntdll dans les logs
- **Quelles protections** : Kernel callbacks, ETW, analyse comportementale

La meilleure défense commence par comprendre l'attaque.
