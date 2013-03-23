/*
 * Simple Wiselib Example
 */

#include <external_interface/external_interface.h>
typedef wiselib::OSMODEL Os;
typedef Os::block_data_t block_data_t;
using namespace wiselib;

#include <util/allocators/malloc_free_allocator.h>
typedef MallocFreeAllocator<Os> Allocator;
Allocator& get_allocator();

#include <algorithms/routing/flooding_nd/flooding_nd.h>
typedef FloodingNd<Os, Os::Radio> FND;
typedef Os::Radio Radio;

class ExampleApplication
{
	public:
		void init( Os::AppMainParameter& value )
		{
			radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet( value );
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );
			debug_->debug( "Hello World from Example Application!\n" );
			//timer_->set_timer<ExampleApplication, //&ExampleApplication::start>( 5000, this, 0 );
			
			fnd_.init(radio_);
			fnd_.reg_recv_callback<ExampleApplication, &ExampleApplication::on_receive>( this );
			
			if(radio_->id() == 0) {
				block_data_t msg[] = { 't', 'a', 'c', 'h', '\0' };
				fnd_.send(0, 5, msg);
			}
		}
		
		void on_receive(Radio::node_id_t from, Radio::size_t size, Radio::block_data_t* data) {
			debug_->debug("%d->%d: %d  %s", from, radio_->id(), (int)data[0], (char*)data);
		}
		
	private:
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;
		Os::Debug::self_pointer_t debug_;
		FND fnd_;
};

Allocator allocator_;
Allocator& get_allocator() { return allocator_; }
// --------------------------------------------------------------------------
wiselib::WiselibApplication<Os, ExampleApplication> example_app;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
  example_app.init( value );
}
