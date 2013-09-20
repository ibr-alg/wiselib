#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"

typedef wiselib::OSMODEL Os;

typedef Os::Debug Debug;
#ifdef SHAWN
#include "external_interface/shawn/shawn_stringtag_debug.h"
#include "external_interface/shawn/shawn_tagid_radio.h"
#include "external_interface/shawn/shawn_tag_neighborhood.h"

typedef wiselib::ShawnStringTagDebugModel<Os> LabelDebug;
typedef wiselib::ShawnTagIdRadio<Os> Radio;
typedef wiselib::ShawnTagNeighborhood<Os, Radio> ShawnNeighborhood;
#else
typedef Os::Debug LabelDebug;
typedef Os::Radio Radio;
#endif

#include "util/pstl/vector_static.h"
#include "algorithms/convergecast/Neighbor.h"
#include "algorithms/convergecast/TreeNeighborhood.h"
#include "algorithms/convergecast/Convergecast.h"
#include "algorithms/convergecast/ConvergecastMessage.h"
#include "algorithms/convergecast/ConvergecastFunctions.h"

typedef Neighbor<Radio> Neighbor_t;
typedef wiselib::vector_static<Os, Neighbor_t*, 16> Container_t;
typedef TreeNeighborhood<Container_t, Neighbor_t*> Neighborhood_t;
typedef ConvergecastMessage<Os, Radio> Message_t;
typedef wiselib::vector_static<Os, Message_t, 16> MessageContainer_t;

class ConvergecastExampleApp
{
public:
   typedef ConvergecastExampleApp self_type;
   typedef self_type* self_pointer_t;

