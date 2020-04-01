/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
* class TGConfConfig - one configuration value
* class TTGDeviceConfig - configuration of the device
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence CC-BY-NC-SA 4.0, NO COMMERCIAL USE
*  see also: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#include <tgConfig.hpp>
#include <tgLogging.hpp>

//constants to create the html-form to change the configuration values
char htmlForm1[] = "<h2>Konfiguration</h2><form action=\"saveconfig\" method=\"POST\">";
char htmlForm2[] = "<label>#fieldname#</label><input type=\"text\" name=\"#fieldname#\" size=#size# value=\"#value#\"/><br/>#description#<br/><br/>";
char htmlForm3[] =  "<button type=\"submit\" name=\"action\">Werte &auml;ndern</button></form>";
char htmlForm4[] =  "<form action=\"writeconfig\"><button type=\"submit\" name=\"action\">Config festschreiben</button></form>";

//constant for the json values
char jsonConfig1[] = "\"#fieldname#\": \"#value#\"";

/**
* Describes one configuration value/parameter
* only used by TGDeviceConfig
*  all attributes are public for easier access
*
* @param aFieldname    name of the value
* @param t_typ         S(tring) I(nteger) or F(loat)
* @param aGroesse      max size of the string
* @param aSecure       secured access (wifiParameter etc.)
* @param t_description description used in htlm form
* @param aps           pointer to char value
* @param api           pointer to int value
* @param apf           pointer to float value
 */
TGConfConfig::TGConfConfig(const char* t_fieldname, const char t_typ, int aGroesse, boolean aSecure, const char* t_description, char *aps, int *api, float *apf)
{
  //store all parameters into the attributes
  strcpy(fieldname,t_fieldname);
  typ = t_typ;
  groesse = aGroesse;
  secure = aSecure;
  strcpy(description,t_description);
  psval = aps;
  pival = api;
  pfval = apf;
  //initialize connect to the next value
  next = NULL;
}

/**
 * constructor
 * @param t_deviceversion version of the configuration structure
 *
 * if the value of version changed, the device do not read from EEPROM
 */
TGDeviceConfig::TGDeviceConfig(const char* t_deviceversion)
{
  deviceversion = t_deviceversion;
}

/**
 * adds a configuration value to the configuration
 * for description off the parameters see TGConfConfig
 *
 * the fieldname should be unique
 */
void TGDeviceConfig::addConfig(const char* t_fieldname, const char t_typ, int aGroesse, boolean aSecure, const char* t_description, char *aps, int* api, float* apf)
{
  //check wether the fieldname is unqiue or not
  TGConfConfig* testElement = getFieldElement(t_fieldname);
  if (testElement != NULL)
    {
       TGLogging::get()->write("git es schon:")->write(testElement->fieldname)->crlf();
       return; //TGMARK besser raise eine Exception (wie geht das in C++ ?)
    }

  //create the configuration element and put it to the end of the linked list
  TGConfConfig *newElement = new TGConfConfig(t_fieldname, t_typ, aGroesse, aSecure, t_description, aps, api, apf);
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

/**
 * get a config value via fieldname
 * @param  fieldname to look for
 * @return           return the conf. element
 */
TGConfConfig* TGDeviceConfig::getFieldElement(const char* fieldname)
{
  TGConfConfig *erg = NULL;
  for (TGConfConfig* i = firstelement; i != NULL; i=i->next)
    if (strcmp(i->fieldname,fieldname) == 0)
      {
        erg = i;
        break;
      }
  return erg;
}

/**
 * copies a value into the char buffer
 * @param fieldname to look for
 * @param t_out     outbuffer
 */
void TGDeviceConfig::getValue(const char* fieldname, char* t_out)
{
  TGConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == 'S')
      strcpy(t_out,i->psval);
    else if (i->typ == 'I')
      sprintf(t_out,"%d",*(i->pival));
    else if (i->typ == 'F')
      sprintf(t_out,"%7.2f",*(i->pfval));
}

