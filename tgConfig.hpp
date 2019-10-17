#ifndef TGCONFIG_H
#define TGCONFIG_H

#include <Arduino.h>
#include <EEPROM.h>

#define configStart 32

class TtgConfConfig
{
  public:
    String fieldname;
    String typ;
    int groesse;
    boolean secure;
    String description;
    String *psval;
    int *pival;
    float *pdval;
    TtgConfConfig *next;
    TtgConfConfig(const String& aFieldname,const String& aTyp, const int aGroesse, const boolean aSecure, const String& aDescription, String *aps, int *api, float *apf);
};

class TtgDeviceConfig
{
  public:
    TtgDeviceConfig(const String& aDeviceVersion);
    void addConfig(const String& aFieldname, const String& aTyp, const int aGroesse, const boolean aSecure, const String& aDescription, String* aps, int* api, float* apf);
    void setValue(const String& fieldname, const String& value);
    String getValue(const String& fieldname);
    int getValueI(const String& fieldname);
    float getValueD(const String& fieldname);
    String getJson(boolean withAll);
    void putJson(const String& json);
    int getEEPROMSize();
    void readEEPROM();
    void writeEEPROM();
    String getHtmlForm();
  protected:
  private:
    String deviceVersion;  //MAX 7 Zeichen
    TtgConfConfig *firstelement = NULL, *lastelement = NULL;
    TtgConfConfig* getFieldElement(const String& fieldname);
};

#endif
