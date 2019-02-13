#ifndef SWITCH_H
#define SWITCH_H
#include <PCF8574.h>

class Switch {
public:
	Switch(byte _pin, byte PinMode = INPUT, bool polarity = HIGH);
	bool poll(); // Returns 1 if switched
	bool on();
	bool pushed(); // will be refreshed by poll()
	bool released(); // will be refreshed by poll()
	bool longPress(); // will be refreshed by poll()
	byte getPin();

	unsigned long _switchedTime, pushedTime;

protected:
	const byte pin;
	const bool polarity;
	bool level, _switched, _longPress, longPressLatch;
};

#endif
