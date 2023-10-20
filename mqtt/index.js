const mqtt = require('mqtt')
const express = require("express");
const bodyParser = require("body-parser");
//const res = require("express/lib/response");
const app = express();
const path = require("path");


// Estou dizendo para o Express usar o EJS como View engine
//app.set('view engine','html');
// Body parser
app.use(bodyParser.urlencoded({extended: false}));
app.use(bodyParser.json());

// routes

app.get("/",(req,res)=>{

    var i = req.query.on;
    if(i != undefined){
        post(i);
    }
    var j = req.query.led;
    if(j != undefined){
        post(j);
    }
    console.log(i);
    res.sendFile(path.join(__dirname,'views/index.html'));
    
});


app.listen(80,()=>{
    console.log("app rodando!");
});


/***
    * Browser
    * This document explains how to use MQTT over WebSocket with the ws and wss protocols.
    * EMQX's default port for ws connection is 8083 and for wss connection is 8084.
    * Note that you need to add a path after the connection address, such as /mqtt.
    */
//const url = 'ws://broker.emqx.io:8083/mqtt'
/***
    * Node.js
    * This document explains how to use MQTT over TCP with both mqtt and mqtts protocols.
    * EMQX's default port for mqtt connections is 1883, while for mqtts it is 8883.
    */
 //const url = 'mqtt://broker.emqx.io:1883'
 //const url = 'mqtt://test.mosquitto.org:1883';
 const url = "mqtt://test.mosquitto.org:1883"
 const topic = 'fioresoft/gas';

// Create an MQTT client instance
const options = {
  // Clean session
  clean: true,
  connectTimeout: 4000,
  // Authentication
  clientId: 'fioresoft@454321',
  username: '',
  password: '',
}
var client  = undefined;
client = mqtt.connect(url,options);


if(client != undefined){
    //client.on('connect', function () {
    //    console.log('Connected');
        // Subscribe to a topic
    client.subscribe(topic);
            

    // Receive messages
    client.on('message', function (topic, message) {
        // message is Buffer
        console.log("message received:" + message.toString())
        var obj = JSON.parse(message);
        
        console.log("signal: " + obj.signal);
        console.log(obj.time);
        console.log(Date.now()/1000);
        console.log(obj.time - Date.now()/1000);
        
        // client.end()
    } );

    function post(i)
    {
        var s = {
            "signal":i,
            "time": Date.now()
        }
        if(client == undefined){
            client  = mqtt.connect(url, options);
        }
        console.log("publicando..." + i);
        if(!client.connected){
            client.reconnect();
        }
        // Publish a message to a topic
        client.publish(topic, JSON.stringify(s));
        //client.subscribe(topic);
    }
}
else{
    console.log("Failed to connect to broker");
}

