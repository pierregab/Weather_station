# Projet Station Météo Arduino

## Introduction
Ce projet consiste à développer une station météo en utilisant deux cartes Arduino : une pour les capteurs et une autre pour afficher les données sur un écran LCD. Ce projet a été conçu dans le cadre du cours "Introduction aux systèmes embarqués" à Télécom Physique Strasbourg.

## Fonctionnalités
- Mesure de différentes grandeurs environnementales : température, humidité, pression atmosphérique, luminosité et qualité de l'air.
- Affichage de l'heure, la température, et deux autres grandeurs sélectionnables sur un écran LCD.

## Matériel Utilisé
- 2 cartes Arduino (Microcontrôleur capteur et Microcontrôleur afficheur)
- Capteurs : Température et humidité (DHT22), qualité de l'air (Winsen MP503), pression atmosphérique (HP206C), luminosité (VEML7700)
- Ecran LCD
- Autres composants : boutons, résistances, câbles, etc.

## Installation et Configuration
1. Assembler le matériel selon le schéma de montage fourni.
2. Charger les codes sources sur les deux cartes Arduino (voir les sections ci-dessous).

## Code pour la Carte Capteur
Le code pour la carte capteur gère la collecte de données environnementales. Il utilise divers protocoles de communication (1-wire, I2C, etc.) pour lire les données des capteurs et les envoyer à la carte afficheur.
