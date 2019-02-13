#include "Switch.h"

// minimalny czas podtrzymania przekaźników
#define MIN_HOLD_TIME 10000L
// maksymalny czas podtrzymania przekaźników
#define MAX_HOLD_TIME 60000L
// pin do którego podpiety jest potencjometr do regulacji czasu podtrzymania
#define HOLD_TIME_PIN A4
// offset przy numeracji pinów (piny 0 i 1 są nieużywane - TX / RX)
#define PIN_OFFSET 2
// liczba przekaźników
#define RELAYS 8

#define DEBUG

#ifdef DEBUG
	#define INIT_SERIAL Serial.begin(9600)
	#define PRINT(x) Serial.print(x)
	#define PRINTLN(x) Serial.println(x);
#else
	#define INIT_SERIAL {}
	#define PRINT(x) {}
	#define PRINTLN(x) {}
#endif


unsigned long holdTime;

unsigned long readHoldTime() {
	unsigned int value = analogRead(HOLD_TIME_PIN);
	unsigned long holdTime = (value / 1023.0) * (MAX_HOLD_TIME - MIN_HOLD_TIME) + MIN_HOLD_TIME;
	return holdTime;
}

class Relay {
	byte pin;
	bool active = false;
	bool longPressed = false;
	unsigned long lastPushed = 0;
	Relay * oppositeRelay;

	void activate() {
		if (!active) {
			if (oppositeRelay->active) {
				oppositeRelay->deactivate();
			}
			PRINT("Activating pin ");
			PRINTLN(pin);
			digitalWrite(pin, LOW);
			active = true;
		}
	}

	void deactivate() {
		if (active) {
			PRINT("Deactivating pin ");
			PRINTLN(pin);
			digitalWrite(pin, HIGH);
			active = false;
		}
	}

public:
	Relay(byte pin) {
		this->pin = pin;

		pinMode(this->pin, OUTPUT);
		digitalWrite(this->pin, HIGH); // inverted logic
	}

	void handleButtonPushed() {
		if (active) {
			deactivate();
		} else {
			activate();
			lastPushed = millis();
		}
	}

	void handleButtonLongPressed() {
		activate();
		longPressed = true;
	}

	void handleButtonReleased() {
		if (longPressed) {
			deactivate();
			longPressed = false;
		}
	}

	void pollExpired() {
		if (active && !longPressed && isExpired()) {
			PRINT("Expired: ");
			PRINTLN(pin);
			deactivate();
		}
	}

	bool isExpired() {
		return (millis() - lastPushed) > holdTime;
	}

	void setOppositeRelay(Relay& relay) {
		this->oppositeRelay = &relay;
		relay.oppositeRelay = this;
	}
};

class Module {
	Relay ** relays = new Relay*[RELAYS];
	Switch ** buttons = new Switch*[RELAYS];

	void updateState(Switch * button, Relay * relay) {
		button->poll();

		if (button->pushed()) {
			PRINT("Pushed button ");
			PRINTLN(button->getPin());
			relay->handleButtonPushed();
		} else if (button->longPress()) {
			PRINT("Long pressed button ");
			PRINTLN(button->getPin());
			relay->handleButtonLongPressed();
		} else if (button->released()) {
			PRINT("Released button ");
			PRINTLN(button->getPin());
			relay->handleButtonReleased();
		} else {
			relay->pollExpired();
		}
	}

public:
	Module() {
		INIT_SERIAL;
		PRINTLN("Initializing module");
		for (byte i = 0; i < RELAYS; i++) {
			buttons[i] = new Switch(i + PIN_OFFSET);
			relays[i] = new Relay(i + 8 + PIN_OFFSET);
		}
		for (byte i = 0; i < RELAYS; i+=2) {
			relays[i]->setOppositeRelay(*relays[i+1]);
		}
	}

	void updateState() {
		for (byte i = 0; i < RELAYS; i++) {
			updateState(buttons[i], relays[i]);
		}
	}
};

Module module;

void setup() {
	PRINTLN("Ready");
}

void loop() {
	holdTime = readHoldTime();
	module.updateState();
}
