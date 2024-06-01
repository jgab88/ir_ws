/*
 Basic MQTT example

 This sketch demonstrates the basic capabilities of the library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 
*/

#include <SPI.h>
#include <NativeEthernet.h>
#include <PubSubClient.h>
#include <AccelStepper.h>

// Define stepper motor pins
#define STEP_PIN_LEFT 2
#define DIR_PIN_LEFT 3
#define STEP_PIN_RIGHT 4
#define DIR_PIN_RIGHT 5

// Initialize the AccelStepper instance
AccelStepper stepperLeft(AccelStepper::DRIVER, STEP_PIN_LEFT, DIR_PIN_LEFT);
AccelStepper stepperRight(AccelStepper::DRIVER, STEP_PIN_RIGHT, DIR_PIN_RIGHT);

// Update these with values suitable for your network.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Replace with your Ethernet board's MAC address
IPAddress ip(192, 168, 0, 110);
IPAddress server(192, 168, 0, 52);

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
    if (frequency > 0) {
      stepperLeft.setMaxSpeed(abs(frequency));
      stepperLeft.setSpeed(abs(frequency));
    } else {
      stepperLeft.setMaxSpeed(abs(frequency));
      stepperLeft.setSpeed(-abs(frequency));
    }
  }
  // Check if the received topic is "Right"
  else if (topicStr == "Right") {
    // Parse the payload as a float value (frequency)
    float frequency = payloadStr.toFloat();

    // Set the right stepper motor's frequency and direction
    if (frequency > 0) {
      stepperRight.setMaxSpeed(abs(frequency));
      stepperRight.setSpeed(abs(frequency));
    } else {
      stepperRight.setMaxSpeed(abs(frequency));
      stepperRight.setSpeed(-abs(frequency));
    }
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();



}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
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

  // Check if the Teensy has obtained an IP address
  IPAddress teensyIP = Ethernet.localIP();
  if (teensyIP == INADDR_NONE) {
    Serial.println("Failed to obtain an IP address");
    // Handle the error condition
  } else {
    Serial.print("Teensy IP address: ");
    Serial.println(teensyIP);
  }
  
  // Set the maximum speed and acceleration for the stepper motors
  stepperLeft.setMaxSpeed(5000); // Replace with your desired maximum speed
  stepperLeft.setAcceleration(100); // Replace with your desired acceleration

  stepperRight.setMaxSpeed(5000); // Replace with your desired maximum speed
  stepperRight.setAcceleration(100); // Replace with your desired acceleration


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