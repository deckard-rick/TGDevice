#ifndef TGCHARBUFFER_H
#define TGCHARBUFFER_H

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
    static const int maxOutBuffer = 150;
    char outbuffer[maxOutBuffer];
    int outpos = 0;
    int replacepos = 0;
};

#endif
