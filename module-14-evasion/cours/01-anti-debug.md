# Techniques Anti-Debug Avancées

Ce fichier a été déplacé et fusionné dans `03-anti-analysis.md` qui couvre :
- Anti-Debug
- Anti-Sandbox
- Anti-VM

Voir [03-anti-analysis.md](./03-anti-analysis.md) pour le contenu complet.

## Sommaire des techniques anti-debug

| Technique | Difficulté à contourner | Fiabilité |
|-----------|------------------------|-----------|
| IsDebuggerPresent | ⭐ | ⭐⭐ |
| PEB.BeingDebugged | ⭐⭐ | ⭐⭐⭐ |
| NtGlobalFlag | ⭐⭐ | ⭐⭐ |
| ProcessDebugPort | ⭐⭐⭐ | ⭐⭐⭐ |
| Timing (RDTSC) | ⭐⭐⭐ | ⭐⭐ |
| Hardware breakpoints | ⭐⭐⭐ | ⭐⭐⭐ |
