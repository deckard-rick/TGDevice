#ifndef TGCONFIG_H
#define TGCONFIG_H

#include <Arduino.h>
#include <EEPROM.h>
#include <tgCharbuffer.hpp>

class TtgConfConfig
{
  public:
    static const int maxFieldLen = 32;
    static const int maxValueLen = 64;
    static const int maxDescriptionLen = 128;
    char fieldname[maxFieldLen];
    char typ;
    int groesse;
    boolean secure;
    char description[maxDescriptionLen];
    char *psval;
    int *pival;
    float *pfval;
    TtgConfConfig *next;
    TtgConfConfig(const char* aFieldname,const char t_typ, const int aGroesse, const boolean aSecure, const char* t_description, char *aps, int *api, float *apf);
};

class TtgDeviceConfig
{
  public:
    const char* deviceversion;
    TtgDeviceConfig(const char* t_deviceversion);
    void addConfig(const char* t_fieldname, const char t_typ, const int aGroesse, const boolean aSecure, const char* t_description, char *aps, int *api, float *apf);
    void setValue(const char* fieldname, const char* value);
    void getValue(const char* fieldname, char* t_out);
    int getValueI(const char* fieldname);
    float getValueD(const char* fieldname);
    void json(boolean withAll, TGCharbuffer* outbuffer);
    void putJson(const String& json);
    int getEEPROMSize();
    void readEEPROM();
    void writeEEPROM();
    void htmlForm(TGCharbuffer* outbuffer);
  protected:
  private:
    int eepromBufferSize = -1;
    TtgConfConfig *firstelement = NULL, *lastelement = NULL;
    TtgConfConfig* getFieldElement(const char* fieldname);
};

#endif
