# SPI_ADXL345 Project

Ce projet utilise un microcontrôleur STM32F103 pour communiquer avec un capteur ADXL345 via le protocole SPI. Le but est de lire les données de l'accéléromètre et de les traiter pour diverses applications.

Tous les projets de ce référentiel utilisent le dossier libs comme source de code, je vous suggère de copier les bibliothèques souhaitées dans le dossier de travail.

## Structure du projet
```
└── 📁SPI_ADXL345
    └── adxl345.h et adxl345.c
    └── general.h et general.c
    └── spi.h et spi.c
    └── uart.h et uart.c
    └── main.c
    └── process.pde
    
```

## Utilisation

1. Initialiser le microcontrôleur et le capteur ADXL345 en utilisant les fonctions définies dans `main.c`.
2. Lire les données de l'accéléromètre en appelant les fonctions appropriées depuis `adxl345.h`.
3. Traiter les données selon les besoins de l'application, en utilisant les utilitaires disponibles dans `general.h` et les fonctions de communication définies dans `uart.h` pour le débogage et la sortie des résultats.

## Dépendances

- **adxl345.h** : Contient les définitions et les fonctions nécessaires pour interfacer le capteur ADXL345.
- **general.h** : Inclut des définitions générales et des fonctions utilitaires utilisées dans le projet.
- **spi.h** : Fournit les fonctions pour configurer et utiliser le protocole SPI avec le microcontrôleur STM32F103.
- **uart.h** : Contient les fonctions pour la communication UART, utile pour le débogage et la communication série.

## Auteur

Ce projet a été développé par **Dan LARA**.
