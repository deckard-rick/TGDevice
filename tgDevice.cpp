/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
* lesson learned:
* 1) delTime a time after a sensor value is invalid is without sense,
*    because we measure all values every x seconds and then is there a new values
*    if this value is incorrect we have to fix the device
*
* 2) we can work with an internal ID of the sensor and actor, there are unique
*    as tupel with the deviceID. All other handling can be done on the web-system
*
* class TGDevice
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence (richtige suchen, NO COMMERCIAÖ USE)
*/


#include "tgDevice.hpp"

boolean timeTest(int checkTime, int sec)
{
  int ms = millis();
  return (((ms - checkTime) / 1000) > sec) || ((ms - checkTime) < 0);
}

ESP8266WebServer server(IPAddress(192,168,12,32),80);

TGDevice *myDevice = NULL;

void serverOnDashboard()
{
  String html = "<html><body>"
        "<h2>Menu</h2>"
        "<p>[<a href=\"/config\">Konfiguration</a>] "
        "[<a href=\"/getconfig\">Konfiguration (json)</a>] "
        "[<a href=\"/getvalues\">Werte (json)</a>] "
        "[<a href=\"/getactors\">Aktoren (json)</a></p>"
        "</body></html>";

  server.send(200, "text/html", html);
}

/**
* TGDevice constructor
* param const String& t_deviceversion Version of the devices
*  stored cofiguration in the devices has to lost if the configuration
*  changes the structure
*  Also stored in device and not only in configuration for better access
*  and sending with html/json
*/
TGDevice::TGDevice(const String& t_deviceversion)
{
  deviceversion = t_deviceversion;
  deviceconfig = new TtgDeviceConfig(t_deviceversion);
  myDevice = this;
}

/**
* public void writelog for logging/Debugging
* param const String& t_s text to log
* param boolean t_crlf  linw ends with CRLF (or not)
*
* uses logModus N=NO, S=serial, T=tcpServer
*
* TODO
* * perhaps as static function for access from other classes
* * modus T for sendeing Messagges to an tcpIP-Terminal
*/
void TGDevice::writelog(const String& s, boolean crLF)
{
  if (logModus == "S")
    if (crLF) Serial.println(s);
    else Serial.print(s);
}

/**
* public void registerSensors
* param TGSensorsList *t_sensors list to register
*
* registered the list of sensors, by this it is possible to use derives classes
*/
void TGDevice::registerSensors(TtgSensorsList* t_sensors)
{
  sensors = t_sensors;
}

/**
* public void registerActors
* param TGActorsList *t_actors list to register
*
* registered the list of actors, by this it is possible to use derives classes
*/
void TGDevice::registerActors(TtgActorsList* t_actors)
{
  actors = t_actors;
}

