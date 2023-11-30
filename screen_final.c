#include <SPI.h> // Inclure la bibliothèque SPI nécessaire pour la communication avec l'écran LCD.
 #define INT0 2
 #define INT1 3
 #define LED 4
 #define BLANK " "

 volatile byte state0 = LOW;
 volatile byte state1 = LOW;
 volatile byte postscaler = LOW; // Variable volatile utilisée pour le comptage dans l'interruption du Timer 2.
 volatile long int seconds = 0;

 volatile bool LEDVar = false;
 unsigned long lastInterruptTime0 = 0;
 unsigned long lastInterruptTime1 = 0;
 const unsigned long debounceDelay = 150; // Délai de rebond en millisecondes
 int hour=0;
 int min=0;
 int sec=0;
 static int var_state;

 // Variable pour suivre l'état d'affichage actuel
 int displayState = 0;

 // Variables globales pour stocker les valeurs des capteurs
 float temperature = 0.0;
 float humidity = 0.0;
 float pressure = 0.0;
 float luminosity = 0.0;
 int airQuality = 0;

// Global flag to indicate if the clock is in setting mode
bool isSettingTime = false;

// Ajoutez une nouvelle variable globale pour suivre le temps depuis le dernier changement d'affichage
unsigned long lastAutoScrollTime = 0;
const unsigned long autoScrollInterval = 5000; // 5 secondes en millisecondes

