/*
TESTING
 
*/

#include <SPI.h>
#include <NativeEthernet.h>
#include <PubSubClient.h>
#include <AccelStepper.h>

// Update these with values suitable for your network.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Replace with your Ethernet board's MAC address
IPAddress ip(192, 168, 0, 110); //Replace with the right ip for the microcontroller
IPAddress server(192, 168, 0, 52); //Replace with the right MQTT server address

// Define stepper motor pins
#define STEP_PIN_LEFT 2
#define DIR_PIN_LEFT 3
#define STEP_PIN_RIGHT 4
#define DIR_PIN_RIGHT 5

// Initialize the AccelStepper instances
AccelStepper stepperLeft(AccelStepper::DRIVER, STEP_PIN_LEFT, DIR_PIN_LEFT);
AccelStepper stepperRight(AccelStepper::DRIVER, STEP_PIN_RIGHT, DIR_PIN_RIGHT);

EthernetClient ethClient;
PubSubClient client(ethClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  String payloadStr = "";

  // Convert the payload to a string
  for (unsigned int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  // Check if the received topic is "Left"
  if (topicStr == "Left") {
    // Parse the payload as a float value (frequency)
    float frequency = payloadStr.toFloat();

    // Set the left stepper motor's frequency and direction
    stepperLeft.setMaxSpeed(abs(frequency));
    if (frequency > 0) {
      digitalWrite(DIR_PIN_LEFT, HIGH); // Set direction pin for clockwise
      stepperLeft.setSpeed(abs(frequency));
    } else {
      digitalWrite(DIR_PIN_LEFT, LOW); // Set direction pin for counterclockwise
      stepperLeft.setSpeed(-abs(frequency));
    }
  }
  // Check if the received topic is "Right"
  else if (topicStr == "Right") {
    // Parse the payload as a float value (frequency)
    float frequency = payloadStr.toFloat();

    // Set the right stepper motor's frequency and direction
    stepperRight.setMaxSpeed(abs(frequency));
    if (frequency > 0) {
      digitalWrite(DIR_PIN_RIGHT, HIGH); // Set direction pin for clockwise
      stepperRight.setSpeed(abs(frequency));
    } else {
      digitalWrite(DIR_PIN_RIGHT, LOW); // Set direction pin for counterclockwise
      stepperRight.setSpeed(-abs(frequency));
    }
  }
}

void setup() {
  Serial.begin(115200); // Use a standard baud rate

  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);

  // Set the maximum acceleration for the stepper motors
  stepperLeft.setAcceleration(100); // Replace with your desired acceleration
  stepperRight.setAcceleration(100); // Replace with your desired acceleration

  // Set the direction pins as outputs
  pinMode(DIR_PIN_LEFT, OUTPUT);
  pinMode(DIR_PIN_RIGHT, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Update the stepper motor positions
  stepperLeft.run();
  stepperRight.run();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      client.subscribe("Left");
      client.subscribe("Right");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}