   void init( Os::AppMainParameter& amp )
   {
      radio_ = &wiselib::FacetProvider<Os, Radio>::get_facet( amp );
      debug_ = &wiselib::FacetProvider<Os, Debug>::get_facet( amp );
      labeldebug_ = &wiselib::FacetProvider<Os, LabelDebug>::get_facet( amp );

      radio_->enable_radio();
      cf_.init( debug_ );

#ifdef SHAWN

      labeldebug_->debug( "%d", radio_->id() );

      ShawnNeighborhood snh;
      snh.init( radio_ );

      Neighbor_t* newNeighbor;
      if ( !snh.is_root() )
      {
         newNeighbor = new Neighbor_t( snh.parent(), Neighbor_t::IN_EDGE );
         n_.add( newNeighbor );
      }
      for ( ShawnNeighborhood::Neighbors::iterator iter = snh.topology().begin(); iter
            != snh.topology().end(); ++iter )
      {
         Radio::node_id_t neighborID = *iter;
         newNeighbor = new Neighbor_t( neighborID, Neighbor_t::OUT_EDGE );
         n_.add( newNeighbor );
      }

      if ( n_.neighbors_begin( Neighbor_t::IN_EDGE ) == n_.neighbors_end() )
      {
         debug_->debug( "node %d is root\n", radio_->id() );
      }
      for ( Neighborhood_t::iterator iter = n_.neighbors_begin( Neighbor_t::IN_EDGE ); iter
            != n_.neighbors_end(); ++iter )
      {
         Neighbor_t* myNeighbor = *iter;
         debug_->debug( "node %d has %d as parent\n", radio_->id(), myNeighbor->id() );
      }
      if ( n_.neighbors_begin( Neighbor_t::OUT_EDGE ) == n_.neighbors_end() )
      {
         debug_->debug( "node %d is leaf\n", radio_->id() );
      }
      for ( Neighborhood_t::iterator iter = n_.neighbors_begin( Neighbor_t::OUT_EDGE ); iter
            != n_.neighbors_end(); ++iter )
      {
         Neighbor_t* myNeighbor = *iter;
         debug_->debug( "node %d has %d as child\n", radio_->id(), myNeighbor->id() );
      }

      cc1_.init( debug_, radio_, &n_, 1 );
      cc1_.reg_function_callback<self_type, &self_type::on_function> ( this );
      cc2_.init( debug_, radio_, &n_, 2 );
      cc2_.reg_function_callback<self_type, &self_type::on_function> ( this );
      cc3_.init( debug_, radio_, &n_, 3 );
      cc3_.reg_function_callback<self_type, &self_type::on_function> ( this );

      if ( n_.neighbors_begin( Neighbor_t::IN_EDGE ) == n_.neighbors_end() )
      {
         cc1_.reg_finished_callback<self_type, &self_type::on_finished> ( this );
         cc1_.startConvergecast();
         cc2_.reg_finished_callback<self_type, &self_type::on_finished> ( this );
         cc2_.startConvergecast();
         cc3_.reg_finished_callback<self_type, &self_type::on_finished> ( this );
         cc3_.startConvergecast();
      }

#else

      // tree: 101 - 103 - 104
      //        |
      //       102
      if ( radio_->id() == 101 )
      {
         // child 102, child 103
         neighbor1_.set_id( 102 );
         neighbor1_.set_state( Neighbor_t::OUT_EDGE );
         neighbor2_.set_id( 103 );
         neighbor2_.set_state( Neighbor_t::OUT_EDGE );
         n_.add( &neighbor1_ );
         n_.add( &neighbor2_ );
      }
      else if ( radio_->id() == 102 )
      {
         // parent 101
         neighbor1_.set_id( 101 );
         neighbor1_.set_state( Neighbor_t::IN_EDGE );
         n_.add( &neighbor1_ );
      }
      else if ( radio_->id() == 103 )
      {
         // parent 101, child 104
         neighbor1_.set_id( 101 );
         neighbor1_.set_state( Neighbor_t::IN_EDGE );
         neighbor2_.set_id( 104 );
         neighbor2_.set_state( Neighbor_t::OUT_EDGE );
         n_.add( &neighbor1_ );
         n_.add( &neighbor2_ );
      }
      else
      {
         // parent 103
         neighbor1_.set_id( 103 );
         neighbor1_.set_state( Neighbor_t::IN_EDGE );
         n_.add( &neighbor1_ );
      }

      cc1_.init( debug_, radio_, &n_, 2 );
      cc1_.reg_function_callback<self_type, &self_type::on_function>( this );

      if ( n_.neighbors_count( Neighbor_t::IN_EDGE ) == 0 )
      {
         debug_->debug( "************START************\n" );

         cc1_.reg_finished_callback<self_type, &self_type::on_finished>( this );
         cc1_.startConvergecast();
      }

#endif

   }

private:
   Message_t on_function( MessageContainer_t &container, Message_t &own )
   {
      Message_t result;

      if ( own.cc_id() == 1 )
      {
         result = cf_.count( container, own );
         debug_->debug( "cc %d: result from %d for function is %d\n", result.cc_id(), radio_->id(),
               result.intvalue1() );
         return result;
      } else if ( own.cc_id() == 2 )
      {
         own.set_intvalue1( radio_->id() );
         result = cf_.sum( container, own );
         debug_->debug( "cc %d: result from %d for function is %d\n", result.cc_id(), radio_->id(),
               result.intvalue1() );
         return result;
      } else
      {
         //own.set_intvalue2(( radio_->id() ) * 1000);
         own.set_floatvalue1( radio_->id() * 1.0 );
         result = cf_.avg( container, own );
         debug_->debug( "cc %d: result from %d for function is int %d float %3.1f \n",
               result.cc_id(), radio_->id(), result.intvalue1(), result.floatvalue1() );
         return result;
      }
   }

   void on_finished( Message_t &result )
   {
      if ( ( result.cc_id() == 1 ) || ( result.cc_id() == 2 ) )
      {
         debug_->debug( "cc %d: THE FINAL RESULT IS %d!\n", result.cc_id(), result.intvalue1() );
      } else
      {
         debug_->debug( "cc %d: THE FINAL RESULT IS %3.1f!\n", result.cc_id(), result.floatvalue1() );
      }
   }

   Radio::self_pointer_t radio_;
   Debug::self_pointer_t debug_;
   LabelDebug::self_pointer_t labeldebug_;

   Neighborhood_t n_;
   Convergecast<Os, Debug, Radio, Neighbor_t, Neighborhood_t, MessageContainer_t, Message_t> cc1_;
   Convergecast<Os, Debug, Radio, Neighbor_t, Neighborhood_t, MessageContainer_t, Message_t> cc2_;
   Convergecast<Os, Debug, Radio, Neighbor_t, Neighborhood_t, MessageContainer_t, Message_t> cc3_;
   ConvergecastFunctions<Debug, MessageContainer_t, Message_t> cf_;

   Neighbor_t neighbor1_;
   Neighbor_t neighbor2_;
};

wiselib::WiselibApplication<Os, ConvergecastExampleApp> app;
void application_main( Os::AppMainParameter& amp )
{
   app.init( amp );
}

