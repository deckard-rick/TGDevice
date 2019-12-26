/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
* lesson learned:
* 1) use yield, because not to run into the internal watchdog exception
*
* 2) char* instead of String
*
* class TGDevice
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence (richtige suchen, NO COMMERCIAÖ USE)
*/

#include <tgDevice.hpp>
#include <tgLogging.hpp>

/*Constants to create the header and footer of all html pages*/
char htmlHeader1[] = "<html><body><h1>Device ID:#deviceid# Version(#deviceversion#)</h1>";
char htmlFooter1[] = "<p>[<a href=\"/\">Main</a>]</br>[<a href=\"/config\">Configuration</a>][<a href=\"/getconfig\">Configuration (json)</a>]";
char htmlFooter2[] = "[<a href=\"/getvalues\">Values (json)</a>]";
char htmlFooter3[] = "[<a href=\"/getactors\">Actors (json)</a>]";
char htmlFooter4[] = "</p><p><small>active since #value#ms.<br/>";
char htmlFooter5[] = "Copyright Andreas Tengicki 2018-, NO COMMERCIAL USE</small></p></body></html>";

/* .. headline of the html Dashboard (Main-HTML-Page)*/
char htmlDashboard[] = "<h2>Dashboard</h2>";

/*.. same for json*/
char jsonHeader1[] = "{ \"Version\": \"#deviceversion#\", \"deviceid\": \"#deviceid#\", \"millis\": \"#millis#\"";

/* a buffer for creating the output (html, json etc..), works with char instead of String */
TGCharbuffer outbuffer;

/**
 * true if millis() is sec after checkTime (or have an overflow)
 * @param  checkTime lastCall of millis()
 * @param  sec       sec to be passed
 * @return           true/false, intervall is finished
 */
bool timeTest(int checkTime, int sec)
{
  int ms = millis();
  return (((ms - checkTime) / 1000) > sec) || ((ms - checkTime) < 0);
}

/**
* TGDevice constructor
* @param const char* t_deviceversion Version of the devices
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
* @param TGSensorsList *t_sensors list to register
*
* registered the list of sensors, by this it is possible to use derived classes
*/
void TGDevice::registerSensorsList(TtgSensorsList* t_sensors)
{
  sensors = t_sensors;
}

/**
* public void registerActorsList
* @param TGActorsList *t_actors list to register
*
* registered the list of actors, by this it is possible to use derived classes
*/
void TGDevice::registerActorsList(TtgActorsList* t_actors)
{
  actors = t_actors;
}

/**
* public void deviceSetup() called from the Arduino setup function
*/
void TGDevice::deviceSetup()
{
  //init serial device if neccessary for logging
  TGLogging::get()->setModus('S');

  TGLogging::get()->write("wait 5s")->crlf();
  delay(5000);
  TGLogging::get()->write("start initialisation")->crlf();

  //derived classes can print a hello message after boot to logging
  doHello();

  //register all config-parameter, sensors and actors with derived classe
  doRegister();

  //load configuration from EEPROM if possible (else default values)
  deviceconfig->readEEPROM();

  //calculation in the configuration after changed, for example timetables
  //TGLogging::get()->write("doAfterConfigChange")->crlf();
  doAfterConfigChange();

  //establish WiFi connection
  //TGLogging::get()->write("WiFi: ")->write(wifiSSID)->write(" (")->write(wifiPWD)->write(")");
  TGLogging::get()->write("WiFi connect to: ")->write(wifiSSID);
  int endtime = millis() + 30000;
  WiFi.begin(String(wifiSSID),String(wifiPWD));
  TGLogging::get()->write("[");
  int cnt = 25;
  while ((WiFi.status() != WL_CONNECTED) and (millis() < endtime))
  {
    delay(250);
    TGLogging::get()->write(".");
    ++cnt;
    if (cnt % 50 == 0)
      {
        TGLogging::get()->crlf();
        cnt = 0;
      }
   }
  TGLogging::get()->write("]")->crlf();

  //if connected activate auto-re-connect
  if (WiFi.status() == WL_CONNECTED)
    {
      WiFi.setAutoReconnect(true); //damit er den erfolgreichen Aufbau wiederholt.
      TGLogging::get()->write("WiFi connected IP:")->write(WiFi.localIP().toString())->crlf();
    }
  else //if not connected, start as WiFi-access-point on default IP 192.168.4.1
    {
      TGLogging::get()->write("WiFi NOT connected")->crlf();
      //stopping reconnecting of client mode, it makes trouble
      WiFi.setAutoReconnect(false);
      //starting server
      if (WiFi.softAP("tgDevice001","11223344"))
        TGLogging::get()->write("WiFi tgDevice001 startet IP:")->write(WiFi.softAPIP().toString())->crlf();
      else //if not successfull write log
        TGLogging::get()->write("Wifi access point NOT started.")->crlf();
    }

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

  //do other setup work, in derived class
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
}

