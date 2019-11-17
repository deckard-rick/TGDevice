#ifndef TGSENSOR_H
#define TGSENSOR_H

#include <Arduino.h>
#include <tgCharbuffer.hpp>

class TtgSensor
{
  public:
    TtgSensor(const char* t_id, float *t_pdelta)
       { strcpy(id,t_id); pdelta = t_pdelta;};
    char id[32];
    float value;
    float newValue;
    float *pdelta;
    int messTime;
    boolean changed;
    int reportTime;
    TtgSensor* next = NULL;
    void messWert();
  protected:
    virtual float doGetMessValue();
};

class TtgSensorsList
{
  public:
    boolean hasMembers();
    void add(TtgSensor *t_value);
    boolean messWerte();
    boolean checkReporting(int t_reportTime);
    void json(const boolean t_angefordert, TGCharbuffer outbuffer);
    virtual void html(TGCharbuffer* outbuffer);
  private:
    TtgSensor *firstelement=NULL, *lastelement=NULL;
};

#endif
