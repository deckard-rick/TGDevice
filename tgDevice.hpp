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
*  In most cases the homeserver do all the managment/controlling of
*  the devices. The better way is, that there is decentralized knowledge
*  in each device. Therefore I dont want to use a homeserver system,
*  (sondern) implementing my own small web based dashboard.
*  Web-Server and devices are connectd via http / json.
*
*  Design
*  TGDevice main class implemented a base devices including http-server
*           for configuration and working with the device
*
*  TGConfig automated Handling of configuraion parameters
*  TGSensor baseclass to mesassure values
*  TGActor  baseclass to controll/switch on/off something, included timer
*  TGCharbuffer buffer to build longer output strings, without Strings
*  TGLogging output to Serial (or TCP-Server later) for debugging
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence (richtige suchen, NO COMMERCIAL USE)
*/

#ifndef TGDEVICE_H
#define TGDEVICE_H

#include <tgCharbuffer.hpp>
#include <tgSensor.hpp>
#include <tgConfig.hpp>
#include <tgActor.hpp>

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
*
* Description of all private stuff in the .cpp file
*/
class TGDevice
{
  public:
    // constructor, parameter is the version of the configuration
    //   after each change of the version, the EEPROM stored configuration values are invalid.
    TGDevice(const char* aDeviceVersion);
    // give the device a list of sensors
    void registerSensorsList(TtgSensorsList* t_sensors);
    // give the device a list of sensors
    void registerActorsList(TtgActorsList* t_actor);
    // setup function, call in main.setup()
    void deviceSetup();
    // loop function, call in main.loop()
    void deviceLoop();
    // to call the http-main-server (device is working without it)
    boolean httpRequest(const char* url, const char* values, const boolean t_withresponse, char* response);
    // to calculate actors (untested)
    virtual void doCalcStatus();
  //protected function can/should be overwritten by a concret device
  //         first example is the project HZLogger, for temperatures measurement
  protected:
    TGDeviceConfig *deviceconfig;            //points to the configuration
    virtual void doHello();                   //put a Hello-Message für the (Serial-)Log there
    void setTimerActive(boolean value=true);  //activate if the device needs time based actions
    virtual void doRegister();                //register your config values, sensors and actors there
    virtual void doAfterConfigChange();       //for work you have todo after configuratione changed
    virtual void doSetup();                   //setup of the derived device, for things not handled by TGDevice
    virtual void doLoop();                    //loop of the derived device, for things not handled by TGDevice
    int maintime = 0;                         //[s]  time of the day in seconds, 00:00:00 - 23:59:59 is 0 - 86399,
                                              //     until now, no weeksdays or dates
    TtgSensorsList *sensors = NULL;           //list of sensors
    TtgActorsList *actors = NULL;             //list of actors
    //TGConfig handles "only" pointers to the configuration values, you need a real storage
    //the four values are protected, because there where initialized from the derived device
    char deviceid[16], wifiSSID[16], wifiPWD[32], host[32]; //main configuration values
  private:
    String logModus = "S";                    //"":no log "S":serial "<ip:port>"logging via ip terminal (later)
    ESP8266WebServer *server = new ESP8266WebServer(80);
    boolean timerActive = false;
    int lastTimeMS = -1;                       //[ms] last call millis()
    int mainTimeMS = 0;                        //[ms] maintime
    int messTime = 60;                         //[s]  time between measures with sensors
    int reportTime = 300;                      //[s]  time before a unchanged value is reported
    int actorTime = 5;                         //[s]
    int lastMessTime = -1, lastActorTime = -1; //[ms] to detect the interval
    int loopDelayMS = 50;                      //[ms] Wie lange der Loop pausiert am Ende
    char urlgettimesec[32] = {'\0'};           //url to get a time from the server/host (future use, format still undefined)
    char urlsensordata[32] = {'\0'};           //url to send sensor values to the server/host (is working)
    char urlactordata[32] = {'\0'};            //url to send actor states to the server/host (future use)
    void htmlHeader();
    void htmlFooter();
    void jsonHeader();
    void jsonSensors(const boolean t_all);
    void jsonActors(const boolean t_all);
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
