const topics = [
  "iot_vertical_farming_ppu2022/centric/sensor_value",
  "iot_vertical_farming_ppu2022/branch1/sensor_value",
  "iot_vertical_farming_ppu2022/branch2/sensor_value",
];
function connectToMqtt() {
  client = new Paho.MQTT.Client("broker.hivemq.com", Number(8000), "clientId2");
  // set callback handlers
  client.onConnectionLost = onConnectionLost;
  client.onMessageArrived = onMessageArrived;
  // connect the client
  client.connect({ onSuccess: onConnect });
  // called when the client connects
  function onConnect() {
    // Once a connection has been made, make a subscription and send a message.
    console.log("Connected successfully");
    client.subscribe("/World");
    message = new Paho.MQTT.Message("Hello");
    message.destinationName = "/World";
    client.send(message);
    client.subscribe("iot_vertical_farming_ppu2022/centric/manual_big_pump1_water_flow");
    client.subscribe("iot_vertical_farming_ppu2022/centric/manual_small_pump1_acid" );
    client.subscribe("iot_vertical_farming_ppu2022/centric/manual_small_pump2_base");
    client.subscribe("iot_vertical_farming_ppu2022/centric/manual_small_pump3_nutrient");
    client.subscribe("iot_vertical_farming_ppu2022/centric/sensor_value");
    client.subscribe("iot_vertical_farming_ppu2022/branch1/manual_light");
    client.subscribe("iot_vertical_farming_ppu2022/branch1/sensor_value");
    client.subscribe("iot_vertical_farming_ppu2022/branch2/manual_light");
    client.subscribe("iot_vertical_farming_ppu2022/branch2/sensor_value");
    client.subscribe("iot_vertical_farming_ppu2022/RPI/update_firebase_range_value");
  }

  // called when the client loses its connection
  function onConnectionLost(responseObject) {
    if (responseObject.errorCode !== 0) {
      console.log("onConnectionLost:" + responseObject.errorMessage);
    }
  }

  // called when a message arrives
  function onMessageArrived(message) {
    console.log("onMessageArrived:"+ message.topic + message.payloadString);
    if (topics.includes(message.topic)) {
      console.log("includes");
      const obj = JSON.parse(message.payloadString, function (key, value) {
        let elem = document.getElementById(key);
        if (elem != null) {
          elem.innerHTML = value;
        }
      });
      // let elem = document.getElementById(message.topic);
      // {elem.innerHTML=message.payloadString;}
    }
  }
}
function sendMessage(message, topic) {
  var message = new Paho.MQTT.Message(message);
  message.destinationName = topic;
  message.qos = 0;
  client.send(message);
}

function takeAction(myradio) {
  sendMessage(myradio.value, myradio.name);
}

function subscribe() {
    sendMessage(
      '{ "ph": "52", "tds": "25", "water_temperature": "2000" }',
      "iot_vertical_farming_ppu2022/branch1/sensor_value"
    ); // code goes here
}
