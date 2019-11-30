#include <tgConfig.hpp>
#include <tgLogging.hpp>

char htmlForm1[] = "<h2>Konfiguration</h2><form action=\"saveconfig\" method=\"POST\">";
char htmlForm2[] = "<label>#fieldname#</label><input type=\"text\" name=\"#fieldname#\" size=#size# value=\"#value#\"/><br/>#description#<br/><br/>";
char htmlForm3[] =  "<button type=\"submit\" name=\"action\">Werte &auml;ndern</button></form>";
char htmlForm4[] =  "<form action=\"writeconfig\"><button type=\"submit\" name=\"action\">Config festschreiben</button></form>";

char jsonConfig1[] = "\"#fieldname#\": \"#value#\"";

TtgConfConfig::TtgConfConfig(const char* t_fieldname, const char t_typ, int aGroesse, boolean aSecure, const char* t_description, char *aps, int *api, float *apf)
{
  strcpy(fieldname,t_fieldname);
  typ = t_typ;
  groesse = aGroesse;
  secure = aSecure;
  strcpy(description,t_description);
  psval = aps;
  pival = api;
  pfval = apf;
  next = NULL;
}

/**
 * constructor
 */
TtgDeviceConfig::TtgDeviceConfig(const char* t_deviceversion)
{
  deviceversion = t_deviceversion;
}

void TtgDeviceConfig::addConfig(const char* t_fieldname, const char t_typ, int aGroesse, boolean aSecure, const char* t_description, char *aps, int* api, float* apf)
{
  //Bei doppelten Namen bekommen wir Probleme, die wir dadurch lösen könnten, das wir beim
  //* get den ersten nehmen
  //* beim Set alle setzen
  //* beim read/write darauf achten
  //writelog("addConfig:"+aFieldname);
  //Aber wir hoffen/probiern erstmal mit eindeutigen Namen zurecht zu kommen

  TtgConfConfig* testElement = getFieldElement(t_fieldname);
  if (testElement != NULL)
    {
       TGLogging::get()->write("git es schon:")->write(testElement->fieldname)->crlf();
       return; //TGMARK besser raise eine Exception (wie geht das in C++ ?)
    }

  TtgConfConfig *newElement = new TtgConfConfig(t_fieldname, t_typ, aGroesse, aSecure, t_description, aps, api, apf);
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

TtgConfConfig* TtgDeviceConfig::getFieldElement(const char* fieldname)
{
  TtgConfConfig *erg = NULL;
  for (TtgConfConfig* i = firstelement; i != NULL; i=i->next)
    if (strcmp(i->fieldname,fieldname) == 0)
      {
        erg = i;
        break;
      }
  return erg;
}

void TtgDeviceConfig::getValue(const char* fieldname, char* t_out)
{
  TtgConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == 'S')
      strcpy(t_out,i->psval);
    else if (i->typ == 'I')
      sprintf(t_out,"%d",*(i->pival));
    else if (i->typ == 'F')
      sprintf(t_out,"%7.2f",*(i->pfval));
  //TGLogging::get()->write ("configGetValue(")->write(fieldname)->write("):")->write(t_out)->crlf();
}

int TtgDeviceConfig::getValueI(const char* fieldname)
{
  int erg = 0;
  TtgConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == 'I')
      erg = *(i->pival);
    else if (i->typ == 'F')
      erg = round(*(i->pfval));
    else
      sscanf(i->psval,"%d",&erg);
  return erg;
}

float TtgDeviceConfig::getValueD(const char* fieldname)
{
  float erg = 0;
  TtgConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == 'I')
      erg = *(i->pival);
    else if (i->typ == 'F')
      erg = *(i->pfval);
    else
      sscanf(i->psval,"%f",&erg);
  return erg;
}

void TtgDeviceConfig::setValue(const char* fieldname, const char* value)
{
  TtgConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == 'S')
      strcpy(i->psval,value); //TODO gf Groesse beachten
    else if (i->typ == 'I')
      sscanf(value,"%d",i->pival);
    else if (i->typ == 'F')
     sscanf(value,"%f",i->pfval);
}

