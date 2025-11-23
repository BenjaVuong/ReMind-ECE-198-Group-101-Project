// C++ code
// Include SoftwareSerial so we can talk to the ESP
#include <SoftwareSerial.h>

// ##########################################################################################################
// ####################################### CONST DEFINITIONS ################################################
// ##########################################################################################################

// defining notes
#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

const int buttonSounds[9]{ NOTE_C3, NOTE_E3, NOTE_G3, NOTE_B3, NOTE_C4, NOTE_E4, NOTE_G4, NOTE_B4, NOTE_C5 };

// Button Pins: All Pulldown
const byte buttonPins[9]{ 3, 4, 7, 8, 9, 10, 11, 12, A4 };

// buzzer pin
int buzzerPin = 13;

// UART Connection to ESP
SoftwareSerial ArduinoSerial(0, 1);  // RX 0, TX 1

// LED1 PIN
const byte led9pin{ A5 };
// SHIFT Register
const byte latchPin{ 5 };
const byte clockPin{ 6 };
const byte dataPin{ 2 };

// Randomization
const byte analogRandomPin{ A0 };
const byte sequenceCap{ 30 };
byte sequence[sequenceCap]{};  // stores the sequence for a game

// State Logic
enum GameState {  // defines different states
  MENU,
  SHOW,
  TEST,
  FAIL,
  WIN
};

GameState state = MENU;  // assigns MENU state as default state
byte gameRound{ 1 };


// Time

unsigned long timestamp_ms{ 0 };
unsigned long showTimeInterval_ms{ 500 };


// Button variables
byte buttonState{ 0 };
byte reading{ 0 };
byte lastReading{ 0 };
unsigned long lastDebounceTime{ 0 };
unsigned long debounceDelay{ 50 };





// ##########################################################################################################
// ################################################### SETUP ################################################
// ##########################################################################################################

