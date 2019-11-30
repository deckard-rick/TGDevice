
#include <tgCharbuffer.hpp>
#include <tgLogging.hpp>
#include <Arduino.h>

void TGCharbuffer::clear()
{
  outpos = 0;
  replacepos = 0;
  outbuffer[0] = '\0';
}

void TGCharbuffer::add(const char* value)
{
  if (outpos+strlen(value) >= TGCharbuffer::maxOutBuffer)
    TGLogging::get()->write("TGCharbuffer::add Puffer zu klein (")->write(value)->write(")")->crlf();
  else
    {
      strcpy(outbuffer+outpos,value);
      replacepos = outpos;
      outpos += strlen(value);
      //TGLogging::get()->write("TGCharbuffer::add(")->write(outpos)->write("):")->crlf();
      //TGLogging::get()->write(outbuffer)->crlf();
    }
}

void TGCharbuffer::replace(const char* id, const long unsigned int ivalue)
{
  char value[20];
  sprintf(value,"%d",ivalue);
  replace(id,value);
}

void TGCharbuffer::replace(const char* id, const int ivalue)
{
  char value[20];
  sprintf(value,"%d",ivalue);
  replace(id,value);
}

void TGCharbuffer::replace(const char* id, const float fvalue)
{
  char value[20];
  sprintf(value,"%7.2f",fvalue);
  replace(id,value);
}

void TGCharbuffer::replace(const char* id, const char* value)
{
  int modus = 0;
  int lpos = 0; int rpos = 0;

  //führende Spaces weg
  int solllen = strlen(value);
  int valStart=0;
  while ((valStart < solllen) and (value[valStart] == ' ')) ++valStart;
  solllen -= valStart;

  //TGLogging::get()->write("replacePos:")->write(replacepos)->crlf();
  for(int i=replacepos; i < strlen(outbuffer); ++i)
    {
      if ((modus == 0) and (outbuffer[i] =='#')) //looking for starting #
        {
          modus = 1;
          lpos = i;
        }
      else if ((modus == 1) and (outbuffer[i] ==  '_')) //blank in id, continue
        ;
      else if ((modus == 1) and (outbuffer[i] ==  '#')) //id founds
        {
          //012345678
          //...#...#
          int istlen = i - lpos + 1;

          bool error = false;
          if (solllen < istlen)
            {
              //Zum linken # (Pos lpos) wird vorgezogen.
              strcpy(outbuffer+lpos, outbuffer+lpos+(istlen-solllen));
              outpos -= istlen-solllen;
            }
          else if (solllen > istlen)
            if (strlen(outbuffer)+(solllen-istlen) > TGCharbuffer::maxOutBuffer)
              {
                TGLogging::get()->write("TGCharbuffer::replace Puffer zu klein")->crlf();
                error = true;
              }
            else
              {
                for(int j=strlen(outbuffer)+1; j > i; --j)
                  outbuffer[j+(solllen-istlen)] = outbuffer[j];
                outpos += solllen-istlen;
              }
          if (!error)
            strncpy(outbuffer+lpos,value+valStart,solllen);
          i = lpos+solllen-1;
          modus = 0;
        }
      //012345678
      //abc#id__#def
      else if ((modus == 1) and (outbuffer[i] != id[i-(lpos+1)]) ) //not found, look for #
        modus = 2;
      else if ((modus == 2) and (outbuffer[i] == '#'))
        modus = 0;
    }
}

char* TGCharbuffer::getout()
{
  return outbuffer;
}
