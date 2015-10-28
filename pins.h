/*
 *
 *
 *    pins.h
 *
 *    Pin definitions.
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

#ifndef SWR_PINS_H
#define SWR_PINS_H

//
//  I/O pin definitions
//

// blinks when a command is issued (comment line to disable)
#define CMD_LED (13)

// analog input for FORWARD power reading
#define FWD_PIN (1)

// analog input for REFLECTED power reading
#define REF_PIN (3)

#endif // SWR_PINS_H
