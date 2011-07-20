// vim: set ts=3 sw=3 expandtab:
#include <iostream>

#include "algorithms/routing/dsdv/dsdv_routing.h"
#include "algorithms/routing/dsr/dsr_routing.h"
#include "algorithms/routing/flooding/flooding_algorithm.h"
#include "algorithms/routing/tree/tree_routing.h"
#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
#include "external_interface/shawn/shawn_dummy_com_uart.h"
#include "external_interface/shawn/shawn_rand.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "util/pstl/map_static_vector.h"
#include "util/wisebed_node_api/remote_uart.h"

// --- Platform configuration

typedef wiselib::iSenseOsModel Os;

typedef wiselib::iSenseSerialComUartModel<Os> uart_t;
typedef wiselib::RemoteUart<OsModel, Os::Radio, flooding_t, Uart, Os::Debug, Os::Timer> remote_uart_t;
typedef wiselib::MapStaticVector<Os, Os::Radio::node_id_t, uint16_t, 64> node_map_t;
typedef wiselib::FloodingAlgorithm<Os, node_map_t> flooding_t;
typedef wiselib::MapStaticVector<Os::Radio::node_id_t,wiselib::DsrRoutingTableValue<Os::Radio,20> > routing_table_t;
typedef wiselib::DsrRouting<Os,routing_table_t> routing_t;

// --- Actual application

class SinkApplication
{
   public:
      void init(Os::AppMainParameter& amp) {
         radio_ = &(wiselib::FacetProvider<Os, Os::Radio>::get_facet(amp));
         uart_ = &(wiselib::FacetProvider<Os, Os::Uart>::get_facet(amp));
         debug_ = &(wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp));
         timer_ = &(wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp));
         random_ = &(wiselib::FacetProvider<Os, Os::Rand>::get_facet(amp));

         remote_uart_.init(routing_, flooding_, uart_, debug_, timer_, random_);
         remote_uart_.enable_serial_comm();
      }
               
   private:
      Os::Debug* debug_;
      Os::Radio* radio_;
      Os::Uart* uart_;
      Os::Timer* timer_;
      Os::Rand* rand_;
      remote_uart_t remote_uart_;
      flooding_t flooding_;
      routing_t routing_;
};


// --- Instantiation

wiselib::WiselibApplication<Os, SinkApplication> sink_app;

void application_main(Os::AppMainParameter& amp) {
   sink_app.init(amp);
}

