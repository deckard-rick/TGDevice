#include <tgSensor.hpp>
#include <tgLogging.hpp>

char htmlSensor1[] = "<table border=\"1\"><tr><th>ID</th>";
char htmlSensor2[] = "<th>#id#</th>";
char htmlSensor3[] = "</tr><tr><td>Value</td>";
char htmlSensor4[] = "<td>#value</td>";
char htmlSensor5[] = "</tr><tr><td>[s]</td>";
char htmlSensor6[] = "<td><small>#sec#</small></td>";
char htmlSensor7[] = "</tr></table>";

char jsonSensors1[] = "\"S#i#\" : {\"id\" : \"#id#\",";
char jsonSensors2[] = "\"sec\" : \"#sec#\", \"value\" : \"#value#\"} ";


float TtgSensor::doGetMessValue()
{
  return 0;
}

void TtgSensor::messWert()
{
  //TGLogging::get()->write("TtgSensor::messWert")->crlf();
  newValue = doGetMessValue();

  //TGLogging::get()->write("newValue:")->write(newValue)->crlf();
  //TGLogging::get()->write("pdelta:")->write(*pdelta)->crlf();

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
  TGLogging::get()->write("TtgSensorsList::messWerte")->crlf();
  boolean needReporting = false;
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
      element->messWert();
      needReporting = needReporting or element->changed;
    }
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

void TtgSensorsList::json(const boolean t_angefordert, TGCharbuffer outbuffer)
{
  outbuffer.add("\"values\" : { ");

  boolean first = true;
  long now = millis();

  char cbuf[10];
  int i=0;
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    if (t_angefordert or element->changed) //Die Reported werden müssen, werden zuvor auf changed gesetzt
      {
        if (!first)
          outbuffer.add(", ");
        first = false;

        outbuffer.add(jsonSensors1);
        sprintf(cbuf,"%d",i); outbuffer.replace("i",cbuf);
        outbuffer.replace("id",element->id);

        long sec = (now - element->messTime) / 1000;
        outbuffer.add(jsonSensors2);
        sprintf(cbuf,"%d",sec); outbuffer.replace("sec",cbuf);
        sprintf(cbuf,"%7.2f",element->value); outbuffer.replace("value",cbuf);

        if (!t_angefordert)
          {
            element->changed = false;
            element->reportTime = now;
          }
        i++;
      }

  outbuffer.add("}");
}

void TtgSensorsList::html(TGCharbuffer* outbuffer)
{
  outbuffer->add(htmlSensor1);

  int now = millis();

  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
       outbuffer->add(htmlSensor2);
       outbuffer->replace("id",element->id);
    }
  outbuffer->add(htmlSensor3);
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
    outbuffer->add(htmlSensor4);
    outbuffer->replace("value",element->value);
   }
  outbuffer->add(htmlSensor5);
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
      int sec = (now - element->messTime) / 1000;
      outbuffer->add(htmlSensor6);
      outbuffer->replace("sec",sec);
    }
  outbuffer->add(htmlSensor7);
}
