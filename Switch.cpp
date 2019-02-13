#include "Arduino.h"
#include "Switch.h"

#define DEBOUNCE_DELAY 50
#define LONG_PRESS_DELAY 400


Switch::Switch(byte _pin, byte PinMode, bool polarity) : pin(_pin), polarity(polarity) {
	pinMode(pin, PinMode);
	_switchedTime = millis();
	level = digitalRead(pin);
}

bool Switch::poll() {
	_longPress = false;// _doubleClick = false;
	bool newlevel = digitalRead(pin);

	if (!longPressLatch) {
		_longPress = on() && ((long) (millis() - pushedTime) > LONG_PRESS_DELAY); // true just one time between polls
		longPressLatch = _longPress; // will be reset at next switch
	}

	if ((newlevel != level) & (millis() - _switchedTime >= DEBOUNCE_DELAY)) {
		_switchedTime = millis();
		level = newlevel;
		_switched = 1;
		longPressLatch = false;

		if (pushed()) {
			pushedTime = millis();
		}
		return _switched;
	}
	return _switched = 0;
}

bool Switch::on() {
	return !(level ^ polarity);
}

bool Switch::pushed() {
	return _switched && !(level ^ polarity);
}

bool Switch::released() {
	return _switched && (level ^ polarity);
}

bool Switch::longPress() {
	return _longPress;
}

byte Switch::getPin() {
	return pin;
}
