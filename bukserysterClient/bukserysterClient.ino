//bukserysterClient.ino
//pulls number of facebook notifications from robottobox and alerts the user's underpants by means of a vibrator
//change server IP and port on line 121.

int vibratorPin=3;

String rx=""; //placeholder for string to integer conversion

#include <ESP8266WiFi.h>

const char* ssid     = "robottoAP";
const char* password = "dobbeltHamster";
const char* host = "bropbox.moore.dk";
const int port = 31337;

//function prototyping
void buzz(int duration);

void setup() {

  WiFi.persistent(false);

  Serial.begin(115200);
  Serial.println("power up.");
  delay(10);
  //set up I/O:
  pinMode(vibratorPin,OUTPUT);

  buzz(500);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Use WiFiClient class to create TCP connections
WiFiClient client;


void loop() {

  delay(5000);

  yield();

  if (!client.connect(host, port)) {
    Serial.println(">>> connection failed");
  }

  unsigned long timeout = millis();

  while (client.available() == 0) {
    if (millis() - timeout > 65000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  if(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}

void buzz(int duration)
{
	digitalWrite(vibratorPin,HIGH);
	delay(duration);
	digitalWrite(vibratorPin,LOW);
	delay(500); //½ a second between each buzz
}

