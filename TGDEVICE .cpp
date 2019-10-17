/**
 * Erkenntnisse
 * 1) delTime wie im HZLogger ist sinnlos, denn wir messen ja immer und haben damit dann doch eine gültige messtime.
 *    an Hand der messTime sehen wir ob der Wert "alt" wird und am Wert sehen wir ob er fehlerhaft wird.
 *
 * 2) id/anschluss wird nicht gebraucht. DeviceID und SensorID sind eindeutig.
 *   "Nur" bei Bus Systemem wie dem HZLoggger müssen wir natürlich die Adressen der sensors einer ID zuordnen.
 *   => Der Anschluss fliegt hier erstmal raus
 */

#include "TGDEVICE.hpp"

boolean timeTest(int checkTime, int sec)
{
  int ms = millis();
  return (((ms - checkTime) / 1000) > sec) || ((ms - checkTime) < 0);
}

TtgConfConfig::TtgConfConfig(const String& aFieldname, const String& aTyp, int aGroesse, boolean aSecure, const String& aDescription, String *aps, int *api, float *apf)
{
  fieldname = aFieldname;
  typ = aTyp;
  groesse = aGroesse;
  secure = aSecure;
  description = aDescription;
  psval = aps;
  pival = api;
  pdval = apf;
  next = NULL;
}

/**
 * constructor
 */
TtgDeviceConfig::TtgDeviceConfig(const String& aDeviceVersion)
{
  deviceVersion = aDeviceVersion;
}

void TtgDeviceConfig::addConfig(const String& aFieldname, const String& aTyp, int aGroesse, boolean aSecure, const String& aDescription, String* aps, int* api, float* apf)
{
  //Bei doppelten Namen bekommen wir Probleme, die wir dadurch lösen könnten, das wir beim
  //* get den ersten nehmen
  //* beim Set alle setzen
  //* beim read/write darauf achten
  //Aber wir hoffen/probiern erstmal mit eindeutigen Namen zurecht zu kommen
  TtgConfConfig* testElement = getFieldElement(aFieldname);
  if (testElement != NULL)
    return; //TGMARK besser raise eine Exception (wie geht das in C++ ?)

  TtgConfConfig *newElement = new TtgConfConfig(aFieldname, aTyp, aGroesse, aSecure, aDescription, aps, api, apf);
  if (firstelement == NULL)
    {
      firstelement = newElement;
      lastelement = firstelement;
    }
  else
    {
      lastelement->next = newElement;
      lastelement = lastelement->next;
    }
}

TtgConfConfig* TtgDeviceConfig::getFieldElement(const String& fieldname)
{
  TtgConfConfig *erg = NULL;
  for (TtgConfConfig* i=firstelement; i != NULL; i=i->next)
    if (i->fieldname = fieldname)
      {
        erg = i;
        break;
      }
  return erg;
}

String TtgDeviceConfig::getValue(const String& fieldname)
{
  String erg = "";
  TtgConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == "S")
      erg = *(i->psval);
    else if (i->typ == "I")
      erg = String(*(i->pival));
    else if (i->typ == "D")
      erg = String(*(i->pdval));
  return erg;
}

int TtgDeviceConfig::getValueI(const String& fieldname)
{
  int erg = 0;
  TtgConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == "I")
      erg = *(i->pival);
    else
      erg = getValue(fieldname).toInt();
  return erg;
}

float TtgDeviceConfig::getValueD(const String& fieldname)
{
  float erg = 0;
  TtgConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == "D")
      erg = *(i->pdval);
    else
      erg = getValue(fieldname).toFloat();
  return erg;
}

void TtgDeviceConfig::setValue(const String& fieldname, const String& value)
{
  TtgConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == "S")
      *(i->psval) = value.substring(0,i->groesse);
    else if (i->typ == "I")
      *(i->pival) = value.toInt();
    else if (i->typ == "D")
      *(i->pdval) = value.toFloat();
}

