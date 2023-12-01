#include <SPI.h> // Inclure la bibliothèque SPI nécessaire pour la communication avec l'écran LCD.

// La fonction setup() est exécutée une seule fois au démarrage du microcontrôleur.
void setup() {
  Serial.begin(9600); // Initialiser la communication série à 9600 bits par seconde.
  SPI.begin(); // Initialiser la communication SPI.
  setupLCD(); // Configurer l'écran LCD avec les paramètres initiaux.
  
  // Définir le texte à afficher sur l'écran LCD.
  setLCDposition(2,5); // Positionner le curseur à la ligne 2, colonne 5.
  writeString("May the force"); // Écrire la première partie du message.
  
  setLCDposition(3,7); // Positionner le curseur à la ligne 3, colonne 7.
  writeString("be with you..."); // Écrire la seconde partie du message.
  
  delay(5000); // Attendre 5 secondes.
}

// La fonction loop() est exécutée en boucle indéfiniment après la fonction setup().
void loop() {
  // Aucun code n'est nécessaire ici.
}

// Envoyer une commande à l'écran LCD en suivant le protocole SPI.
void sendLCDCommand(byte cmd) {
  SPI.transfer(0b11111000); // Envoyer l'octet indiquant une commande.

  byte b = cmd & 0b00001111; // Isoler les 4 bits de poids faible de la commande.
  b = inverse(b); // Inverser les bits (selon le protocole de l'écran LCD).
  b = b << 4; // Décaler les bits à la position correcte pour la transmission.
  SPI.transfer(b); // Envoyer les bits de poids faible.

  byte a = cmd & 0b11110000; // Isoler les 4 bits de poids fort de la commande.
  a = a >> 4; // Décaler les bits à la position correcte.
  a = inverse(a); // Inverser les bits.
  a = a << 4; // Décaler les bits à la position correcte pour la transmission.
  SPI.transfer(a); // Envoyer les bits de poids fort.
}

// Envoyer des données à l'écran LCD en suivant le protocole SPI.
void sendLCDData(byte data) {
  // La procédure est identique à sendLCDCommand, mais avec un octet initial différent.
  SPI.transfer(0b11111010); // Envoyer l'octet indiquant des données.
  
  byte b = data & 0b00001111; // Isoler les 4 bits de poids faible de la commande.
  b = inverse(b); // Inverser les bits (selon le protocole de l'écran LCD).
  b = b << 4; // Décaler les bits à la position correcte pour la transmission.
  SPI.transfer(b); // Envoyer les bits de poids faible.

  byte a = data & 0b11110000; // Isoler les 4 bits de poids fort de la commande.
  a = a >> 4; // Décaler les bits à la position correcte.
  a = inverse(a); // Inverser les bits.
  a = a << 4; // Décaler les bits à la position correcte pour la transmission.
  SPI.transfer(a); // Envoyer les bits de poids fort.
}

// Configurer l'écran LCD en envoyant une série de commandes initiales.
void setupLCD() {
  delay(40); // Attendre au moins 40 millisecondes après la mise sous tension.
  sendLCDCommand(0x08); //display OFF
  sendLCDCommand(0b00100010); // RE=1
  sendLCDCommand(0b00001001); // NW=4
  sendLCDCommand(0b00000110); // BDS=0 et BDC=1
  sendLCDCommand(0b00111000); // RE=0
  sendLCDCommand(0x0C); // display ON
  sendLCDCommand(0b00000001); // clear display
  //delay(250);
}

// Positionner le curseur de l'écran LCD à une position spécifique.
void setLCDposition(byte line, byte column) {
  byte address = 0x00; // Initialise l'adresse à 0. Cette adresse correspond à l'emplacement du curseur dans la mémoire DDRAM de l'écran.

  // Sélectionne l'adresse de départ en fonction de la ligne choisie. Chaque ligne de l'écran LCD a une adresse de départ différente.
  switch(line) {
    case 1:
      address = 0x00; // Adresse de départ de la première ligne.
      break;
    case 2:
      address = 0x20; // Adresse de départ de la deuxième ligne.
      break;
    case 3:
      address = 0x40; // Adresse de départ de la troisième ligne.
      break;
    case 4:
      address = 0x60; // Adresse de départ de la quatrième ligne.
      break;
  }
  // Ajoute le numéro de la colonne à l'adresse de départ pour obtenir l'adresse complète. 
  // La colonne commence à 1, donc on soustrait 1 pour obtenir le décalage correct.
  address += (column - 1);

  // Envoie la commande pour positionner le curseur à l'adresse calculée.
  // 0b10000000 est le bit de commande pour définir l'adresse DDRAM sur l'écran LCD.
  sendLCDCommand(address | 0b10000000);
}


// Écrire une chaîne de caractères sur l'écran LCD à la position courante du curseur.
void writeString(const char* str) {
  while(*str) {
    sendLCDData(*str++); // Envoyer chaque caractère de la chaîne.
  }
}

// Inverser l'ordre des bits dans un octet pour les 4 bits de poids faible.
byte inverse(byte cmd) {
  byte a = (cmd & 0b0001) << 3; // Prendre le bit de poids faible et le déplacer à la position de poids fort.
  byte b = (cmd & 0b0010) << 1; // Prendre le deuxième bit et le déplacer de deux positions à gauche.
  byte c = (cmd & 0b0100) >> 1; // Prendre le troisième bit et le déplacer de deux positions à droite.
  byte d = (cmd & 0b1000) >> 3; // Prendre le bit de poids fort et le déplacer à la position de poids faible.
  return a | b | c | d; // Combiner les bits inversés.
}

