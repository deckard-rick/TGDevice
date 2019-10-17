#ifndef TGDEVICE_H
#define TGDEVICE_H

#include <tgConfig.hpp>
#include <tgSensor.hpp>
#include <tgActor.hpp>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define NOVAL -9999

class TtgDevice
{
  public:
    TtgDevice(const String& aDeviceVersion);
    void writelog(const String& s, boolean crLF=true);
    void deviceSetup();
    void registerSensors(TtgSensorsList* t_sensors);
    void registerActors(TtgActorsList* t_actor);
    virtual void doCalcStatus();
    boolean httpRequest(const String& url, const String& values, const boolean t_withresponse, String& response);
    void deviceLoop();
  protected:
    TtgDeviceConfig *deviceConfig;
    virtual void doHello();
    void setTimerActive(boolean value=true);
    virtual void doRegister();
    virtual void doAfterConfigChange();
    virtual void doSetup();
    virtual void doLoop();
    int maintime = 0;   //[s]  Wir schalten im Garten unabhängig vom Wochentag, d.h. Verwaltung von 00:00:00 bis 23:59:59 reicht, also 0 bis 86399 Sekunden reicht.
    TtgSensorsList *sensors = NULL;
    TtgActorsList *actors = NULL;
  private:
    String deviceVersion;
    String logModus = "S"; //"":nix "S":serial "<ip:port>"für Debugging via Netzwerk (später)
    String deviceID, wifiSSID, wifiPWD, host;
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
    String urlgettimesec;
    String urlsensordata;
    String urlactordata;
    String htmlHeader();
    String htmlFooter();
    String jsonHeader();
    String getValuesJson(const boolean t_angefordert);
    String getActorsJson(const boolean t_angefordert);
    void serverOnDashboard();
    String getHtmlConfig();
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