String TtgDeviceConfig::getJson(boolean all)
{
  String json = "\"configs\" : { ";

  boolean d_first = true;
  for (TtgConfConfig* i=firstelement; i != NULL; i=i->next)
    {
      if (all || !i->secure)
        {
          if (!d_first) json += ",";
          json += "\""+i->fieldname+"\": \""+getValue(i->fieldname)+"\"";
          d_first = false;
        }
    }
  json += "}";
  return json;
}

void TtgDeviceConfig::putJson(const String& json)
{
  /*
  * 07.10.2019
  * Damit (also ohne Wiederholgruppen, können wir das lesen vereinfachen
  * { setzt auf Modus 1, dann kommt ein Feldname, dann :, dann ein Wert und ggf ein ,
  * } können wir überlesen, wenn ein feld1 : { feld2 kommt, führt das reset zum lesen von feld2
  */
  int modus = 0;
  String fieldname = "";
  String fieldvalue = "";
  for (int i=0; i<sizeof(json); i++)
    {
      char c = json[i];
      if ((modus == 0) and (c == '{')) modus = 1;
      else if ((modus == 1) and (c == '"')) modus = 2;
      else if ((modus == 2) and (c == '"')) modus = 3;
      else if (modus == 2) fieldname += c;
      else if ((modus == 3) and (c == ':')) modus = 4;
      else if (((modus == 2) or (modus == 4)) and (c == '{')) modus = 1;
      else if ((modus == 4) and (c == '"')) modus = 5;
      else if ((modus == 5) and (c == '"'))
        {
          if ((fieldname == "DeviceID") and (fieldvalue != getValue("DeviceID")))
            return;
          setValue(fieldname,fieldvalue);
          fieldname = "";
          fieldvalue = "";
          modus = 1;
        }
      else if (modus == 5) fieldvalue += c;
    }
}

int TtgDeviceConfig::getEEPROMSize()
{
  int erg = 0;
  for (TtgConfConfig* i=firstelement; i != NULL; i=i->next)
    if (i->typ == "S")
      erg += i->groesse;
    else if (i->typ == "I")
      erg += sizeof(int);
    else if (i->typ == "D")
      erg += sizeof(float);
    else
      ;
  return erg;
}

//https://playground.arduino.cc/Code/EEPROMLoadAndSaveSettings
void TtgDeviceConfig::readEEPROM()
{
  //writelog("load Config");

  boolean valid = true;
  unsigned int i=0;
  for (int j=0; j<sizeof(deviceVersion); j++)
    {
      if (EEPROM.read(configStart + i) != deviceVersion[j])
        {
          valid = false;
          break;
        }
      i++;
    }
  if (valid)
    {
      //for (unsigned int i=0; i<sizeof(configs); i++)
      //  *((char*)&configs + i) = EEPROM.read(configStart + i);
      for (TtgConfConfig* elem = firstelement; elem != NULL; elem = elem->next)
        if (elem->typ == "S")
          for(unsigned int j=0; j<elem->groesse; j++)
            {
              //TGMARK TODO ich darf nicht länger schreiben als bis zum \0 bzw muss das danach auch \0 sein.
              //Und wenn ich für den String nicht genug Platz habe, dann scheppert es auch!!
              *(elem->psval + j) = EEPROM.read(configStart + i);
              i++;
            }
        else if (elem->typ == "I")
          for(unsigned int j=0; j<sizeof(int); j++)
            {
              *((char*)elem->pival + j) = EEPROM.read(configStart + i);
              i++;
            }
        else if (elem->typ == "D")
          for(unsigned int j=0; j<sizeof(float); j++)
            {
              *((char*)elem->pival + j) = EEPROM.read(configStart + i);
              i++;
            }
    }
}

