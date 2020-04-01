/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
* class TGLogging - for debug output
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence CC-BY-NC-SA 4.0, NO COMMERCIAL USE
*  see also: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#include <tgDebug.hpp>

//initalize the static object
TGDebug* TGDebug::log = nullptr;

/**
 * get the unique logging object
 * @return logging object
 */
TGDebug* TGDebug::get()
{
  //if not exists then create
  if (log == nullptr)
    {
      log = new TGDebug();
      Serial.begin(9600);
    }
  return log;
}

void TGDebug::openLevel(int lvl, char* msg)
{
  this->changeLevel(lvl,+1,msg);
}

void TGDebug::closeLevel(int lvl, char* msg)
{
  this->changeLevel(lvl,-1,msg);
}

void TGDebug::changeLevel(int lvl, int step, char* msg)
{
  levels[lvl] = levels[lvl] + step;
  //this->write("level[")->write(lvl)->write("]")->write(msg)->write(":")->write(levels[lvl])->crlf();
}

/**
 * write int value to debugging output
 * @param  int value
 * @return  the logging object
 */
TGDebug* TGDebug::write(const int value)
{
  Serial.print(value);
  return this;
}

/**
 * write float value to debugging output
 * @param float value
 * @return  the logging object
 */
TGDebug* TGDebug::write(const float value)
{
  Serial.print(value);
  return this;
}

/**
 * write int value to debugging output
 * @param  nullterminated string value
 * @return  the logging object
 */
TGDebug* TGDebug::write(const char* value)
{
  Serial.print(value);
  return this;
}

/**
 * write String value to debugging output
 * @param  String value
 * @return  the logging object
 */
TGDebug* TGDebug::write(const String& value)
{
  char buf[128];
  value.toCharArray(buf,128);
  write(buf);
}

/**
 * write cr/lf to debugging output
 * @return  the logging object
 */

TGDebug* TGDebug::crlf()
{
  Serial.println("");
  return this;
}
