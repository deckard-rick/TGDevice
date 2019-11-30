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

#include <tgDevice.hpp>
#include <tgLogging.hpp>

char htmlHeader1[] = "<html><body><h1>Device ID:#deviceid# Version(#deviceversion#)</h1>";
char htmlFooter1[] = "<p>[<a href=\"/\">Main</a>]</br>[<a href=\"/config\">Configuration</a>][<a href=\"/getconfig\">Configuration (json)</a>]";
char htmlFooter2[] = "[<a href=\"/getvalues\">Values (json)</a>]";
char htmlFooter3[] = "[<a href=\"/getactors\">Actors (json)</a>]";
char htmlFooter4[] = "</p><p><small>active since #value#ms.<br/>";
char htmlFooter5[] = "Copyright Andreas Tengicki 2018-, NO COMMERCIAL USE</small></p></body></html>";

char htmlDashboard[] = "<h2>Dashboard</h2>";

char jsonHeader1[] = "{ \"Version\": \"#deviceversion#\", \"deviceid\": \"#deviceid#\", \"millis\": \"#millis#\"";

TGCharbuffer outbuffer;

bool timeTest(int checkTime, int sec)
{
  int ms = millis();
  return (((ms - checkTime) / 1000) > sec) || ((ms - checkTime) < 0);
}

/**
* TGDevice constructor
* param const String& t_deviceversion Version of the devices
*  stored cofiguration in the devices has to lost if the configuration
*  changes the structure
*  Also stored in device and not only in configuration for better access
*  and sending with html/json
*/
TGDevice::TGDevice(const char* t_deviceversion)
{
  deviceconfig = new TtgDeviceConfig(t_deviceversion);
}

/**
* public void registerSensorsList
* param TGSensorsList *t_sensors list to register
*
* registered the list of sensors, by this it is possible to use derives classes
*/
void TGDevice::registerSensorsList(TtgSensorsList* t_sensors)
{
  sensors = t_sensors;
}

/**
* public void registerActorsList
* param TGActorsList *t_actors list to register
*
* registered the list of actors, by this it is possible to use derives classes
*/
void TGDevice::registerActorsList(TtgActorsList* t_actors)
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
  TGLogging::get()->setModus('S');

  TGLogging::get()->write("wait 5s")->crlf();
  delay(5000);

  //derived classes can print a hello message after boot to logging
  TGLogging::get()->write("doHello")->crlf();
  doHello();

  //register all config-parameter, sensors and actors with derived classe
  TGLogging::get()->write("doRegister")->crlf();
  doRegister();

  //initialize EEPROM for read/write configuration for using after reboot
  TGLogging::get()->write("init EEPROM")->crlf();
  EEPROM.begin(deviceconfig->getEEPROMSize());

  //loas configuration from EEPROM if possible (else default values)
  //TGLogging::get()->write("load configuration");
  //
  //deviceconfig->readEEPROM();

  //calculation in the configuration after changed, for example timetables
  TGLogging::get()->write("doAfterConfigChange")->crlf();
  doAfterConfigChange();

  //establish WiFi connection
  TGLogging::get()->write("WiFi: ")->write(wifiSSID)->write(" (")->write(wifiPWD)->write(")");
  WiFi.begin(String(wifiSSID),String(wifiPWD));
  TGLogging::get()->write("[");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    TGLogging::get()->write(".");
   }
  TGLogging::get()->write("]")->crlf();
  TGLogging::get()->write("WiFi connected IP:")->write(WiFi.localIP().toString())->crlf();

  //initialize http-Server
  TGLogging::get()->write("init Server")->crlf();
  //https://stackoverflow.com/questions/32900314/esp8266webserver-setting-a-value-inside-a-class
  //server->on("/data.html", std::bind(&WebServer::handleSomeData, this));
  server->on("/",std::bind(&TGDevice::serverOnDashboard, this));
  server->on("/config",std::bind(&TGDevice::serverOnConfig, this));
  server->on("/saveconfig",std::bind(&TGDevice::serverOnSaveConfig, this));
  server->on("/writeconfig",std::bind(&TGDevice::serverOnWriteConfig, this));
  server->on("/getconfig",std::bind(&TGDevice::serverOnGetConfig, this));
  server->on("/putconfig",std::bind(&TGDevice::serverOnPutConfig, this));
  if ((sensors != NULL) and sensors->hasMembers())
    server->on("/getvalues",std::bind(&TGDevice::serverOnGetValues, this));
  if ((actors != NULL) and actors->hasMembers())
    {
      server->on("/getactors",std::bind(&TGDevice::serverOnGetActors, this));
      server->on("/setactor",std::bind(&TGDevice::serverOnSetActor, this));
    }

  //do other setup work, for examples http-server here and sensors in derived class
  TGLogging::get()->write("doSetup")->crlf();
  doSetup();

  //start http-Server
  TGLogging::get()->write("start http-server")->crlf();
  server->begin();

  //wait a moment
  TGLogging::get()->write("wait 1s")->crlf();
  delay(1000);

  //initializing finished
  TGLogging::get()->write("end initialisation")->crlf();
}

