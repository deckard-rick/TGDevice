/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
*  Design
*  TGCharbuffer buffer to build longer output strings, without Strings
*
*  TGDevice main class implemented a base devices including http-server
*           for configuration and working with the device
*  TGConfig automated Handling of configuraion parameters
*  TGSensor baseclass to mesassure values
*  TGActor  baseclass to controll/switch on/off something, included timer
*  TGLogging output to Serial (or TCP-Server later) for debugging
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence CC-BY-NC-SA 4.0, NO COMMERCIAL USE
*  see also: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#ifndef TGCHARBUFFER_H
#define TGCHARBUFFER_H

class TGCharbuffer
{
  public:
    /**
     * public access to the buffer char* value
     * @return char buffer
     */
    char* get();
    /**
     * set buffer to empty
     */
    void clear();
    /**
     * add null terminated string to the buffer
     * @param value string to add, null terminated
     */
    void add(const char* value);
    void add(const int ivalue);
    /**
     * look only in the last added part for #id# and replace with value
     * @param id    field to look for
     * @param value value to set (char*. int, float or long unsigned int)
     */
    void replace(const char* id, const char* value);
    void replace(const char* id, const int ivalue);
    void replace(const char* id, const float fvalue);
    void replace(const char* id, const long unsigned int ivalue);
  protected:
  private:
    static const int maxOutBuffer = 2048; //max length of the buffer
    char outbuffer[maxOutBuffer];  //the buffer
    int outpos = 0;  //last position in buffer
    int replacepos = 0;  //last starting position, from here searcing for replacements
};

#endif
