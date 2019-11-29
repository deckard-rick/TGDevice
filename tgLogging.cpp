/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*/

#include <tgLogging.hpp>

TGLogging* TGLogging::log = nullptr;

TGLogging* TGLogging::get()
{
  if (log == nullptr)
    log = new TGLogging();
  return log;
}

void TGLogging::setModus(char t_modus)
{
  modus = t_modus;
  if (modus == 'S')
    Serial.begin(9600);
}

TGLogging* TGLogging::write(const int out)
{
  if (modus == 'S')
    Serial.print(out);
  return this;
}

TGLogging* TGLogging::write(const float out)
{
  if (modus == 'S')
    Serial.print(out);
  return this;
}

TGLogging* TGLogging::write(const char* out)
{
  if (modus == 'S')
    Serial.print(out);
  yield();
  return this;
}

TGLogging* TGLogging::write(const String& out)
{
  char buf[128];
  out.toCharArray(buf,128);
  write(buf);
}

TGLogging* TGLogging::crlf()
{
  if (modus == 'S')
    Serial.println("");
  return this;
}
