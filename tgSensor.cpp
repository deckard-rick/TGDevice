/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
* class TGSensor - a sensor base class
* class TGSensorList - list of sensors
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence CC-BY-NC-SA 4.0, NO COMMERCIAL USE
*  see also: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#include <tgSensor.hpp>
#include <tgLogging.hpp>

//constants to create the html-output of the values (horizontal)
char htmlHSensor1[] = "<table border=\"1\"><tr><th>ID</th>";
char htmlHSensor2[] = "<th>#id#</th>";
char htmlHSensor3[] = "</tr><tr><td>Value</td>";
char htmlHSensor4[] = "<td>#value#</td>";
char htmlHSensor5[] = "</tr><tr><td>[s]</td>";
char htmlHSensor6[] = "<td><small>#sec#</small></td>";
char htmlHSensor7[] = "</tr></table>";

//constants to create the html-output of the values (vertical)
char htmlVSensor1[] = "<table border=\"1\"><tr><th>ID</th><th>Value</th><th>[s]</th></tr>";
char htmlVSensor2[] = "<tr><td>#id#</td><td>#value#</td><td><small>#sec#</small></td></tr>";
char htmlVSensor3[] = "</table>";

//constant for the json values
char jsonSensors1[] = "\"S#i#\" : {\"id\" : \"#id#\",";
char jsonSensors2[] = "\"sec\" : \"#sec#\", \"value\" : \"#value#\"} ";

/**
 * virtual TtgSensor::dogetvalue
 * to get the newValue from the sensor
 */
void TGSensor::dogetvalue()
{
  newValue = 0.0;
}

/**
 * measure one value, set changed ig changed and new messtime
 */
void TGSensor::measure()
{
  //TGLogging::get()->write("TtgSensor::messWert")->crlf();
  dogetvalue();

  if (abs(newValue - value) > *pdelta)
    changed = true;

  value = newValue;
  messTime = millis();
}

/**
 * check weather there are sensors or not
 * @return true if there are sensors
 */
boolean TGSensorsList::hasMembers()
{
  return firstelement != NULL;
}

/**
 * add a sensor to the list
 * @param TGSensor value sensor to add
 */
void TGSensorsList::add(TGSensor* value)
{
  if (firstelement == NULL)
    {
      firstelement = value;
      lastelement = firstelement;
    }
  else
    {
      lastelement->next = value;
      lastelement = lastelement->next;
    }
}

/**
 * read all sensors
 * @return if there is a changed greater then delta, true for needReporting
 */
boolean TGSensorsList::measure()
{
  boolean needReporting = false;
  for (TGSensor *element = firstelement; element != NULL; element = element->next)
    {
      element->measure();
      needReporting = needReporting or element->changed;
      //https://forum.arduino.cc/index.php?topic=442570.0
      yield();
    }

  //TGLogging::get()->crlf();
  return needReporting;
}

/**
 * check wheather there are more values to report after reporttime
 * @param  t_reportTime time [s] after a value as to be reportted
 * @return boolean if there a values for reporting
 */
boolean TGSensorsList::checkReporting(int t_reportTime)
{
    boolean needReporting = false;
    int now = millis();

    for (TGSensor *element = firstelement; element != NULL; element = element->next)
      if (((now - element->reportTime) / 1000 > t_reportTime) or (now - element->reportTime < 0))
        {
          element->changed = true;
          needReporting = true;
        }
    return needReporting;
}

/**
 * creates a json output with all values
 * @param t_all true all, else only values who need reporting
 * @param outbuffer buffer for the result
 */
void TGSensorsList::json(const boolean t_angefordert, TGCharbuffer* outbuffer)
{
  outbuffer->add("\"values\" : { ");

  boolean first = true;
  long now = millis();

  char cbuf[10];
  int i=0;
  for (TGSensor *element = firstelement; element != NULL; element = element->next)
    if (t_angefordert or element->changed) //Die Reported werden müssen, werden zuvor auf changed gesetzt
      {
        if (!first)
          outbuffer->add(", ");
        first = false;

        outbuffer->add(jsonSensors1);
        sprintf(cbuf,"%d",i); outbuffer->replace("i",cbuf);
        outbuffer->replace("id",element->id);

        long sec = (now - element->messTime) / 1000;
        outbuffer->add(jsonSensors2);
        sprintf(cbuf,"%d",sec); outbuffer->replace("sec",cbuf);
        sprintf(cbuf,"%7.2f",element->value); outbuffer->replace("value",cbuf);

        if (!t_angefordert)
          {
            element->changed = false;
            element->reportTime = now;
          }
        i++;
      }

  outbuffer->add("}");
}

/**
 * creates a html table output with all values
 * @param outbuffer [description]
 */
void TGSensorsList::html(TGCharbuffer* outbuffer)
{
  htmlV(outbuffer);
}

/**
 * html output in horizontal
 * @param outbuffer [description]
 */
void TGSensorsList::htmlH(TGCharbuffer* outbuffer)
{
  outbuffer->add(htmlHSensor1);

  int now = millis();
  for (TGSensor *element = firstelement; element != NULL; element = element->next)
    {
       outbuffer->add(htmlHSensor2);
       outbuffer->replace("id",element->id);
       yield();
    }

  outbuffer->add(htmlHSensor3);
  for (TGSensor *element = firstelement; element != NULL; element = element->next)
    {
    outbuffer->add(htmlHSensor4);
    outbuffer->replace("value",element->value);
    yield();
   }

  outbuffer->add(htmlHSensor5);
  for (TGSensor *element = firstelement; element != NULL; element = element->next)
    {
      int sec = (now - element->messTime) / 1000;
      outbuffer->add(htmlHSensor6);
      outbuffer->replace("sec",sec);
      yield();
    }

  outbuffer->add(htmlHSensor7);
}

/**
 * html output in vertical
 * @param outbuffer [description]
 */
void TGSensorsList::htmlV(TGCharbuffer* outbuffer)
{
  outbuffer->add(htmlVSensor1);

  for (TGSensor *element = firstelement; element != NULL; element = element->next)
    {
       outbuffer->add(htmlVSensor2);
       outbuffer->replace("id",element->id);
       outbuffer->replace("value",element->value);
       int now = millis();
       int sec = (now - element->messTime) / 1000;
       outbuffer->replace("sec",sec);
       yield();
    }
  outbuffer->add(htmlVSensor3);
}
