#include <tgCOnfig.hpp>

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
TtgDeviceConfig::TtgDeviceConfig(const String& t_deviceversion)
{
  deviceversion = t_deviceversion;
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
  for (int j=0; j<sizeof(deviceversion); j++)
    {
      if (EEPROM.read(configStart + i) != deviceversion[j])
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
  for(unsigned int j=0; j<sizeof(deviceversion); j++)
    {
      EEPROM.write(configStart + i, deviceversion[j]);
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
