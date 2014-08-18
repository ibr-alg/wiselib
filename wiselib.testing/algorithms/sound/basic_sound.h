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

         struct midi_track_chunk {
            char id[4];
            uint32_t length;
         };

         struct MidiChannel {
            uint8_t notes[120];
         };

         int init(DAC& dac, Debug& debug) { 
            dac_ = &dac;
            debug_ = &debug;
            return OsModel::SUCCESS;
         }

         void play( uint8_t midi_data[], uint16_t file_lenght) {
            
            struct midi_header file_header;
            file_header.chunk_id[0] = midi_data[0];
            file_header.chunk_id[1] = midi_data[1];
            file_header.chunk_id[2] = midi_data[2];
            file_header.chunk_id[3] = midi_data[3];

            file_header.size = midi_data[7];
            file_header.format_type = midi_data[9];
            file_header.number_of_tracks = midi_data[11];
            file_header.time_division = ((uint16_t)midi_data[12] << 8)+midi_data[13];

            double ms_per_quarter_note = 198951/1000;
            double tick_in_ms = ms_per_quarter_note/file_header.time_division;

            struct midi_track_chunk track;

            track.id[0] = midi_data[14];
            track.id[1] = midi_data[15];
            track.id[2] = midi_data[16];
            track.id[3] = midi_data[17];

            track.length = (((uint32_t)((uint16_t)midi_data[18] << 8)+midi_data[19]) << 16) + (((uint16_t)midi_data[20] << 8)+midi_data[21]);

            uint32_t index = 22;
            struct MidiChannel channels[16]; 

            for(int channel_index = 0; channel_index < 16; channel_index++) {
               for(int note_index = 0; note_index < 120; note_index++) {
                  channels[channel_index].notes[note_index] = 0;
               }
            }

            #ifdef ISENSE_JENNIC_JN5148
               vAHI_WatchdogStop();
            #endif

            while(index < file_lenght) {
               uint32_t delta_time = 0;
               if(midi_data[index] > 127)  {                // state-byte
                  if((midi_data[index] >> 4) == 8) {        // note off
                     uint8_t channel = midi_data[index] & 0x0F;
                     uint8_t note = midi_data[index+1];

                     channels[channel].notes[note] = 0; 
                     index = index + 3;

                     uint8_t vlv_length = 0;
                     uint8_t vlv_value[255];
                     while((midi_data[index+vlv_length] & 128) == 128) {
                        vlv_value[vlv_length] = midi_data[index+vlv_length];
                        vlv_length++;
                     }
                     vlv_value[vlv_length] = midi_data[index+vlv_length];
                     vlv_length++;

                     delta_time = vlvToInt(vlv_value, vlv_length);
                     index += vlv_length;
                  }
                  else if((midi_data[index] >> 4) == 9) {   // note on
                     uint8_t channel = midi_data[index] & 0x0F;
                     uint8_t note = midi_data[index+1];

                     channels[channel].notes[note] = 1; 
                     index += 3;          

                     uint8_t vlv_length = 0;
                     uint8_t vlv_value[255];
                     while((midi_data[index+vlv_length] & 128) == 128) {
                        vlv_value[vlv_length] = midi_data[index+vlv_length];
                        vlv_length++;
                     }
                     vlv_value[vlv_length] = midi_data[index+vlv_length];
                     vlv_length++;

                     delta_time = vlvToInt(vlv_value, vlv_length);
                     index += vlv_length;

                  }
                  else if((midi_data[index] >> 4) == 0xA) {
                     index = index + 3;
                  }
                  else if((midi_data[index] >> 4) == 0xB) {
                     index = index + 3;
                  }
                  else if((midi_data[index] >> 4) == 0xC) {
                     index = index + 2;
                  }
                  else if((midi_data[index] >> 4) == 0xD) {
                     index = index + 2;
                  }
                  else if((midi_data[index] >> 4) == 0xE) {
                     index = index + 3;
                  }
                  else if((midi_data[index] >> 4) == 0xF) {
                     if (midi_data[index] == 255){
                        if(midi_data[index+1] == 81)  { // 0xFF 0x51 tt tt tt - change tempo
                           uint32_t new_tempo = (midi_data[index+2] << 16) + (midi_data[index+3] << 8) + midi_data[index+4];
    
                           ms_per_quarter_note = new_tempo/1000;
                           tick_in_ms = ms_per_quarter_note/file_header.time_division;
                           //tick_in_ms *= 0.9;
                           index += 5;
                        }
                     }
                     ++index;
                  }
               }
               else {                                     // data-byte
                  ++index;
               }

               double frequencies[120];
               uint8_t frequencies_index = 0;

               for(int channel_index = 0; channel_index < 16; channel_index++) {
                  for(int note_index = 0; note_index < 120; note_index++) {
                     if(channels[channel_index].notes[note_index] > 0) {
                        frequencies[frequencies_index] = getFrequency(note_index);
                        frequencies_index++;
                     }
                  }
               }

               uint32_t evt_length = (uint32_t)(delta_time*tick_in_ms);

               if(frequencies_index > 0) {
                  debug().debug("play %i tones with lenght of %i ms", frequencies_index, evt_length);
                  sound(frequencies, frequencies_index, evt_length);
               }
               else {
                  debug().debug("wait %i ms", evt_length);
                  wait(evt_length);
               }
            }

            #ifdef ISENSE
                  dac().write(0);
            #endif
            #ifdef ISENSE_JENNIC_JN5148
               vAHI_WatchdogRestart();
            #endif
         }

         uint32_t vlvToInt(uint8_t vlv_bytes[], uint8_t vlv_length) {
            uint32_t delta_time = 0;
            for(int i = 0; i < vlv_length; i++) {
               delta_time |= (vlv_bytes[vlv_length-i-1]&127) << (7*i);
            }   
            return delta_time;
         }

         double getFrequency(uint8_t midi_note) {
            uint8_t note_type = midi_note%12;
            uint8_t ocatve = (uint8_t)(midi_note/12);
            double frequency = 0;

            switch(note_type) {
               case 0: 
                  frequency = 16.35*pow(2,ocatve); //C
                  break;
               case 1: 
                  frequency = 17.32*pow(2,ocatve); //Cis
                  break;
               case 2: 
                  frequency = 18.35*pow(2,ocatve); //D
                  break;
               case 3: 
                  frequency = 19.45*pow(2,ocatve); //Dis
                  break;
               case 4: 
                  frequency = 20.60*pow(2,ocatve); //E
                  break;
               case 5: 
                  frequency = 21.83*pow(2,ocatve); //F
                  break;
               case 6: 
                  frequency = 23.12*pow(2,ocatve); //Fis
                  break;
               case 7: 
                  frequency = 24.50*pow(2,ocatve); //G
                  break;
               case 8: 
                  frequency = 25.96*pow(2,ocatve); //Gis
                  break;
               case 9: 
                  frequency = 27.50*pow(2,ocatve); //A
                  break;
               case 10: 
                  frequency = 29.14*pow(2,ocatve); //Ais
                  break;
               case 11: 
                  frequency = 30.87*pow(2,ocatve); //B
                  break;
               default:
                  break;
            } 
            return frequency;
         }

         double pow(double x, int exp) {
             if(exp==0)
                 return 1;

             double tmp = x;
             for(int e = 1; e < exp; e++){
                 x*=tmp;
             }
             return x;
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
            #ifdef ISENSE_JENNIC_JN5148
               vAHI_WatchdogStop();
               uint32_t int_vec = disable_interrupts();
            #endif

            vAHI_TickTimerConfigure( E_AHI_TICK_TIMER_DISABLE );
            vAHI_TickTimerWrite(0);
            vAHI_TickTimerInterval(1);
            vAHI_TickTimerConfigure( E_AHI_TICK_TIMER_CONT );

            uint16_t sine_length = sizeof(sine_lut)/sizeof(*sine_lut);
            int sampleRate = 8000;
            
            double delta_phi = (double) (( frequency / sampleRate) * (double)sine_length);
            double phase = 0.0;

            // duration *= 8 because we need to call dac.write 8 times a millisecound and we use --duration after each loop
            duration << 3;

            while (duration > 0) {
               --duration;
               phase += delta_phi;
               
               if(phase >= sine_length)
                  phase = phase-sine_length;

               vAHI_TickTimerWrite(0);
               while(u32AHI_TickTimerRead() < 2000) {
                  #ifdef ISENSE
                     dac().write( sine_lut[(int)phase] );
                  #endif
               }
            }

            #ifdef ISENSE_JENNIC_JN5148
               vAHI_WatchdogRestart();
               restore_interrupts(int_vec);
            #endif
         }

         void sound_test(double frequencies[], uint8_t frequency_count, uint32_t duration) {
            #ifdef ISENSE_JENNIC_JN5148
               vAHI_WatchdogStop();
               uint32_t int_vec = disable_interrupts();
            #endif

            // init tick timer
            vAHI_TickTimerConfigure( E_AHI_TICK_TIMER_DISABLE );
            vAHI_TickTimerWrite(0);
            vAHI_TickTimerInterval(1);
            vAHI_TickTimerConfigure( E_AHI_TICK_TIMER_CONT );

            // determine length of sine table
            uint16_t sine_length = sizeof(sine_lut)/sizeof(*sine_lut);
            
            int sampleRate = 8000;
            
            float delta_phi[255];
            float phase[255];
            
            // get sine delta for each note
            for(int i = 0; i < frequency_count; i++) {
               delta_phi[i] = (float) (( frequencies[i] / sampleRate) * (float)sine_length);
               phase[i] = 0.0;
            }

            // duration *= 8 because we need to call dac.write 8 times a millisecound and we use --duration after each loop
            duration = duration << 3; 
            uint32_t output = 0;

            while (duration > 0) {
               --duration;
               output = 0;
               for(int i = 0; i < frequency_count; i++) {
                  phase[i] += delta_phi[i];
               
                  if(phase[i] >= sine_length)
                     phase[i] -= sine_length;

                  output += sine_lut[(int)phase[i]];
               }              

               output = output/frequency_count;

               vAHI_TickTimerWrite(0);
               // 1000 mu_s accord 16000 ticks - with a sample rate of 8 MHz we need to call adc.write every 125 mu_s which accord 2000 ticks  
               while(u32AHI_TickTimerRead() < 2000) {
                  #ifdef ISENSE
                     dac().write( output );
                  #endif
               }
            }

            #ifdef ISENSE_JENNIC_JN5148
               vAHI_WatchdogRestart();
               restore_interrupts(int_vec);
            #endif          
         }

         void sound( double frequencies[], uint8_t frequency_count, uint32_t duration ) {
            #ifdef ISENSE_JENNIC_JN5148
               vAHI_WatchdogStop();
               uint32_t int_vec = disable_interrupts();
            #endif

            // init tick timer
            vAHI_TickTimerConfigure( E_AHI_TICK_TIMER_DISABLE );
            vAHI_TickTimerWrite(0);
            vAHI_TickTimerInterval(1);
            vAHI_TickTimerConfigure( E_AHI_TICK_TIMER_CONT );

            // determine length of sine table
            uint16_t sine_length = sizeof(sine_lut)/sizeof(*sine_lut);
            
            int sampleRate = 8000;
            
            float delta_phi[255];
            float phase[255];
            
            // get sine delta for each note
            for(int i = 0; i < frequency_count; i++) {
               delta_phi[i] = (float) (( frequencies[i] / sampleRate) * (float)sine_length);
               phase[i] = 0.0;
            }

            // duration *= 8 because we need to call dac.write 8 times a millisecound and we use --duration after each loop
            duration = duration << 3; 
            uint32_t output = 0;
            //bool firstLoop = true;

            while (duration > 0) {
               --duration;
               output = 0;

               vAHI_TickTimerWrite(0);

               for(int i = 0; i < frequency_count; i++) {
                  phase[i] += delta_phi[i];
               
                  if(phase[i] >= sine_length)
                     phase[i] -= sine_length;

                  output += sine_lut[(int)phase[i]];
               }  

               if(frequency_count < 8) {
                  while(u32AHI_TickTimerRead() < 2800){ // 2800 for 6 tones - 3732 for 8 tones
                     // wait: beacuse the ticks in the for loop for 1-5 tones are significantly less
                  }
               }

               output = output/frequency_count; // 220 ticks

               vAHI_TickTimerWrite(0);
               // 1000 mu_s accord 16000 ticks - with a sample rate of 8 MHz we need to call adc.write every 125 mu_s which accord 2000 ticks  
               while(u32AHI_TickTimerRead() < 2000) {
                  #ifdef ISENSE
                     dac().write( output );
                  #endif
               }
            }

            #ifdef ISENSE_JENNIC_JN5148
               vAHI_WatchdogRestart();
               restore_interrupts(int_vec);
            #endif          
         }

         void wait( uint32_t duration ) {
            #ifdef ISENSE_JENNIC_JN5148
               vAHI_WatchdogStop();
               uint32_t int_vec = disable_interrupts();
            #endif
    
            vAHI_TickTimerConfigure( E_AHI_TICK_TIMER_DISABLE );
            vAHI_TickTimerWrite(0);
            vAHI_TickTimerInterval(1);
            vAHI_TickTimerConfigure( E_AHI_TICK_TIMER_CONT );    

            while (duration > 0) {
               --duration;

               vAHI_TickTimerWrite(0);
               while(u32AHI_TickTimerRead() < 10000){
                  // wait: beacuse all sounds are wait min. 10000 ticks (needed by sound with 6 tones) 
                  //       and wait have to be equal to the other sounds 
               }

               vAHI_TickTimerWrite(0);
               while(u32AHI_TickTimerRead() < 16000) { // 1ms = 1000 mu_s => 16000 Hz is 1 Tick in 0,0625 mu_s means 16000 ticks per millisecound
                  // wait a millisecound
               }
            }

            #ifdef ISENSE_JENNIC_JN5148
               vAHI_WatchdogRestart();
               restore_interrupts(int_vec);
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
