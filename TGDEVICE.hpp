/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
*  Motivation
*  There are no results by loking for base classes (or a framework) to
*  easy implement IoT-Devices with sensors and actors with an
*  ESP8622 (NodeMCU) with Arduino.
*  All examples/systems founds are not object orientated, therefor is
*  many copy and paste needed by implementing more than one device and/or
*  they are connected to special servers via special protocolls
*  In most cases the homeserver do all the managment/controlling of they
*  the devices. The better way is, that there is decentalized knowledge
*  in each device. Therefore I dont want to use a homeserver system,
*  (sondern) implementing my own small web based dashboard.
*  Web-Server and devices are connectd via http in json.
*
*  Design
*  TGDevice main class implemented a base devices including http-server
            for configuration and working with the device
*  TGConfig automated Handling og configuraion parameters
*  TGSensor baseclass to mesassure Values
*  TGActor  baseclass to controll/switch on/off sometging, included timer
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence (richtige suchen, NO COMMERCIAÖ USE)
*/

#ifndef TGDEVICE_H
#define TGDEVICE_H

#include <TGCharbuffer.hpp>
#include <TGSensor.hpp>
#include <TGConfig.hpp>
#include <TGActor.hpp>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define NOVAL -9999

/**
* class TGDevice
*
* Main CLass implements a (base) device, includes
* * configuration
* * get values from sensors
* * set actors
* * automated dashboard/menu via http on the device
*/
class TGDevice
{
  public:
    TGDevice(const char* aDeviceVersion);
    void writelog(const String& s, boolean crLF=true);
    void registerSensorsList(TtgSensorsList* t_sensors);
    void registerActorsList(TtgActorsList* t_actor);
    void deviceSetup();
    virtual void doCalcStatus();
    boolean httpRequest(const char* url, const char* values, const boolean t_withresponse, char* response);
    void deviceLoop();
  protected:
    TtgDeviceConfig *deviceconfig;
    virtual void doHello();
    void setTimerActive(boolean value=true);
    virtual void doRegister();
    virtual void doAfterConfigChange();
    virtual void doSetup();
    virtual void doLoop();
    int maintime = 0;   //[s]  Wir schalten im Garten unabhängig vom Wochentag, d.h. Verwaltung von 00:00:00 bis 23:59:59 reicht, also 0 bis 86399 Sekunden reicht.
    TtgSensorsList *sensors = NULL;
    TtgActorsList *actors = NULL;
    char deviceid[16], wifiSSID[16], wifiPWD[32], host[32];
  private:
    String logModus = "S"; //"":nix "S":serial "<ip:port>"für Debugging via Netzwerk (später)
    ESP8266WebServer *server = new ESP8266WebServer(80);
    boolean timerActive = false;
    int lastTimeMS = -1;
    int mainTimeMS = 0; //[ms] damit die Weiterschaltung in ms funnktioniert, aber
    int messTime = 60; //[s] 1m
    int reportTime = 300; //[s] 5m
    int delTime = 600; //[s] 10m
    int actorTime = 5; //[s]
    int lastMessTime = -1, lastActorTime = -1;
    int loopDelayMS = 50; //[ms] Wie lange der Loop pausiert am Ende
    char urlgettimesec[32] = {'\0'};
    char urlsensordata[32] = {'\0'};
    char urlactordata[32] = {'\0'};
    void htmlHeader();
    void htmlFooter();
    void jsonHeader();
    void jsonSensors(const boolean t_angefordert);
    void jsonActors(const boolean t_angefordert);
    void htmlConfig();
    void serverOnDashboard();
    void serverOnConfig();
    void serverOnSaveConfig();
    void serverOnWriteConfig();
    void serverOnGetConfig();
    void serverOnPutConfig();
    void serverOnGetValues();
    void serverOnGetActors();
    void serverOnSetActor();
    void timer();
};

#endif
