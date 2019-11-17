
#include <tgCharbuffer.hpp>
#include <Arduino.h>

void TGCharbuffer::clear()
{
  outpos = 0;
  replacepos = 0;
  outbuffer[0] = '\0';
}

void TGCharbuffer::add(const char* value)
{
  replacepos = outpos;
  strcpy(outbuffer,value);
  outpos += strlen(value);
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
          if (solllen < istlen)
            //for(int j=i; j < strlen(outbuffer)+1; ++j)
            //  outbuffer[j-(istlen-solllen)] = outbuffer[j];
            strcpy(outbuffer+i-(istlen-solllen),outbuffer+i);
          else if (solllen > istlen)
            for(int j=strlen(outbuffer)+1; j > i; --j)
              outbuffer[j+(istlen-solllen)] = outbuffer[j];
          //for (int j=0; j < strlen(value); ++j)
            //outbuffer[lpos+j] = value[j];
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