/**
* public void deviceSetup() called from the Arduino setup function
*
* TODO
* * TimeOut for Wifi connectig
* * establish as Server if WiFi Client is not posible
*/
void TGDevice::deviceSetup()
{
  //init serial device if neccessary for logging
  if (logModus == "S")
    Serial.begin(9600);

  writelog("wait 10s");
  delay(10000);

  //derived classes can print a hello message after boot to logging
  writelog("doHello");
  doHello();

  //register all config-parameter, sensors and actors with derived classe
  writelog("doRegister");
  doRegister();

  //initialize EEPROM for read/write configuration for using after reboot
  //writelog("init EEPROM");
  //EEPROM.begin(deviceconfig->getEEPROMSize());

  //loas configuration from EEPROM if possible (else default values)
  //writelog("load configuration");
  //
  //deviceconfig->readEEPROM();

  //calculation in the configuration after changed, for example timetables
  writelog("doAfterConfigChange");
  doAfterConfigChange();

  //establish WiFi connection
  writelog("WiFi: "+String(wifiSSID)+" ("+String(wifiPWD)+")");
  WiFi.begin(String(wifiSSID),String(wifiPWD));
  writelog("<",false);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    writelog(".",false);
   }
  writelog(">");
  writelog("WiFi connected IP:",false);
  writelog(WiFi.localIP().toString());

  //initialize http-Server
  writelog("init Server");
  //https://stackoverflow.com/questions/32900314/esp8266webserver-setting-a-value-inside-a-class
  //server.on("/data.html", std::bind(&WebServer::handleSomeData, this));
  //server.on("/",std::bind(&TGDevice::serverOnDashboard, this));
  server.on("/",serverOnDashboard);
  //server.on("/config",std::bind(&TGDevice::serverOnConfig, this));
  //server.on("/saveconfig",std::bind(&TGDevice::serverOnSaveConfig, this));
  //server.on("/writeconfig",std::bind(&TGDevice::serverOnWriteConfig, this));
  //server.on("/getconfig",std::bind(&TGDevice::serverOnGetConfig, this));
  //server.on("/putconfig",std::bind(&TGDevice::serverOnPutConfig, this));
  //if ((sensors != NULL) and sensors->hasMembers())
  //  server.on("/getvalues",std::bind(&TGDevice::serverOnGetValues, this));
  if ((actors != NULL) and actors->hasMembers())
    {
      //server.on("/getactors",std::bind(&TGDevice::serverOnGetActors, this));
      //server.on("/setactor",std::bind(&TGDevice::serverOnSetActor, this));
    }

  //do other setup work, for examples http-server here and sensors in derived class
  writelog("doSetup");
  doSetup();

  //start http-Server
  writelog("start http-server");
  server.begin();

  //wait a moment
  writelog("wait 250");
  delay(250);

  //initializing finished
  writelog("end initialisation");
}

/**
* protected void doHello for hello message after reboot
*/
void TGDevice::doHello()
{
  writelog("start initialisation");
}

/**
* protected doRegister, registration of parameters, sensors and t_actors
*/
void TGDevice::doRegister()
{
  //Default configuration parameter this base device is using
  deviceconfig->addConfig("deviceID","S",16,false,"Gerätename",deviceID,NULL,NULL);
  deviceconfig->addConfig("wifiSSID","S",16,true,"Netzwerkkennung",wifiSSID,NULL,NULL);
  deviceconfig->addConfig("wifiPWD","S",32,true,"Netzwerkpasswort",wifiPWD,NULL,NULL);
  deviceconfig->addConfig("host","S",32,false,"Host für Konfig, Daten und Dashboard",host,NULL,NULL);
  deviceconfig->addConfig("loopDelay","I",0,false,"[ms] Pause des Loops",NULL,&loopDelayMS,NULL);

  //if timer is active server page which delivers time in secs
  if (timerActive)
    deviceconfig->addConfig("urlgettimesec","S",32,false,"Webseite von der der Host die aktuellen Sekunden liefert",urlgettimesec,NULL,NULL);

  //parameter needed for sensors handling is sensors are registered
  if ((sensors != NULL) and sensors->hasMembers())
    {
      deviceconfig->addConfig("messtime","I",0,false,"alle wieviel Sekunden gemessen wird",NULL,&messTime,NULL);
      deviceconfig->addConfig("reporttime","I",0,false,"nach wieviel Sekunden der Wert auch ohne Änderung über Delta reported wird",NULL,&reportTime,NULL);
      deviceconfig->addConfig("urlsensordata","S",32,false,"Webseite an die die Werte reported werden",urlsensordata,NULL,NULL);
    }

  //parameter needed for actors handling is sensors are registered
  if ((actors != NULL) and actors->hasMembers())
    {
      deviceconfig->addConfig("actortime","I",0,false,"Sekunden nach denen auf Auto-Aktion geprüft wird",NULL,&messTime,NULL);
      deviceconfig->addConfig("urlactordata","S",32,false,"Webseite an die Aktionen reported werden",urlactordata,NULL,NULL);
    }
}

/**
* protected void doCalcStatus entry point for derived classes to calculate
*    actor stati depending on other sensors/actors
*
* TODO is that really necessary, or is it only needed in GardenMainDevice
*/
void TGDevice::doCalcStatus()
{
}

/**
* protected void doAfterConfigChange entry point for derived classes
*    actor configuration changed for recalculating depending values
*/
void TGDevice::doAfterConfigChange()
{
}

