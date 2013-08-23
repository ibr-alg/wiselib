/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/


/****
 
 main() is now in arduino_application.h, so all
 code can exist in a single compilation unit.

 
 
#include "external_interface/arduino/arduino_os.h"

void application_main(wiselib::ArduinoOsModel&);

int main(int argc, const char** argv )
{
	wiselib::ArduinoOsModel app_main_arg;
	
	init();
	
#if defined(USBCON)
	USBDevice.attach();
#endif
	
	pinMode(13, OUTPUT);
    Serial.begin(9600);
	application_main(app_main_arg);
	
	for(;;) { }
	
	return 0;
}
****/