/**
* protected void doHello for hello message after reboot
*/
void TGDevice::doHello()
{
  TGLogging::get()->write("start initialisation")->crlf();
}

/**
* protected doRegister, registration of parameters, sensors and t_actors
*/
void TGDevice::doRegister()
{
  //Default configuration parameter this base device is using
  deviceconfig->addConfig("deviceid",'S',16,false,"Ger&auml;tename",deviceid,NULL,NULL);
  deviceconfig->addConfig("wifiSSID",'S',16,true,"Netzwerkkennung",wifiSSID,NULL,NULL);
  deviceconfig->addConfig("wifiPWD",'S',32,true,"Netzwerkpasswort",wifiPWD,NULL,NULL);
  deviceconfig->addConfig("host",'S',32,false,"Host f&uuml;r Konfig, Daten und Dashboard",host,NULL,NULL);
  deviceconfig->addConfig("loopDelay",'I',0,false,"[ms] Pause des Loops",NULL,&loopDelayMS,NULL);

  //if timer is active server page which delivers time in secs
  if (timerActive)
    deviceconfig->addConfig("urlgettimesec",'S',32,false,"Webseite von der der Host die aktuellen Sekunden liefert",urlgettimesec,NULL,NULL);

  //parameter needed for sensors handling is sensors are registered
  if ((sensors != NULL) and sensors->hasMembers())
    {
      deviceconfig->addConfig("messtime",'I',0,false,"alle wieviel Sekunden gemessen wird",NULL,&messTime,NULL);
      deviceconfig->addConfig("reporttime",'I',0,false,"nach wieviel Sekunden der Wert auch ohne &Auml;nderung &uuml;ber Delta reported wird",NULL,&reportTime,NULL);
      deviceconfig->addConfig("urlsensordata",'S',32,false,"Webseite an die die Werte reported werden",urlsensordata,NULL,NULL);
    }

  //parameter needed for actors handling is sensors are registered
  if ((actors != NULL) and actors->hasMembers())
    {
      deviceconfig->addConfig("actortime",'I',0,false,"Sekunden nach denen auf Auto-Aktion geprüft wird",NULL,&messTime,NULL);
      deviceconfig->addConfig("urlactordata",'S',32,false,"Webseite an die Aktionen reported werden",urlactordata,NULL,NULL);
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
* private void htmlHeader
*    default HTML-Header
*/
void TGDevice::htmlHeader()
{
  outbuffer.clear();
  outbuffer.add(htmlHeader1);
  outbuffer.replace("deviceid",deviceid);
  outbuffer.replace("deviceversion",deviceconfig->deviceversion);
}

/**
* private void htmlFooter
*    default HTML-Footer
*/
void TGDevice::htmlFooter()
{
  outbuffer.add(htmlFooter1);
  if ((sensors != NULL) and (sensors->hasMembers()))
    outbuffer.add(htmlFooter2);
  if ((actors != NULL) and (actors->hasMembers()))
      outbuffer.add(htmlFooter3);
  outbuffer.add(htmlFooter4);
  outbuffer.replace("value",millis());
  outbuffer.add(htmlFooter5);
}

void TGDevice::serverOnDashboard()
{
  htmlHeader();
  outbuffer.add(htmlDashboard);
  if (sensors != NULL)
    sensors->html(&outbuffer);
  if (actors != NULL)
    actors->html(&outbuffer);
  htmlFooter();
  server->send(200, "text/html", outbuffer.getout());
}

void TGDevice::htmlConfig()
{
  htmlHeader();
  deviceconfig->htmlForm(&outbuffer);
  htmlFooter();
}

void TGDevice::serverOnConfig()
{
  htmlConfig();
  server->send(200, "text/html", outbuffer.getout());
}

void TGDevice::serverOnSaveConfig()
{
  TGLogging::get()->write("serverOnSaveConfig")->crlf();
  char fieldname[TtgConfConfig::maxFieldLen];
  char value[TtgConfConfig::maxValueLen];
  for(int i=0; i<server->args(); i++)
    {
      server->argName(i).toCharArray(fieldname,30);
      server->arg(i).toCharArray(value,30);
      deviceconfig->setValue(fieldname,value);
    }
  doAfterConfigChange();
  htmlConfig();
  server->send(200, "text/html", outbuffer.getout());
}

void TGDevice::serverOnWriteConfig()
{
  TGLogging::get()->write("serverOnWriteConfig")->crlf();
  deviceconfig->writeEEPROM();
  htmlConfig();
  server->send(200, "text/html", outbuffer.getout());
}

void TGDevice::jsonHeader()
{
  outbuffer.clear();
  outbuffer.add(jsonHeader1);
  outbuffer.replace("deviceid",deviceid);
  outbuffer.replace("deviceversion",deviceconfig->deviceversion);
  outbuffer.replace("millis",millis());
}

void TGDevice::serverOnGetConfig()
{
  TGLogging::get()->write("serverOnGetConfig")->crlf();
  boolean all = false;
  if (server->args() > 0)
    if (server->argName(0) == "all");
      all = server->arg(0) == "Y";

  jsonHeader();
  outbuffer.add(", ");
  deviceconfig->json(all,&outbuffer);
  outbuffer.add(" }");

  server->send(200, "application/json", outbuffer.getout());
}

void TGDevice::serverOnPutConfig()
{
  TGLogging::get()->write("serverOnPutConfig")->crlf();

  //TODO json String/CharBuffer noch aus dem Request holen
  String json;
  deviceconfig->putJson(json);
  doAfterConfigChange();

  jsonHeader();
  outbuffer.add(", ");
  deviceconfig->json(false,&outbuffer);
  outbuffer.add(" }");

  server->send(200, "application/json", outbuffer.getout());
}

void TGDevice::jsonSensors(const boolean t_angefordert)
{
  TGLogging::get()->write("jsonSensors")->crlf();
  jsonHeader();
  outbuffer.add(", ");
  sensors->json(t_angefordert,&outbuffer);
  outbuffer.add(" }");
}

void TGDevice::serverOnGetValues()
{
  TGLogging::get()->write("serverOnGetValues")->crlf();
  jsonSensors(true);
  server->send(200, "application/json", outbuffer.getout());
}

void TGDevice::jsonActors(const boolean t_angefordert)
{
  jsonHeader();
  outbuffer.add(", ");
  actors->json(t_angefordert,&outbuffer);
  outbuffer.add(" }");
}

void TGDevice::serverOnGetActors()
{
  TGLogging::get()->write("serverOnGetActors")->crlf();
  jsonActors(true);
  server->send(200, "application/json", outbuffer.getout());
}

/**
 *   Syntax
 *   setActor?id=<id>[&status=<status>][%endtime=<endtime>]
 */
void TGDevice::serverOnSetActor()
{
  char id[20] = "";
  char status[5] = "";
  int endtime = -1;

  for(int i=0; i<server->args(); i++)
    if (server->argName(i) == "id")
      server->arg(i).toCharArray(id,20);
    else if (server->argName(i) == "status")
      server->arg(i).toCharArray(status,5);
    else if (server->argName(i) == "endtime")
      endtime = server->arg(i).toInt();

  if ((strlen(id) > 0) and (strlen(status) > 0))
    actors->setStatus(id,status[0]);
  if ((strlen(id) > 0) and (endtime >= 0))
    actors->setEndtime(id,endtime);

  jsonActors(false);
  server->send(200, "text/html", outbuffer.getout());
}

// http JSON Post:
// https://techtutorialsx.com/2016/07/21/esp8266-post-requests/
boolean TGDevice::httpRequest(const char* url, const char* values, const boolean t_withresponse, char* response)
{
  boolean erg = false;

  TGLogging::get()->write("httpRequest")->crlf();
  TGLogging::get()->write("host: \"")->write(host)->write("\"")->crlf();
  TGLogging::get()->write("url: \"")->write(url)->write("\"")->crlf();

  if ((strlen(host) == 0) or (strlen(url) == 0))
    return erg;

  char uri[128] = "";
  strcpy(uri,host);
  uri[strlen(host)] = '/'; uri[strlen(host)+1] = '\0';
  strcpy(uri+strlen(host)+1,url);
  TGLogging::get()->write("Value Request to: \"")->write(uri)->write("\"")->crlf();

  //TODO Hier brechen wir den Request aus Testzweocken noch ab
  return erg;

  HTTPClient *http = new HTTPClient;    //Declare object of class HTTPClient

  TGLogging::get()->write("Values: \"")->write(values)->write("\"")->crlf();

  http->begin(url);   //Specify request destination

  int httpCode = 0;
  if (strlen(uri) == 0)
    httpCode = http->GET();
  else
    {
      http->addHeader("Content-Type", "application/json");  //Specify content-type header
      httpCode = http->POST(values);
    }

  TGLogging::get()->write("httpCode:")->write(httpCode)->crlf();
  if (httpCode < 0)
    TGLogging::get()->write("[HTTP] ... failed, error: ")->write(http->errorToString(httpCode))->crlf();

  if (t_withresponse)
    {
      //response = http->getString();          //Get the response payload
      //  getString is working with A StringStream and need to much resorves,
      // we can get the answert directly from the stream
      int i = 0;
      WiFiClient* stream = http->getStreamPtr();
      while(stream->available() > 0)
        {
          *(response+i) = stream->read();
          ++i;
        }
      *(response+i) = '\0';
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
  if ((lastTimeMS = -1) or (maintime > 86399) or (lastTimeMS > ms))
    {
      httpRequest(urlgettimesec, "", true, outbuffer.getout());
      //Fraglich ist was geben ich hier zurück, am einfachsten wären unverpackt die Sekunden, oder halt xml oder noch besser json
      sscanf(outbuffer.getout(),"%d",&maintime);
      mainTimeMS = maintime * 1000;
    }
  else
    mainTimeMS += (ms - lastTimeMS);
  maintime = mainTimeMS / 1000;
  lastTimeMS = ms;
}

void TGDevice::deviceLoop()
{
  timer();

  server->handleClient();

  if ((sensors != NULL) and sensors->hasMembers())
    {
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
          jsonSensors(false);
          httpRequest(urlsensordata, outbuffer.getout(), false, NULL);
        }
    }

  if ((actors != NULL) and actors->hasMembers())
    {
      if (timeTest(lastActorTime,actorTime))
        {
          boolean needReporting = actors->action();

          if (needReporting)
            {
              jsonActors(false);
              httpRequest(urlactordata, outbuffer.getout(), false, NULL);
            }
        }
    }

  //TGLogging::get()->write("doLoop");
  doLoop();

  //TGLogging::get()->write("delay");
  delay(loopDelayMS);
}

void TGDevice::doLoop()
{

}
