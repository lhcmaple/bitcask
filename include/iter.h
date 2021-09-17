#ifndef ITER_H
#define ITER_H

class Iter {
public:
    virtual bool isValid() = 0;
    virtual void seekToFirst() = 0;
    virtual void next() = 0;
    virtual void *get() = 0;
    virtual ~Iter() {}
};

#endif