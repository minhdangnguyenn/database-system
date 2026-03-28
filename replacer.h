#ifndef REPLACER_H
#define REPLACER_H

class Repalcer {
public: 
    virtual bool evictVictim() = 0;
    virtual void  pin(int, int) = 0;
    virtual unpin (int, int) = 0;
    virtual size_t size() = 0;
    virtual ~Replacer() = default;
}

#endif // REPLACER_H