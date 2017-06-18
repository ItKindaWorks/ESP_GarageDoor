#include "ESPHelper.h"


//setup macros for time
#define SECOND  1000L
#define MINUTE  SECOND * 60L
#define HOUR  MINUTE * 60L

#define STATUS "/status"
#define RELAY_TOPIC "/home/down/garage"
#define HOSTNAME "RLY-GarageDoor"
#define RELAY_PIN 2
#define SENSOR_PIN 3
#define WAIT_TIME 19L * SECOND

char* relayTopic = RELAY_TOPIC;
String statusTopic;
char replyTopic[50];

const int relayPin = RELAY_PIN;
const int sensorPin = SENSOR_PIN;

//will track the door state but is currently unused.
bool currentState = false;
bool targetState = true;

int currentMode = 1;
int lastPostedMode = 1;

bool running = false;
unsigned long toggleStartTime = 0;

netInfo homeNet = {	.mqttHost = "YOUR MQTT-IP",			//can be blank if not using MQTT
					.mqttUser = "YOUR MQTT USERNAME", 	//can be blank
					.mqttPass = "YOUR MQTT PASSWORD", 	//can be blank
					.mqttPort = 1883,					//default port for MQTT is 1883 - only chance if needed.
					.ssid = "YOUR SSID", 
					.pass = "YOUR NETWORK PASS"};

ESPHelper myESP(&homeNet);

void setup() {

	pinMode(relayPin, OUTPUT);
 	digitalWrite(relayPin, HIGH);

 	pinMode(sensorPin, INPUT);

	//generate status topic string
	statusTopic = relayTopic;
	statusTopic.concat(STATUS);
	statusTopic.toCharArray(replyTopic, 50);

	myESP.OTA_enable();
	myESP.OTA_setPassword("OTA_PASS");
	myESP.OTA_setHostnameWithVersion(HOSTNAME);

	myESP.enableHeartbeat(1);
	myESP.setHopping(false);
	myESP.addSubscription(RELAY_TOPIC);
	myESP.begin();
	myESP.setCallback(callback);

}

	
void loop(){
	if(myESP.loop() == FULL_CONNECTION){
		bool prevState = currentState;

		//get a sensor reading (checks twice just in case the sensor gave a false output for a moment)
		if(digitalRead(sensorPin) != prevState){
			delay(200);
			if(digitalRead(sensorPin) != prevState){
				//if the sensor has really changed then change the door state
				currentState = !currentState;
			}
		}


		//opening or closing
		if(currentState != targetState){

			//if currently open and needs to close, set mode to 3 (closing)
			if(currentState == LOW){
				currentMode = 3;
			}

			//if currently open and needs to close, set mode to 2 (opening)
			else if(currentState == HIGH){
				currentMode = 2;
			}

			//update MQTT if needed
			if(lastPostedMode != currentMode){
				lastPostedMode = currentMode;

				//if currently open, send the signal that the system is closing the door
				if(currentMode == 3){
					myESP.publish(replyTopic, "3");
				}

				//else if currently closed, send the signal to the system that it is opening.
				else if(currentMode == 2){
					myESP.publish(replyTopic, "2");
				}
			}

			//toggle the door state
			toggleDoor();
		}







		//is open or closed
		else{
			//set mode to 0 for door is open
			if(currentState == LOW){
				currentMode = 0;
			}

			//set mode to 1 for door is closed
			else if(currentState == HIGH){
				currentMode = 1;
			}

			//update MQTT if needed
			if(lastPostedMode != currentMode){
				lastPostedMode = currentMode;

				//if currently open, send the signal that the system is closing the door
				if(currentMode == 0){
					myESP.publish(replyTopic, "0");
				}

				//else if currently closed, send the signal to the system that it is opening.
				else if(currentMode == 1){
					myESP.publish(replyTopic, "1");
				}
			}
		}


	}
}


void callback(char* topic, byte* payload, unsigned int length) {

	//open
	if(payload[0] == '0'){
		targetState = false;
	}

	//close
	else if(payload[0] == '1'){
		targetState = true;
	}

	//toggle
	else if(payload[0] == '2'){
		targetState = !targetState;
	}


}

//toggle the garage door between open and closed
void toggleDoor(){
	//check to make sure that we dont trigger a door toggle while it is still moving from a previous command
	if(millis() - toggleStartTime > WAIT_TIME){
		
		//if not running then toggle door and note the start time
		if(!running){
			digitalWrite(relayPin, HIGH);
			delay(100);
			digitalWrite(relayPin, LOW);
			delay(200);

			toggleStartTime = millis();
			running = true;
		}

		//otherwise set running to false
		else{
			running = false;
		}
	}
}




















