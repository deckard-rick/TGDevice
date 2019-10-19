#ifndef TGSENSOR_H
#define TGSENSOR_H

#include <Arduino.h>

class TtgSensor
{
  public:
    TtgSensor(const String& t_id, float *t_pdelta) {id = t_id; pdelta = t_pdelta;};
    String id;
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
    String getJson(const boolean t_angefordert);
    virtual String getHTML();
  private:
    TtgSensor *firstelement=NULL, *lastelement=NULL;
};

#endif
