# Guide des Fonctionnalités Visuelles - MyOS

## Nouveau Logo de Démarrage

Au démarrage de votre OS, vous verrez maintenant un logo ASCII stylisé :

```
  __  __       ___  ____  
 |  \/  |_   _/ _ \/ ___| 
 | |\/| | | | | | \___ \ 
 | |  | | |_| | |_| |__) |
 |_|  |_|\__, |\___/____/ 
         |___/            

    ┌─────────────────────────┐
    │   Systeme d'Exploitation │
    │      Color Edition      │
    └─────────────────────────┘
```

## Nouvelles Commandes

### `charset` - Explorer les caractères disponibles

Cette commande affiche tous les caractères utilisables pour créer des motifs :
- Caractères ASCII de base
- Caractères étendus pour faire des motifs (blocs, lignes, etc.)

### `background <pattern>` - Motifs de fond d'écran

Applique différents motifs de fond à votre écran :

#### Motifs disponibles :

1. **`background clear`** - Efface le fond
2. **`background dots`** - Motif de points discrets
3. **`background lines`** - Lignes horizontales subtiles  
4. **`background grid`** - Grille complète avec intersections

#### Exemples d'utilisation :

```bash
> background dots     # Applique un motif de points
> background grid     # Applique une grille
> background clear    # Revient à un fond vide
```

## Comment ça marche

En mode texte VGA (80x25), nous ne pouvons pas afficher de vraies images, mais nous pouvons :

1. **Utiliser des caractères étendus** comme motifs de fond
2. **Jouer avec les couleurs** pour créer des effets visuels
3. **Superposer du texte** par-dessus les motifs

## Caractères spéciaux utiles

- `░ ▒ ▓ █` : Différents niveaux de gris/blocs
- `─ │ ┌ ┐ └ ┘ ┼` : Lignes et bordures
- `∙ · ■` : Points et formes géométriques

## Conseils d'utilisation

1. Les motifs restent en arrière-plan - vous pouvez taper dessus
2. Utilisez `clear` pour revenir à un écran propre
3. Les motifs sont préservés même quand vous changez de couleur avec `color`
4. Combine les motifs avec différentes couleurs pour plus d'effet !

## Évolution future

Pour avoir de vraies images, il faudrait :
- Passer en mode graphique VGA/VESA
- Implémenter un pilote graphique
- Gérer les formats d'images (BMP, PPM, etc.)

Mais pour l'instant, ces motifs ASCII donnent déjà un style unique à votre OS !