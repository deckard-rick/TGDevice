/**
 * Erkenntnisse
 * 1) delTime wie im HZLogger ist sinnlos, denn wir messen ja immer und haben damit dann doch eine gültige messtime.
 *    an Hand der messTime sehen wir ob der Wert "alt" wird und am Wert sehen wir ob er fehlerhaft wird.
 *
 * 2) id/anschluss wird nicht gebraucht. DeviceID und SensorID sind eindeutig.
 *   "Nur" bei Bus Systemem wie dem HZLoggger müssen wir natürlich die Adressen der sensors einer ID zuordnen.
 *   => Der Anschluss fliegt hier erstmal raus
 */

#include "tgDevice.hpp"

boolean timeTest(int checkTime, int sec)
{
  int ms = millis();
  return (((ms - checkTime) / 1000) > sec) || ((ms - checkTime) < 0);
}

TtgDevice::TtgDevice(const String& aDeviceVersion)
{
  deviceVersion = aDeviceVersion;
  deviceConfig = new TtgDeviceConfig(aDeviceVersion);
}

void TtgDevice::writelog(const String& s, boolean crLF)
{
  if (logModus == "S")
    if (crLF) Serial.println(s);
    else Serial.print(s);
}

void TtgDevice::registerSensors(TtgSensorsList* t_sensors)
{
  sensors = t_sensors;
}

void TtgDevice::registerActors(TtgActorsList* t_actors)
{
  actors = t_actors;
}

void TtgDevice::deviceSetup()
{
  if (logModus == "S")
    Serial.begin(9600);

  doHello();

  doRegister();

  writelog("init EEPROM");
  EEPROM.begin(deviceConfig->getEEPROMSize());

  writelog("load Config");
  deviceConfig->readEEPROM();
  doAfterConfigChange();

  doSetup();

  writelog("Start Server");
  server->begin();

  writelog("wait 250");
  delay(250);

  writelog("Ende Initialisierung");
}

void TtgDevice::doHello()
{
  writelog("START Initialisierung");
}

void TtgDevice::doRegister()
{
  deviceConfig->addConfig("deviceID","S",16,false,"Gerätename",&deviceID,NULL,NULL);
  deviceConfig->addConfig("wifiSSID","S",32,true,"Netzwerkkennung",&wifiSSID,NULL,NULL);
  deviceConfig->addConfig("wifiPWD","S",32,true,"Netzwerkpasswort",&wifiPWD,NULL,NULL);
  deviceConfig->addConfig("host","S",32,false,"Host für Konfig, Daten und Dashboard",&host,NULL,NULL);
  deviceConfig->addConfig("loopDelay","I",0,false,"[ms] Pause des Loops",NULL,&loopDelayMS,NULL);
  if (timerActive)
    deviceConfig->addConfig("urlgettimesec","S",32,false,"Webseite von der der Host die aktuellen Sekunden liefert",&urlgettimesec,NULL,NULL);

  if ((sensors != NULL) and sensors->hasSensors())
    {
      deviceConfig->addConfig("messtime","I",0,false,"alle wieviel Sekunden gemessen wird",NULL,&messTime,NULL);
      deviceConfig->addConfig("reporttime","I",0,false,"nach wieviel Sekunden der Wert auch ohne Änderung über Delta reported wird",NULL,&reportTime,NULL);
      deviceConfig->addConfig("urlsensordata","S",32,false,"Webseite an die die Werte reported werden",&urlsensordata,NULL,NULL);
    }

  if ((actors != NULL) and actors->hasMembers())
    {
      deviceConfig->addConfig("actortime","I",0,false,"Sekunden nach denen auf Auto-Aktion geprüft wird",NULL,&messTime,NULL);
      deviceConfig->addConfig("urlactordata","S",32,false,"Webseite an die Aktionen reported werden",&urlactordata,NULL,NULL);
    }
}

void TtgDevice::doCalcStatus()
{

}

void TtgDevice::doAfterConfigChange()
{

}

void TtgDevice::doSetup()
{
  WiFi.begin(wifiSSID,wifiPWD);

  //TODO hier brauchen wir noch ein TimeOut
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    writelog(".",false);
   }
  writelog(".");

  writelog("WiFi connected IP:",false);
  writelog(WiFi.localIP().toString());

  writelog("init Server");
  //https://stackoverflow.com/questions/32900314/esp8266webserver-setting-a-value-inside-a-class
  //server.on("/data.html", std::bind(&WebServer::handleSomeData, this));
  server->on("/",std::bind(&TtgDevice::serverOnDashboard, this));
  server->on("/config",std::bind(&TtgDevice::serverOnConfig, this));
  server->on("/saveconfig",std::bind(&TtgDevice::serverOnSaveConfig, this));
  server->on("/writeconfig",std::bind(&TtgDevice::serverOnWriteConfig, this));
  server->on("/getconfig",std::bind(&TtgDevice::serverOnGetConfig, this));
  server->on("/putconfig",std::bind(&TtgDevice::serverOnPutConfig, this));
  server->on("/getvalues",std::bind(&TtgDevice::serverOnGetValues, this));
  server->on("/getactors",std::bind(&TtgDevice::serverOnGetActors, this));
  server->on("/setactor",std::bind(&TtgDevice::serverOnSetActor, this));
}

