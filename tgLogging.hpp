/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*/
#ifndef TGLOGGING_H
#define TGLOGGING_H

#include <Arduino.h>
#include <tgLogging.hpp>

class TGLogging
{
  public:
    static TGLogging* get();
    void setModus(char t_modus);
    TGLogging* write(const char* out);
    TGLogging* write(const int out);
    TGLogging* write(const float out);
    TGLogging* write(const String& out);
    TGLogging* crlf();
  private:
    static TGLogging* log;
    char modus = ' ';
};
#endif
