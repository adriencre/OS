# 🔐 Générateur de Mots de Passe - Guide d'Utilisation

## Vue d'ensemble
Le générateur de mots de passe de NOVA est un outil de sécurité intégré qui permet de créer des mots de passe sécurisés et d'analyser leur force.

## Commandes de Base

### Génération Simple
```bash
password                    # Génère un mot de passe par défaut (12 caractères)
```

### Options Avancées

#### Longueur Personnalisée
```bash
password -l 16              # Mot de passe de 16 caractères
password -l 8               # Mot de passe de 8 caractères (minimum)
password -l 64              # Mot de passe de 64 caractères (maximum)
```

#### Types de Caractères
```bash
password -t 1               # Seulement minuscules (a-z)
password -t 2               # Seulement majuscules (A-Z)
password -t 3               # Seulement chiffres (0-9)
password -t 4               # Seulement symboles (!@#$%^&*...)
password -t 5               # Minuscules + majuscules
password -t 7               # Minuscules + majuscules + chiffres
password -t 15              # Tous les types (par défaut)
```

#### Génération Multiple
```bash
password -n 5               # Génère 5 mots de passe
password -n 10 -l 20        # 10 mots de passe de 20 caractères
```

#### Exclusion de Caractères
```bash
password -s                 # Exclut les caractères similaires (0,O,l,1,|)
password -a                 # Exclut les caractères ambigus ({},[],(),...)
password -s -a              # Exclut les deux types
```

#### Analyse de Force
```bash
password -c MonMotDePasse   # Analyse la force d'un mot de passe existant
```

#### Aide
```bash
password help               # Affiche l'aide complète
password -h                 # Affiche l'aide complète
```

## Exemples Pratiques

### Mots de Passe pour Comptes Web
```bash
password -l 16 -t 15 -s     # 16 caractères, tous types, sans caractères similaires
```

### Codes PIN Sécurisés
```bash
password -l 8 -t 3          # 8 chiffres pour un code PIN
```

### Mots de Passe pour Applications
```bash
password -n 3 -l 12 -t 7    # 3 mots de passe de 12 caractères (lettres + chiffres)
```

### Mots de Passe pour Systèmes Sensibles
```bash
password -l 32 -t 15 -s -a  # 32 caractères, tous types, sans caractères problématiques
```

## Critères de Force

Le générateur analyse automatiquement la force des mots de passe selon :

- **Longueur** : Minimum 8 caractères recommandé
- **Variété** : Au moins 3 types de caractères différents
- **Complexité** : Éviter les patterns prévisibles

### Niveaux de Force
- **FAIBLE** : < 8 caractères ou < 3 types de caractères
- **FORT** : ≥ 8 caractères et ≥ 3 types de caractères

## Conseils de Sécurité

1. **Longueur** : Privilégiez des mots de passe d'au moins 12 caractères
2. **Variété** : Utilisez tous les types de caractères disponibles
3. **Unicité** : Un mot de passe unique par compte/service
4. **Stockage** : Ne stockez jamais vos mots de passe en clair
5. **Renouvellement** : Changez régulièrement vos mots de passe sensibles

## Intégration avec NOVA

Le générateur de mots de passe s'intègre parfaitement avec :
- **Système de santé** : Protection des dossiers médicaux
- **Gestionnaire de fichiers** : Sécurisation des documents
- **Éditeur de texte** : Protection des fichiers sensibles

## Fonctionnalités Techniques

- **Générateur pseudo-aléatoire** : Basé sur le timer système
- **Mélange Fisher-Yates** : Pour une distribution uniforme
- **Analyse en temps réel** : Évaluation immédiate de la force
- **Interface ASCII** : Compatible avec le terminal VGA

---

*Générateur de mots de passe NOVA v1.0 - Système d'exploitation sécurisé*
