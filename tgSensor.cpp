#include <tgSensor.hpp>
#include <tgLogging.hpp>

char htmlHSensor1[] = "<table border=\"1\"><tr><th>ID</th>";
char htmlHSensor2[] = "<th>#id#</th>";
char htmlHSensor3[] = "</tr><tr><td>Value</td>";
char htmlHSensor4[] = "<td>#value#</td>";
char htmlHSensor5[] = "</tr><tr><td>[s]</td>";
char htmlHSensor6[] = "<td><small>#sec#</small></td>";
char htmlHSensor7[] = "</tr></table>";

char htmlVSensor1[] = "<table border=\"1\"><tr><th>ID</th><th>Value</th><th>[s]</th></tr>";
char htmlVSensor2[] = "<tr><td>#id#</td><td>#value#</td><td><small>#sec#</small></td></tr>";
char htmlVSensor3[] = "</table>";

char jsonSensors1[] = "\"S#i#\" : {\"id\" : \"#id#\",";
char jsonSensors2[] = "\"sec\" : \"#sec#\", \"value\" : \"#value#\"} ";

void TtgSensor::doGetMessValue(float* pvalue)
{
  *pvalue = 0.0;
}

void TtgSensor::messWert()
{
  //TGLogging::get()->write("TtgSensor::messWert")->crlf();
  doGetMessValue(&newValue);

  if (abs(newValue - value) > *pdelta)
    changed = true;

  value = newValue;
  messTime = millis();
}

boolean TtgSensorsList::hasMembers()
{
  return firstelement != NULL;
}

void TtgSensorsList::add(TtgSensor* value)
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

boolean TtgSensorsList::messWerte()
{
  boolean needReporting = false;
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
      element->messWert();
      needReporting = needReporting or element->changed;
      //https://forum.arduino.cc/index.php?topic=442570.0
      yield();
    }

  TGLogging::get()->crlf();
  return needReporting;
}

boolean TtgSensorsList::checkReporting(int t_reportTime)
{
    boolean needReporting = false;
    int now = millis();

    for (TtgSensor *element = firstelement; element != NULL; element = element->next)
      if (((now - element->reportTime) /1000 > t_reportTime) or (now - element->reportTime < 0))
        {
          element->changed = true;
          needReporting = true;
        }
    return needReporting;
}

void TtgSensorsList::json(const boolean t_angefordert, TGCharbuffer* outbuffer)
{
  outbuffer->add("\"values\" : { ");

  boolean first = true;
  long now = millis();

  char cbuf[10];
  int i=0;
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
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
void TtgSensorsList::html(TGCharbuffer* outbuffer)
{
  htmlV(outbuffer);
}

void TtgSensorsList::htmlH(TGCharbuffer* outbuffer)
{
  outbuffer->add(htmlHSensor1);

  int now = millis();
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
       outbuffer->add(htmlHSensor2);
       outbuffer->replace("id",element->id);
       yield();
    }

  outbuffer->add(htmlHSensor3);
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
    outbuffer->add(htmlHSensor4);
    outbuffer->replace("value",element->value);
    yield();
   }

  outbuffer->add(htmlHSensor5);
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
      int sec = (now - element->messTime) / 1000;
      outbuffer->add(htmlHSensor6);
      outbuffer->replace("sec",sec);
      yield();
    }

  outbuffer->add(htmlHSensor7);
}

void TtgSensorsList::htmlV(TGCharbuffer* outbuffer)
{
  outbuffer->add(htmlVSensor1);

  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
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
