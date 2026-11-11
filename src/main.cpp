
#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#include <SoftwareSerial.h>

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__HARDSERIAL


HardwareSerial Serial1(PC11, PC10); // RX, TX
// RemoteXY connection settings 
#define REMOTEXY_SERIAL Serial1
#define REMOTEXY_SERIAL_SPEED 9600

#include <RemoteXY.h>


// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =   // 99 bytes V19 
  { 255,4,0,4,0,92,0,19,0,0,0,0,25,2,106,200,200,84,1,1,
  5,0,1,17,29,40,32,3,17,96,31,7,121,31,0,1,18,33,40,31,
  3,50,96,33,7,95,31,0,1,19,38,40,31,101,17,95,31,7,37,31,
  0,1,20,43,40,31,101,50,95,33,7,191,31,0,73,45,31,7,76,152,
  4,39,11,1,0,2,37,0,0,0,0,0,0,64,64,0,0,0,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t B_VERT; // =1 if button pressed, else =0, from 0 to 1
  uint8_t B_JAUNE; // =1 if button pressed, else =0, from 0 to 1
  uint8_t B_ROUGE; // =1 if button pressed, else =0, from 0 to 1
  uint8_t B_BLEU; // =1 if button pressed, else =0, from 0 to 1

    // output variables
  float COEURS; // from 0 to 3

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////


#define TFT_CS    10            
#define TFT_RST   9             
#define TFT_DC    8 
#define TOUCH_CS  7
#define NOIR      0x0000
#define BLEU      0x001F
#define VERT      0x07E0
#define CYAN      0x07FF
#define ROUGE     0xF800
#define MAGENTA   0xF81F
#define ORANGE    0xFBA0
#define JAUNE     0xFFE0
#define BLANC     0xFFFF

#define TS_MINX 300 // pression min à x
#define TS_MAXX 3800
#define TS_MINY 300
#define TS_MAXY 3800

// Section Constantes
#define MAX_SEQUENCE 100

// Ordre des boutons : 0 = vert, 1 = rouge, 2 = jaune, 3 = bleu
const int BOUTON_X[4] = {15, 170, 15, 170};
const int BOUTON_Y[4] = {45, 45, 140, 140};
const int BOUTON_L = 145;
const int BOUTON_H = 85;
const uint16_t BOUTON_COULEUR[4] = {VERT, ROUGE, JAUNE, BLEU};

// Section Sous-Programmes
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts = XPT2046_Touchscreen(TOUCH_CS);
int score = 0;
int coeurs = 3;
int sequence[MAX_SEQUENCE]; // suite de couleurs à reproduire
int longueur = 0;           // nombre de couleurs dans la suite
int position = 0;           // prochaine couleur attendue du joueur

void afficherScore(){
  tft.fillRect(90,5,140,30, NOIR);
  tft.setCursor(15,10);
  tft.setTextColor(BLANC);
  tft.setTextSize(2);
  tft.print("Score : ");
  tft.print(score);
}

void afficherCoeurs(){
  RemoteXY.COEURS = coeurs;
  tft.fillRect(240, 0, 80, 30, NOIR);
  for (int i = 0; i<coeurs; i++){
    int x = 250 + (i*22);
    int y = 10;
    tft.fillRect(x + 1, y, 5, 4, ROUGE);
    tft.fillRect(x + 7, y, 5, 4, ROUGE);
    tft.fillTriangle(x, y + 4, x + 12, y + 4, x + 6, y + 16, ROUGE);
  }
}

void dessinerBoutons() {
  for (int i = 0; i < 4; i++) {
    tft.fillRect(BOUTON_X[i], BOUTON_Y[i], BOUTON_L, BOUTON_H, BOUTON_COULEUR[i]);
  }
}

// Fait clignoter un bouton en blanc
void allumerBouton(int b) {
  tft.fillRect(BOUTON_X[b], BOUTON_Y[b], BOUTON_L, BOUTON_H, BLANC);
  RemoteXYEngine.delay(300);
  tft.fillRect(BOUTON_X[b], BOUTON_Y[b], BOUTON_L, BOUTON_H, BOUTON_COULEUR[b]);
}

// Renvoie le bouton appuyé (0 à 3) sur l'écran ou sur RemoteXY, -1 si aucun
int lireBouton() {
  if (RemoteXY.B_VERT)  return 0;
  if (RemoteXY.B_ROUGE) return 1;
  if (RemoteXY.B_JAUNE) return 2;
  if (RemoteXY.B_BLEU)  return 3;

  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int pixelX = map(p.x, 200, 3800, 320, 0);
    int pixelY = map(p.y, 200, 3800, 240, 0);

    for (int i = 0; i < 4; i++) {
      if (pixelX >= BOUTON_X[i] && pixelX <= BOUTON_X[i] + BOUTON_L &&
          pixelY >= BOUTON_Y[i] && pixelY <= BOUTON_Y[i] + BOUTON_H) {
        return i;
      }
    }
  }
  return -1;
}

// Attend que le joueur relâche tout (évite de compter deux fois le même appui)
void attendreRelachement() {
  while (ts.touched() || RemoteXY.B_VERT || RemoteXY.B_ROUGE || RemoteXY.B_JAUNE || RemoteXY.B_BLEU) {
    RemoteXYEngine.handler();
  }
}

// Joue toute la suite, puis c'est au joueur de la reproduire
void montrerSequence() {
  RemoteXYEngine.delay(600);
  for (int i = 0; i < longueur; i++) {
    allumerBouton(sequence[i]);
    RemoteXYEngine.delay(200);
  }
  position = 0;
}

// Ajoute une couleur au hasard à la suite et la rejoue
void nouveauTour() {
  if (longueur < MAX_SEQUENCE) {
    sequence[longueur] = random(4);
    longueur++;
  }
  montrerSequence();
}

void nouvellePartie() {
  score = 0;
  coeurs = 3;
  longueur = 0;
  tft.fillScreen(NOIR);
  afficherScore();
  afficherCoeurs();
  dessinerBoutons();
  nouveauTour();
}

void perdu() {
  tft.fillScreen(NOIR);
  tft.setTextColor(ROUGE);
  tft.setTextSize(4);
  tft.setCursor(100, 90);
  tft.print("PERDU");
  tft.setTextColor(BLANC);
  tft.setTextSize(2);
  tft.setCursor(100, 140);
  tft.print("Score : ");
  tft.print(score);
  RemoteXYEngine.delay(3000);
  nouvellePartie();
}

void setup() {
  //remoteXY
  RemoteXY_Init();

  Serial.begin(115200);
  ts.begin(); // met T_CS à l'état haut avant de parler à l'écran
  tft.begin(8000000); // 8 MHz : à 40 MHz l'écran reste blanc avec ce câblage
  tft.setRotation(1);
  ts.setRotation(1);
  randomSeed(analogRead(A0)); // broche en l'air : suite différente à chaque démarrage
  nouvellePartie();
}

void loop(){
  //remoteXY
  RemoteXYEngine.handler ();

  int bouton = lireBouton();
  if (bouton == -1) return;

  allumerBouton(bouton);
  attendreRelachement();

  if (bouton == sequence[position]) {
    // Bonne couleur
    position++;
    if (position == longueur) {
      // Suite terminée : un point et une couleur de plus
      score++;
      afficherScore();
      nouveauTour();
    }
  }
  else {
    // Mauvaise couleur : on perd un coeur et la suite est rejouée
    coeurs--;
    afficherCoeurs();
    if (coeurs == 0) perdu();
    else montrerSequence();
  }
}