void TtgDeviceConfig::json(boolean all, TGCharbuffer* outbuffer)
{
  outbuffer->add("\"configs\" : { ");

  boolean first = true;
  for (TtgConfConfig* i=firstelement; i != NULL; i=i->next)
    {
      if (all || !i->secure)
        {
          if (!first)
            outbuffer->add(", ");
          first = false;

          outbuffer->add(jsonConfig1);
          outbuffer->replace("fieldname",i->fieldname);
          char value[128];
          getValue(i->fieldname,value);
          outbuffer->replace("value",value);
        }
    }
  outbuffer->add("}");
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
  char value[TtgConfConfig::maxValueLen] = "";
  for (int i=0; i < json.length(); i++)
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
          getValue("DeviceID",value);
          if ((fieldname == "DeviceID") and (fieldvalue != String(value)))
            return;
          //setValue(fieldname,fieldvalue);
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
    if (i->typ == 'S')
      erg += i->groesse;
    else if (i->typ == 'I')
      erg += sizeof(int);
    else if (i->typ == 'F')
      erg += sizeof(float);
    else
      ;
  TGLogging::get()->write("EEPROM-Size:")->write(erg)->crlf();
  return erg;
}

//https://playground.arduino.cc/Code/EEPROMLoadAndSaveSettings
void TtgDeviceConfig::readEEPROM()
{
  TGLogging::get()->write("load Config")->crlf();

  boolean valid = true;
  unsigned int i=0;
  for (int j=0; j < strlen(deviceversion); j++)
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
        {
          if (elem->typ == 'S')
            for(unsigned int j=0; j<elem->groesse; j++)
              {
                //TGMARK TODO ich darf nicht länger schreiben als bis zum \0 bzw muss das danach auch \0 sein.
                //Und wenn ich für den String nicht genug Platz habe, dann scheppert es auch!!
                elem->psval[j] = EEPROM.read(configStart + i);
                i++;
              }
          else if (elem->typ == 'I')
            for(unsigned int j=0; j<sizeof(int); j++)
              {
                *((char*)(elem->pival + j)) = EEPROM.read(configStart + i);
                i++;
              }
          else if (elem->typ == 'F')
            for(unsigned int j=0; j<sizeof(float); j++)
              {
                *((char*)(elem->pfval + j)) = EEPROM.read(configStart + i);
                i++;
              }
          //+getValue(elem->fieldname)
          //TGLogging::get()->write("readEEPROM:")->write(elem->fieldname)->write(" => ")->crlf();
        }
    }
}

void TtgDeviceConfig::writeEEPROM()
{
  TGLogging::get()->write("write Config")->crlf();

  //for (unsigned int i=0; i<sizeof(configs); i++)
  //  EEPROM.write(configStart + i, *((char*)&configs + i));
  unsigned int i=0;
  for(unsigned int j=0; j < strlen(deviceversion); j++)
    {
      EEPROM.write(configStart + i, deviceversion[j]);
      i++;
    }
  for (TtgConfConfig* elem = firstelement; elem != NULL; elem = elem->next)
    if (elem->typ == 'S')
      for(unsigned int j=0; j<elem->groesse; j++)
        { //TGMARK TODO ich darf nicht länger schreiben als bis zum \0 bzw muss das danach auch \0 sein.
          EEPROM.write(configStart + i, elem->psval[j]) ;
          i++;
        }
    else if (elem->typ == 'I')
      for(unsigned int j=0; j<sizeof(int); j++)
        {
          EEPROM.write(configStart + i, *((char*)(elem->pival + j)));
          i++;
        }
    else if (elem->typ == 'F')
      for(unsigned int j=0; j<sizeof(float); j++)
        {
          EEPROM.write(configStart + i, *((char*)(elem->pfval + j)));
          i++;
        }
  EEPROM.commit();
}

void TtgDeviceConfig::htmlForm(TGCharbuffer* outbuffer)
//TODO Paramter all für die Securen Parameter fehlt noch
{
  outbuffer->add(htmlForm1);

  char value[TtgConfConfig::maxValueLen];
  for (TtgConfConfig* i=firstelement; i != NULL; i=i->next)
    {
      outbuffer->add(htmlForm2);
      outbuffer->replace("fieldname",i->fieldname);
      sprintf(value,"%d",i->groesse); outbuffer->replace("size",value);
      outbuffer->replace("description",i->description);
      getValue(i->fieldname,value);   outbuffer->replace("value",value);
    }
  outbuffer->add(htmlForm3);
  outbuffer->add(htmlForm4);
}
