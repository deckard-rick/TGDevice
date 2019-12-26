/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
*  Design
*  TGConfig automated Handling of configuraion parameters
*
*  TGDevice main class implemented a base devices including http-server
            for configuration and working with the device
*  TGSensor baseclass to mesassure values
*  TGActor  baseclass to controll/switch on/off something, included timer
*  TGCharbuffer buffer to build longer output strings, without Strings
*  TGLogging output to Serial (or TCP-Server later) for debugging
*
*  TGConfig
*    register device configuration values
*    htlm form to show and edit the values
*    get and set(put) the values via json
*    read and store the values in EEPROM
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence (richtige suchen, NO COMMERCIAL USE)
*/

#ifndef TGCONFIG_H
#define TGCONFIG_H

#include <Arduino.h>
#include <EEPROM.h>
#include <tgCharbuffer.hpp>

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
class TGConfConfig
{
  public:
    static const int maxFieldLen = 32;         //max length of fieldname
    static const int maxValueLen = 64;         //max length of string value
    static const int maxDescriptionLen = 128;  //max length of description
    char fieldname[maxFieldLen];               //attributes, see constructor description
    char typ;
    int groesse;
    boolean secure;
    char description[maxDescriptionLen];
    char *psval;
    int *pival;
    float *pfval;
    TGConfConfig *next;
    //constructor                    //linked list
    TGConfConfig(const char* aFieldname,const char t_typ, const int aGroesse, const boolean aSecure, const char* t_description, char *aps, int *api, float *apf);
};

/**
 * configuration of the device
 * if the device version change, it cannot read anymore from EEPROM
 *
 * @param t_deviceversion version of the configuration (example tgHZ05)
 */
class TGDeviceConfig
{
  public:
    const char* deviceversion;  //version of the configuration
    //constructor
    TGDeviceConfig(const char* t_deviceversion);
    //add a configuration value
    void addConfig(const char* t_fieldname, const char t_typ, const int aGroesse, const boolean aSecure, const char* t_description, char *aps, int *api, float *apf);
    //set value via fieldname, allways as char*
    void setValue(const char* fieldname, const char* value);
    //get value vie fieldname into char* Buffer
    void getValue(const char* fieldname, char* t_out);
    //get value as int
    int getValueI(const char* fieldname);
    //get value as float
    float getValueD(const char* fieldname);
    //get configuration values as json
    void json(boolean withAll, TGCharbuffer* outbuffer);
    //set configuration values via json
    void putJson(const String& json);
    //calculate size of config values for EEPROM storage
    int getEEPROMSize();
    //read configuration from EEPROM
    void readEEPROM();
    //write configuration to EEPROM
    void writeEEPROM();
    //generate html form into outbuffer
    void htmlForm(TGCharbuffer* outbuffer);
  protected:
  private:
    int eepromBufferSize = -1;  //cached bufferSize
    //managment of the linked list
    TGConfConfig *firstelement = NULL, *lastelement = NULL;
    //get element via fieldname
    TGConfConfig* getFieldElement(const char* fieldname);
};

#endif
