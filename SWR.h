/*
 *
 *
 *    SWR.h
 *
 *    SWR calculator.
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

#ifndef SWR_H
#define SWR_H

// use a voltage calculation (sensor is a voltage sensor, not a power sensor)
#define USE_VOLTAGE_CALC

// includes
#include "CircularBuffer.h"
using namespace KK5JY::Collections;

//
//  SWR calculation type
//
class SWR {
	private:
		byte m_FwdPin, m_RefPin;
		word m_Forward, m_Reflected;
		word m_WindowPWR, m_WindowSWR;
		float m_MaxSWR, m_SWR;
		word m_MinPower;

		// SWR averaging
		CircularBuffer<float> *m_BufferSWR;
		float m_SumSWR;

		// power averaging
		CircularBuffer<word> *m_BufferFWD;
		CircularBuffer<word> *m_BufferREF;
		word m_SumFWD;
		word m_SumREF;
		
	public:
		// get the maximum SWR value permitted
		float MaxSWR() const { return m_MaxSWR; }
		
		// set the maximum SWR value permitted
		void MaxSWR(float value) {
			if (value >= 2)
				m_MaxSWR = value;
		}
		
		// query the forward raw value
		word ForwardRaw() const { return m_Forward; }
		
		// query the reflected raw value
		word ReflectedRaw() const { return m_Reflected; }
		
		// query the real SWR value
		float Value() const { return m_SWR; }
		
		// query the SWR window
		word WindowSWR() const { return m_WindowSWR; }
		
		// set the SWR window
		void WindowSWR(byte len) { InitSWR(len); }

		// query the SWR window
		word WindowPower() const { return m_WindowPWR; }
		
		// set the SWR window
		void WindowPower(byte len) { InitPower(len); }
		
		// query the minimum power value
		word MinPower() const { return m_MinPower; }
		
		// set the minimum power value
		void MinPower(word val) { m_MinPower = val; }

	private:
		void InitSWR(byte len) {
			m_WindowSWR = len;
			if (m_BufferSWR) delete m_BufferSWR;
			if (len == 0) return;
			m_BufferSWR = new CircularBuffer<float>(SWR_WINDOW);
			for (byte i = 0; i != SWR_WINDOW; ++i) {
				m_BufferSWR->Add(1.0);
			}
			m_SumSWR = SWR_WINDOW;
		}

		void InitPower(byte len) {
			m_WindowPWR = len;
			if (m_BufferFWD) delete m_BufferFWD;
			if (m_BufferREF) delete m_BufferREF;
			if (len == 0) return;
			m_BufferFWD = new CircularBuffer<word>(PWR_WINDOW);
			m_BufferREF = new CircularBuffer<word>(PWR_WINDOW);
			for (byte i = 0; i != PWR_WINDOW; ++i) {
				m_BufferFWD->Add((word)0);
				m_BufferREF->Add((word)0);
			}
			m_SumFWD = m_SumREF = 0;
		}
		
	public:
		//
		//  SWR(...) - constructor
		//
		SWR(byte fwdPin, byte refPin)
			: m_BufferFWD(0), m_BufferREF(0), m_BufferSWR(0)
		{
			m_FwdPin = fwdPin;
			m_RefPin = refPin;
			m_MaxSWR = 25.0;
			m_WindowPWR = m_WindowSWR = 0;
			m_MinPower = 0;
		}


		//
		//  Poll() - read the SWR from the A/D inputs
		//
		void Poll() {
			// read forward voltage
			word fwd = analogRead(m_FwdPin);
			
			// read reverse voltage
			word ref = analogRead(m_RefPin);
			
			if (m_BufferFWD) {
				word w;
				m_BufferFWD->Remove(w);
				m_SumFWD -= w;
				m_BufferFWD->Add(fwd);
				m_SumFWD += fwd;
				fwd = m_SumFWD / PWR_WINDOW;
			}

			if (m_BufferREF) {
				word w;
				m_BufferREF->Remove(w);
				m_SumREF -= w;
				m_BufferREF->Add(ref);
				m_SumREF += ref;
				ref = m_SumREF / PWR_WINDOW;
			}
			
			m_Forward = fwd;
			m_Reflected = ref;
			
			// compute SWR and return
			float wf;
			if (m_Reflected == 0 || m_Forward < m_MinPower) {
				wf = 1.0;
			} else if (m_Reflected >= m_Forward) {
				wf = m_MaxSWR;
			} else {
				#ifdef USE_VOLTAGE_CALC
				wf = (float)(m_Forward + m_Reflected) / (float)(m_Forward - m_Reflected);
				#else
				wf = (float)m_Forward / (float)m_Reflected;
				wf = sqrt(wf);
				wf = ((float)(1) + wf) / ((float)(1) - wf);
				#endif
				wf = abs(wf);
			}
				
			if (m_BufferSWR) {
				float s;
				m_BufferSWR->Remove(s);
				m_SumSWR -= s;
				m_BufferSWR->Add(wf);
				m_SumSWR += wf;
				wf = m_SumSWR / SWR_WINDOW;
				if (wf < 1.0) wf = 1.0;
			}

			// clip the SWR at a reasonable value
			if (wf > m_MaxSWR) wf = m_MaxSWR;

			// store the final result
			m_SWR = wf;
		}
};

#endif // SWR_H