void setup() {

  /* ====================== I/O INTIALIZATION =====================*/

  pinMode(LED_BUILTIN, OUTPUT);

  // Button Setup
  for (byte i{ 0 }; i < 9; i++) {
    pinMode(buttonPins[i], INPUT);
  }

  // SR (LED) Setup
  pinMode(led9pin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Set up the serial
  Serial.begin(9600);
  ArduinoSerial.begin(3200);

  ledDisplayOff();


  /* ====================== GAME DATA INTIALIZATION =====================*/
  // Generate a random seed for random function
  randomSeed(analogRead(analogRandomPin));
  state = MENU;
}





// ##########################################################################################################
// ############################################### GAME LOOP ################################################
// ##########################################################################################################

void loop() {


  buttonRead();


  switch (state) {

    case MENU:

      // press any key to start (delay, generate sequence, switch to show)
      if (buttonState != 0) {

        // make the new random sequence for game
        generateRandomSequence(sequence);

        //////////// !!!!!! some sounds and cool lights and whatnot can go here
        ledDisplayStart();
        gameStartSound();
        delay(500);
        ////////////

        // Reset game variables
        gameRound = 1;

        // state: MENU -> SHOW
        state = SHOW;
      }

      break;


    case SHOW:

      // light up leds according to sequence up to the ith led
      for (byte i{ 0 }; i < gameRound; i++) {

        ledDisplayOn(sequence[i]);
        timestamp_ms = millis();
        buttonSound(sequence[i]);
        while ((millis() - timestamp_ms) < showTimeInterval_ms) {
        };

        ledDisplayOff();
        timestamp_ms = millis();
        while ((millis() - timestamp_ms) < showTimeInterval_ms) {};
      }

      // state: SHOW -> TEST
      state = TEST;
      break;


    case TEST:

      // MEMORY TEST
      for (byte i{ 0 }; i < gameRound; i++) {

        // trap them here until they press a button
        buttonState = 0;
        while (buttonState == 0) {
          buttonRead();
        };
        buttonSound(buttonState);


        // if they press a button, check if they selected the right one
        if (buttonState != sequence[i]) {
          // wrong answer, move to FAIL state
          state = FAIL;
          break;  // exit for loop to go to fail state
        }         // if its correct then iterate to the next sequence value

        // Trap the user until they release the button
        while (digitalRead(buttonPins[buttonState - 1]) == HIGH) {
          ledDisplayOn(buttonState);  // light up the clicked button
        }
        ledDisplayOff();  // turn off button after its unpressed
      }

      // if they user is in the FAIL state then break the case
      if (state == FAIL) {
        break;
      }

      // POST TEST LOGIC
      // after passing the for loop, increment the round and move back to the SHOW state.
      if (gameRound == sequenceCap) {  // win after 30 rounds
        state = WIN;

      } else {  // move on to next round if not yet at 30
        gameRound++;
        // Show success LED pattern
        delay(1000);
        ledDisplaySuccess();
        successSound();
        delay(500);
        ledDisplayOff();
        delay(500);
        state = SHOW;
      }

      break;


    case FAIL:
      // play sad music, return to menu

      ledDisplayFail();
      failureSound();

      // Tell ESP about high score
      ArduinoSerial.print("Score:");
      ArduinoSerial.print(gameRound - 1);
      ArduinoSerial.println("\0");

      delay(500);
      ledDisplayOff();
      buttonState = 0;
      // state: FAIL -> MENU
      state = MENU;
      break;


    case WIN:
      // menu logic

      // state : WIN -> MENU
      state = MENU;
      break;
  }
}






// ##########################################################################################################
// ########################################## HELPER FUNCTIONS ##############################################
// ##########################################################################################################
/* ========================== Random Functions ===============================*/

// FUNCTION: Takes in the sequence array and randomizes its elements from [1,9]
void generateRandomSequence(byte sequence[]) {

  for (byte i{ 0 }; i < sequenceCap; i++) {
    sequence[i] = random(1, 10);  // rand int [1,9]
  }
}




/* ========================== LED Functions ===============================*/
// FUNCTION: Turns on the nth LED
void ledDisplayOn(byte n) {

  ledDisplayOff();

  if (n == 9) {
    digitalWrite(led9pin, HIGH);

  } else if (n > 0) {
    byte led_n{ 1 << (n - 1) };

    digitalWrite(latchPin, LOW);                   // close the latch
    shiftOut(dataPin, clockPin, MSBFIRST, led_n);  // light up the LED
    digitalWrite(latchPin, HIGH);                  // open the latch
    digitalWrite(latchPin, LOW);                   // close the latch
  }
}


// FUNCTION: Turns off all LEDs
void ledDisplayOff() {

  digitalWrite(led9pin, LOW);

  digitalWrite(latchPin, LOW);               // close the latch
  shiftOut(dataPin, clockPin, MSBFIRST, 0);  // clear data
  digitalWrite(latchPin, HIGH);              // open the latch
  digitalWrite(latchPin, LOW);               // open the latch
}


// FUNCTION: Ignites all odd LEDs
void ledDisplayFail() {

  digitalWrite(led9pin, HIGH);

  digitalWrite(latchPin, LOW);                        // close the latch
  shiftOut(dataPin, clockPin, MSBFIRST, 0b01010101);  // Odd lights lit
  digitalWrite(latchPin, HIGH);                       // open the latch
  digitalWrite(latchPin, LOW);                        // close the latch
}


// FUNCTION: Ignites all even LEDs
void ledDisplaySuccess() {

  digitalWrite(led9pin, LOW);

  digitalWrite(latchPin, LOW);                        // close the latch
  shiftOut(dataPin, clockPin, MSBFIRST, 0b10101010);  // Odd lights lit
  digitalWrite(latchPin, HIGH);                       // open the latch
  digitalWrite(latchPin, LOW);                        // close the latch
}

// FUNCTION: Ignites all even LEDs
void ledDisplayStart() {

  digitalWrite(led9pin, LOW);
  digitalWrite(latchPin, LOW);  // close the latch

  for (byte i{ 1 }; i <= 9; i++) {
    ledDisplayOn(i);
    delay(100);
  }

  ledDisplayOff();
}



/* ========================== Button Functions ===============================*/
// FUNCTION: Alters the button state according to which button is pressed by the user
//   Debounces input
void buttonRead() {

  // ================== PRESS DETECTION ====================
  // Read the latest button press, store in 'reading'
  bool buttonDetected{ false };
  for (byte i{ 0 }; i < 9; i++) {
    if (digitalRead(buttonPins[i]) == HIGH) {
      reading = i + 1;
      buttonDetected = true;
    }
  }
  //  if a button is not detected, get a reading of 0;
  if (!buttonDetected) {
    reading = 0;
  }


  // Debouncing ---------------
  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
    }
  }

  lastReading = reading;
}


/* ======================== Buzzer Functions ================================*/
void gameStartSound() {
  tone(buzzerPin, NOTE_G5, 120);
  delay(150);
  tone(buzzerPin, NOTE_A5, 120);
  delay(150);
  tone(buzzerPin, NOTE_C6, 250);
  delay(300);
  noTone(buzzerPin);
}

void successSound() {
  tone(buzzerPin, NOTE_C5, 120);
  delay(140);
  tone(buzzerPin, NOTE_E5, 120);
  delay(140);
  tone(buzzerPin, NOTE_G5, 120);
  delay(140);
  tone(buzzerPin, NOTE_B5, 200);
  delay(220);
  tone(buzzerPin, NOTE_C6, 350);
  delay(400);
  noTone(buzzerPin);
}

void failureSound() {
  tone(buzzerPin, NOTE_E4, 250);
  delay(300);
  tone(buzzerPin, NOTE_C4, 400);
  delay(450);
  noTone(buzzerPin);
}

void buttonSound(byte state) {
  tone(buzzerPin, buttonSounds[state], showTimeInterval_ms);
}
