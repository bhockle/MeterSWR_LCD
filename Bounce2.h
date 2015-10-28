/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *      
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *      
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *   MA 02110-1301, USA.
 */

/*  * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 Main code by Thomas O Fredericks (tof@t-o-f.info)
 Previous contributions by Eric Lowry, Jim Schimpf and Tom Harkaway
* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 Modified 2014 by Matt Roberts to merge the H and CPP files,
 and to clean up the formatting.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Uncomment the following line for "LOCK-OUT" debounce method
//#define BOUNCE_LOCK-OUT

#ifndef Bounce2_h
#define Bounce2_h

#include <inttypes.h>
#include <Arduino.h>

// define USE_ELAPSED to use wrap-friendly elapsed time calculations
#define USE_ELAPSED
#ifdef USE_ELAPSED
#include "Elapsed.h"
#endif

//
//  Bounce - debouncer class
//
class Bounce {
	private:
		unsigned long previous_millis;
		unsigned long interval_millis;
		uint8_t debouncedState;
		uint8_t unstableState;
		uint8_t pin;
		uint8_t stateChanged;

	public:
		// Create an instance of the bounce library
		Bounce() { this->interval_millis = 10; }

		// Attach to a pin (and also sets initial state)
		void attach(int pin);

		// Sets the debounce interval
		void interval(unsigned long interval_millis) { this->interval_millis = interval_millis; }
		
		// query the debounce interval
		unsigned long interval() const { return this->interval_millis; }

		// Updates the pin
		// Returns 1 if the state changed
		// Returns 0 if the state did not change
		bool update();

		// Returns the updated pin state
		uint8_t read() { return debouncedState; }
};

//
//  Bounce::attach(pin)
//
inline void Bounce::attach(int p) {
	pin = p;
	pinMode(pin, INPUT_PULLUP);	// configure pin as input
	digitalWrite(pin, HIGH);	// enable pull-up on pin
	debouncedState = unstableState = digitalRead(pin);
	#ifdef BOUNCE_LOCK-OUT
	previous_millis = 0;
	#else
	previous_millis = millis();
	#endif
}

//
//  Bounce::update()
//
inline bool Bounce::update()
{
#ifdef BOUNCE_LOCK-OUT
    stateChanged = false;

	// Ignore everything if we are locked out
	#ifdef USE_ELAPSED
	if (Elapsed(millis(), previous_millis) >= interval_millis) {
	#else
	if (millis() - previous_millis >= interval_millis) {
	#endif
		uint8_t currentState = digitalRead(pin);
		if (debouncedState != currentState ) {
			previous_millis = millis();
			debouncedState = currentState;
			stateChanged = true;
		}
	}
	return stateChanged;

#else
	// Lire l'etat de l'interrupteur dans une variable temporaire.
	uint8_t currentState = digitalRead(pin);
	stateChanged = false;

	// Redemarrer le compteur timeStamp tant et aussi longtemps que
	// la lecture ne se stabilise pas.
	if ( currentState != unstableState ) {
		previous_millis = millis();
	#ifdef USE_ELAPSED
	} else if (Elapsed(millis(), previous_millis) >= interval_millis) {
	#else
	} else 	if ( millis() - previous_millis >= interval_millis ) {
	#endif
		// Rendu ici, la lecture est stable

		// Est-ce que la lecture est différente de l'etat emmagasine de l'interrupteur?
		if ( currentState != debouncedState ) {
			debouncedState = currentState;
			stateChanged = true;
		}
	}
	 
	unstableState = currentState;
	return stateChanged;
#endif // BOUNCE_LOCK-OUT
}
#endif // __BOUNCE_H
