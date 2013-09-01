/*
 * Simple Wiselib Example
 */

#define WISELIB_ARDUINO_DEBUG_NO_DELAY 1

#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"

typedef wiselib::OSMODEL Os;

#define CALIB_CURRENT (48.0 / 183.0)
#define CALIB_VOLTAGE (4.973 / 566.0)

enum { SAMPLE_START_PIN = 1, SAMPLE_PINS = 3 };

class ExampleApplication;

volatile ::uint16_t samples[SAMPLE_PINS];
volatile ::uint8_t sample_idx = 0;
volatile bool adc_done = false;

ISR(ADC_vect) {
   // trigged when a reading is available
   
   sample_idx = (ADMUX & 0x07) - SAMPLE_START_PIN;
   //low = ADCL;
   //high = ADCH;
   samples[sample_idx] = ADCW; //(high << 8) | low;
   sample_idx++;
   
   if(sample_idx >= SAMPLE_PINS) {
      ADMUX = (ADMUX & 0xf8) | (SAMPLE_START_PIN & 0x07);
      sample_idx = 0;
      adc_done = true;
      ADCSRA &= ~_BV( ADIE );  // turn off ADC interrupt
         //_BV(REFS0) | (SAMPLE_START_PIN & 0x07);
   }
   else {
      ADMUX = (ADMUX & 0xf8) | ((sample_idx + SAMPLE_START_PIN) & 0x07);
      ADCSRA |= _BV(ADIE);
      sbi(ADCSRA, ADSC);
      //set_sleep_mode(SLEEP_MODE_ADC);
      //sleep_mode();
   }
}

class ExampleApplication
{
   public:
      
      void init(Os::AppMainParameter& amp) {
         Serial.begin(115200);
         delay(500);
         Serial.println("messduino v1.0 ");
         delay(500);
         Serial.println(SAMPLE_START_PIN);
         delay(500);
         // set the analog reference (high two bits of ADMUX) and select the
         // channel (low 4 bits).  this also sets ADLAR (left-adjust result)
         // to 0 (the default).
         ADMUX = _BV(REFS0) | (SAMPLE_START_PIN & 0x07);
         
         bool started = false;
         
         while(true) {
            //Serial.println(sample_idx);
            //delay(100);
            if(adc_done) {
               started = false;
               //Serial.print(sample_idx);
               if(sample_idx == 0) {
                  Serial.print(millis());
                  Serial.print(" ");
                  Serial.print(samples[0]);
                  Serial.print(" ");
                  Serial.print(samples[1]);
                  Serial.print(" ");
                  Serial.print(samples[2]);
                  Serial.println();
                  delay(10);
               }
               adc_done = false;
            }
            
            if(!started) {
               started = true;
               //sei();
               // start conversion
               ADCSRA |= _BV(ADSC) | _BV(ADIE);
               set_sleep_mode(SLEEP_MODE_ADC);
               sbi(ADCSRA, ADSC);
               //sei();
               sleep_mode();
               //cli();
               //Serial.println("conv started");
            }
            if(started) {
               set_sleep_mode(SLEEP_MODE_ADC);
               sleep_mode();
            }
         }
      }
};

   
  //uint8_t sampleIndex;
//digitalWrite( 13, HIGH );
  // save this sample, start next; if we've just collected a set of three, pass them to the orientation object and stop ADC (Sample( -1 ))
  //samples[ sampleIndex = ( ADMUX & 0x07 ) - startPin ] = ADCW;  // get sampled pin number from the MUX
  //Sample( ++sampleIndex < 3 ? sampleIndex : ( orientation.AcceptSample( samples ), -1 ) );
//digitalWrite( 13, LOW );
//}


// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}
/* vim: set ts=3 sw=3 tw=78 expandtab :*/