void TtgDeviceConfig::writeEEPROM()
{
  //writelog("write Config");

  //for (unsigned int i=0; i<sizeof(configs); i++)
  //  EEPROM.write(configStart + i, *((char*)&configs + i));
  unsigned int i=0;
  for(unsigned int j=0; j<sizeof(deviceVersion); j++)
    {
      EEPROM.write(configStart + i, deviceVersion[j]);
      i++;
    }
  for (TtgConfConfig* elem = firstelement; elem != NULL; elem = elem->next)
    if (elem->typ == "S")
      for(unsigned int j=0; j<elem->groesse; j++)
        { //TGMARK TODO ich darf nicht länger schreiben als bis zum \0 bzw muss das danach auch \0 sein.
          EEPROM.write(configStart + i, (*elem->psval)[j]);
          i++;
        }
    else if (elem->typ == "I")
      for(unsigned int j=0; j<sizeof(int); j++)
        {
          EEPROM.write(configStart + i, *((char*)elem->pival + j));
          i++;
        }
    else if (elem->typ == "D")
      for(unsigned int j=0; j<sizeof(float); j++)
        {
          EEPROM.write(configStart + i, *((char*)elem->pival + j));
          i++;
        }
  EEPROM.commit();
}

String TtgDeviceConfig::getHtmlForm()
//TODO Paramter all für die Securen Parameter fehlt noch
{
  String html = "<h2>Konfiguration</h2>";
  html += "<form action=\"saveconfig\" method=\"POST\">";
  for (TtgConfConfig* i=firstelement; i != NULL; i=i->next)
    {
      html += "<label>"+i->fieldname+"</label><input type=\"text\" name=\""+i->fieldname+"\"  size="+String(i->groesse);
      html += " value=\""+getValue(i->fieldname)+"\"/><br/>";
      html += i->description+"<br/><br/>";
    }
  /**/
  html += "<button type=\"submit\" name=\"action\">Werte &auml;ndern</button>";
  html += "</form>";
  html += "<form action=\"writeconfig\">";
  html += "<button type=\"submit\" name=\"action\">Config festschreiben</button>";
  html += "</form>";
  return html;
}


float TtgSensor::doGetMessValue()
{
  return 0;
}

void TtgSensor::messWert()
{
  newValue = doGetMessValue();

  if (abs(newValue - value) > *(pdelta))
    changed = true;
  messTime = millis();
}

boolean TtgSensorsList::hasSensors()
{
  return firstelement != NULL;
}

void TtgSensorsList::add(TtgSensor* value)
{
  if (firstelement == NULL)
    {
      firstelement = value;
      lastelement = firstelement;
    }
  else
    {
      lastelement->next = value;
      lastelement = lastelement->next;
    }
}

boolean TtgSensorsList::messWerte()
{
  boolean needReporting = false;
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
      element->messWert();
      needReporting = needReporting or element->changed;
    }
  return needReporting;
}

boolean TtgSensorsList::checkReporting(int t_reportTime)
{
    boolean needReporting = false;
    for (TtgSensor *element = firstelement; element != NULL; element = element->next)
      if (timeTest(element->reportTime,t_reportTime))
        {
          element->changed = true;
          needReporting = true;
        }
    return needReporting;
}

//Man könnte das auch in die sensorsGruppe tun, Zugriff auf deviceVersion und DeviceID notwendig, oder Header Trennen
String TtgSensorsList::getJson(const boolean t_angefordert)
{
  String json = "\"values\" : { ";

  boolean j_first = true;
  long now = millis();

  int i=0;
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    if (t_angefordert or element->changed) //Die Reported werden müssen, werden zuvor auf changed gesetzt
      {
        if (!(j_first)) json += ", ";
        j_first = false;
        json += "\"VAL"+String(i)+"\" : {";
        json += "\"id\" : \""+element->id+"\",";
        long sec = (now - element->messTime) / 1000;
        json += "\"sec\" : \""+String(sec)+"\",";
        json += ", \"value\" : \""+String(element->value)+"\"";
        json += "}, ";
        if (!t_angefordert)
          {
            element->changed = false;
            element->reportTime = now;
          }
        i++;
      }

  json += "} }";

  return json;
}

String TtgSensorsList::getHTML()
{
  String html = "<table><tr>";
  int now = millis();

  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    html += "<th>"+element->id+"</th>";
  html += "</tr><tr>";
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    html += "<td>"+String(element->value)+"</td>";
  html += "</tr><tr>";
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
      int sec = (now - element->messTime) / 1000;
      html += "<td><small>"+String(sec)+"</small></td>";
    }
  html += "</tr></table>";
  return html;
}

