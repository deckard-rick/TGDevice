/**
*  Projekt TGDevice (Baseclasses/Framework for Arduino ESP8622 devices)
*
*  TGCharbuffer buffer to build longer output strings, without Strings
*
*  Copyright Andreas Tengicki 2018, Germany, 64347 Griesheim (tgdevice@tengicki.de)
*  Licence CC-BY-NC-SA 4.0, NO COMMERCIAL USE
*  see also: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#include <tgCharbuffer.hpp>
#include <tgLogging.hpp>
#include <Arduino.h>

/**
 * public access to the buffer
 * @return the buffer
 */
char* TGCharbuffer::get()
{
  return outbuffer;
}

/**
 * clears the buffer, after this it is empty
 */
void TGCharbuffer::clear()
{
  outpos = 0;
  replacepos = 0;
  outbuffer[0] = '\0';
}

/**
 * adds a null terminted string the the buffer
 * @param value string to add
 */
void TGCharbuffer::add(const char* value)
{
  if (outpos+strlen(value) >= TGCharbuffer::maxOutBuffer)
    TGLogging::get()->write("TGCharbuffer::add Puffer zu klein (")->write(value)->write(")")->crlf();
  else
    {
      strcpy(outbuffer+outpos,value);
      replacepos = outpos;
      outpos += strlen(value);
    }
}

/**
 * replace field #id# with value
 * @param id     field id
 * @param ivalue long unsigned int value
 */
void TGCharbuffer::replace(const char* id, const long unsigned int ivalue)
{
  char value[20];
  sprintf(value,"%d",ivalue);
  replace(id,value);
}

/**
 * replace field #id# with value
 * @param id     field id
 * @param ivalue int value
 */
void TGCharbuffer::replace(const char* id, const int ivalue)
{
  char value[20];
  sprintf(value,"%d",ivalue);
  replace(id,value);
}

/**
 * replace field #id# with value
 * @param id     field id
 * @param fvalue float value
 */
void TGCharbuffer::replace(const char* id, const float fvalue)
{
  char value[20];
  sprintf(value,"%7.2f",fvalue); //' is for thousand seperator BUT DOES NOT WORK'
  //TODO thousand seperator, but need to do it, by my own
  replace(id,value);
}

/**
 * replace field #id# with value
 * @param id     field id
 * @param value nullterminated string value
 */
void TGCharbuffer::replace(const char* id, const char* value)
{
  int modus = 0;
  int lpos = 0; int rpos = 0;

  //delete leading spaces
  int solllen = strlen(value);
  int valStart=0;
  while ((valStart < solllen) and (value[valStart] == ' ')) ++valStart;
  solllen -= valStart;

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
              //to the left # (pos lpos) shift left
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