/**
* protected void doSetuo entry point for derived classes
*    to setup sensors and actors
*/
void TGDevice::doSetup()
{
}

/**
* private String htmlHeader
*    default HTML-Header
*/
String TGDevice::htmlHeader()
{
  String html = "<html><body>";
  //html += "<h1>Device ID:"+String(deviceID)+" Version("+deviceversion+")</h1>";
  return html;
}

/**
* private String htmlFooter
*    default HTML-Footer
*/
String TGDevice::htmlFooter()
{
  String html = "<p>[<a href=\"/\">Main</a>]</br>";
  //html += "<small>Copyright Andreas Tengicki 2018-, NO COMMERCIAL USE</small>";
  html += "</body></html>";
  return html;
}

void TGDevice::onDashboard()
{
  writelog("serverOnDashboard - start");
  //String html = htmlHeader();
  //html += "<h2>Dashboard</h2>";
  //if (sensors != NULL)
  //  html += sensors->getHTML();
  //if (actors != NULL)
  //  html += actors->getHTML();
  String html = "<html><body>"
        "<h2>Menu</h2>"
        "<p>[<a href=\"/config\">Konfiguration</a>] "
        "[<a href=\"/getconfig\">Konfiguration (json)</a>] "
        "[<a href=\"/getvalues\">Werte (json)</a>] "
        "[<a href=\"/getactors\">Aktoren (json)</a></p>"
        "</body></html>";
  //html += htmlFooter();

  server.send(200, "text/html", html);

  writelog("serverOnDashboard - ende");
}

String TGDevice::getHtmlConfig()
{
  String html = htmlHeader();
  html += deviceconfig->getHtmlForm();
  html += htmlFooter();
  return html;
}

void TGDevice::serverOnConfig()
{
  writelog("serverOnConfig");
  server.send(200, "text/html", getHtmlConfig());
}

void TGDevice::serverOnSaveConfig()
{
  writelog("serverOnSaveConfig");
  for(int i=0; i<server.args(); i++)
    deviceconfig->setValue(server.argName(i), server.arg(i));
  doAfterConfigChange();
  server.send(200, "text/html", getHtmlConfig());
}

void TGDevice::serverOnWriteConfig()
{
  writelog("serverOnWriteConfig");
  deviceconfig->writeEEPROM();
  server.send(200, "text/html", getHtmlConfig());
}

String TGDevice::jsonHeader()
{
  writelog("jsonHeader");
  String json = "{ \"Version\": \""+deviceversion+"\", \"DeviceID\": \""+deviceID+"\"";
  return json;
}

void TGDevice::serverOnGetConfig()
{
  writelog("serverOnGetConfig");
  boolean all = false;
  if (server.args() > 0)
    if (server.argName(0) == "all");
      all = server.arg(0) == "Y";

  String json = jsonHeader() + ", " + deviceconfig->getJson(all) + " }";
  server.send(200, "application/json", json);
}

void TGDevice::serverOnPutConfig()
{
  writelog("serverOnPutConfig");
  /*
  * 07.10.2019
  * Damit (also ohne Wiederholgruppen, können wir das lesen vereinfachen
  * { setzt auf Modus 1, dann kommt ein Feldname, dann :, dann ein Wert und ggf ein ,
  * } können wir überlesen, wenn ein feld1 : { feld2 kommt, führt das reset zum lesen von feld2
  */
  String json = ""; //hier muss der POST-Body der Abfrage rein

  deviceconfig->putJson(json);

  //calculateTimes();
}

String TGDevice::getValuesJson(const boolean t_angefordert)
{
  writelog("getValuesJson");
  return jsonHeader() + ", " + sensors->getJson(t_angefordert) + " }";
}

void TGDevice::serverOnGetValues()
{
  writelog("serverOnGetValues");
  server.send(200, "application/json", getValuesJson(true));
}

String TGDevice::getActorsJson(const boolean t_angefordert)
{
  return jsonHeader() + ", " + actors->getJson(t_angefordert) + " }";
}

void TGDevice::serverOnGetActors()
{
  writelog("serverOnGetActors");
  server.send(200, "application/json", getActorsJson(true));
}

