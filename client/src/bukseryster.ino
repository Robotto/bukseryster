//bukseryster.ino
//pulls number of facebook notifications from robottobox and alerts the user's underpants by means of a vibrator
//change server IP and port on line 121.

enum fonaStates {
	off,
	on,
	connected,
	gprsActive
};

//APN settings for /some/ danish service providers: http://www.appsandroid.dk/apn-indstillinger-mmsgprs.html

//FONA PINOUT:
//int VioPin=X; //determines I/O voltage, set to VCC of arduino
int KeyPin=16; //power on/off pin - pull to GND for 2 seconds to toggle power for FONA
int PSPin=14; //Power status pin 0=off 1=on
int NSPin=10; //Network status pin:
int ResetPin=15; //Pull low for 100ms to hard reset the fona
//int RIPin=X; //Ring indicator pin, will pulse low for 120ms when a call is received. It can also be configured to pulse when an SMS is received.

int vibratorPin=3;

String rx=""; //placeholder for string to integer conversion
unsigned long statusTimer=0; //for telling what state the fona board is in
fonaStates status=off; //startup state is off.. probably..

void setup()
{
	//while(!Serial);

	Serial.begin(115200);
	Serial1.begin(19200);
	Serial1.setTimeout(5000);

	Serial.println("power up.");

	//set up I/O:
	TX_RX_LED_INIT; //built in macro for the pro micro (leonardo)
	pinMode(PSPin,INPUT);
	pinMode(NSPin,INPUT);
	digitalWrite(KeyPin,HIGH); //set high before setting to output, to avoid low-pulse and accidentally powering on the FONA too soon
	pinMode(KeyPin,OUTPUT);
	pinMode(ResetPin,OUTPUT);
	pinMode(vibratorPin,OUTPUT);

	if(!digitalRead(PSPin)) //if FONA is off, turn it on.
	{
		Serial.println("Fona is off.. Turning on..");
		digitalWrite(KeyPin,LOW);
		delay(2000);
		digitalWrite(KeyPin,HIGH);
	}

	attachInterrupt(4,updateFonaStatus,RISING); //attach status pin (NSPin) interrupt on pin 7

	while(status==off) ShowSerialData();

	Serial.println("Fona is on..");

	Serial.println("Awaiting status: Connected.");
	while(status!=connected) ShowSerialData();
	Serial.println("Connected!");

	delay(3000);
	ShowSerialData();
	ShowSerialData();
	ShowSerialData();

	Serial1.println("at+cstt=\"internet\""); //configure GPRS
	delay(1000);
	Serial.println();
	ShowSerialData();

	Serial1.println("at+ciicr"); //bring up GPRS connection
	delay(1000);
	Serial.println();
	ShowSerialData();
	Serial.println();

	Serial.println("Awaiting Active GPRS connection.");
	while(status!=gprsActive) ShowSerialData();
	delay(1000);

	Serial1.println("at+cifsr"); //get IP address
	delay(1000);
	Serial.print("IPv4: ");
	ShowSerialData();
}



void loop()
{
	int likes=getLikes();
	for(int i=0;i<likes;i++) buzz(1000); //one buzz per like
	Serial.print("Notifications: ");
	Serial.println(likes);
    delay(300000); //5 minutes

}
void buzz(int duration)
{
	digitalWrite(vibratorPin,HIGH);
	delay(duration);
	digitalWrite(vibratorPin,LOW);
	delay(500); //½ a second between each buzz
}

void ShowSerialData()
{
  while(Serial1.available()!=0)
  {
    delay(50); //seems to fit the timing of the FONA board nicely
    Serial.write(Serial1.read());
  }
}

int getLikes(void)
{
	Serial1.println("at+cipstart=\"tcp\",\"62.212.66.171\",\"31337\""); //init conn
	delay(1000);
	ShowSerialData();
	Serial1.println("at+cipsend"); //send some data
	delay(250);
	ShowSerialData();
	Serial1.print("yes hello.");
	ShowSerialData();
	Serial1.println((char)26);
	ShowSerialData();
	delay(2500);
	
	//HERE's where the magic happens:
	while(!Serial1.available());

	int inChar=Serial1.read();
	String rx = ""+(char)inChar;
	int likes = rx.toInt();

	//End of magic
	Serial1.println("at+cipclose");

	delay(500);
	ShowSerialData();
	ShowSerialData();

	return likes;

}

//	int NSPin=10; //Network status pin:
/*  NSPIN:
	64ms on, 800ms off - the module is running but hasn't made connection to the cellular network yet
	64ms on, 3 seconds off - the module has made contact with the cellular network and can send/receive voice and SMS
	64ms on, 300ms off - the GPRS data connection you requested is active
	By watching the blinks you can get a visual feedback on whats going on.
*/
void updateFonaStatus(void)
{
	unsigned long timeSinceLastRisingEdge=millis()-statusTimer;
	if(timeSinceLastRisingEdge>1500) status=connected;
	else if(timeSinceLastRisingEdge>500) status=on;
	else status=gprsActive;
	statusTimer=millis();
}