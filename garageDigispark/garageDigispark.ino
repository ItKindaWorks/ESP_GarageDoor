/*    
    garageDoorSensor.ino
    Copyright (c) 2017 ItKindaWorks All right reserved.
    github.com/ItKindaWorks

    garageDoorSensor.ino is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    garageDoorSensor.ino is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with garageDoorSensor.ino.  If not, see <http://www.gnu.org/licenses/>.
*/

//pin for the ping sensor
const int pingPin = 2;

//pin to output the state of the ping sensor at the trigger distance
const int statePin = 1;

//distance which triggers the sensor
const int triggerDistance = 36;

//number of inches registered by the ultrasonic sensor
int inches = 0;

void setup() {
  pinMode(statePin, OUTPUT);
  digitalWrite(statePin, LOW);
  delay(100);
}

void loop() {
  //get a new reading from the sensor
  ping();

  //if the distance is less than the trigger distance then the
  //garage door is open (door is blocking the sensors view) so send
  //a low signal to the ESP
  if(inches < triggerDistance){
    digitalWrite(statePin, LOW);
  }

  //otherwise the garage door is closed and send a high signal to the ESP
  else{
    digitalWrite(statePin, HIGH);
  }

  delay(100);
}




void ping(){
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
}

long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}