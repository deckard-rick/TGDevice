
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
    TGLogging::get()->write("TGCharbuffer::add Puffer zu klein")->crlf();
  else
    {
      strcpy(outbuffer+outpos,value);
      replacepos = outpos;
      outpos += strlen(value);
    }
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
  TGLogging::get()->write("replacePos:")->write(replacepos)->crlf();
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
          int solllen = strlen(value);
          int istlen = i - lpos + 1;
          bool error = false;
          if (solllen < istlen)
            //for(int j=i; j < strlen(outbuffer)+1; ++j)
            //  outbuffer[j-(istlen-solllen)] = outbuffer[j];
            //Das rechte # (Pos i) wird vorgezogen.
            strcpy(outbuffer+i-(istlen-solllen),outbuffer+i);
          else if (solllen > istlen)
            if (strlen(outbuffer)+(solllen-istlen) > TGCharbuffer::maxOutBuffer)
              {
                TGLogging::get()->write("TGCharbuffer::replace Puffer zu klein")->crlf();
                error = true;
              }
            else
              {
                //TGLogging::get()->write("i:")->write(i)->crlf();
                //TGLogging::get()->write("1:")->write(outbuffer)->crlf();
                for(int j=strlen(outbuffer)+1; j > i; --j)
                  outbuffer[j+(solllen-istlen)] = outbuffer[j];
                //TGLogging::get()->write("2:")->write(outbuffer)->crlf();
              }
          //for (int j=0; j < strlen(value); ++j)
            //outbuffer[lpos+j] = value[j];
          if (!error)
            strncpy(outbuffer+lpos,value,strlen(value));
          i = lpos;
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
