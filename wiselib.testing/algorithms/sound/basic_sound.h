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

         struct midi_header {
            char chunk_id[4];
            uint8_t size;
            uint8_t format_type;
            uint16_t number_of_tracks;
            uint16_t time_division;        
         };

         struct midi_message {
            uint8_t type;
            uint16_t length;
            uint8_t data_bytes[]; 
         };

         struct midi_event {
            uint16_t delta_time;
            struct midi_message message[];
         };

         struct midi_track_chunk {
            char id[4];
            uint32_t length;
            struct midi_event events[];
         };

         int init(DAC& dac, Debug& debug) { 
            dac_ = &dac;
            debug_ = &debug;
            return OsModel::SUCCESS;
         }

         void play( const uint16_t midi_data[] ) {
            struct midi_header file_header;
            file_header.chunk_id[0] = midi_data[0] >> 8;
            file_header.chunk_id[1] = midi_data[0];
            file_header.chunk_id[2] = midi_data[1] >> 8;
            file_header.chunk_id[3] = midi_data[1];
            file_header.size = midi_data[3];
            file_header.format_type = midi_data[4];
            file_header.time_division = midi_data[5];

            debug().debug("Playing midi file in fomrat %i and time division %i", file_header.format_type, file_header.time_division);

            struct midi_track_chunk track;

            track.id[0] = midi_data[6] >> 8;
            track.id[1] = midi_data[6];
            track.id[2] = midi_data[7] >> 8;
            track.id[3] = midi_data[7];

            track.length = ((uint32_t) midi_data[8] << 16) + midi_data[9];

            uint32_t idx = 10;
            uint32_t file_lenght = sizeof(midi_data)/sizeof(*midi_data);

            while(idx < file_lenght) {
               if(midi_data[idx] > 127)  {                // state-byte
                  if((midi_data[idx] >> 4) == 8) {        // note off
                     uint8_t channel = midi_data[idx];
                     idx++;
                     uint16_t note = midi_data[idx];
                     idx++;
                     uint16_t strength = midi_data[idx];
                     //STOP
                     idx++;
                  }
                  else if((midi_data[idx] >> 4) == 9) {   // note on
                     uint8_t channel = midi_data[idx];
                     idx++;
                     uint16_t note = midi_data[idx];
                     idx++;
                     uint16_t strength = midi_data[idx];
                     //PLAY
                     idx++;                                          
                  }
                  idx++;
               }
               else {                                     // data-byte
                  idx++;
               }
            }
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
