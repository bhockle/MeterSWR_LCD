/*
 *
 *
 *    LightBar.h
 *
 *    Software-driven GPIO LED light bar interface.
 *
 *    License: GNU General Public License Version 3.0.
 *    
 *    Copyright (C) 2014 by Matthew K. Roberts, KK5JY. All rights reserved.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *    
 *    This program is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see: http://www.gnu.org/licenses/
 *
 *
 */

#ifndef __LIGHT_BAR_H
#define __LIGHT_BAR_H

#include "Arduino.h"

//
//  a bar-graph item
//
struct LightBarItem {
	// the "on" threshold
	float Threshold;
	
	// the I/O pin
	uint8_t Pin;
	
	// true if the threshold is inverted (an "off" threshold)
	bool Inverted;
};


//
//  a bar-graph
//
class LightBar {
	private:
		const LightBarItem *m_Elements;
		const uint8_t m_Length;
		
	public:
		LightBar(const LightBarItem *elements, const size_t len) : m_Elements(elements), m_Length(len) {
			// nop
		}
		
		void Initialize() {
			for (uint8_t i = 0; i != m_Length; ++i) {
				pinMode(m_Elements[i].Pin, OUTPUT);
				digitalWrite(m_Elements[i].Pin, m_Elements[i].Inverted ? HIGH : LOW);
			}
		}
		
		void Update(float value) const {
			const LightBarItem *e = 0;
			uint8_t i = 0;
			for (i = 0, e = m_Elements; i != m_Length; ++i, ++e) {
				bool state = (value >= e->Threshold);
				if (e->Inverted) state = !state;
				byte pin = state ? HIGH : LOW;
				digitalWrite(e->Pin, pin);
				
				#if DEBUG
				Serial.print("DEBUG: bar[");
				Serial.print(m_Elements[i].Pin);
				Serial.print("] = ");
				Serial.println(pin);
				#endif
			}
		}
};

#endif
