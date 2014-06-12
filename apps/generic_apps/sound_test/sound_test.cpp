/*
* Author: Fabian Bormann - Google Summer of Code 2014 - Software Audio Interface
*/

#include "external_interface/external_interface.h"
#include "algorithms/sound/basic_sound.h"

typedef wiselib::OSMODEL Os;
typedef wiselib::BasicSound<Os, Os::DAC, Os::Debug> basic_sound_t;

class AudioApplication {
	public:
		void init( Os::AppMainParameter& value ) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			dac_ = &wiselib::FacetProvider<Os, Os::DAC>::get_facet(value);

			debug_->debug( "Audio Application booting");
			
			dac_->init();
			basic_sound_.init(*dac_, *debug_);

			playSong();
		}

		/**
		* Alle meine Entchen - German Children's Song
		**/
		void playSong() {
			basic_sound_.tone(C4, 250);
			basic_sound_.tone(D4, 250);
			basic_sound_.tone(E4, 250);
			basic_sound_.tone(F4, 250);

			basic_sound_.tone(G4, 500);
			basic_sound_.tone(G4, 500);

			basic_sound_.tone(A4, 250);
			basic_sound_.tone(A4, 250);
			basic_sound_.tone(A4, 250);
			basic_sound_.tone(A4, 250);

			basic_sound_.tone(G4, 1000);

			basic_sound_.tone(A4, 250);
			basic_sound_.tone(A4, 250);
			basic_sound_.tone(A4, 250);
			basic_sound_.tone(A4, 250);

			basic_sound_.tone(G4, 1000);

			basic_sound_.tone(F4, 250);
			basic_sound_.tone(F4, 250);
			basic_sound_.tone(F4, 250);
			basic_sound_.tone(F4, 250);

			basic_sound_.tone(E4, 500);
			basic_sound_.tone(E4, 500);

			basic_sound_.tone(D4, 250);
			basic_sound_.tone(D4, 250);
			basic_sound_.tone(D4, 250);
			basic_sound_.tone(D4, 250);

			basic_sound_.tone(C4, 1000);
		}

	private:
		basic_sound_t basic_sound_;
	    Os::Debug::self_pointer_t debug_;
		Os::DAC::self_pointer_t dac_;
};
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, AudioApplication> sound_test_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  sound_test_app.init( value );
}
