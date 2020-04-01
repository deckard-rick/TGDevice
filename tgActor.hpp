/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
*  Design
*  TGActor  baseclass to controll/switch on/off something, included timer
*
*  TGDevice main class implemented a base devices including http-server
            for configuration and working with the device
*  TGConfig automated Handling of configuraion parameters
*  TGSensor baseclass to mesassure values
*  TGCharbuffer buffer to build longer output strings, without Strings
*  TGLogging output to Serial (or TCP-Server later) for debugging
*
*  TGActor
*    TODO untested, first device with actors has to be build
*    controllinng an actor via time automatic, or setting by
*    a function of the device
*
*    Documentation will bee completed, if code is used in a real
*    (my next) project (GRDMAIN). Controlling water pumps and light in
*    the garden.
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence CC-BY-NC-SA 4.0, NO COMMERCIAL USE
*  see also: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#ifndef TGACTOR_H
#define TGACTOR_H

#include <Arduino.h>
#include <tgCharbuffer.hpp>

/**
 * represents an actor
 * @param t_id       id of the actor
 * @param t_maintime *pointer* to the maintime
 */
class TGActor
{
  public:
    //constructor
    TGActor(const char* t_id, int *t_maintime)
      { strcpy(id,t_id);  maintime = t_maintime;};
    //set time for automatic
    int setAutoTimes(int t_start, int t_time);
    //public attributes for easy access
    char id[32];              //id of the actor
    char status='N';          //Y on, O autoON, F autoOFF, N off
    boolean changed = false;  //status has changed
    int autoStart = 0;        //time[s] activating each day
    int autoEnd = 0;          //time[s] deactivating each day
    int endTime = 0;          //time to deactivate
    TGActor* next = NULL;    //link to the next action
    void action();            //make action if changed
    virtual void doCalcStatus();   //calculate new status
    void setStatus(char t_status); //set status from extern
  protected:
    virtual void doAction();       //for derived actor todo the action
    virtual void doActivate();
    virtual void doDeactivate();
  private:
    int *maintime;            //pointer to maintime
};

/**
 * List with actors
 * @return self
 */
class TGActorsList
{
  public:
    boolean hasMembers();
    TGActor* add(TGActor *t_value);
    boolean action();
    void json(const boolean t_angefordert, TGCharbuffer* outbuffer);
    void setStatus(char* t_id, char t_status);
    void setEndtime(char* t_id, int t_endtime);
    virtual void html(TGCharbuffer* outbuffer);
  protected:
    virtual void doCalcStatus();
  private:
    TGActor *firstelement=NULL, *lastelement=NULL;
};

#endif
