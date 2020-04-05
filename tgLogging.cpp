/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
* class TGLogging - for debug output
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence CC-BY-NC-SA 4.0, NO COMMERCIAL USE
*  see also: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#include <tgLogging.hpp>

//initalize the static object
TGLogging* TGLogging::log = nullptr;

/**
 * get the unique logging object
 * @return logging object
 */
TGLogging* TGLogging::get()
{
  //if not exists then create
  if (log == nullptr)
    log = new TGLogging();
  return log;
}

/**
 * set the modus of the debug output
 * @param S Serial
 *
 * TODO logging to tcp-Server or something else
 */
void TGLogging::setModus(char t_modus)
{
  modus = t_modus;
  if (modus == 'S')
    Serial.begin(9600);
}

/**
 * write int value to debugging output
 * @param  int value
 * @return  the logging object
 */
TGLogging* TGLogging::write(const int value)
{
  if (modus == 'S')
    Serial.print(value);
  /* schöne Idee mit dem Internet debugging
     aber ich brauche auch eine Empfangsstation
     (oder einen syslog Server)
  else if (modus == 'I')
    WiFiClient client;

    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        return;
    }
    /* This will send the data to the server
    client.print("hello world");
    client.stop();
  */
  return this;
}

/**
 * write float value to debugging output
 * @param float value
 * @return  the logging object
 */
TGLogging* TGLogging::write(const float value)
{
  if (modus == 'S')
    Serial.print(value);
  return this;
}

/**
 * write int value to debugging output
 * @param  nullterminated string value
 * @return  the logging object
 */
TGLogging* TGLogging::write(const char* value)
{
  if (modus == 'S')
    Serial.print(value);
  yield();
  return this;
}

/**
 * write String value to debugging output
 * @param  String value
 * @return  the logging object
 */
TGLogging* TGLogging::write(const String& value)
{
  char buf[128];
  value.toCharArray(buf,128);
  write(buf);
}

/**
 * write cr/lf to debugging output
 * @return  the logging object
 */

TGLogging* TGLogging::crlf()
{
  if (modus == 'S')
    Serial.println("");
  return this;
}

/**
TGLogging* TGLogging::writeState(const int state)
{
  if (state == 0) write("CLOSED");
  else if (state == 1) write("LISTEN");
  else if (state == 2) write("SYN_SENT");
  else if (state == 3) write("SYN_RCVD");
  else if (state == 4) write("ESTABLISHED");
  else if (state == 5) write("FIN_WAIT_1");
  else if (state == 6) write("FIN_WAIT_2");
  else if (state == 7) write("CLOSE_WAIT");
  else if (state == 8) write("CLOSING");
  else if (state == 9) write("LAST_ACK");
  else if (state == 10) write("TIME_WAIT");
  else write("UNKNOWN:")->write(state);
  return this;
}
*/