/**
 * returns value as integer
 * @param  fieldname to look for
 * @return           value as integer
 */
int TGDeviceConfig::getValueI(const char* fieldname)
{
  int erg = 0;
  TGConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == 'I')
      erg = *(i->pival);
    else if (i->typ == 'F')
      erg = round(*(i->pfval));
    else
      sscanf(i->psval,"%d",&erg);
  return erg;
}

/**
 * returns value as float
 * @param  fieldname to look for
 * @return           value as float (names D like double)
 */
float TGDeviceConfig::getValueD(const char* fieldname)
{
  float erg = 0;
  TGConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == 'I')
      erg = *(i->pival);
    else if (i->typ == 'F')
      erg = *(i->pfval);
    else
      sscanf(i->psval,"%f",&erg);
  return erg;
}

/**
 * sets a configuration value
 * @param fieldname configuration element to set
 * @param value     value to set
 */
void TGDeviceConfig::setValue(const char* fieldname, const char* value)
{
  TGConfConfig* i = getFieldElement(fieldname);
  if (i != NULL)
    if (i->typ == 'S')
      strcpy(i->psval,value); //TODO gf Groesse beachten
    else if (i->typ == 'I')
      sscanf(value,"%d",i->pival);
    else if (i->typ == 'F')
     sscanf(value,"%f",i->pfval);
}

/**
 * creates json representation of the configuration values
 *
 * @param all       if true, secured parameter also
 * @param outbuffer buffer to copy into
 */
