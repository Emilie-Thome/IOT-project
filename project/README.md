This is the final project. It represents a smart house with its smart tools:
  - an automated lamp
  - a meteo station
  - a watch
The client-server represents the owner of the house.

The testbed:
Nodes                         UID   File                      Frontend
m3-215.grenoble.iot-lab.info  9175  client-server.iotlab-m3   approach_door ; trigger_watch
m3-216.grenoble.iot-lab.info  b369  automated-lamp.iotlab-m3  none
m3-219.grenoble.iot-lab.info  b481  border-router.iotlab-m3   none
m3-222.grenoble.iot-lab.info  3860  watch.iotlab-m3           none
m3-223.grenoble.iot-lab.info  b768  meteo-station.iotlab-m3   meteo

2001:660:5307:3110 was used:
sudo tunslip6.py -v2 -L -a m3-219 -p 20000 2001:660:5307:3110::/64
iotlab-node --update ~/border-router.iotlab-m3 -l grenoble,m3,219
iotlab-node --update ~/watch.iotlab-m3 -l grenoble,m3,222
iotlab-node --update ~/client-server.iotlab-m3 -l grenoble,m3,215
iotlab-node --update ~/meteo-station.iotlab-m3 -l grenoble,m3,223
iotlab-node --update ~/automated-lamp.iotlab-m3 -l grenoble,m3,216


Frontend commands:
aiocoap-client coap://[2001:660:5307:3110::9175]:5683/approach_door
aiocoap-client coap://[2001:660:5307:3110::9175]:5683/trigger_watch
aiocoap-client coap://[2001:660:5307:3110::b768]:5683/meteo