/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
*  Design
*  TGLogging output to Serial (or TCP-Server later) for debugging

*  TGDevice main class implemented a base devices including http-server
            for configuration and working with the device
*  TGConfig automated Handling of configuraion parameters
*  TGSensor baseclass to mesassure values
*  TGActor  baseclass to controll/switch on/off something, included timer*
*  TGCharbuffer buffer to build longer output strings, without Strings
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
#ifndef TGLOGGING_H
#define TGLOGGING_H

#include <Arduino.h>
#include <tgLogging.hpp>

/**
 * Logging class for debug
 */
class TGLogging
{
  public:
    /**
     * static funtion for access to the logger
     * @return unique logging object
     */
    static TGLogging* get();
    /**
     * (S)erial, (N)one
     * @param t_modus S or N
     *
     * TODO later with logging to tcp-server or someting else
     */
    void setModus(char t_modus);
    /**
     * write a null terminated string, int, float or String value
     * @param  out value to write
     * @return     logging object for concatination
     */
    TGLogging* write(const char* value);
    TGLogging* write(const int value);
    TGLogging* write(const float value);
    TGLogging* write(const String& value);
    //TGLogging* writeState(const int state);
    /**
     * output of a CR/LF
     * @return     logging object for concatination
     */
    TGLogging* crlf();
  private:
    static TGLogging* log;  //static unique logging object
    char modus = ' ';       //modus S/N/...
};

#endif
