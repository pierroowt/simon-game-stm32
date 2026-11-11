# Simon Game sur STM32

Jeu de mémoire « Simon » sur carte STM32 Nucleo L476RG avec écran tactile TFT, programmé en C++. Projet de 2e année de BUT GEII (IUT de Toulouse), réalisé en binôme.

## Principe du jeu

Le jeu montre une suite de couleurs en faisant clignoter quatre boutons à l'écran. Le joueur la reproduit en touchant l'écran, ou depuis l'application RemoteXY sur son téléphone. Chaque suite réussie rapporte un point et ajoute une couleur. Une erreur coûte un cœur sur trois et la suite est rejouée ; à zéro cœur, la partie recommence.

## Matériel

| Élément | Rôle |
|---|---|
| STM32 Nucleo L476RG | Microcontrôleur |
| Écran TFT 320 × 240, contrôleur ILI9341 | Affichage, piloté en SPI |
| Dalle tactile, contrôleur XPT2046 | Saisie, sur le même bus SPI |
| Liaison série UART (PC10/PC11, 9600 bauds) | Communication avec l'application RemoteXY |

Câblage de l'écran : CS sur D10, DC sur D8, RST sur D9, CS du tactile sur D7.

## Points techniques

- Bus SPI partagé entre l'écran et la dalle tactile, cadencé à 8 MHz : à 40 MHz, l'écran restait blanc avec notre câblage.
- Conversion des coordonnées brutes de la dalle tactile en pixels pour savoir quel bouton est touché.
- Attente du relâchement avant de lire un nouvel appui, pour ne pas compter deux fois le même.
- Graine aléatoire lue sur une entrée analogique non connectée, pour obtenir une suite différente à chaque démarrage.

## Compiler et téléverser

Le projet utilise PlatformIO avec le framework Arduino. Ouvrir le dossier dans VS Code avec l'extension PlatformIO, puis lancer Build et Upload avec la carte branchée en USB. Les bibliothèques (Adafruit ILI9341, Adafruit GFX, XPT2046_Touchscreen, RemoteXY) sont installées automatiquement.

## Auteurs

Pierre Gazeau et Victor Gonzaga, BUT2 GEII, IUT de Toulouse, 2026. Le cadre du projet PlatformIO a été fourni par l'équipe enseignante.