void TGDeviceConfig::json(boolean all, TGCharbuffer* outbuffer)
{
  outbuffer->add("\"configs\" : { ");

  boolean first = true;
  for (TGConfConfig* i=firstelement; i != NULL; i=i->next)
    {
      if ((all || !i->secure) and (strcmp(i->fieldname,"deviceid") != 0)) //deviceID steht im Header
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

/**
 * set values via json
 * acceppts values only if "Version"=<configversion> and "deviceid"=<deviceID>
 *
 * @param json [description]
 */
void TGDeviceConfig::putJson(const String& json)
{
  /*
  * 07.10.2019
  * fieldnames are unique, config values are in a subgroup without repeatation,
  * because of this we can ignore the strucure and only look for "fieldname" : "fieldvalue"
  */
  int modus = 0;
  String fieldname = "";
  String fieldvalue = "";
  char value[TGConfConfig::maxValueLen] = "";
  int valid = 0;
  for (int i=0; i < json.length(); i++)
    {
      char c = json[i];
      if ((modus == 0) and (c == '{')) modus = 1;
      else if ((modus == 1) and (c == '"')) modus = 2;
      else if ((modus == 2) and (c == '"')) modus = 3;
      else if (modus == 2) fieldname += c;
      else if ((modus == 3) and (c == ':')) modus = 4;
      else if (((modus == 2) or (modus == 4)) and (c == '{'))
        {modus = 1; fieldname = ""; }
      else if ((modus == 4) and (c == '"')) modus = 5;
      else if ((modus == 5) and (c == '"'))
        {
          TGLogging::get()->write("put:")->write(fieldname)->write(":")->write(fieldvalue)->crlf();
          if (fieldname == "Version")
            {
              getValue("Version",value);
              if (fieldvalue == String(deviceversion))
                ++valid;
              else
              {
                TGLogging::get()->write("INVALID Version:")->write(deviceversion)->crlf();
                return;
              }
            }
          else if (fieldname == "deviceid")
            {
              getValue("deviceid",value);
              if (String(value) == fieldvalue)
                ++valid;
              else
              {
                TGLogging::get()->write("INVALID DeviceID:")->write(value)->crlf();
                return;
              }
            }
          else if ((valid==2) && (fieldname.substring(1,4) != "wifi"))
            setValue(fieldname.c_str(),fieldvalue.c_str());
          fieldname = "";
          fieldvalue = "";
          modus = 1;
        }
      else if (modus == 5) fieldvalue += c;
      yield();
    }
}

/**
 * calculates the size of the EEPROM space
 * @return size in EEPROM needed
 */
int TGDeviceConfig::getEEPROMSize()
{
  if (eepromBufferSize > 0)
    return eepromBufferSize;

  eepromBufferSize = strlen(deviceversion);
  for (TGConfConfig* i=firstelement; i != NULL; i=i->next)
    if (i->typ == 'S')
      eepromBufferSize += i->groesse;
    else if (i->typ == 'I')
      eepromBufferSize += sizeof(int);
    else if (i->typ == 'F')
      eepromBufferSize += sizeof(float);
    else
      ;
  return eepromBufferSize;
}

/**
 * read the configuration values from then EEPROM
 *
 * read only if the configVersion is like the one stored in the first bytes
 * https://playground.arduino.cc/Code/EEPROMLoadAndSaveSettings
 */
void TGDeviceConfig::readEEPROM()
{
  TGLogging::get()->write("readEEPROM: ")->crlf();
  //initialize EEPROM for read/write configuration for using after reboot
  EEPROM.begin(getEEPROMSize());

  boolean valid = true;
  int i=0; //position in EEPROM to read from
  for (int j=0; j < strlen(deviceversion); j++)
    {
      if (EEPROM.read(i) != deviceversion[j])
        {
          valid = false;
          break;
        }
      i++;
    }
  if (valid)
    {
      for (TGConfConfig* elem = firstelement; elem != NULL; elem = elem->next)
        {
          if (elem->typ == 'S')
            for(unsigned int j=0; j<elem->groesse; j++)
              {
                elem->psval[j] = EEPROM.read(i);
                i++;
              }
          else if (elem->typ == 'I')
            for(unsigned int j=0; j<sizeof(int); j++)
              {
                *(((char*)(elem->pival))+j) = EEPROM.read(i);
                i++;
              }
          else if (elem->typ == 'F')
            for(unsigned int j=0; j<sizeof(float); j++)
              {
                *(((char*)(elem->pfval))+j) = EEPROM.read(i);
                i++;
              }
        }
    }
  EEPROM.end();
}

/**
 * write the configuration values to the EEPROM
 */
void TGDeviceConfig::writeEEPROM()
{
  TGLogging::get()->write("writeEEPROM: ")->crlf();

  //calculate size
  EEPROM.begin(getEEPROMSize());

  //write device version
  int i=0;
  for(unsigned int j=0; j < strlen(deviceversion); j++)
    {
      EEPROM.write(i, deviceversion[j]);
      i++;
    }

  //write configuration elements depending on the type
  for (TGConfConfig* elem = firstelement; elem != NULL; elem = elem->next)
    {
      if (elem->typ == 'S')
        for(unsigned int j=0; j<elem->groesse; j++)
          { //TGMARK TODO ich darf nicht länger schreiben als bis zum \0 bzw muss das danach auch \0 sein.
            EEPROM.write(i, elem->psval[j]) ;
            i++;
          }
      else if (elem->typ == 'I')
        for(unsigned int j=0; j<sizeof(int); j++)
          {
            EEPROM.write(i, *(((char*)(elem->pival))+j));
            i++;
          }
      else if (elem->typ == 'F')
        for(unsigned int j=0; j<sizeof(float); j++)
          {
            EEPROM.write(i, *(((char*)(elem->pfval))+j));
            i++;
          }
      }
  EEPROM.commit();
  EEPROM.end();
}

/**
 * creates a html form to show and edit the configuration values
 * @param outbuffer char buffet to write the result into
 *
 * TODO Paramter all für die Securen Parameter fehlt noch
 */
void TGDeviceConfig::htmlForm(TGCharbuffer* outbuffer)
{
  outbuffer->add(htmlForm1);

  char value[TGConfConfig::maxValueLen];
  for (TGConfConfig* i=firstelement; i != NULL; i=i->next)
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
