#include <tgActor.hpp>

int TtgActor::setAutoTimes(int t_start, int t_time)
{
  autoStart = t_start;
  autoEnd = autoStart + t_start;
  if (autoEnd >= 86400) autoEnd = 86399; //wir gehen noch nicht über Mitternacht drüber
  return autoEnd;
}

void TtgActor::doCalcStatus()
{
  if ((status == 'A') or (status == 'F')) //autoModus
    {
      boolean c_value = ((autoStart <= *maintime) and (*maintime < autoEnd)) or (*maintime < endTime);
      if (c_value) setStatus('A'); else setStatus('F');
    }
}

void TtgActor::setStatus(char t_status)
{
  if (status != t_status)
    {
      status = t_status;
      changed = true;
    }
}

void TtgActor::action()
{
  if (changed)
    if ((status == 'Y') or (status == 'A'))
      doActivate();
    else if ((status == 'N') or (status == 'O'))
      doDeactivate();
    else
      doAction();
}

void TtgActor::doActivate()
{

}

void TtgActor::doDeactivate()
{

}

void TtgActor::doAction()
{

}

boolean TtgActorsList::hasMembers()
{
  return firstelement != NULL;
}

TtgActor* TtgActorsList::add(TtgActor* t_value)
{
  if (firstelement == NULL)
    {
      firstelement = t_value;
      lastelement = firstelement;
    }
  else
    {
      lastelement->next = t_value;
      lastelement = lastelement->next;
    }
  return t_value;
}

boolean TtgActorsList::action()
{
  boolean needReporting = false;

  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    element->doCalcStatus();

  doCalcStatus();

  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    {
      element->action();
      needReporting = needReporting or element->changed;
    }

  return needReporting;
}

void TtgActorsList::doCalcStatus()
{
}

void TtgActorsList::setStatus(String t_id, char t_status)
{
  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    if (element->id == t_id)
      {
        element->setStatus(t_status);
        element->action();
      }
}

void TtgActorsList::setEndtime(String t_id, int t_endtime)
{
  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    if (element->id == t_id)
      element->endTime = t_endtime;  //activation via automatic timer (max after actionTime [s]
}

String TtgActorsList::getJson(boolean t_angefordert)
{
  String json = "\"actors\" : { ";

  boolean first = true;
  int i = 0;
  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    if (t_angefordert or element->changed)
      {
        if (!first) json += ", ";
        json += "\"A"+String(i)+"\" : {";
        json += "\"id\" : \""+element->id+"\",";  //Die ID ist innerhalb eines Devices eindeutig
        json += ", \"status\" : \""+String(element->status)+"\"";
        json += "\"autostart\" : \""+String(element->autoStart)+"\",";
        json += "\"autoend\" : \""+String(element->autoEnd)+"\",";
        json += "\"endtime\" : \""+String(element->endTime)+"\",";
        json += "}, ";

        if (!t_angefordert) element->changed = false;
      }

  json += "}";

  return json;
}

String TtgActorsList::getHTML()
{
  String html = "<table>";
  int f_endtime = (millis() / 1000) + 600;

  for (TtgActor *element = firstelement; element != NULL; element = element->next)
    {
      char f_status = element->status;
      String f_color;
      if (f_status == 'Y')
        {
          f_status = 'N';
          f_color = "red";
        }
      else if (f_status == 'N')
        {
          f_status = 'A';
          f_color = "lightGreen";
        }
      if (f_status == 'A')
        {
          f_status = 'F';
          f_color = "lightRed";
        }
      if (f_status == 'F')
        {
          f_status = 'N';
          f_color = "red";
        }
      html += "<tr><th><button style=\"color="+f_color+"\">"+element->id+"</button>";  //action = id=element->&status=f_status
      if ((element->status == 'A') or (element->status == 'F'))
        html += "<td><button style=\"color=\"lightGreen\">Intervall 10min</button>";  //action = id=element->&endtime=f_endtime
      html += "</tr>";
    }
  html += "</table>";

  return html;
}