int TtgActor::setAutoTimes(int t_start, int t_time)
{
  autoStart = t_start;
  autoEnd = autoStart + t_start;
  if (autoEnd >= 86400) autoEnd = 86399; //wir gehen noch nicht über Mitternacht drüber
  return autoEnd;
}

void TtgActor::doCalcStatus()
{
  if ((status == 'A') or (status == 'F')) //autoModus
    {
      boolean c_value = ((autoStart <= *maintime) and (*maintime < autoEnd)) or (*maintime < endTime);
      if (c_value) setStatus('A'); else setStatus('F');
    }
}

void TtgActor::setStatus(char t_status)
{
  if (status != t_status)
    {
      status = t_status;
      changed = true;
    }
}

void TtgActor::action()
{
  if (changed)
    if ((status == 'Y') or (status == 'A'))
      doActivate();
    else if ((status == 'N') or (status == 'O'))
      doDeactivate();
    else
      doAction();
}

void TtgActor::doActivate()
{

}

void TtgActor::doDeactivate()
{

}

void TtgActor::doAction()
{

}

boolean TtgActorsList::hasMembers()
{
  return firstelement != NULL;
}

TtgActor* TtgActorsList::add(TtgActor* t_value)
{
  if (firstelement == NULL)
    {
      firstelement = t_value;
      lastelement = firstelement;
    }
  else
    {
      lastelement->next = t_value;
      lastelement = lastelement->next;
    }
  return t_value;
}

boolean TtgActorsList::action(TtgDevice *t_device)
{
  boolean needReporting = false;

  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    element->doCalcStatus();

  t_device->doCalcStatus();

  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    {
      element->action();
      needReporting = needReporting or element->changed;
    }

  return needReporting;
}

void TtgActorsList::setStatus(String t_id, char t_status)
{
  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    if (element->id == t_id)
      {
        element->setStatus(t_status);
        element->action();
      }
}

void TtgActorsList::setEndtime(String t_id, int t_endtime)
{
  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    if (element->id == t_id)
      element->endTime = t_endtime;  //activation via automatic timer (max after actionTime [s]
}

String TtgActorsList::getJson(boolean t_angefordert)
{
  String json = "\"actors\" : { ";

  boolean first = true;
  int i = 0;
  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    if (t_angefordert or element->changed)
      {
        if (!first) json += ", ";
        json += "\"A"+String(i)+"\" : {";
        json += "\"id\" : \""+element->id+"\",";  //Die ID ist innerhalb eines Devices eindeutig
        json += ", \"status\" : \""+String(element->status)+"\"";
        json += "\"autostart\" : \""+String(element->autoStart)+"\",";
        json += "\"autoend\" : \""+String(element->autoEnd)+"\",";
        json += "\"endtime\" : \""+String(element->endTime)+"\",";
        json += "}, ";

        if (!t_angefordert) element->changed = false;
      }

  json += "}";

  return json;
}

String TtgActorsList::getHTML()
{
  String html = "<table>";
  int f_endtime = (millis() / 1000) + 600;

  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    {
      char f_status = element->status;
      String f_color;
      if (f_status == 'Y')
        {
          f_status = 'N';
          f_color = "red";
        }
      else if (f_status == 'N')
        {
          f_status = 'A';
          f_color = "lightGreen";
        }
      if (f_status == 'A')
        {
          f_status = 'F';
          f_color = "lightRed";
        }
      if (f_status == 'F')
        {
          f_status = 'N';
          f_color = "red";
        }
      html += "<tr><th><button style=\"color="+f_color+"\">"+element->id+"</button>";  //action = id=element->&status=f_status
      if ((element->status == 'A') or (element->status == 'F'))
        html += "<td><button style=\"color=\"lightGreen\">Intervall 10min</button>";  //action = id=element->&endtime=f_endtime
      html += "</tr>";
    }
  html += "</table>";

  return html;
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
          boolean needReporting = actors->action(this);

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
