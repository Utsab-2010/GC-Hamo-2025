#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
int i = 0;
// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI  3
#define OLED_CLK   5
#define OLED_DC    6
#define OLED_CS    A1
#define OLED_RESET A2
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

#define BTN_F 4     // Forward button
#define BTN_B 7     // Backward button
#define BTN_INT 2   // Interrupt button (must be pin 2 or 3 on Arduino Uno)
#define POT A0      // Potentiometer

volatile bool interruptMode = false;  // Flag for interrupt mode
volatile bool motor_move =false;

int modeState = LOW;
int lastmodeState = LOW;

void modeChange(int currentState, int previousState) {

    // Falling Edge Detection (HIGH to LOW)
    if (currentState == LOW && previousState == HIGH) {
        // Serial.println("Falling Edge Detected");
        interruptMode = !interruptMode;
        char message[3];                // Message buffer
        if (interruptMode==true) {
            strcpy(message, "PP");      // Send "PP" when entering interrupt mode
        } else if(interruptMode==false) {
            strcpy(message, "NM");      // Send "NM" when returning to normal mode
        }

        if (radio.write(&message, sizeof(message))) {
            Serial.print("Sent: ");
            Serial.println(message);
            display.print("Mode:");
            display.println(message);
        } else {
            Serial.println("Failed to send message.");
        }
        display.display();
        delay(2000);
        // Call your falling edge function here
    }
}


void setup() {
    Serial.begin(9600);
    Serial.println("Transmitter Ready");

    display.begin(SSD1306_SWITCHCAPVCC);
    display.display();
    pinMode(BTN_F, INPUT_PULLUP);
    pinMode(BTN_B, INPUT_PULLUP);
    pinMode(BTN_INT, INPUT_PULLUP);
    pinMode(POT, INPUT);

    // NRF24L01 setup
    radio.begin();
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_LOW);
    radio.stopListening();
    delay(2000);
}

void loop() {

    display.clearDisplay();
    display.setTextSize(2); // Set text size
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(54,5);
    if (interruptMode) {
        display.print("PP");      // Send "PP" when entering interrupt mode
    } else {
        display.print("NM");      // Send "NM" when returning to normal mode
    }
    
    display.setCursor(24, 32);
    
    char message[10];
    modeState = digitalRead(BTN_INT);
    modeChange(modeState,lastmodeState);

    if ((digitalRead(BTN_B) == digitalRead(BTN_F)) && motor_move) {
        strcpy(message, "Stop");
        if (radio.write(&message, sizeof(message))) {
            Serial.print("Sent: ");
            Serial.println(message);
            display.println(message);
        } else {
            Serial.println("Failed to send message.");
            display.print("Failed!");
        }
        motor_move=false;
    }
    else if (digitalRead(BTN_F) == LOW) {
        strcpy(message, interruptMode ? "Left" : "Forward");
        if (radio.write(&message, sizeof(message))) {
            Serial.print("Sent: ");
            Serial.println(message);
            display.println(message);

        } else {
            Serial.println("Failed to send message.");
            display.print("Failed!");
        }
        motor_move=true;
    }
    else if (digitalRead(BTN_B) == LOW) {
        strcpy(message, interruptMode ? "Right" : "Backward");
        if (radio.write(&message, sizeof(message))) {
            Serial.print("Sent: ");
            Serial.println(message);            
            display.println(message);


        } else {
            Serial.println("Failed to send message.");
            display.print("Failed!");
        }
        motor_move=true;
    }
    else if (!interruptMode) {
        int potValue = analogRead(POT);
        potValue = map(potValue, 0, 1023, 0, 180);
        char potMessage[10];
        snprintf(potMessage, sizeof(potMessage), "%d", potValue);
        if (radio.write(&potMessage, sizeof(potMessage))) {
            Serial.print("Sent Pot Value: ");
            Serial.println(potMessage);
            display.print("POT:");
            display.println(potMessage);

        } else {
            Serial.println("Failed to send Pot Value.");
            display.print("Failed!");
        }
    }
    lastmodeState = modeState;
    display.display();
    delay(50); // Small delay to avoid bouncing

}
