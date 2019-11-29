#ifndef TGACTOR_H
#define TGACTOR_H

#include <Arduino.h>
#include <tgCharbuffer.hpp>

class TtgActor
{
  public:
    TtgActor(const char* t_id, int *t_maintime)
      { strcpy(id,t_id);  maintime = t_maintime;};
    int setAutoTimes(int t_start, int t_time);
    char id[32];
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
    void json(const boolean t_angefordert, TGCharbuffer* outbuffer);
    void setStatus(char* t_id, char t_status);
    void setEndtime(char* t_id, int t_endtime);
    virtual void html(TGCharbuffer* outbuffer);
  protected:
    virtual void doCalcStatus();
  private:
    TtgActor *firstelement=NULL, *lastelement=NULL;
};

#endif
