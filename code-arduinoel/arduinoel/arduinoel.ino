///////////////////////////
//     ARDUINOEL         //
///////////////////////////
/*                                        +-----+
             +----[PWR]-------------------| USB |--+
             |                            +-----+  |
             |         GND/RST2  [ ][ ]            |
             |       MOSI2/SCK2  [ ][ ]  A5/SCL[ ] | 
             |          5V/MISO2 [ ][ ]  A4/SDA[ ] |   
             |                             AREF[ ] |
             |                              GND[ ] |
             | [ ]N/C                    SCK/13[ ] |  
             | [ ]IOREF                 MISO/12[ ] |   
             | [ ]RST                   MOSI/11[ ]~|   
             | [ ]3V3    +---+               10[ ]~|   
             | [ ]5v    -| A |-               9[ ]~|   
             | [ ]GND   -| R |-               8[ ] |  
             | [ ]GND   -| D |-                    |
             | [ ]Vin   -| U |-               7[X] |   led4
             |          -| I |-               6[X]~|   led3
 brochePotar | [X]A0    -| N |-               5[X]~|   led2
             | [ ]A1    -| O |-               4[X] |   led1
             | [ ]A2     +---+           INT1/3[X]~|   brochePiezo
             | [ ]A3                     INT0/2[X] |   brocheBouton
             | [ ]A4/SDA  RST SCK MISO     TX>1[ ] |  
             | [ ]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |  
             |            [ ] [ ] [ ]              |
             |  UNO_R3    GND MOSI 5V  ____________/
              \_______________________/
Matériel :
- un piezo
- une resistance de 150ohms pour ne pas trop alimenter le piezo
- 4 led au minimum (de couleur Rouge, bleu, vert, jaune par exemple).
- 4 résitance de 330ohms
- un bouton poussoir
- une resistance de pull-up (10kohms).
- un potentiomètre rotatif
- des fils dupont.
- une breadbord


Schéma de l'Arduino en ASCII-ART CC-By http://busyducks.com/ascii-art-arduinos
Inspiré du programme "Christmas tunes player" par Tom de Simone.
Circuit et vidéo visible ici : http://meatfinish.wordpress.com/2010/12/12/arduino-christmas-tunes-player/
   ___
 / ___ \
|_|   | |
     /_/ 
     _   ___   _ 
    |_| |___|_| |_
         ___|_   _|
        |___| |_|
Les petits Débrouillards - CC-By-Sa http://creativecommons.org/licenses/by-nc-sa/3.0/
*/

int brochePiezo = 3;
int brochePotar = 0;
int brocheBouton = 2;
int led1 = 4;
int led2 = 5;
int led3 = 6;
int led4 = 7;

int etatBouton = LOW;
boolean buttonClear = true;
int songChoice;
int ledPattern = true;

/* Change le tempo. Une plus petite valeur -> plus rapide; une plus grande -> plus lent. Il est recommandé de ne pas chnager cette valeur. 
Utiliser playTune() pour établir différentes valeur de tempo dans parseTune() pour chaque musique */
const int beatLength = 50; 

// Generate a tone by passing a square wave of a certain period to the piezo
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(brochePiezo, HIGH);
    delayMicroseconds(tone);
    digitalWrite(brochePiezo, LOW);
    delayMicroseconds(tone);
  }
}

/* Cela détermine quelle période, en microsecondes, à utiliser pour l'onde carrée pour une note donnée. Pour calculer,
p = ((1 / freq) * 1,000,000) / 2. Nous divisons par 2 car le signal sera HIGH pendant p microsecondes et ensuite LOW
pour p microsecondes. 
Fréquences pour les notes obtenues à partir de http://www.phy.mtu.edu/~suits/notefreqs.html
La gamme définie ci-dessous couvre 2 octaves de C4 (C moyen ou 261,63 Hz) à B5 (987,77 Hz). N'hésitez pas à modifier. */
void playNote(char note, int duration, boolean sharp) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C', 'D', 'E', 'F', 'G', 'A', 'B' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956, 851, 758, 716, 636, 568, 506 };
  
  // Versiuon dièse (sharp) de chaque note. la première valeur est pour "c#"
  char names_sharp[] = { 'c', 'd', 'f', 'g', 'a', 'C', 'D', 'F', 'G', 'A' };
  int tones_sharp[] = { 1804, 1607, 1351, 1204, 1073, 902, 804, 676, 602, 536 };
  
  // Joue le ton correspondant au nom de la note 
  if (sharp == false) {
    for (int i = 0; i < 14; i++) {
      if (names[i] == note) {
        playTone(tones[i], duration);
      }
    }
  } else {
    for (int i = 0; i < 10; i++) {
      if (names_sharp[i] == note) {
        playTone(tones_sharp[i], duration);
      }
    }
  }
}

