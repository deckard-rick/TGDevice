#ifndef TGCHARBUFFER_H
#define TGCHARBUFFER_H

const int maxOutBuffer = 500;

class TGCharbuffer
{
  public:
    char* getout();
    void clear();
    void add(const char* value);
    void replace(const char* id, const char* value);
    void replace(const char* id, const int ivalue);
    void replace(const char* id, const float fvalue);
  protected:

  private:
    char outbuffer[maxOutBuffer];
    int outpos = 0;
    int replacepos = 0;
};

#endif