String TtgDevice::htmlHeader()
{
  String html = "<html><body>";
  html += "<h1>Device ID:"+deviceID+" Version("+deviceVersion+")</h1>";
  return html;
}

String TtgDevice::htmlFooter()
{
  return "<p><a href=\"/\">Main</a></br>"
         "<small>Copyright Andreas Tengicki 2018-2020</small>"
         "</body></html>";
}

void TtgDevice::serverOnDashboard()
{
  String html = htmlHeader();
  html += "<h2>Dashboard</h2>";
  if (sensors != NULL)
    html += sensors->getHTML();
  if (actors != NULL)
    html += actors->getHTML();
  html += "<h2>Menu</h2>";
  html += "<p>[<a href=\"/config\">Konfiguration</a>] ";
  html +=    "[<a href=\"/getconfig\">Konfiguration (json)</a>] ";
  html +=    "[<a href=\"/getvalues\">Werte (json)</a>] ";
  html +=    "[<a href=\"/getactors\">Aktoren (json)</a></p>";
  html += htmlFooter();

  server->send(200, "text/html", html);
}

String TtgDevice::getHtmlConfig()
{
  String html = htmlHeader();
  html += deviceConfig->getHtmlForm();
  html += htmlFooter();
  return html;
}

void TtgDevice::serverOnConfig()
{
  server->send(200, "text/html", getHtmlConfig());
}

void TtgDevice::serverOnSaveConfig()
{
  for(int i=0; i<server->args(); i++)
    deviceConfig->setValue(server->argName(i), server->arg(i));
  doAfterConfigChange();
  server->send(200, "text/html", getHtmlConfig());
}

void TtgDevice::serverOnWriteConfig()
{
  deviceConfig->writeEEPROM();
  server->send(200, "text/html", getHtmlConfig());
}

String TtgDevice::jsonHeader()
{
  return "{ \"Version\": \""+deviceVersion+"\", \"DeviceID\": \""+deviceID+"\"";
}

void TtgDevice::serverOnGetConfig()
{
  boolean all = false;
  if (server->args() > 0)
    if (server->argName(0) == "all");
      all = server->arg(0) == "Y";

  String json = jsonHeader() + ", " + deviceConfig->getJson(all) + " }";
  server->send(200, "application/json", json);
}

void TtgDevice::serverOnPutConfig()
{
  /*
  * 07.10.2019
  * Damit (also ohne Wiederholgruppen, können wir das lesen vereinfachen
  * { setzt auf Modus 1, dann kommt ein Feldname, dann :, dann ein Wert und ggf ein ,
  * } können wir überlesen, wenn ein feld1 : { feld2 kommt, führt das reset zum lesen von feld2
  */
  String json = ""; //hier muss der POST-Body der Abfrage rein

  deviceConfig->putJson(json);

  //calculateTimes();
}

String TtgDevice::getValuesJson(const boolean t_angefordert)
{
  return jsonHeader() + ", " + sensors->getJson(t_angefordert) + " }";
}

void TtgDevice::serverOnGetValues()
{
  server->send(200, "application/json", getValuesJson(true));
}

String TtgDevice::getActorsJson(const boolean t_angefordert)
{
  return jsonHeader() + ", " + actors->getJson(t_angefordert) + " }";
}

void TtgDevice::serverOnGetActors()
{
  server->send(200, "application/json", getActorsJson(true));
}

/**
 *   Syntax
 *   setActor?id=<id>[&status=<status>][%endtime=<endtime>]
 */
void TtgDevice::serverOnSetActor()
{
  String id="";
  String status="";
  String endtime="";

  for(int i=0; i<server->args(); i++)
    if (server->argName(i) == "id")
      id = server->arg(i);
    else if (server->argName(i) == "status")
      id = server->arg(i);
    else if (server->argName(i) == "endtime")
      id = server->arg(i);

  if ((id != "") and (status != ""))
    actors->setStatus(id,(char)(status[0]));
  if ((id != "") and (endtime != ""))
    actors->setEndtime(id,endtime.toInt());

  server->send(200, "text/html", getActorsJson(false));
}

// http JSON Post:
// https://techtutorialsx.com/2016/07/21/esp8266-post-requests/
boolean TtgDevice::httpRequest(const String& url, const String& values, const boolean t_withresponse, String& response)
{
  boolean erg = false;

  if ((host == "") or (url==""))
    return erg;

  String aurl = host+"\"+"+url;
  writelog("Value Request to: \""+aurl+"\n");

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

void TtgDevice::setTimerActive(boolean value)
{
  timerActive = value;
}

void TtgDevice::timer()
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

void TtgDevice::deviceLoop()
{
  timer();

  server->handleClient();

  if ((sensors != NULL) and sensors->hasSensors())
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
          String d_response = "";
          httpRequest(urlsensordata, getValuesJson(false), false, d_response);
        }
    }

  if ((actors != NULL) and actors->hasMembers())
    {
      if (timeTest(lastActorTime,actorTime))
        {
          boolean needReporting = actors->action();

          if (needReporting)
            {
              String d_response = "";
              httpRequest(urlactordata, getActorsJson(false), false, d_response);
            }
        }
    }

  doLoop();

  delay(loopDelayMS);
}

void TtgDevice::doLoop()
{

}
