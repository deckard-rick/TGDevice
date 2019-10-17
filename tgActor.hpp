#ifndef TGACTOR_H
#define TGACTOR_H

#include <Arduino.h>

class TtgActor
{
  public:
    TtgActor(const String& t_id, int *t_maintime) {id = t_id; maintime = t_maintime;};
    int setAutoTimes(int t_start, int t_time);
    String id;
    char status='N';  //Y on, O autoON, F autoOFF, N off
    boolean changed = false;
    int autoStart = 0;
    int autoEnd = 0;
    int endTime = 0;
    TtgActor* next = NULL;
    void action();
    virtual void doCalcStatus();
    void setStatus(char t_status);
  protected:
    virtual void doAction();
    virtual void doActivate();
    virtual void doDeactivate();
  private:
    int *maintime;
};

class TtgActorsList
{
  public:
    boolean hasMembers();
    TtgActor* add(TtgActor *t_value);
    boolean action();
    String getJson(const boolean t_angefordert);
    void setStatus(String t_id, char t_status);
    void setEndtime(String t_id, int t_endtime);
    virtual String getHTML();
  protected:
    virtual void doCalcStatus();
  private:
    TtgActor *firstelement=NULL, *lastelement=NULL;
};

#endif
