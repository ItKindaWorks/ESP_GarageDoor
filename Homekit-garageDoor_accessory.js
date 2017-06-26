var Accessory = require('../').Accessory;
var Service = require('../').Service;
var Characteristic = require('../').Characteristic;
var uuid = require('../').uuid;


////////////////CHANGE THIS INFO FOR YOUR OWN SETUP!!!!!!!!!!!!!//////////////////////////
////////////////CHANGE THIS INFO FOR YOUR OWN SETUP!!!!!!!!!!!!!//////////////////////////
////////////////CHANGE THIS INFO FOR YOUR OWN SETUP!!!!!!!!!!!!!//////////////////////////
var NAME = "Garage Door";                   //This should be unique as it is part of the uuid
var USERNAME = "aa:bb:cc:dd:ee:ff";
var SERIAL = 'aabbcceeff'

var MQTT_IP = 'Your.Mqtt.IP.Address'
var MQTT_ID = 'someUsername' + SERIAL


var activateTopic = '/home/down/garage'
var statusTopic = activateTopic + "/status";
////////////////CHANGE THIS INFO FOR YOUR OWN SETUP!!!!!!!!!!!!!//////////////////////////
////////////////CHANGE THIS INFO FOR YOUR OWN SETUP!!!!!!!!!!!!!//////////////////////////
////////////////CHANGE THIS INFO FOR YOUR OWN SETUP!!!!!!!!!!!!!//////////////////////////


// MQTT Setup
var mqtt = require('mqtt');
var options = {
  port: 1883,
  host: MQTT_IP,
  clientId: MQTT_ID
};

var client = mqtt.connect(options);
client.on('message', function(topic, message) {
  if(topic == statusTopic){
    garageController.setCurrentState(parseInt(message));
  }
});

client.subscribe(statusTopic, {qos: 1});




var garageController = {
  name: NAME, //name of accessory
  pincode: "031-45-154",
  username: USERNAME, // MAC like address used by HomeKit to differentiate accessories. 
  manufacturer: "HAP-NodeJS", //manufacturer (optional)
  model: "v1.0", //model (optional)
  serialNumber: SERIAL, //serial number (optional)

  targetState: Characteristic.CurrentDoorState.CLOSED, //curent power status
  currentState: Characteristic.CurrentDoorState.CLOSED,

  outputLogs: false, //output logs

  //set the target state of the door (Called from IOS)
  setTargetState: function(state) { //set power of accessory

    if(state != this.targetState){

      this.targetState = state;
      
      //tell the garage door to open
      if(state == Characteristic.TargetDoorState.OPEN){
        client.publish(activateTopic, '0', {retain: true});
      }

      //tell the garage door to close
      else if(state == Characteristic.TargetDoorState.CLOSED){
        client.publish(activateTopic, '1', {retain: true});
      }

      this.updateIOS();
    }

  },

  //IOS will call this to get the target state
  getTargetState: function(){
    return this.targetState;
  },

  /*
  OPEN = 0;
  CLOSED = 1;
  OPENING = 2;
  CLOSING = 3;
  */

  //set the current state (called from MQTT)
  setCurrentState: function(state){
    if(state != this.currentState){

      this.currentState = state;
    }
    this.updateIOS();
  },

  //IOS wil call this to get the current state
  getCurrentState: function() { //get power of accessory
    return this.currentState;
  },


  //update the IOS device with the current and target states
  updateIOS: function(){
    garageAccessory
      .getService(Service.GarageDoorOpener)
      .getCharacteristic(Characteristic.TargetDoorState)
      .updateValue(this.targetState);

    garageAccessory
      .getService(Service.GarageDoorOpener)
      .getCharacteristic(Characteristic.CurrentDoorState)
      .updateValue(this.currentState);
  },

  identify: function() { //identify the accessory
    if(this.outputLogs) console.log("Identify the '%s'", this.name);
  }
}







// Generate a consistent UUID for our light Accessory that will remain the same even when
// restarting our server. We use the `uuid.generate` helper function to create a deterministic
// UUID based on an arbitrary "namespace" and the word "garage".
var garageUUID = uuid.generate('hap-nodejs:accessories:garage' + garageController.name);

// This is the Accessory that we'll return to HAP-NodeJS.
var garageAccessory = exports.accessory = new Accessory(garageController.name, garageUUID);

// Add properties for publishing (in case we're using Core.js and not BridgedCore.js)
garageAccessory.username = garageController.username;
garageAccessory.pincode = garageController.pincode;








// tiggers for the target state
garageAccessory
  .addService(Service.GarageDoorOpener, garageController.name)
  .getCharacteristic(Characteristic.TargetDoorState)
  .on('set', function(value, callback) {
    garageController.setTargetState(value);
    callback();
  })


  .on('get', function(callback) {
    callback(null, garageController.getTargetState());
  });


// triggers for the current state
garageAccessory
  .getService(Service.GarageDoorOpener, garageController.name)
  .getCharacteristic(Characteristic.CurrentDoorState)

  .on('get', function(callback) {
    callback(null, garageController.getCurrentState());
  });


//triggers for the accessory info
garageAccessory
  .getService(Service.AccessoryInformation)
    .setCharacteristic(Characteristic.Manufacturer, garageController.manufacturer)
    .setCharacteristic(Characteristic.Model, garageController.model)
    .setCharacteristic(Characteristic.SerialNumber, garageController.serialNumber);

// listen for the "identify" event for this Accessory
garageAccessory.on('identify', function(paired, callback) {
  garageController.identify();
  callback();
});