/**
 *   Syntax
 *   setActor?id=<id>[&status=<status>][%endtime=<endtime>]
 */
void TGDevice::serverOnSetActor()
{
  String id="";
  String status="";
  String endtime="";

  for(int i=0; i<server.args(); i++)
    if (server.argName(i) == "id")
      id = server.arg(i);
    else if (server.argName(i) == "status")
      id = server.arg(i);
    else if (server.argName(i) == "endtime")
      id = server.arg(i);

  if ((id != "") and (status != ""))
    actors->setStatus(id,(char)(status[0]));
  if ((id != "") and (endtime != ""))
    actors->setEndtime(id,endtime.toInt());

  server.send(200, "text/html", getActorsJson(false));
}

// http JSON Post:
// https://techtutorialsx.com/2016/07/21/esp8266-post-requests/
boolean TGDevice::httpRequest(const char* url, const String& values, const boolean t_withresponse, String& response)
{
  boolean erg = false;

  writelog("httpRequest");
  writelog("host: \""+String(host)+"\"");
  writelog("url: \""+String(url)+"\"");

  if ((String(host) == "") or (String(url)==""))
    return erg;

  String aurl = String(host)+"\""+String(url);
  writelog("Value Request to: \""+aurl+"\"");

  return erg;

  HTTPClient *http = new HTTPClient;    //Declare object of class HTTPClient

  writelog("Values: \""+values+"\"\n");

  http->begin(url);   //Specify request destination

  int httpCode = 0;
  if (values != "")
    httpCode = http->GET();
  else
    {
      http->addHeader("Content-Type", "application/json");  //Specify content-type header
      httpCode = http->POST(values);
    }

  writelog("httpCode:"+String(httpCode)+":\n");
  if (httpCode < 0)
    writelog("[HTTP] ... failed, error: "+http->errorToString(httpCode)+"\n");

  if (t_withresponse)
    {
      response = http->getString();          //Get the response payload
      writelog("Response:"+response+":\n");
    }

  http->end();  //Close connection

  erg = true;

  return erg;
}

void TGDevice::setTimerActive(boolean value)
{
  timerActive = value;
}

void TGDevice::timer()
{
  if (!timerActive)
    return;

  int ms = millis();
  mainTimeMS += (ms - lastTimeMS);
  maintime = mainTimeMS / 1000;
  if ((lastTimeMS = -1) or (maintime > 86399) or (lastTimeMS > ms))
    {
      String httpResponse = "";
      httpRequest(urlgettimesec, "", true, httpResponse);
      //Fraglich ist was geben ich hier zurück, am einfachsten wären unverpackt die Sekunden, oder halt xml oder noch besser json
      maintime = httpResponse.toInt();
      mainTimeMS = maintime * 1000;
    }
  lastTimeMS = ms;
}

void TGDevice::deviceLoop()
{
  //timer();

  //writelog("handleClient");
  server.handleClient();

  if ((sensors != NULL) and sensors->hasMembers())
    {
      //writelog("in sensor Loop");
      boolean needReporting = false;
      if (timeTest(lastMessTime,messTime))
        {
          needReporting = sensors->messWerte();
          lastMessTime = millis();
        }

      //Es können ja immer Werte auch ohne Messung in das repotTime Fenster rutschen, also immer prüfen
      needReporting = needReporting or sensors->checkReporting(reportTime);
      //Wegen der StringVerabeitung sollten wir aber prüfen ob was gesendet werden muss, bevor wir Strings bauen
      if (needReporting)
        {
          String d_response = "";
          //httpRequest(urlsensordata, getValuesJson(false), false, d_response);
        }
    }

  if ((actors != NULL) and actors->hasMembers())
    {
      //writelog("in actors Loop");
      if (timeTest(lastActorTime,actorTime))
        {
          boolean needReporting = actors->action();

          if (needReporting)
            {
              String d_response = "";
              //httpRequest(urlactordata, getActorsJson(false), false, d_response);
            }
        }
    }

  //writelog("doLoop");
  doLoop();

  //writelog("delay");
  delay(loopDelayMS);
}

void TGDevice::doLoop()
{

}
