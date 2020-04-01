/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
*  Design
*  TGDebug output to Serial for debugging, with some level extras

*  Copyright Andreas Tengicki 2020, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence CC-BY-NC-SA 4.0, NO COMMERCIAL USE
*  see also: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/
#ifndef TGDEBUG_H
#define TGDEBUG_H

#include <Arduino.h>

/**
 * Logging class for debug
 */
class TGDebug
{
  public:
    /**
     * static funtion for access to the logger
     * @return unique logging object
     */
    static TGDebug* get();

    void openLevel(int lvl, char* msg);
    void closeLevel(int lvl, char* msg);
    void changeLevel(int lvl, int step, char* msg);

    /**
     * write a null terminated string, int, float or String value
     * @param  out value to write
     * @return     logging object for concatination
     */
    TGDebug* write(const char* value);
    TGDebug* write(const int value);
    TGDebug* write(const float value);
    TGDebug* write(const String& value);
    /**
     * output of a CR/LF
     * @return     logging object for concatination
     */
    TGDebug* crlf();
  private:
    static TGDebug* log;  //static unique logging object
    int levels[5];
};

#endif
