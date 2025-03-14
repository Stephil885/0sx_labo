//AMAYE DJEBI EZECHIEL

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialisation de l'écran LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Définition des broches
const int LDR_PIN = A0;
const int LED_PIN = 8;
const int JOY_X_PIN = A1;
const int JOY_Y_PIN = A2;
const int JOY_BUTTON_PIN = 2;

// Constantes pour `map()`
const int JOY_MIN = 0, JOY_MAX = 1023;
const int VITESSE_MIN = -25, VITESSE_MAX = 120;
const int ANGLE_MIN = -90, ANGLE_MAX = 90;
const int LUMI_MIN = 0, LUMI_MAX = 100;

// Positions du LCD
const int CURSOR_LUM = 0, CURSOR_POURCENT = 6;
const int CURSOR_PHARES = 0, CURSOR_ONOFF = 8;
const int CURSOR_MOUV = 0, CURSOR_VITESSE = 8;
const int CURSOR_DIR = 0, CURSOR_ANGLE = 5, CURSOR_SENS = 9;

// Variables globales des capteurs
int luminositeBrute = 0;
int pourcentageLuminosite = 0;
bool phareAllume = false;

// Variables du joystick
int joyX = 0, joyY = 0;
int vitesse = 0, angleDirection = 0;
String mouvement = "Avance", direction = "D";

// Variables de gestion du temps
unsigned long debutObscurite = 0;
unsigned long dernierMajLCD = 0;
unsigned long dernierMajBouton = 0;
unsigned long dernierEnvoiSerial = 0;

// Variables de gestion de l'affichage
bool pageLCD = 0;
bool boutonEtatPrecedent = HIGH;

// ----------------- CARACTÈRE PERSONNALISÉ -----------------
byte caracterePerso[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b10101,
    0b01110,
    0b00100,
    0b00000,
    0b00000
};

// ----------------- FONCTION DÉMARRAGE -----------------
void demarrage() {
    lcd.createChar(0, caracterePerso); // Créer le caractère personnalisé

    lcd.setCursor(0, 0);
    lcd.print("AMAYE");  // Afficher ton nom

    lcd.setCursor(0, 1);
    lcd.write(byte(0));  // Afficher le caractère personnalisé

    lcd.setCursor(14, 1);
    lcd.print("93");  // Afficher les deux derniers chiffres de ton numéro étudiant

    delay(3000);  // Attendre 3 secondes
    lcd.clear();  // Effacer l'écran
}

// ----------------- SETUP -----------------
void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(JOY_BUTTON_PIN, INPUT_PULLUP);
    
    lcd.init();
    lcd.backlight();

    demarrage();  // Appel de la fonction de démarrage
}

// ----------------- LECTURE DES CAPTEURS -----------------
void lireCapteurs() {
    luminositeBrute = analogRead(LDR_PIN);
    pourcentageLuminosite = map(luminositeBrute, JOY_MIN, JOY_MAX, LUMI_MIN, LUMI_MAX);
}

// ----------------- GESTION DES PHARES -----------------
void gererPhares() {
    unsigned long tempsActuel = millis();
    if (pourcentageLuminosite < 50) {
        if (debutObscurite == 0) debutObscurite = tempsActuel;
        if (tempsActuel - debutObscurite >= 5000) {
            phareAllume = true;
            digitalWrite(LED_PIN, HIGH);
        }
    } else {
        debutObscurite = 0;
        phareAllume = false;
        digitalWrite(LED_PIN, LOW);
    }
}

// ----------------- LECTURE DU JOYSTICK -----------------
void lireJoystick() {
    joyX = analogRead(JOY_X_PIN);
    joyY = analogRead(JOY_Y_PIN);

    // Vérifier si le joystick est au repos (zone morte)
    if (joyY > 500 && joyY < 524) {  // Zone morte autour de 512
        vitesse = 0;
        mouvement = "Stop";
    } else {
        vitesse = abs(map(joyY, JOY_MIN, JOY_MAX, VITESSE_MIN, VITESSE_MAX));
        mouvement = (joyY < 512) ? "Recule" : "Avance";
    }

    angleDirection = map(joyX, JOY_MIN, JOY_MAX, ANGLE_MIN, ANGLE_MAX);
    
    direction = (angleDirection < -5) ? "G" : (angleDirection > 5) ?"D":"N";
}


// ----------------- MISE À JOUR DE L'ÉCRAN LCD -----------------
void mettreAJourLCD() {
    unsigned long tempsActuel = millis();
    if (tempsActuel - dernierMajLCD < 100) return;

    lcd.clear();
    if (pageLCD == 0) {
        lcd.setCursor(CURSOR_LUM, 0);
        lcd.print("Lum: ");
        lcd.setCursor(CURSOR_POURCENT, 0);
        lcd.print(pourcentageLuminosite);
        lcd.print("%");

        lcd.setCursor(CURSOR_PHARES, 1);
        lcd.print("Phares: ");
        lcd.setCursor(CURSOR_ONOFF, 1);
        lcd.print(phareAllume ? "ON" : "OFF");
    } else {
        lcd.setCursor(CURSOR_MOUV, 0);
        lcd.print(mouvement);
        lcd.setCursor(CURSOR_VITESSE, 0);
        lcd.print(vitesse);
        lcd.print("km/h");

        lcd.setCursor(CURSOR_DIR, 1);
        lcd.print("Dir:");
        //lcd.setCursor(CURSOR_ANGLE, 1);
        //lcd.print(angleDirection);
        lcd.setCursor(CURSOR_SENS, 1);
        lcd.print(direction);
    }
    dernierMajLCD = tempsActuel;
}

// ----------------- GESTION DU BOUTON -----------------
void gererBouton() {
    unsigned long tempsActuel = millis();
    if (tempsActuel - dernierMajBouton < 200) return;

    bool boutonPresse = digitalRead(JOY_BUTTON_PIN) == LOW;
    if (boutonPresse && boutonEtatPrecedent) {
        pageLCD = !pageLCD;
    }
    boutonEtatPrecedent = boutonPresse;
    dernierMajBouton = tempsActuel;
}

// ----------------- TRANSMISSION SÉRIE -----------------
void envoyerDonneesSerie() {
    unsigned long tempsActuel = millis();
    if (tempsActuel - dernierEnvoiSerial < 100) return;

    Serial.print("etd:2403493,"); 
    Serial.print("x:");
    Serial.print(angleDirection);
    Serial.print(",y:");
    Serial.print(vitesse);
    Serial.print(",sys:");
    Serial.println(phareAllume ? 1 : 0);
    
    dernierEnvoiSerial = tempsActuel;
}

// ----------------- BOUCLE PRINCIPALE -----------------
void loop() {
    lireCapteurs();
    gererPhares();
    lireJoystick();
    gererBouton();
    mettreAJourLCD();
    envoyerDonneesSerie();
}