/**
* protected doRegister, registration of parameters, sensors and t_actors
*/
void TGDevice::doRegister()
{
  //Default configuration parameter this base device is using
  //Parameter 5 is the description for the conifguration html page, it is in german
  deviceconfig->addConfig("deviceid",'S',16,false,"Ger&auml;tename",deviceid,NULL,NULL);
  deviceconfig->addConfig("wifiSSID",'S',16,true,"Netzwerkkennung",wifiSSID,NULL,NULL);
  deviceconfig->addConfig("wifiPWD",'S',32,true,"Netzwerkpasswort",wifiPWD,NULL,NULL);
  deviceconfig->addConfig("host",'S',32,false,"Host f&uuml;r Konfig, Daten und Dashboard",host,NULL,NULL);
  deviceconfig->addConfig("loopDelay",'I',0,false,"[ms] Pause des Loops",NULL,&loopDelayMS,NULL);

  //if timer is active server page which delivers time in secs
  if (timerActive)
    deviceconfig->addConfig("urlgettimesec",'S',32,false,"Webseite von der der Host die aktuellen Sekunden liefert",urlgettimesec,NULL,NULL);

  //parameter needed for sensors handling if sensors are registered
  if ((sensors != NULL) and sensors->hasMembers())
    {
      deviceconfig->addConfig("messtime",'I',0,false,"alle wieviel Sekunden gemessen wird",NULL,&messTime,NULL);
      deviceconfig->addConfig("reporttime",'I',0,false,"nach wieviel Sekunden der Wert auch ohne &Auml;nderung &uuml;ber Delta reported wird",NULL,&reportTime,NULL);
      deviceconfig->addConfig("urlsensordata",'S',32,false,"Webseite an die die Werte reported werden",urlsensordata,NULL,NULL);
    }

  //parameter needed for actors handling if actors are registered
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

/**
 * http server index page (/)
 *   with the overview (dashboard)
 */
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

/**
 * write html config form into outbuffer
 */
void TGDevice::htmlConfig()
{
  htmlHeader();
  deviceconfig->htmlForm(&outbuffer);
  htmlFooter();
}

/**
 * http server html config form (/config)
 */
void TGDevice::serverOnConfig()
{
  htmlConfig();
  server->send(200, "text/html", outbuffer.getout());
}

/**
 * http server html save config form (/saveconfig)
 */
void TGDevice::serverOnSaveConfig()
{
  TGLogging::get()->write("serverOnSaveConfig")->crlf();

  //get all fieldname and values out of server.args
  //and write it to the configuration
  //all based/converted to char*
  char fieldname[TtgConfConfig::maxFieldLen];
  char value[TtgConfConfig::maxValueLen];
  for(int i=0; i<server->args(); i++)
    {
      server->argName(i).toCharArray(fieldname,30);
      server->arg(i).toCharArray(value,30);
      deviceconfig->setValue(fieldname,value);
    }
  //re-calculate depending configuration values
  doAfterConfigChange();
  //create html for for output
  htmlConfig();
  //send response
  server->send(200, "text/html", outbuffer.getout());
}

/**
 * http server write config to EEPROM (/writeconfig)
 */
void TGDevice::serverOnWriteConfig()
{
  TGLogging::get()->write("serverOnWriteConfig")->crlf();
  deviceconfig->writeEEPROM();
  htmlConfig();
  server->send(200, "text/html", outbuffer.getout());
}

/**
* private void jsonHeader
*    default JSON-Header
*/
void TGDevice::jsonHeader()
{
  outbuffer.clear();
  outbuffer.add(jsonHeader1);
  outbuffer.replace("deviceid",deviceid);
  outbuffer.replace("deviceversion",deviceconfig->deviceversion);
  outbuffer.replace("millis",millis());
}

/**
 * http server get config as json (/getconfig)
 */
void TGDevice::serverOnGetConfig()
{
  TGLogging::get()->write("serverOnGetConfig")->crlf();
  //check is all=Y is set, to send secured configuration values also
  boolean all = false;
  if (server->args() > 0)
    if (server->argName(0) == "all");
      all = server->arg(0) == "Y";

  //create json in outbuffer
  jsonHeader();
  outbuffer.add(", ");
  deviceconfig->json(all,&outbuffer);
  outbuffer.add(" }");

  //send response
  server->send(200, "application/json", outbuffer.getout());
}

/**
 * http server put config as json (/getconfig)
 *   to set configuration values from outside
 *   for details see TGConfig, thera are some dependancies to consider.
 *
 * TODO function is still working with String, but is not called very often
 */
void TGDevice::serverOnPutConfig()
{
  TGLogging::get()->write("serverOnPutConfig")->crlf();

  //https://techtutorialsx.com/2017/03/26/esp8266-webserver-accessing-the-body-of-a-http-request/
  if (server->hasArg("plain"))
    {
      String json = server->arg("plain");
      deviceconfig->putJson(json);
      doAfterConfigChange();

      //if we set the values from outside, they need to save permamently
      deviceconfig->writeEEPROM();

      //create the response
      jsonHeader();
      outbuffer.add(", ");
      deviceconfig->json(false,&outbuffer);
      outbuffer.add(" }");

      server->send(200, "application/json", outbuffer.getout());
    }
  else
    server->send(200, "text/plain", "NO DATA");
}

/**
* private void jsonSensors
*    generate json with sensor values
* @param t_all [all or only to reported]
*/
void TGDevice::jsonSensors(const boolean t_all)
{
  TGLogging::get()->write("jsonSensors")->crlf();
  jsonHeader();
  outbuffer.add(", ");
  sensors->json(t_all,&outbuffer);
  outbuffer.add(" }");
}

/**
 * http server onGetValues (/getvalues)
 *   sensor values as json
 */
void TGDevice::serverOnGetValues()
{
  TGLogging::get()->write("serverOnGetValues")->crlf();
  jsonSensors(true);

  server->send(200, "application/json", outbuffer.getout());
}

/**
* private void jsonSensors
*    generate json with actors status
* @param t_all [all or only to reported]
*/
void TGDevice::jsonActors(const boolean t_all)
{
  jsonHeader();
  outbuffer.add(", ");
  actors->json(t_all,&outbuffer);
  outbuffer.add(" }");
}

/**
 * http server onGetActors (/getactors)
 *   sensor values as json
 */
void TGDevice::serverOnGetActors()
{
  TGLogging::get()->write("serverOnGetActors")->crlf();
  jsonActors(true);
  server->send(200, "application/json", outbuffer.getout());
}

/**
 * http server onSetActors (/setactor)
 *   set actor value
 *   Syntax
 *   setactor?id=<id>[&status=<status>][%endtime=<endtime>]
 *
 *   TODO all is for future use, check wether id is usefull or not
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
/**
 * send a http get/post request to Host
 * @param  url            url data send to
 * @param  values         optional, with values then use post
 * @param  t_withresponse get the response
 * @param  response       char array to store the result
 * @return                successfull or not (true/false)
 */
boolean TGDevice::httpRequest(const char* url, const char* values, const boolean t_withresponse, char* response)
{
  boolean erg = false;

  if ((strlen(host) == 0) or (strlen(url) == 0))
    return erg;

  char uri[128] = "";
  strcpy(uri,host);
  uri[strlen(host)] = '/'; uri[strlen(host)+1] = '\0';
  strcpy(uri+strlen(host)+1,url);

  TGLogging::get()->write("httpRequest")->crlf();
  TGLogging::get()->write("to: \"")->write(uri)->write("\"")->crlf();
  TGLogging::get()->write("Values: \"")->write(values)->write("\"")->crlf();

  HTTPClient http;    //Declare object of class HTTPClient

  http.begin(String(uri));   //Specify request destination
  yield();

  int httpCode = 0;
  if (strlen(values) == 0)
    httpCode = http.GET();
  else
    {
      TGLogging::get()->write("POST")->crlf();
      http.addHeader("Content-Type", "application/json");  //Specify content-type header
      httpCode = http.POST(String(values));
    }
  yield();

  TGLogging::get()->write("httpCode:")->write(httpCode)->crlf();
  if (httpCode < 0)
    TGLogging::get()->write("[HTTP] ... failed, error: ")->write(http.errorToString(httpCode))->crlf();
  yield();

  if (t_withresponse and (httpCode >= 0))
    {
      //response = http->getString();          //Get the response payload
      //  getString is working with A StringStream and need to much resorves,
      // we can get the answert directly from the stream
      int i = 0;
      WiFiClient* stream = http.getStreamPtr();
      while(stream->available() > 0)
        {
          *(response+i) = stream->read();
          ++i;
        }
      *(response+i) = '\0';
      TGLogging::get()->write("response:")->write(response)->crlf();
    }
  yield();

  http.end();  //Close connection
  erg = true;

  return erg;
}

/**
 * set timer functions active or inactive
 * @param value [description]
 */
void TGDevice::setTimerActive(boolean value)
{
  timerActive = value;
}

/**
 * calculates a timer information via differences of time between two millis()
 * starttime (on a day) between 0 and 86399s is initialized via server/host
 *
 * TODO testing, and implementation the function of the server/host
 */
void TGDevice::timer()
{
  //if not active then nothing to do
  if (!timerActive)
    return;

  //actual millis()
  int ms = millis();
  //if first milli(), or after a day or after restartting millis() from zero
  //get initial value from server/host
  if ((lastTimeMS = -1) or (maintime > 86399) or (lastTimeMS > ms))
    {
      httpRequest(urlgettimesec, "", true, outbuffer.getout());
      //TODO which format has the response, only the seconds, or more informations in json
      sscanf(outbuffer.getout(),"%d",&maintime);
      mainTimeMS = maintime * 1000;
    }
  else  //calculate as difference to last call
    mainTimeMS += (ms - lastTimeMS);
  //transform im [s]
  maintime = mainTimeMS / 1000;
  //store last access
  lastTimeMS = ms;
}

/**
 * main device loop
 * call it from arduino main loop
 */
void TGDevice::deviceLoop()
{
  //calculate timer
  timer();

  //server does his job
  server->handleClient();

  //if we have sensors
  if ((sensors != NULL) and sensors->hasMembers())
    {
      boolean needReporting = false;
      //check measure interval
      if (timeTest(lastMessTime,messTime))
        {
          TGLogging::get()->write("deviceLoop: messWerte")->crlf();
          //read sensor values, needReporting is true if a sensor values changes
          //rapidly, to report it immediatly
          needReporting = sensors->messWerte();
          lastMessTime = millis();
        }

      //if the reporting of a sensor value is older reportTime, we have to report again
      needReporting = needReporting or sensors->checkReporting(reportTime);
      if (needReporting)
        {
          TGLogging::get()->write("deviceLoop: reporting")->crlf();
          //create outbuffer with sensor values
          jsonSensors(false);
          char response[16];
          //send the values to server/host
          httpRequest(urlsensordata, outbuffer.getout(), false, response);
        }
    }

  //if we have actors
  if ((actors != NULL) and actors->hasMembers())
    {
      //check testing for action intervall
      if (timeTest(lastActorTime,actorTime))
        {
          //check and doaction in the actors
          boolean needReporting = actors->action();

          if (needReporting)
            {
              //create outbuffer with actor values
              jsonActors(false);
              //send actor values to server/host
              char response[16];
              httpRequest(urlactordata, outbuffer.getout(), false, response);
            }
        }
    }

  //place for looping functions of derived devices
  doLoop();

  //small loop delay in [ms]
  delay(loopDelayMS);
}

/**
 * "abstract" function for individual loop-functions for the derived device
 */
void TGDevice::doLoop()
{

}
