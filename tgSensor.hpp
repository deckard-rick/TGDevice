/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
*  Design
*  TGSensor baseclass to mesassure values

*  TGDevice main class implemented a base devices including http-server
            for configuration and working with the device
*  TGConfig automated Handling of configuraion parameters
*  TGActor  baseclass to controll/switch on/off something, included timer*
*  TGCharbuffer buffer to build longer output strings, without Strings
*  TGLogging output to Serial (or TCP-Server later) for debugging
*
*  TGSensor
*    * main class for sensors. report values after reporttime, or if value
*      changed more then an individual delta.
*    * organizied in a linked List
*    * json and html output with actual values
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence CC-BY-NC-SA 4.0, NO COMMERCIAL USE
*  see also: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#ifndef TGSENSOR_H
#define TGSENSOR_H

#include <Arduino.h>
#include <tgCharbuffer.hpp>

/**
 * * main class for sensors. report values after reporttime, or if value
 *      changed more then an individual delta.
 * * organizied in a linked List
 * * json and html output with actual values
 *
 * @param t_id     ID of the sensor, identified
 * @param t_pdelta link to a variable with a delta value, for immediatly reporting
 */
class TGSensor
{
  public:
    TGSensor(const char* t_id, float *t_pdelta)
       { strcpy(id,t_id);
         pdelta = t_pdelta; };
    //public values are easier for access
    char id[32];             // id of the sensor
    float value = 0;         // last value
    float newValue = 0;      // newest value after measure before changed test
    float *pdelta;           // pointer to the delta vslue für immediatly reporting
    int messTime = 0;        // last time value measured
    boolean changed = false; // value has changed
    int reportTime = 0;      // last time value reporting
    TGSensor* next = NULL;  // link to the next sensor
    //void initSensor(const char* t_id, float *t_pdelta);
    /**
     * masurement handling
     */
    void measure();
  protected:
    /**
     * get the new value from the sensor
     */
    virtual void dogetvalue();
};

/**
 * anchor for the list of sensors
 */
class TGSensorsList
{
  public:
    /**
     * check whether there are sensors or not
     * @return true is sensors available
     */
    boolean hasMembers();
    /**
     * add a sensort to the list
     * @param TtgSensor sensor to add
     */
    void add(TGSensor *t_value);
    /**
     * measure all sensor values
     * @return true is sensor need reporting
     */
    boolean measure();
    /**
     * check reporting is neccessary or not
     * @param  int t_reportTime time a value is allways reported
     * @return  true if there are values for reporting
     */
    boolean checkReporting(int t_reportTime);
    /**
     * creates json in the outbuffer with all, or the values which should be reported
     * @param boolean t_all all values or only which should be reported
     * @param TGCharbuffer outbuffer  for the output
     */
    void json(const boolean t_all, TGCharbuffer* outbuffer);
    /**
     * creates html in the outbuffer with all values
     * @param TGCharbuffer outbuffer  for the output
     */
    void html(TGCharbuffer* outbuffer);
  private:
    // Start and End of the list of sensors
    TGSensor *firstelement=NULL, *lastelement=NULL;
    /**
     * horizontal HTML output
     */
    void htmlH(TGCharbuffer* outbuffer);
    /**
     * vertical HTML output
     */
    void htmlV(TGCharbuffer* outbuffer);
};

#endif