/* Code pour utiliser un bouton poussoir pour arrêter/démarrer 
Note : pour arrêter une musique en cours, vous devez appuyer un petit moment. */
void updateetatBouton() {
  int val = digitalRead(brocheBouton);
  if (val == HIGH) {
    buttonClear = true;
  } else {
    if (buttonClear == true) {
      if (etatBouton == LOW) {
        etatBouton = HIGH;
      } else {
        etatBouton = LOW;
      }
      buttonClear = false;
    }
  }
}

// Fait clignoter les leds pendant la musique.
void alternateLeds() {
  if (ledPattern == true) {
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, LOW);
    digitalWrite(led4, HIGH);
    ledPattern = false;
  } else {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, LOW);
    ledPattern = true;
  }
}

/* Une chaine de caractère représente une musique. le programme analyse cette chaine pour jouer les notes avec un piezo.

Pramètres :
  char notes[]: La chaine de carcatère qui représente les notes. La méthode d'analyse de la chaine est décrite au début de fichier.
  int beatLength : Change le tempo. Une plus petite valeur -> plus rapide; une plus grande -> plus lent.
  boolean loopSong : si "true", La musique sera jouée jusqu'à la fin (ou jusqu'à ce que vous pressiez le bouton)
*/
void parseTune(char notes[], int beatLength, boolean loopSong) {
  boolean play = true;
  
  // 1 iteration de cette boucle == 1 note jouée
  for (int i = 0; notes[i] != '.' && play == true; i++) { // l'itération est arrêtée si le caractère "." est le prochain caratère
    updateetatBouton();
    if (etatBouton == LOW) { // Pour chaque note, on vérifie si le bouton a été pressé pour demander l'arrêt de la musique.
      play = false;
    } else {
      if (notes[i] == ',') { // "," signifie un silence
      
        // Regarde le nombre (max. 2 digits) qui suit le caratère "," pour fixer la durée du silence
        char len[3];
        int count = 0;
        while (notes[i+1] >= '0' && notes[i+1] <= '9' && count < 2) {
          len[count] = notes[i+1];
          count++;
          i++;
        }
        len[count] = '\0';
        int duration = atoi(len);
        
        delay(duration * beatLength); // durée du silence
      } else { // Joue la prochaine note représentée par un caractère : "c4", "a#12" par exemple.
        alternateLeds(); // alterne l'allumage des différentes leds pour les faire "danser".
        char note = notes[i];
        boolean sharp;
        
        // Si le prochain caractère ets un "#" alors on doit jouer la note en dièse
        if (notes[i+1] == '#') {
          i++;
          sharp = true;
        } else {
          sharp = false;
        }
        
        // Regarde le nombre (max. 2 digits) qui suit le nom de la note pour établir sa durée.
        char len[3];
        int count = 0;
        while (notes[i+1] >= '0' && notes[i+1] <= '9' && count < 2) {
          len[count] = notes[i+1];
          count++;
          i++;
        }
        len[count] = '\0';
        int duration = atoi(len);
        
        playNote(note, duration * beatLength, sharp);
      }
      
      delay(beatLength / 2); // pause entre en les notes.
    }
  }
  
  if (loopSong == false) {
    etatBouton = LOW;
  }
}

