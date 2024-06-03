#include <SPI.h>
#include <NativeEthernet.h>
#include <PubSubClient.h>
#include <AccelStepper.h>

// Update these with values suitable for your network.
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Replace with your Ethernet board's MAC address
IPAddress ip(172, 16, 0, 110);
IPAddress server(172, 16, 0, 45);

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

// Scale factor for converting trigger values to motor frequency
float scaleFactor = 2.0; // Initial scale factor (adjust as needed)

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  String payloadStr = "";
  for (unsigned int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  if (topicStr == "IR/Controller/Left/Speed") {
    // Handle left speed
    Serial.print("Left Speed: ");
    Serial.println(payloadStr);
    // Parse the payload as an integer value (trigger value)
    int triggerValue = payloadStr.toInt();
    // Scale the trigger value to motor frequency
    float speed = triggerValue * scaleFactor;
    // Set the left stepper motor's speed (direction is handled by AccelStepper)
    stepperLeft.setMaxSpeed(10000); // Set the maximum speed (adjust as needed)
    stepperLeft.setSpeed(speed);
  } else if (topicStr == "IR/Controller/Right/Speed") {
    // Handle right speed
    Serial.print("Right Speed: ");
    Serial.println(payloadStr);
    // Parse the payload as an integer value (trigger value)
    int triggerValue = payloadStr.toInt();
    // Scale the trigger value to motor frequency
    float speed = triggerValue * scaleFactor;
    // Set the right stepper motor's speed (direction is handled by AccelStepper)
    stepperRight.setMaxSpeed(10000); // Set the maximum speed (adjust as needed)
    stepperRight.setSpeed(speed);
  } else if (topicStr == "IR/Controller/Dir_mode") {
    // Handle direction mode
    Serial.print("Direction Mode: ");
    Serial.println(payloadStr);
    // Add your code to handle the direction mode here
  } else if (topicStr == "IR/Controller/Scale_factor") {
    // Handle scale factor
    Serial.print("Scale Factor: ");
    Serial.println(payloadStr);
    // Parse the payload as a float value (scale factor)
    scaleFactor = payloadStr.toFloat();
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
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      client.subscribe("IR/Controller/Left/Speed");
      client.subscribe("IR/Controller/Right/Speed");
      client.subscribe("IR/Controller/Dir_mode");
      client.subscribe("IR/Controller/Scale_factor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Call client.loop() frequently

  // Update the stepper motor positions
  stepperLeft.run();
  stepperRight.run();

  // Add your code to handle stepper motor movements based on the received MQTT messages
}