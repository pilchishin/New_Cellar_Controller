#ifndef WIRE_H
#define WIRE_H
#include <stdint.h>
class TwoWire {
public:
    void begin() {}
    void end() {}
    void setWireTimeout(uint32_t timeout, bool reset) {}
};
extern TwoWire Wire;
#endif
