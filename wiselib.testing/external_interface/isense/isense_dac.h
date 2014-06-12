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

/*
* Author: Fabian Bormann - Google Summer of Code 2014 - Software Audio Interface
*/

#ifndef _ISENSE_DAC_H
#define	_ISENSE_DAC_H

#include <AppHardwareApi.h>

namespace wiselib {
   	template<typename OsModel_P>
   	class iSenseDAC {
   	public:
      	typedef OsModel_P OsModel;
      	typedef iSenseDAC<OsModel> self_type;
      	typedef self_type* self_pointer_t;

      	iSenseDAC(isense::Os& os) : os_ (os) {}

      	void init() {
      		uint8_t output =  E_AHI_AP_DAC_2;
			vAHI_ApConfigure(	E_AHI_AP_REGULATOR_ENABLE, 	// Enable/Disable the analogue peripheral regulator (sourced by VDD1)
						E_AHI_AP_INT_DISABLE,				// Enable/Disable interrupt when ADC conversion completes
						E_AHI_AP_SAMPLE_2, 					// Sampling interval in terms of divided clock periods (2,4,6 or 8)
						E_AHI_AP_CLOCKDIV_2MHZ,				// Clock divisor (E_AHI_AP_CLOCKDIV_2MHZ, 1MHZ, E_AHI_AP_CLOCKDIV_500KHZ, E_AHI_AP_CLOCKDIV_250KHZ), 500 is recommended
						E_AHI_AP_INTREF	);					// Source of reference voltages Vref (ext or int)

			/* wait until adc powered up */
			while(!bAHI_APRegulatorEnabled()) { /* WAIT */ }

			// E_AHI_AP_INPUT_RANGE_1
			// E_AHI_AP_INPUT_RANGE_2
			vAHI_DacEnable(		output,				// E_AHI_AP_DAC_1 or E_AHI_AP_DAC_2
						E_AHI_AP_INPUT_RANGE_2,		// Outpult voltage range (1=Vref=1.18V, 2=2*Vref=2.36V)
						E_AHI_DAC_RETAIN_DISABLE, 	// Unused, always use DISABLE
						0 );						// Caution: Only used on JN5148 - unused on the JN5139
		}

		void write( uint16_t value ) {
			// JN5139: 11 bit --> 0 - 2047
			// JN5148: 12 bit --> 0 - 4095
			vAHI_DacOutput(	E_AHI_AP_DAC_2,				// E_AHI_AP_DAC_1 or E_AHI_AP_DAC_2
					value );					// Only the 11 or 12 least significant bits will be used!

			// Wait for the DAC to output the correct value
			// This is disabled for speed reasons (works just fine without it!)
			//while(bAHI_DacPoll()) { /* WAIT */ }
		}

   	private:
      	isense::Os& os() { 
      		return os_; 
      	}
      	isense::Os& os_;
   	};
}
#endif
