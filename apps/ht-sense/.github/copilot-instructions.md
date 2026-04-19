# HT Sense Project

The HT Sense is a data visualization dashboard to show humidity and temperature
data collected from an Arduino project.

This is a hobby project for learning Arduino and data visualization.

## Project Organization

The project is organised in three parts:

* arduino
* client
* MQTT broker

### Arduino

The arduino folder contains the source for the Arduino sketch to work with
HT sensor and collect data.

The project is using the PlatformIO IDE (in VSCode).

### Client

The client is a web application using the Lit library to render components.

Mostly the application presents a simple dashboard based
on the EChart.js library (https://echarts.apache.org/) to create a gauge component
used to render the humidity and temperature data collected by a sensor connected
to the Arduino board.

Data will come in as a continuous stream to be consumed using RxJS and rendered
in a Lit element with the gauge component.

The client communicates with the server by subscribing to an MQTT topic using
the MQTT.js library (https://github.com/mqttjs/MQTT.js#readme) to handle the low
level communication.

### MQTT Broker

The MQTT broker is based on a Docker container running EMQX (https://github.com/emqx/emqx)

```shell
docker run -d --name emqx \
  -p 1883:1883 -p 8083:8083 -p 8084:8084 \
  -p 8883:8883 -p 18083:18083 \
  emqx/emqx-enterprise:latest
```

The MQTT broker receives a stream of data from the Arduino R4 WiFi board and
accepts subscriptions (from clients) that intend to consume the data stream.

Events from the Arduino board and later toward the clients can be dispatched
with QoS 0 (fire and forget).
