#include <tgActor.hpp>

const char* actorStatus[] = {"Y","N","A","F"};
const char* actorColor[] = {"green","red","lightgreen","lightred"};

const char htmlActor1[] = "<table>";
char htmlActor2[] = "<tr><th><button style=\"color=#color#\">#id#</button>";  //action = id=element->&status=f_status
char htmlActor3[] = "<td><button style=\"color=\"#color#\">Intervall 10min</button>";  //action = id=element->&endtime=f_endtime
char htmlActor4[] = "</tr>";
char htmlActor5[] = "</table>";

char jsonActors1[] = "\"A#i#\" : {\"id\" : \"#id#\", \"status\" : \"#status#\", ";
char jsonActors2[] = "\"autostart\" : \"#autostart#\", \"autoend\" : \"#autoend#\", \"endtime\" : \"#endtime#\"}";

int TGActor::setAutoTimes(int t_start, int t_time)
{
  autoStart = t_start;
  autoEnd = autoStart + t_time;
  if (autoEnd >= 86400) autoEnd = 86399; //wir gehen noch nicht über Mitternacht drüber
  return autoEnd;
}

void TGActor::doCalcStatus()
{
  if ((status == 'A') or (status == 'F')) //autoModus
    {
      boolean c_value = ((autoStart <= *maintime) and (*maintime < autoEnd)) or (*maintime < endTime);
      if (c_value) setStatus('A'); else setStatus('F');
    }
}

void TGActor::setStatus(char t_status)
{
  if (status != t_status)
    {
      status = t_status;
      changed = true;
    }
}

void TGActor::action()
{
  if (changed)
    if ((status == 'Y') or (status == 'A'))
      doActivate();
    else if ((status == 'N') or (status == 'O'))
      doDeactivate();
    else
      doAction();
}

void TGActor::doActivate()
{

}

void TGActor::doDeactivate()
{

}

void TGActor::doAction()
{

}

boolean TGActorsList::hasMembers()
{
  return firstelement != NULL;
}

TGActor* TGActorsList::add(TGActor* t_value)
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

boolean TGActorsList::action()
{
  boolean needReporting = false;

  for (TGActor *element = firstelement; element != NULL; element = element->next)
    element->doCalcStatus();

  doCalcStatus();

  for (TGActor *element = firstelement; element != NULL; element = element->next)
    {
      element->action();
      needReporting = needReporting or element->changed;
    }

  return needReporting;
}

void TGActorsList::doCalcStatus()
{
}

void TGActorsList::setStatus(char* t_id, char t_status)
{
  for (TGActor *element = firstelement; element != NULL; element = element->next)
    if (strcmp(element->id,t_id) == 0)
      {
        element->setStatus(t_status);
        element->action();
      }
}

void TGActorsList::setEndtime(char* t_id, int t_endtime)
{
  for (TGActor *element = firstelement; element != NULL; element = element->next)
    if (strcmp(element->id,t_id) == 0)
      element->endTime = t_endtime;  //activation via automatic timer (max after actionTime [s]
}

void TGActorsList::json(boolean t_angefordert, TGCharbuffer* outbuffer)
{
  outbuffer->add("\"actors\" : { ");

  char cbuf[10];
  boolean first = true;
  int i = 0;
  for (TGActor *element = firstelement; element != NULL; element = element->next)
    if (t_angefordert or element->changed)
      {
        if (!first)
          outbuffer->add(", ");
        first = false;

        outbuffer->add(jsonActors1);
        sprintf(cbuf,"%d",i); outbuffer->replace("i",cbuf);
        outbuffer->replace("id",element->id);
        outbuffer->replace("status",element->status);

        outbuffer->add(jsonActors2);
        sprintf(cbuf,"%d",element->autoStart); outbuffer->replace("autostart",cbuf);
        sprintf(cbuf,"%d",element->autoEnd); outbuffer->replace("autoend",cbuf);
        sprintf(cbuf,"%d",element->endTime); outbuffer->replace("endtime",cbuf);

        if (!t_angefordert) element->changed = false;
        ++i;
      }

  outbuffer->add("}");
}

void TGActorsList::html(TGCharbuffer* outbuffer)
{
  outbuffer->add(htmlActor1) ; String html = "<table>";
  int f_endtime = (millis() / 1000) + 600;

  for (TGActor *element = firstelement; element != NULL; element = element->next)
    {
      int index = 1;
      switch (element->status)
        {
          case 'Y': index = 1; break;
          case 'N': index = 2; break;
          case 'A': index = 3; break;
          case 'F': index = 4; break;
          default:  index = 1; break;
        }
      outbuffer->add(htmlActor2);
      outbuffer->replace("id",element->id);
      outbuffer->replace("color",actorColor[index]);
      outbuffer->replace("status",actorStatus[index]);
      if ((element->status == 'A') or (element->status == 'F'))
        {
          outbuffer->add(htmlActor3);
          outbuffer->replace("id",element->id);
          outbuffer->replace("color",actorColor[2]);
        }
      outbuffer->add(htmlActor4);
    }
  outbuffer->add(htmlActor5);
}
