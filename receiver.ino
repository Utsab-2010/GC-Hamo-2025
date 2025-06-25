#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

#define IN1 0   // L298N Motor Driver Input 1
#define IN2 1   // L298N Motor Driver Input 2
#define IN3 7   // L298N Motor Driver Input 3
#define IN4 8   // L298N Motor Driver Input 4

Servo servo1, servo2, servo3, servo4;

int potValue = 90; // Default servo angle
bool specialMode = false; // PP/NM mode active flag

void setup() {
    Serial.begin(9600);

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // Attach servos to PWM pins
    servo1.attach(3);
    servo2.attach(4);
    servo3.attach(5);
    servo4.attach(6);

    // Set servos 3 & 4 to default 90 degrees
    servo3.write(90);
    servo4.write(90);

    // NRF24L01 Setup
    radio.begin();
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_LOW);
    radio.startListening();
}

void loop() {
    if (radio.available()) {
        char receivedMessage[10] = "";
        radio.read(&receivedMessage, sizeof(receivedMessage));
        String message = receivedMessage;
        // Serial.println("Received: " + message);

        // Motor Control
        if (message == "Forward") {
            moveForward();
            Serial.println("Forward");
        } else if (message == "Backward") {
            moveBackward();
            Serial.println("Backward");
        } else if (message == "Left") {
            rotateLeft();
            Serial.println("Left");
        } else if (message == "Right") {
            rotateRight();
            Serial.println("Right");
        }
        else{
            stationary();
        }

        // Special Servo Modes
        if (message == "PP") {
          Serial.println("PP");
            specialMode = true;
            servo1.write(0);
            servo2.write(180);
            servo3.write(0);
            servo4.write(180);
        } else if (message == "NM") {
          Serial.println("NM");
            specialMode = false;
            servo3.write(90);
            servo4.write(90);
        }

        // Potentiometer Control for Servos 1 & 2
        if(!specialMode) {
            Serial.println(message);
            int pot = (int) message[0]; // Convert received string to integer
            // if (!specialMode) {
            potValue = pot;  
            Serial.println(potValue);
            servo1.write(potValue);
            servo2.write(potValue);
            // }
        }
    }
}

// Motor Control Functions
void moveForward() {
    analogWrite(IN1, HIGH);
    analogWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void moveBackward() {
    analogWrite(IN1, LOW);
    analogWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void rotateLeft() {
    analogWrite(IN1, LOW);
    analogWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void rotateRight() {
    analogWrite(IN1, HIGH);
    analogWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void stationary(){
    analogWrite(IN1, LOW);
    analogWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}