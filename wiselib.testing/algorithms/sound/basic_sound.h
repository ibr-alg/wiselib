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

#ifndef __ALGORITHMS_SOUND_BASIC_SOUND_H__
#define __ALGORITHMS_SOUND_BASIC_SOUND_H__

#include <util/standalone_math.h>
//#include "external_interface/external_interface.h"
#include "sine_lut.h"
#include "notes.h"

namespace wiselib {

template<typename OsModel_P, typename DAC_P, typename Debug_P>
   class BasicSound {

      public:
         typedef OsModel_P OsModel;
         typedef StandaloneMath<OsModel> Math;
         typedef Debug_P Debug;
         typedef DAC_P DAC;

         int init(DAC& dac, Debug& debug) { 
            dac_ = &dac;
            debug_ = &debug;
            return OsModel::SUCCESS;
         }

         uint16_t flipBytes(uint16_t bytes) {
            uint16_t bytes_low = bytes << 8;
            uint16_t bytes_high = bytes >> 8;  
            return bytes_low+bytes_high;          
         }

         void play( const uint8_t sounddata_data[] , uint32_t length ) {
            #ifdef ISENSE_JENNIC_JN5148
               // Since we are performing a very-long loop, we need to disable the watchdog
               vAHI_WatchdogStop();
               // Disable interrupts, so that we are not disturbed by them (they may cause pop / click sounds)
               uint32_t int_vec = disable_interrupts();
            #endif

            uint32_t idx = 0;
            while (idx < length) {
               uint8_t times = 67;
               while (times > 0) {
                  times--;
                  #ifdef ISENSE
                     dac().write( (sounddata_data[idx]*12) );
                  #endif
               }
               idx++;
            }

            #ifdef ISENSE_JENNIC_JN5148
               // Restore the interrupts (we want to receive Radio / UART packets again!)
               restore_interrupts(int_vec);
               // Restart the Watchdog (we want to detect endless loops again!)
               vAHI_WatchdogRestart();
            #endif
         }

         void tone(double frequency, uint32_t duration) {
            debug().debug( "play_sine(freq= %d duration= %i)", frequency, (int)duration );

            #ifdef ISENSE_JENNIC_JN5148
               // Since we are performing a very-long loop, we need to disable the watchdog
               vAHI_WatchdogStop();
               // Disable interrupts, so that we are not disturbed by them (they may cause pop / click sounds)
               uint32_t int_vec = disable_interrupts();
            #endif

            uint16_t sine_length = sizeof(sine_lut)/sizeof(*sine_lut);

            const int sampleRate = 16000;
            const double delta_phi = (double) (( frequency / sampleRate) * (double)sine_length);
            double phase = 0.0;
            uint16_t output = 0;

            while (duration > 0) {
               duration--;
               uint8 times = 22;
               while(times > 0) {
                  times--;
                  phase = phase+delta_phi;
                  if (phase >= (double)sine_length)
                     phase = phase-(double)sine_length;

                  uint16_t floorIndex = Math::trunc(phase);
                  output = sine_lut[floorIndex];

                  #ifdef ISENSE
                     dac().write( output );
                  #endif
               }
            }

            #ifdef ISENSE_JENNIC_JN5148
               // Restore the interrupts (we want to receive Radio / UART packets again!)
               restore_interrupts(int_vec);
               // Restart the Watchdog (we want to detect endless loops again!)
               vAHI_WatchdogRestart();
            #endif
         }

      private:
         Debug& debug() { 
            return *debug_; 
         }

         DAC& dac() { 
            return *dac_; 
         }

         typename Debug::self_pointer_t debug_;
         typename DAC::self_pointer_t dac_;
   };
}
#endif
