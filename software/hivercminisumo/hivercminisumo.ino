#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float txValue = 0;
const int button = 0;      // button on PIN G0
const int readPin = 32;    // analog pin G32
const int LEDpin = 2;      // LED on pin G2
bool convert = false;
String rxString = "";
std::string rxValue;       // rxValue gathers input data

// Flags to keep track of motor state
bool moveForward = false;
bool moveBackward = false;
bool moveLeft = false;
bool moveRight = false;

// UART service UUID data
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
 
// Name of each connection, change this to be unique per participant
#define BLENAME "magnoliarobot"

// Pins for motor control, swap as needed depending on how they connected their motor
const int BI1 = 19;  // Motor B direction pin 1
const int BI2 = 18;  // Motor B direction pin 2
const int PWMB = 21; // Motor B PWM pin (D4)
const int AI1 = 26;  // Motor A direction pin 1
const int AI2 = 25;  // Motor A direction pin 2
const int PWMA = 27; // Motor A PWM pin (D2)


void controlMotors() {
  if (moveLeft) {
    digitalWrite(BI1, HIGH);
    digitalWrite(BI2, LOW);
    analogWrite(PWMB, 255);
     digitalWrite(AI1, HIGH);
   digitalWrite(AI2, LOW);
    analogWrite(PWMA, 255);
  } else if (moveRight) {
    digitalWrite(BI1, LOW);
    digitalWrite(BI2, HIGH);
    analogWrite(PWMB, 255);
    digitalWrite(AI1, LOW);
    digitalWrite(AI2, HIGH);
    analogWrite(PWMA, 255);
  } else if (moveForward) {
    digitalWrite(BI1, LOW);
    digitalWrite(BI2, HIGH);
    analogWrite(PWMB, 255);
    digitalWrite(AI1, HIGH);
    digitalWrite(AI2, LOW);
    analogWrite(PWMA, 255);
  } else if (moveBackward) {
    digitalWrite(BI1, HIGH);
    digitalWrite(BI2, LOW);
    analogWrite(PWMB, 255);
    digitalWrite(AI1, LOW);
    digitalWrite(AI2, HIGH);
    analogWrite(PWMA, 255);
  } else {
    digitalWrite(BI1, HIGH);
    digitalWrite(BI2, HIGH);
    analogWrite(PWMB, 0);
    digitalWrite(AI1, HIGH);
    digitalWrite(AI2, HIGH);
    analogWrite(PWMA, 0);
  }
}

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0)  {
      convert = true;      // flag to invoke convertControlpad routine
      Serial.println(" ");
      Serial.print("Received data: ");
      for (int i = 0; i < rxValue.length(); i++) { 
        Serial.print(rxValue[i]);
        rxString = rxString + rxValue[i]; // build string from received data 
      } 
    } 
  } 
}; 

// ***************************** SETUP *******************************
void setup() { 
  Serial.begin(115200); 
  pinMode(LEDpin, OUTPUT); 
  pinMode(button, INPUT);
  pinMode(BI1, OUTPUT);
  pinMode(BI2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(AI1, OUTPUT);
  pinMode(AI2, OUTPUT);
  pinMode(PWMA, OUTPUT);

  BLEDevice::init(BLENAME); // give the BLE device a name
  
  BLEServer *pServer = BLEDevice::createServer(); // create BLE server
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY);                    
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  
  pService->start(); // start the service

  pServer->getAdvertising()->start(); // start advertising
  Serial.println("Waiting a client connection to notify...");
  Serial.println(" ");
}

// *************************** MAIN PROGRAM *********************************
void loop() {
  if (deviceConnected) {
    txValue = analogRead(readPin);        // pick up value on readPin
    char txString[8];                     // convert the value to a char array
    dtostrf(txValue, 1, 2, txString); // float_val, min_width, decimal_digits, char_buffer   
    pCharacteristic->setValue(txString);  // prepare to send array
    if (digitalRead(button) == LOW) {     // send when button is pressed
      pCharacteristic->notify();          // send the value to the app!
      pCharacteristic->setValue(" ");     // send a space
      pCharacteristic->notify();
      digitalWrite(LEDpin, HIGH);         // switch on the LED
      Serial.println(" ");
      Serial.print("*** Peripheral sends: "); 
      Serial.print(txString);             // report value on serial line
      Serial.println(" ***");
    }
    else digitalWrite(LEDpin, LOW);

    // Continuously check the motor flags and control motors
    controlMotors();

    if (convert) convertControlpad();
  }
  delay(50);
}

// ************************* CONVERT CONTROLPAD CODE ************************
void convertControlpad() {
  convert = false;
  Serial.print("      Received string: ");
  Serial.println(rxString); // Print the received string

  if (rxString == "!B516") {
    Serial.println("********** Start Action UP");
    moveForward = true;
  } else if (rxString == "!B615") {
    Serial.println("********** Start Action DOWN");
    moveBackward = true;
  } else if (rxString == "!B714") {
    Serial.println("********** Start Action LEFT");
    moveLeft = true;
  } else if (rxString == "!B813") {
    Serial.println("********** Start Action RIGHT");
    moveRight = true;
  } else if (rxString == "!B606" ||
             rxString == "!B10;" || rxString == "!B20:" || rxString == "!B309" || rxString == "!B408" ||
             rxString == "!B507" || rxString == "!B705" || rxString == "!B804") {
    Serial.println("********** Stop Action");
    moveForward = false;
    moveBackward = false;
    moveLeft = false;
    moveRight = false;
  }

  rxString = "";
}
