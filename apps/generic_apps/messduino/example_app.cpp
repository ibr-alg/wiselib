/*
 * Simple Wiselib Example
 */

#define WISELIB_ARDUINO_DEBUG_NO_DELAY 1

#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"

typedef wiselib::OSMODEL Os;

#define CALIB_CURRENT (48.0 / 183.0)
#define CALIB_VOLTAGE (4.973 / 566.0)

class ExampleApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {
         //radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
         //clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet( value );
         //timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         //debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

         Serial.println("messduino start v1.0");
         
  // CPU Sleep Modes 
  // SM2 SM1 SM0 Sleep Mode
  // 0    0  0 Idle
  // 0    0  1 ADC Noise Reduction
  // 0    1  0 Power-down
  // 0    1  1 Power-save
  // 1    0  0 Reserved
  // 1    0  1 Reserved
  // 1    1  0 Standby(1)

  cbi( SMCR, SE );     // sleep enable, power down mode
  cbi( SMCR,SM0 );     // ADC noise reduction
  cbi( SMCR,SM1 );     // ADC noise reduction
  sbi( SMCR,SM2 );     // ADC noise reduction


         
         analogReference(INTERNAL2V56);
         
         while(true) {
             //debug_->debug("%lu ms %f mA %f V", (unsigned long)millis(),
                     //(float)analogRead(A0) * CALIB_CURRENT,
                     //(float)analogRead(A1) * CALIB_VOLTAGE);
             Serial.print(millis());
             Serial.print(" ");
             //Serial.print((double)analogRead(A0) * CALIB_CURRENT);
             Serial.print(analogRead(A0));
             Serial.print(" ");
             Serial.println(analogRead(A1));
             //Serial.println((double)analogRead(A1) * CALIB_VOLTAGE);
             
             /*
             
             pinMode(12, OUTPUT);
             digitalWrite(12, HIGH);
             
             pinMode(9, OUTPUT);
             digitalWrite(9, LOW);
             
             delay(3000);
             
             pinMode(12, OUTPUT);
             digitalWrite(12, LOW);
             
             pinMode(9, INPUT);
             digitalWrite(9, HIGH);
             */
             
         }
      }
   private:
      //Os::Clock::self_pointer_t clock_;
      //Os::Debug::self_pointer_t debug_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}