// Ecrivez vos musiques ici en utilisant la méthode décrite au début de ce fichier. Jusqu'à 4 musiques.
void playTune (int tune) {
  if (tune == 1) { // Mon Beau sapin
    char notes[] = "c4f3f1f6g2a3a1a6f2g2a2a#4e4g4f4c4f3f1f6g2a3a1a6f2g2a2a#4e4g4f4,2C2C2a2D6C2C2a#6a#2a#2g2C6a#2a#2a2a4c4f3f1f6g2a3a1a6f2g2a2a#4e4g4f4,8.";
    parseTune(notes, beatLength * 2, false);
  } else if (tune == 2) { // Petit papa noël  
    char notes[] = "g4C4C4C4D4C12C2D2E4E4E4F4E12D4C6C2C2C2b2a2g12g2g2C8C2C2b2C2D12g4C4C4C4D4C12C2D2E4E4E4F4E12D4C6C2C2C2b2a2g12g2g2C8C2C2D2D2C16a2a2a2a2a4a2b2C6a2a4g4C2C2C2C2C4b2C2D16D#2E2E2E2E4D2E2F6D2C4a#2D#2E2E2E2F4F2F2G12g4C4C4C4D4C12C2D2E4E4E4F4E12D4C6C2C2C2b2a2g12g2g2C8C2C2b2C2D12g4C4C4C4D4C12C2D2E4E4E4F4E12D4C6C2C2C2b2a2g12g2g2C8C2C2D2D2C12g4a4C4D4F4G16,8.";
    parseTune(notes, beatLength * 1.50, false);
  } else if (tune == 3) { // Il est né le divin enfant
    char notes[] = "a4D4D4F#2D2a4D4D8D4D2E2F#4G2F#2E4D4E2C#2a4a4D4D4F#2D2a4D4D8D4E4F#4G2F#2E4A4D8F#4F#2G2A4G2F#2G4B4A8F#4F#2G2A4B2A2G4F#4F#4E4F#4F#2G2A4G2F#2G4B4A8F#4G4A4B2A2G4F#4E8a4D4D4F#2D2a4D4D8D4D2E2F#4G2F#2E4D4E2C#2a4a4D4D4F#2D2a4D4D8D4E4F#4G2F#2E4A4D8,8.";
    parseTune(notes, beatLength * 1.25, false);
  } else if (tune == 4) { // Vive le vent (Jingle Bells)
    char notes[] = "b4b4b8b4b4b8b4D4g6a2b12,4C4C4C6C2C4b4b4b2b2b4a4a4b4a8D8b4b4b8b4b4b8b4D4g6a2b12,4,C4C4C6C2C4b4b4b2b2D4D4C4a4g12,8.";
    parseTune(notes, beatLength, false);
   /* Autres chants de noël :
   Deck the Halls - D6C2b4a4g4a4b4g4a2b2C2a2b6a2g4f#4g6,2D6C2b4a4g4a4b4g4a2b2C2a2b6a2g4f#4g6,2a6b2C4a4b6C2D4a4b2C#2D4E2F#2G4F#4E4D6,2D6C2b4a4g4a4b4g4E2E2E2E2D6C2b4a4g8,8.
   The Holly and the Ivy - g4g2g2g4E4D4b6g2g2g2g4E4D8D2C2b2a2g4b2b2e2e2d4g2a2b2C2b4a4g8,8.
   We Wish You a Merry Christmas - d4g4g2a2g2f#2e4c4e4a4a2b2a2g2f#4d4f#4b4b2C2b2a2g4e4d2d2e4a4f#4g8,8.
   */
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(brochePiezo, OUTPUT);
  pinMode(brocheBouton, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
}

void loop() {
  /* On démarre en silence. L'utilisateur peut sélectionner une des 4 musiques en tournant le motentiomètre.
  Chacune des 4 led représente une chanson.
  Pour jiuer la musique il faut appuyer sur le bouton poussoir.
  Si on appuye sur le bouton poussoir pendant que la mélodie est jouée, la mélodie est arrêtée.
  A la fin de la mélodie, on se retrouve au point de départ (attente de la sélection d'une musique et appuye sur le bouton) */
  int val = analogRead(brochePotar);
  Serial.println(val);
  if (val < 255) {
    songChoice = 1;
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
  } else if (val < 512) {
    songChoice = 2;
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
  } else if (val < 767) {
    songChoice = 3;
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, LOW);
  } else {
    songChoice = 4;
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, HIGH);
  }
  
  updateetatBouton();
  if (etatBouton == HIGH) {
    playTune(songChoice);
  }
}