volatile unsigned long buttonPressTime0 = 0; // Temps d'appui sur le bouton INT0
const unsigned long longPressTime = 1000; // 1 seconde en millisecondes


 //----------------------------------SPI-------------------------------------

 // Envoyer une commande à l'écran LCD en suivant le protocole SPI.
 void sendLCDCommand(byte command) {
   SPI.transfer(0b11111000); // Envoyer l'octet indiquant une commande.

   byte b = command & 0b00001111; // Isoler les 4 bits de poids faible de la commande.
   b = inverse(b); // Inverser les bits (selon le protocole de l'écran LCD).
   b = b << 4; // Décaler les bits à la position correcte pour la transmission.
   SPI.transfer(b); // Envoyer les bits de poids faible.

   byte a = command & 0b11110000; // Isoler les 4 bits de poids fort de la commande.
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

 //-------------------------------HORLOGE-----------------------------

 void LEDvariation() {
   if (LEDVar) {
     digitalWrite(LED, HIGH);
   } else {
     digitalWrite(LED, LOW); // Éteindre la LED
   }
 }


 void displayTime() {
   // Créez des chaînes pour contenir les valeurs formatées de l'heure, des minutes et des secondes
   char hourStr[3], minStr[3], secStr[3];
   // Utilisez sprintf pour formater les valeurs avec un zéro de remplissage si nécessaire
   sprintf(hourStr, "%02d", hour);
   sprintf(minStr, "%02d", min);
   sprintf(secStr, "%02d", sec);

   // Créez une chaîne pour contenir la ligne complète de texte à afficher
   char timeStr[20]; // Assurez-vous que c'est assez grand pour tout le texte

   // Construisez la chaîne finale en fonction de l'état actuel
   if (LEDVar) {
     switch (var_state) {
       case 0: // Affichage normal
         sprintf(timeStr, "Heure:%s-%s-%s", hourStr, minStr, secStr);
         break;
       case 1: // Réglage des heures
         sprintf(timeStr, "Heure:%s-%s-  ", hourStr, minStr);
         break;
       case 2: // Réglage des minutes
         sprintf(timeStr, "Heure:%s-  -%s", hourStr, secStr);
         break;
       case 3: // Réglage des secondes
         sprintf(timeStr, "Heure:  -%s-%s", minStr, secStr);
         break;
     }
   } else {
     // Si LEDVar n'est pas vrai, afficher l'heure normalement
     sprintf(timeStr, "Heure:%s-%s-%s", hourStr, minStr, secStr);
   }

   // Positionnez le curseur et écrivez la chaîne formatée sur l'écran LCD
   setLCDposition(1, 1);
   writeString(timeStr);
 }

 //---------------------------------TIMER------------------------------------

void updateClock() {
  if (!isSettingTime) {
    seconds++;
    hour = (seconds / 3600) % 24;
    min = (seconds / 60) % 60;
    sec = seconds % 60;
  }
}

void adjustTime(int adjustment) {
  // Adjust the clock based on the current setting mode
  switch(var_state) {
    case 1: // Adjust seconds
      sec = (sec + adjustment) % 60;
      break;
    case 2: // Adjust minutes
      min = (min + adjustment) % 60;
      break;
    case 3: // Adjust hours
      hour = (hour + adjustment) % 24;
      break;
  }

  // Recalculate the total seconds after adjustment
  seconds = hour * 3600 + min * 60 + sec;
}

ISR(INT0_vect) {
  if (millis() - lastInterruptTime0 > debounceDelay) {
    lastInterruptTime0 = millis();
    if (isSettingTime) {
      adjustTime(1); // Increment the current time component
    }
  }
}

ISR(INT1_vect) {
  if (millis() - lastInterruptTime1 > debounceDelay) {
    lastInterruptTime1 = millis();
    var_state = (var_state + 1) % 4;

    // Enable or disable time setting mode
    isSettingTime = (var_state != 0);
  }
}

ISR(TIMER2_OVF_vect) {
  TCNT2 = 131;
  postscaler++;
  if (postscaler >= 125) {
    postscaler = 0;
    LEDVar = !LEDVar;
    updateClock(); // Update clock only if not in setting mode
  }

  if (postscaler % 62 == 0) {
    LEDVar = !LEDVar;
  }
}

 //------------------------------------SETUP + LOOP --------------------------------------

 void setup() {
   SPI.begin();
   Serial.begin(9600);
   pinMode(INT0, INPUT_PULLUP);
   pinMode(INT1, INPUT_PULLUP);
   pinMode(LED,OUTPUT);

   //Config TIMER2
   TCCR2A &= 0b11111100;
   TCCR2B &= 0b11110111;
   TCCR2B |= 0b00000111;
   TIMSK2 |= 0b000000001;

   EICRA |= 0b00000100; // Any logical change on TN0 generates an interrupt request.
   EICRA |= 0b00000001; // Any logical change on INT1 generates an interrupt request.
   EIMSK |= 0b00000001; // Configure INT1
   EIMSK |= 0b00000010; // Configure INT0

   interrupts();
   setupLCD();
 }

 void loop() {

   // Vérifier si des données sont disponibles sur le port série
   if (Serial.available() > 0) {
     char dataType = Serial.read(); // Lire le type de données (T, H, P, Q, L)
     String dataValue = Serial.readStringUntil('\n'); // Lire la valeur jusqu'au caractère de nouvelle ligne

     // Assigner les données lues aux variables des capteurs
     switch (dataType) {
       case 'T': // Température
         temperature = dataValue.toFloat();
         break;
       case 'H': // Humidité
         humidity = dataValue.toFloat();
         break;
       case 'P': // Pression
         pressure = dataValue.toFloat();
         break;
       case 'Q': // Qualité de l'air
         airQuality = dataValue.toInt();
         break;
       case 'L': // Luminosité
         luminosity = dataValue.toFloat();
         break;
       default:
         // Type de données non reconnu
         break;
     }
   }
   // Mise à jour de l'affichage basé sur l'état actuel déterminé par les boutons
   updateDisplay();

   // Mise à jour de l'heure à chaque seconde
   displayTime();

   if (millis() - lastAutoScrollTime >= autoScrollInterval) {
     displayState = (displayState + 1) % 4; // Changez l'affichage
     lastAutoScrollTime = millis(); // Réinitialisez le compteur de temps
     clearScreen(0);
     updateDisplay(); // Mettre à jour l'affichage avec le nouvel état
   }

   bool buttonState = digitalRead(INT0);
   if (buttonState == LOW) {
     lastAutoScrollTime = millis(); // Réinitialisez le compteur de temps pour suspendre le défilement automatique
     if (var_state == 0) { 
         displayState = (displayState + 1) % 4; // Fait défiler à travers 0, 1, 2, 3
     }
     while(digitalRead(INT0) == LOW); // Attend que le bouton soit relâché
     clearScreen(0);
     updateDisplay(); // Mettre à jour l'affichage avec le nouvel état
   }


   LEDvariation();

 }


// Generic function to display sensor values
void displaySensorValue(float value, const char* unit, const char* sensorType, int line) {
    char valueStr[16]; // Buffer for the converted value

    if (sensorType == "Qualite de l'air") {
      itoa(value, valueStr,10);
    }
    else  {
      dtostrf(value, 5, 1, valueStr); // Convert float to string with proper formatting
    }
    char fullStr[20]; // Buffer for the complete string to be displayed
    sprintf(fullStr, "%s:%s%s", sensorType, valueStr, unit); // Construct the string

    setLCDposition(line, 1); // Set the cursor position on the LCD
    writeString(fullStr); // Display the string on the LCD
}

// Update the display function to use the generic function
void updateDisplay() {
    displaySensorValue(temperature, "C", "Temp", 2); // Display temperature

    switch (displayState) {
        case 0:
            displaySensorValue(humidity, "%", "Humidite", 3);
            displaySensorValue(pressure, "hPa", "Pression", 4);
            break;
        case 1:
            displaySensorValue(pressure, "hPa", "Pression", 3);
            displaySensorValue(luminosity, "lux", "Luminosite", 4);
            break;
        case 2:
            displaySensorValue(luminosity, "lux", "Luminosite", 3);
            displaySensorValue((float)airQuality, "", "Qualite de l'air", 4);
            break;
        case 3:
            displaySensorValue((float)airQuality, "", "Qualite de l'air", 3);
            displaySensorValue(pressure, "%", "Humidite", 4);
            break;
    }
}

 // Function to clear a single line on the LCD
 void clearLine(byte line) {
   setLCDposition(line, 1); // Set the position to the start of the line
   for (int i = 0; i < 20; i++) { // Assuming 20 characters per line
     sendLCDData(' '); // Write a space to each position
   }
 }

// Function to clear the entire screen
// clearMethod: 0 for writing spaces, 1 for clear display command
void clearScreen(byte clearMethod) {
  if (clearMethod == 1) {
    // Clear by writing spaces to each position
    for (byte line = 1; line <= 4; line++) {
      clearLine(line);
    }
  } else {
    // Clear using the clear display command
    sendLCDCommand(0b00000001); // Clear display command
    delay(2); // LCD clear command can take up to 1.52ms, so delay for 2ms
  }
}
