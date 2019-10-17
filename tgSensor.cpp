#include <tgSensor.hpp>

float TtgSensor::doGetMessValue()
{
  return 0;
}

void TtgSensor::messWert()
{
  newValue = doGetMessValue();

  if (abs(newValue - value) > *(pdelta))
    changed = true;
  messTime = millis();
}

boolean TtgSensorsList::hasSensors()
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

//Man könnte das auch in die sensorsGruppe tun, Zugriff auf deviceVersion und DeviceID notwendig, oder Header Trennen
String TtgSensorsList::getJson(const boolean t_angefordert)
{
  String json = "\"values\" : { ";

  boolean j_first = true;
  long now = millis();

  int i=0;
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    if (t_angefordert or element->changed) //Die Reported werden müssen, werden zuvor auf changed gesetzt
      {
        if (!(j_first)) json += ", ";
        j_first = false;
        json += "\"VAL"+String(i)+"\" : {";
        json += "\"id\" : \""+element->id+"\",";
        long sec = (now - element->messTime) / 1000;
        json += "\"sec\" : \""+String(sec)+"\",";
        json += ", \"value\" : \""+String(element->value)+"\"";
        json += "}, ";
        if (!t_angefordert)
          {
            element->changed = false;
            element->reportTime = now;
          }
        i++;
      }

  json += "} }";

  return json;
}

String TtgSensorsList::getHTML()
{
  String html = "<table><tr>";
  int now = millis();

  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    html += "<th>"+element->id+"</th>";
  html += "</tr><tr>";
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    html += "<td>"+String(element->value)+"</td>";
  html += "</tr><tr>";
  for (TtgSensor *element = firstelement; element != NULL; element = element->next)
    {
      int sec = (now - element->messTime) / 1000;
      html += "<td><small>"+String(sec)+"</small></td>";
    }
  html += "</tr></table>";
  return html;
}
