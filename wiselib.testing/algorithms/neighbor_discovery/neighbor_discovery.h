#ifndef NEIGHBOR_DISCOVERY_H
#define	NEIGHBOR_DISCOVERY_H

#include "util/pstl/vector_static.h"
#include "util/delegates/delegate.hpp"
#include "neighbor_discovery_config.h"
#include "neighbor_discovery_message.h"
#include "neighbor.h"
#include "protocol_settings.h"
#include "protocol_payload.h"
#include "protocol.h"
#include "beacon.h"

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename Clock_P,
				typename Timer_P,
				typename Debug_P>
	class NeighborDiscovery_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Timer_P Timer;
		typedef Debug_P Debug;
		typedef Clock_P Clock;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Clock::time_t time_t;
		typedef typename Radio::ExtendedData ExData;
		typedef typename Radio::TxPower TxPower;
		typedef typename Timer::millis_t millis_t;
		typedef NeighborDiscovery_Type<Os, Radio, Clock, Timer, Debug> self_t;
		typedef NeighborDiscoveryMessage_Type<Os, Radio> Message;
		typedef Neighbor_Type<Os, Radio, Clock, Timer, Debug> Neighbor;
		typedef ProtocolPayload_Type< Os, Radio, Debug> ProtocolPayload;
		typedef ProtocolSettings_Type<Os, Radio, Timer, Debug> ProtocolSettings;
		typedef Protocol_Type<Os, Radio, Clock, Timer, Debug> Protocol;
		typedef Beacon_Type<Os, Radio, Clock, Timer, Debug> Beacon;
		typedef vector_static<Os, Neighbor, NB_MAX_NEIGHBORS> Neighbor_vector;
		typedef typename Neighbor_vector::iterator Neighbor_vector_iterator;
		typedef vector_static<Os, ProtocolPayload, NB_MAX_REGISTERED_PROTOCOLS> ProtocolPayload_vector;
		typedef typename ProtocolPayload_vector::iterator ProtocolPayload_vector_iterator;
		typedef vector_static<Os, Protocol, NB_MAX_REGISTERED_PROTOCOLS> Protocol_vector;
		typedef typename Protocol_vector::iterator Protocol_vector_iterator;
		typedef delegate4<void, uint8_t, node_id_t, size_t, uint8_t*> event_notifier_delegate_t;
		// --------------------------------------------------------------------
		NeighborDiscovery_Type()	:
			status								( WAITING_STATUS ),
			beacon_period						( NB_BEACON_PERIOD ),
			channel								( NB_CHANNEL ),
			transmission_power_dB				( NB_TRANSMISSION_POWER_DB ),
			protocol_max_payload_size			( NB_MAX_PROTOCOL_PAYLOAD_SIZE ),
			transmission_power_dB_strategy		( FIXED_PERIOD ),
			protocol_max_payload_size_strategy	( FIXED_PAYLOAD_SIZE ),
			beacon_period_strategy				( FIXED_TRANSM ),
			relax_millis						( NB_RELAX_MILLIS ),
			nb_daemon_period					( 1000 )
		{};
		// --------------------------------------------------------------------
		~NeighborDiscovery_Type()
		{};
		// --------------------------------------------------------------------
		void enable()
		{
			Protocol p;
			Neighbor n;
			n.set_id( radio().id() );
			n.set_active();
			Neighbor_vector neighbors;
			neighbors.push_back( n );
			ProtocolPayload pp;
			pp.set_protocol_id( NB_PROTOCOL_ID );
			pp.set_payload_size( 0 );
			uint8_t ef = ProtocolSettings::NEW_NB|ProtocolSettings::UPDATE_NB|ProtocolSettings::NEW_PAYLOAD|ProtocolSettings::LOST_NB|ProtocolSettings::TRANS_DB_UPDATE|ProtocolSettings::BEACON_PERIOD_UPDATE|ProtocolSettings::NEIGHBOR_REMOVED;
			ProtocolSettings ps( 255, 0, 255, 0, 100, 0, 100, 0, ef, -6, 100, 3000, 100, ProtocolSettings::RATIO_DIVIDER, 2, ProtocolSettings::MEAN_DEAD_TIME_PERIOD, 100, 100, ProtocolSettings::R_NR_NORMAL, 1, 1, pp );
			p.set_protocol_id( NB_PROTOCOL_ID );
			p.set_neighborhood( neighbors );
			p.set_protocol_settings( ps );
			protocols.push_back( p );
			set_status( ACTIVE_STATUS );
			radio().enable_radio();
			recv_callback_id_ = radio().template reg_recv_callback<self_t, &self_t::receive>( this );
			beacons();
			nb_daemon();
		};
		// --------------------------------------------------------------------
		void disable()
		{
			set_status( WAITING_STATUS );
			radio().template unreg_recv_callback( recv_callback_id_ );
		};
		// --------------------------------------------------------------------
		void send( node_id_t _dest, size_t _len, block_data_t* _data, message_id_t _msg_id )
		{
			Message message;
			message.set_msg_id( _msg_id );
			message.set_payload( _len, _data );
			TxPower power;
			power.set_dB( get_transmission_power_dB() );
			radio().set_channel( get_channel() );
			radio().set_power( power );
			radio().send( _dest, message.buffer_size(), (uint8_t*) &message );
		}
		// --------------------------------------------------------------------
		void beacons( void* _data = NULL )
		{
#ifdef NB_DEBUG_BEACONS
			debug().debug( "NeighborDiscovery-beacons %x - Entering.\n", radio().id() );
#endif
			if ( get_status() == ACTIVE_STATUS )
			{
				Protocol* p_ptr = get_protocol_ref( NB_PROTOCOL_ID );
				if ( p_ptr != NULL )
				{
#ifdef NB_DEBUG_BEACONS
					debug().debug( "NeighborDiscovery-beacons %x - Protocol exists.\n", radio().id() );
#endif
					millis_t bp = get_beacon_period();
					Neighbor* n = p_ptr->get_neighbor_ref( radio().id() );
					if ( n != NULL )
					{
#ifdef NB_DEBUG_BEACONS
						debug().debug( "NeighborDiscovery-beacons %x - Neighbor exists.\n", radio().id() );
#endif
						n->inc_beacon_period_update_counter();
						n->set_beacon_period( bp );
						n->inc_total_beacons();
						Beacon beacon;
						beacon.set_protocol_payloads( protocols );
						Neighbor_vector nv = p_ptr->get_neighborhood();
						beacon.set_neighborhood( nv, radio().id() );
						beacon.set_beacon_period( bp );
						beacon.set_beacon_period_update_counter( n->get_beacon_period_update_counter() );
						block_data_t buff[Radio::MAX_MESSAGE_LENGTH];
						beacon.serialize( buff );
						size_t beacon_size = beacon.serial_size();
						send( Radio::BROADCAST_ADDRESS, beacon_size, buff, NB_MESSAGE );
#ifdef NB_DEBUG_BEACONS
						debug().debug( "NeighborDiscovery-beacons %x - Sending beacon.\n", radio().id() );
						beacon.print( debug(), radio() );
#endif
						timer().template set_timer<self_t, &self_t::beacons> ( bp, this, 0 );
					}
					else
					{
#ifdef NB_DEBUG_BEACONS
						debug().debug( "NeighborDiscovery-beacons %x - Neighbor does not exist.\n", radio().id() );
#endif
					}
				}
				else
				{
#ifdef NB_DEBUG_BEACONS
					debug().debug( "NeighborDiscovery-beacons %x - Protocol does not exist.\n", radio().id() );
#endif
				}
			}
#ifdef NB_DEBUG_BEACONS
			debug().debug( "NeighborDiscovery-beacons %x - Exiting.\n", radio().id() );
#endif
		}
		// --------------------------------------------------------------------
		void receive( node_id_t _from, size_t _len, block_data_t * _msg, ExData const &_ex )
		{
			if ( _from != radio().id() )
			{
#ifdef NB_DEBUG_RECEIVE
			debug().debug( "NeighborDiscovery-receive %x - From %x Entering.\n", radio().id(), _from );
#endif
				message_id_t msg_id = *_msg;
				if ( msg_id == NB_MESSAGE )
				{
					Message *message = (Message*) _msg;
					Beacon beacon;
					beacon.de_serialize( message->payload() );
#ifdef NB_DEBUG_RECEIVE
					debug().debug( "NeighborDiscovery-receive %x - Received beacon message.\n", radio().id() );
#endif
					time_t current_time = clock().time();
					uint8_t beacon_lqi = _ex.link_metric();
					for ( Protocol_vector_iterator pit = protocols.begin(); pit != protocols.end(); ++pit )
					{
						uint8_t found_flag = 0;
						Neighbor new_neighbor;
						Neighbor_vector_iterator update_neighbor_it = pit->get_neighborhood_ref()->begin();
						for ( Neighbor_vector_iterator nit = pit->get_neighborhood_ref()->begin(); nit != pit->get_neighborhood_ref()->end(); ++nit )
						{
							if ( _from == nit->get_id() )
							{
#ifdef NB_DEBUG_RECEIVE
								debug().debug( "NeighborDiscovery-receive %x - Neighbor %x is known for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
#endif
								update_neighbor_it = nit;
								found_flag = 1;
								uint32_t dead_time = ( clock().seconds( current_time ) - clock().seconds( nit->get_last_beacon() ) ) * 1000 + ( clock().milliseconds( current_time ) - clock().milliseconds( nit->get_last_beacon() ) );
								if ( beacon.get_beacon_period() == nit->get_beacon_period() )
								{
									if ( dead_time < beacon.get_beacon_period() + NB_RELAX_MILLIS )
									{
#ifdef NB_DEBUG_RECEIVE
										debug().debug( "NeighborDiscovery-receive %x - Neighbor %x is on time same as advertised for protocol %i with dead_time : %d.\n", radio().id(), _from, pit->get_protocol_id(), dead_time );
#endif
										new_neighbor = *nit;
										new_neighbor.inc_total_beacons( 1 * pit->resolve_beacon_weight( _from ) );
										new_neighbor.inc_total_beacons_expected( 1 * pit->resolve_beacon_weight( _from ) );
										new_neighbor.update_link_stab_ratio();
										new_neighbor.update_avg_LQI( beacon_lqi, 1 );
										new_neighbor.set_beacon_period( beacon.get_beacon_period() );
										new_neighbor.set_beacon_period_update_counter( beacon.get_beacon_period_update_counter() );
										new_neighbor.set_last_beacon( current_time );
									}
									else
									{
#ifdef NB_DEBUG_RECEIVE
										debug().debug( "NeighborDiscovery-receive %x - Neighbor %x was late same as advertised for protocol %i with dead_time : %d.\n", radio().id(), _from, pit->get_protocol_id(), dead_time );
#endif
										new_neighbor = *nit;
										new_neighbor.inc_total_beacons( 1 * pit->resolve_beacon_weight( _from ) );
										new_neighbor.inc_total_beacons_expected( ( dead_time / nit->get_beacon_period() ) * ( pit->resolve_lost_beacon_weight( _from ) ) );
										new_neighbor.update_link_stab_ratio();
										new_neighbor.update_avg_LQI( beacon_lqi, 1 );
										new_neighbor.set_beacon_period( beacon.get_beacon_period() );
										new_neighbor.set_beacon_period_update_counter( beacon.get_beacon_period_update_counter() );
										new_neighbor.set_last_beacon( current_time );
									}
								}
								else
								{
									if ( dead_time < beacon.get_beacon_period() + NB_RELAX_MILLIS )
									{
#ifdef NB_DEBUG_RECEIVE
										debug().debug( "NeighborDiscovery-receive %x - Neighbor %x is on time same as advertised for protocol %i with dead_time : %d.\n", radio().id(), _from, pit->get_protocol_id(), dead_time );
#endif
										new_neighbor = *nit;
										new_neighbor.inc_total_beacons( 1 * pit->resolve_beacon_weight( _from ) );
										new_neighbor.inc_total_beacons_expected( 1 * pit->resolve_beacon_weight( _from ) );
										new_neighbor.update_link_stab_ratio();
										new_neighbor.update_avg_LQI( beacon_lqi, 1 );
										new_neighbor.set_beacon_period( beacon.get_beacon_period() );
										new_neighbor.set_beacon_period_update_counter( beacon.get_beacon_period_update_counter() );
										new_neighbor.set_last_beacon( current_time );
									}
									else
									{
#ifdef NB_DEBUG_RECEIVE
										debug().debug( "NeighborDiscovery-receive %x - Neighbor %x is late and not as advertised for protocol %id with dead_time : %d.\n", radio().id(), _from, pit->get_protocol_id(), dead_time );
#endif
										//TODO overflow here.
										uint32_t last_beacon_period_update = beacon.get_beacon_period_update_counter() * beacon.get_beacon_period();
										millis_t approximate_beacon_period = 0;
										if ( pit->get_protocol_settings_ref()->get_dead_time_strategy() == ProtocolSettings::NEW_DEAD_TIME_PERIOD )
										{
											approximate_beacon_period = beacon.get_beacon_period();
										}
										else if ( pit->get_protocol_settings_ref()->get_dead_time_strategy() == ProtocolSettings::OLD_DEAD_TIME_PERIOD )
										{
											approximate_beacon_period = nit->get_beacon_period();
										}
										else if ( pit->get_protocol_settings_ref()->get_dead_time_strategy() == ProtocolSettings::MEAN_DEAD_TIME_PERIOD )
										{
											approximate_beacon_period = ( beacon.get_beacon_period() + nit->get_beacon_period() ) / 2;
										}
										else if ( pit->get_protocol_settings_ref()->get_dead_time_strategy() == ProtocolSettings::WEIGHTED_MEAN_DEAD_TIME_PERIOD )
										{
											approximate_beacon_period = ( beacon.get_beacon_period() * pit->get_protocol_settings_ref()->get_new_dead_time_period_weight() + nit->get_beacon_period() * pit->get_protocol_settings_ref()->get_old_dead_time_period_weight() ) / ( pit->get_protocol_settings_ref()->get_old_dead_time_period_weight() + pit->get_protocol_settings_ref()->get_new_dead_time_period_weight() );
										}
										uint32_t dead_time_messages_lost = ( dead_time - last_beacon_period_update ) * approximate_beacon_period;
										new_neighbor = *nit;
										new_neighbor.inc_total_beacons( 1 * pit->resolve_beacon_weight( _from ) );
										new_neighbor.inc_total_beacons_expected( ( dead_time_messages_lost + beacon.get_beacon_period_update_counter() ) * ( pit->resolve_lost_beacon_weight( _from ) ) );
										new_neighbor.update_link_stab_ratio();
										new_neighbor.update_avg_LQI( beacon_lqi, 1 );
										new_neighbor.set_beacon_period( beacon.get_beacon_period() );
										new_neighbor.set_beacon_period_update_counter( beacon.get_beacon_period_update_counter() );
										new_neighbor.set_last_beacon( current_time );
									}
								}
							}
						}
						if ( found_flag == 0 )
						{
#ifdef NB_DEBUG_RECEIVE
							debug().debug( "NeighborDiscovery-receive %x - Neighbor %x is unknown for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
#endif
							new_neighbor.set_id( _from );
							new_neighbor.set_total_beacons( 1 );
							new_neighbor.set_total_beacons_expected( 1 );
							new_neighbor.set_link_stab_ratio( 100 );
							new_neighbor.set_link_stab_ratio_inverse( 0 );
							new_neighbor.set_avg_LQI( beacon_lqi );
							new_neighbor.set_beacon_period( beacon.get_beacon_period() );
							new_neighbor.set_beacon_period_update_counter( beacon.get_beacon_period_update_counter() );
							new_neighbor.set_last_beacon( current_time );
						}
						for ( Neighbor_vector_iterator nit = beacon.get_neighborhood_ref()->begin(); nit != beacon.get_neighborhood_ref()->end(); ++nit )
						{
							if ( radio().id() == nit->get_id() )
							{
#ifdef NB_DEBUG_RECEIVE
								debug().debug( "NeighborDiscovery-receive %x - Neighbor %x was aware of node for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
#endif
								new_neighbor.set_avg_LQI_inverse( nit->get_avg_LQI() );
								new_neighbor.set_link_stab_ratio_inverse( nit->get_link_stab_ratio() );
								new_neighbor.update_link_stab_ratio_inverse( pit->resolve_beacon_weight( _from ), pit->resolve_lost_beacon_weight( _from ) );
							}
						}
#ifdef NB_DEBUG_RECEIVE
						debug().debug( "$$$\n" );
						new_neighbor.print( debug(), radio() );
						debug().debug("beacon_lqi : %i\n", beacon_lqi );
						debug().debug( "$$$\n" );
#endif
						uint8_t events_flag = 0;
						if	(	( new_neighbor.get_avg_LQI() <= pit->get_protocol_settings_ref()->get_max_avg_LQI_threshold() ) &&
								( new_neighbor.get_avg_LQI() >= pit->get_protocol_settings_ref()->get_min_avg_LQI_threshold() ) &&
								( new_neighbor.get_avg_LQI_inverse() <= pit->get_protocol_settings_ref()->get_max_avg_LQI_inverse_threshold() ) &&
								( new_neighbor.get_avg_LQI_inverse() >= pit->get_protocol_settings_ref()->get_min_avg_LQI_inverse_threshold() ) &&
								( new_neighbor.get_link_stab_ratio() <= pit->get_protocol_settings_ref()->get_max_link_stab_ratio_threshold() ) &&
								( new_neighbor.get_link_stab_ratio() >= pit->get_protocol_settings_ref()->get_min_link_stab_ratio_threshold() ) &&
								( new_neighbor.get_link_stab_ratio_inverse() <= pit->get_protocol_settings_ref()->get_max_link_stab_ratio_inverse_threshold() ) &&
								( new_neighbor.get_link_stab_ratio_inverse() >= pit->get_protocol_settings_ref()->get_min_link_stab_ratio_inverse_threshold() ) )
						{
							new_neighbor.set_active();
							if ( found_flag == 1 )
							{
								events_flag = events_flag | ProtocolSettings::UPDATE_NB;
								if ( new_neighbor.get_beacon_period() != update_neighbor_it->get_beacon_period() )
								{
									events_flag = events_flag | ProtocolSettings::BEACON_PERIOD_UPDATE;
								}
								*update_neighbor_it = new_neighbor;
								pit->resolve_overflow_strategy( _from );
#ifdef NB_DEBUG_RECEIVE
								debug().debug( "NeighborDiscovery-receive %x - Neighbor %x was updated and active for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
#endif
							}
							else
							{
								events_flag = events_flag | ProtocolSettings::NEW_NB;
								if ( pit->get_neighborhood_ref()->size() == pit->get_neighborhood_ref()->max_size() )
								{
									uint8_t rs = remove_worst_neighbor( *pit );
									if ( rs == 0 )
									{
										pit->get_neighborhood_ref()->push_back( new_neighbor );
										pit->resolve_overflow_strategy( _from );
#ifdef NB_DEBUG_RECEIVE
										debug().debug("NeighborDiscovery-receive %x - Neighbor %x was inserted and active for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
										new_neighbor.print( debug(), radio() );
#endif
									}
#ifdef NB_DEBUG_RECEIVE
									else
									{
										debug().debug("NeighborDiscovery-receive %x - Neighbor %x was could be inserted and would be active for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
										new_neighbor.print( debug(), radio() );
									}
#endif
								}
								else
								{
									pit->get_neighborhood_ref()->push_back( new_neighbor );
									pit->resolve_overflow_strategy( _from );
#ifdef NB_DEBUG_RECEIVE
										debug().debug("NeighborDiscovery-receive %x - Neighbor %x was inserted and active for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
										new_neighbor.print( debug(), radio() );
#endif
								}
							}
							uint8_t payload_found_flag = 0;
							ProtocolPayload pp;
							for ( ProtocolPayload_vector_iterator ppit = beacon.get_protocol_payloads_ref()->begin(); ppit != beacon.get_protocol_payloads_ref()->end(); ++ppit )
							{
								if ( ppit->get_protocol_id() == pit->get_protocol_id() )
								{
#ifdef NB_DEBUG_RECEIVE
									debug().debug("NeighborDiscovery-receive %x - Beacon carried a payload for protocol %i.\n", radio().id(), pit->get_protocol_id() );
#endif
									events_flag = events_flag | ProtocolSettings::NEW_PAYLOAD;
									pp = *ppit;
									payload_found_flag = 1;
								}
							}
#ifdef NB_DEBUG_RECEIVE
							if ( payload_found_flag == 0 )
							{
								debug().debug("NeighborDiscovery-receive %x - Beacon did not carry a payload for protocol %i.\n", radio().id(), pit->get_protocol_id() );
							}
#endif
							events_flag = pit->get_protocol_settings_ref()->get_events_flag() & events_flag;
							if ( events_flag != 0 )
							{
								pit->get_event_notifier_callback()( events_flag, _from, pp.get_payload_size(), pp.get_payload_data() );
							}
						}
						else
						{
							new_neighbor.set_active( 0 );
							if ( found_flag == 1 )
							{
								events_flag = events_flag | ProtocolSettings::LOST_NB;
								*update_neighbor_it = new_neighbor;
#ifdef NB_DEBUG_RECEIVE
								debug().debug("NeighborDiscovery-receive %x - Neighbor %x was updated but inactive for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
#endif
							}
							else
							{
								if (pit->get_neighborhood_ref()->size() == pit->get_neighborhood_ref()->max_size() )
								{
									uint8_t rs = remove_worst_neighbor( *pit );
									if ( rs == 0 )
									{
										pit->get_neighborhood_ref()->push_back( new_neighbor );
#ifdef NB_DEBUG_RECEIVE
										debug().debug("NeighborDiscovery-receive %x - Neighbor %x was inserted but inactive for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
										new_neighbor.print( debug(), radio() );
#endif
									}
#ifdef NB_DEBUG_RECEIVE
									else
									{
										debug().debug("NeighborDiscovery-receive %x - Neighbor %x could not be inserted but would be inactive for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
										new_neighbor.print( debug(), radio() );
									}
#endif
								}
								else
								{
									pit->get_neighborhood_ref()->push_back( new_neighbor );
#ifdef NB_DEBUG_RECEIVE
									debug().debug("NeighborDiscovery-receive %x - Neighbor %x was inserted but inactive for protocol %i.\n", radio().id(), _from, pit->get_protocol_id() );
									new_neighbor.print( debug(), radio() );
#endif
								}
							}
							events_flag = pit->get_protocol_settings_ref()->get_events_flag() & events_flag;
							if ( events_flag != 0 )
							{
								pit->get_event_notifier_callback()( events_flag, _from, 0, NULL );
							}
						}
					}
				}
#ifdef NB_DEBUG_RECEIVE
				debug().debug("NeighborDiscovery-receive %x - From %x Exiting.\n", radio().id(), _from );
#endif
			}
		}
		// --------------------------------------------------------------------
		template<class T, void(T::*TMethod)(uint8_t, node_id_t, size_t, uint8_t*) >
		uint8_t register_protocol( uint8_t _pid, ProtocolSettings _psett, T *_obj_pnt )
		{
#ifdef NB_DEBUG_REGISTER_PROTOCOL
			debug().debug("NeighborDiscovery-register_protocols %x - Entering for protocol_id = %i.\n", radio().id(), _pid );
#endif
			if ( protocols.max_size() == protocols.size() )
			{
				return PROT_LIST_FULL;
			}
			if ( ( _psett.get_protocol_payload_ref()->get_payload_size() > protocol_max_payload_size ) && ( protocol_max_payload_size_strategy == FIXED_PAYLOAD_SIZE ) )
			{
				return PAYLOAD_SIZE_OUT_OF_BOUNDS;
			}
#ifdef NB_DEBUG_REGISTER_PROTOCOL
			debug().debug("NeighborDiscovery-register_protocols %x - Before protocol loop for size calculations for protocol_id = %i.\n", radio().id(), _pid );
#endif
			size_t protocol_total_payload_size = 0;
			for ( Protocol_vector_iterator it = protocols.begin(); it != protocols.end(); ++it )
			{
				protocol_total_payload_size = it->get_protocol_settings_ref()->get_protocol_payload_ref()->serial_size() + protocol_total_payload_size;
			}
			size_t neighbors_total_payload_size = 0;
			Protocol* prot_ref = get_protocol_ref( NB_PROTOCOL_ID );
			if ( prot_ref != NULL )
			{
				Neighbor_vector* n_ref = prot_ref->get_neighborhood_ref();
#ifdef NB_DEBUG_REGISTER_PROTOCOL
			debug().debug("NeighborDiscovery-register_protocols %x - Before neighbor loop for size calculations for protocol_id = %i.\n", radio().id(), _pid );
#endif
				for ( Neighbor_vector_iterator it = n_ref->begin(); it != n_ref->end(); ++it )
				{
					neighbors_total_payload_size = it->serial_size() + neighbors_total_payload_size;
				}
			}
			Beacon b;
			if ( protocol_total_payload_size + neighbors_total_payload_size + b.serial_size() + sizeof(message_id_t) + sizeof(size_t) + _psett.get_protocol_payload_ref()->serial_size() > Radio::MAX_MESSAGE_LENGTH )
			{
				return NO_PAYLOAD_SPACE;
			}
			for ( Protocol_vector_iterator it = protocols.begin(); it != protocols.end(); ++it )
			{
				if ( ( it->get_protocol_id() == _pid ) || ( it->get_protocol_settings_ref()->get_protocol_payload_ref()->get_protocol_id() == _pid ) )
				{
					return PROT_NUM_IN_USE;
				}
			}
			Protocol p;
			p.set_protocol_id( _pid );
			p.set_protocol_settings( _psett );
			p.set_event_notifier_callback( event_notifier_delegate_t::template from_method<T, TMethod > ( _obj_pnt ) );
			protocols.push_back( p );

#ifdef NB_DEBUG_REGISTER_PROTOCOL
			debug().debug("NeighborDiscovery-register_protocols %x - Exiting for protocol_id = %i.\n", radio().id(), _pid );
#endif
			return SUCCESS;
		}
		// --------------------------------------------------------------------
		void nb_daemon( void* user_data = NULL )
		{
#ifdef NB_DEBUG_NB_DAEMON
			debug().debug("NeighborDiscovery-nb_daemon %x - Entering.\n", radio().id() );
#endif
			if ( status == ACTIVE_STATUS )
			{
				time_t current_time = clock().time();
				for ( Protocol_vector_iterator pit = protocols.begin(); pit != protocols.end(); ++pit )
				{
					for ( Neighbor_vector_iterator nit = pit->get_neighborhood_ref()->begin(); nit != pit->get_neighborhood_ref()->end(); ++nit )
					{
						uint32_t dead_time = ( clock().seconds( current_time ) - clock().seconds( nit->get_last_beacon() ) ) * 1000 + ( clock().milliseconds( current_time ) - clock().milliseconds( nit->get_last_beacon() ) );
						if ( ( dead_time > nit->get_beacon_period() + NB_RELAX_MILLIS ) && ( nit->get_id() != radio().id() ) )
						{
#ifdef NB_DEBUG_NB_DAEMON
							debug().debug("NeighborDiscovery-nb_daemon %x - Teasing node %x.", radio().id(), nit->get_id() );
#endif
							nit->inc_total_beacons_expected( dead_time / nit->get_beacon_period() * ( pit->resolve_lost_beacon_weight( nit->get_id() ) ) );
							nit->update_link_stab_ratio();
							nit->set_beacon_period( nit->get_beacon_period() );
							nit->set_last_beacon( current_time );
							uint8_t events_flag = 0;
							if 	( ( nit->get_link_stab_ratio() <= pit->get_protocol_settings_ref()->get_max_link_stab_ratio_threshold() ) &&
								( nit->get_link_stab_ratio() >= pit->get_protocol_settings_ref()->get_min_link_stab_ratio_threshold() ) &&
								( nit->get_link_stab_ratio_inverse() <= pit->get_protocol_settings_ref()->get_max_link_stab_ratio_inverse_threshold() ) &&
								( nit->get_link_stab_ratio_inverse() >= pit->get_protocol_settings_ref()->get_min_link_stab_ratio_inverse_threshold() ) )
							{
								nit->set_active();
								events_flag = events_flag | ProtocolSettings::UPDATE_NB;
							}
							else
							{
								nit->set_active( 0 );
								events_flag = events_flag | ProtocolSettings::LOST_NB;
							}
							events_flag = pit->get_protocol_settings_ref()->get_events_flag() & events_flag;
							if ( events_flag != 0 )
							{
								pit->get_event_notifier_callback()( events_flag, nit->get_id(), 0, NULL );
							}
						}
					}
				}
				timer().template set_timer<self_t, &self_t::nb_daemon> ( nb_daemon_period, this, 0 );
			}
#ifdef NB_DEBUG_NB_DAEMON
			debug().debug("NeighborDiscovery-nb_daemon %x - Exiting.\n", radio().id() );
#endif
		}
		// --------------------------------------------------------------------
		uint8_t remove_worst_neighbor( Protocol& p_ref )
		{
			uint8_t min_link_stab_ratio = 100;
			uint8_t min_link_stab_ratio_inverse = 100;
			uint8_t max_avg_lqi = 0;
			uint8_t max_avg_lqi_inverse = 0;
			Neighbor_vector_iterator mlsr		= p_ref.get_neighborhood_ref()->begin();
			Neighbor_vector_iterator mlsr_in	= p_ref.get_neighborhood_ref()->begin();
			Neighbor_vector_iterator mal		= p_ref.get_neighborhood_ref()->begin();
			Neighbor_vector_iterator mal_in		= p_ref.get_neighborhood_ref()->begin();
			for ( Neighbor_vector_iterator nit = p_ref.get_neighborhood_ref()->begin(); nit != p_ref.get_neighborhood_ref()->end(); ++nit )
			{
				if ( ( min_link_stab_ratio > nit->get_link_stab_ratio() ) && ( nit->get_active() == 0 ) )
				{
					mlsr = nit;
					min_link_stab_ratio = nit->get_link_stab_ratio();
				}
				if ( ( min_link_stab_ratio > nit->get_link_stab_ratio_inverse() ) && ( nit->get_active() == 0 ) )
				{
					mlsr_in = nit;
					min_link_stab_ratio_inverse = nit->get_link_stab_ratio_inverse();
				}
				if ( ( max_avg_lqi < nit->get_avg_LQI() ) && ( nit->get_active() == 0 ) )
				{
					mal_in = nit;
					max_avg_lqi = nit->get_avg_LQI();
				}
				if ( ( max_avg_lqi_inverse < nit->get_avg_LQI_inverse() ) && ( nit->get_active() == 0 ) )
				{
					mal_in = nit;
					max_avg_lqi_inverse = nit->get_avg_LQI_inverse();
				}
			}
			if ( min_link_stab_ratio !=0 )
			{
				p_ref.get_neighborhood_ref()->erase( mlsr );
				return ProtocolSettings::NEIGHBOR_REMOVED;
			}
			if ( min_link_stab_ratio_inverse !=0 )
			{
				p_ref.get_neighborhood_ref()->erase( mlsr_in );
				return ProtocolSettings::NEIGHBOR_REMOVED;
			}
			if ( max_avg_lqi !=0 )
			{
				p_ref.get_neighborhood_ref()->erase( mal );
				return ProtocolSettings::NEIGHBOR_REMOVED;
			}
			if ( max_avg_lqi_inverse !=0 )
			{
				p_ref.get_neighborhood_ref()->erase( mal_in );
				return ProtocolSettings::NEIGHBOR_REMOVED;
			}
			return 0;
		}
		// --------------------------------------------------------------------
		uint8_t register_protocol( Protocol p )
		{
			if ( protocols.max_size() == protocols.size() )
			{
				return PROT_LIST_FULL;
			}
			if ( ( p.get_protocol_settings_ref()->get_protocol_payload_ref()->serial_size() > protocol_max_payload_size ) && ( protocol_max_payload_size_strategy == FIXED_PAYLOAD_SIZE ) )
			{
				return PAYLOAD_SIZE_OUT_OF_BOUNDS;
			}
			uint8_t protocol_total_payload_size = 0;
			for ( Protocol_vector_iterator it = protocols.begin(); it != protocols.end(); ++it )
			{
				protocol_total_payload_size = it->get_protocol_settings_ref()->get_protocol_payload_ref()->serial_size() + protocol_total_payload_size;
			}
			size_t neighbors_total_payload_size = 0;
			Neighbor_vector* n_ref = get_protocol_ref( NB_PROTOCOL_ID )->get_neighborhood_ref();
			for ( Neighbor_vector_iterator it = n_ref->begin(); it != n_ref->end(); ++it )
			{
				neighbors_total_payload_size = it->serial_size() + neighbors_total_payload_size;
			}
			Beacon b;
			if ( protocol_total_payload_size + neighbors_total_payload_size + b.serial_size() + sizeof(message_id_t) + sizeof(size_t) + p.get_protocol_settings_ref()->get_protocol_payload_ref()->serial_size() > Radio::MAX_MESSAGE_LENGTH )
			{
				return NO_PAYLOAD_SPACE;
			}
			for ( Protocol_vector_iterator it = protocols.begin(); it != protocols.end(); ++it )
			{
				if (	( it->get_protocol_id() == p.get_protocol_id() ) ||
						( it->get_protocol_id() == p.get_protocol_settings_ref()->get_protocol_payload_ref()->get_protocol_id() ) ||
						( it->get_protocol_settings_ref()->get_protocol_payload_ref()->get_protocol_id() == p.get_protocol_id() ) ||
						( it->get_protocol_settings_ref()->get_protocol_payload_ref()->get_protocol_id() == p.get_protocol_settings_ref()->get_protocol_payload_ref()->get_protocol_id() )
					)
				{
					return PROT_NUM_IN_USE;
				}
			}
			protocols.push_back( p );
			return SUCCESS;
		}
		// --------------------------------------------------------------------
		uint8_t unregister_protocol( uint8_t _pid )
		{
			for ( Protocol_vector_iterator it = protocols.begin(); it != protocols.end(); ++it )
			{
				if ( it->get_protocol_id() == _pid )
				{
					protocols.erase( it );
					return SUCCESS;
				}
			}
			return INV_PROT_ID;
		}
		// --------------------------------------------------------------------
		Protocol* get_protocol_ref( uint8_t _pid )
		{
			for ( Protocol_vector_iterator it = protocols.begin(); it != protocols.end(); ++it )
			{
				if ( it->get_protocol_id() == _pid )
				{
					return &(*it);
				}
			}
			return NULL;
		}
		// --------------------------------------------------------------------
		int get_status()
		{
			return status;
		}
		// --------------------------------------------------------------------
		void set_status( int _st )
		{
			status = _st;
		}
		// --------------------------------------------------------------------
		millis_t get_beacon_period()
		{
			millis_t beacon_period_old = beacon_period;
			if ( beacon_period_strategy == FIXED_PERIOD )
			{
				return beacon_period;
			}
			else if ( beacon_period_strategy == LEAST_PERIOD )
			{
				millis_t least = 0xfff;
				for ( Protocol_vector_iterator it = protocols.begin(); it != protocols.end(); ++it )
				{
					if ( it->get_protocol_settings_ref()->get_proposed_beacon_period() < least )
					{
						least = it->get_protocol_settings_ref()->get_proposed_beacon_period();
					}
				}
				beacon_period = least;
			}
			else if ( beacon_period_strategy == MEAN_PERIOD )
			{
				millis_t sum, num;
				sum = num = 0;
				for ( Protocol_vector_iterator it = protocols.begin(); it != protocols.end(); ++it )
				{
					sum = it->get_protocol_settings_ref()->get_proposed_beacon_period() + sum;
					num = num + 1;
				}
				millis_t mean = sum / num;
				beacon_period = mean;
			}
			else if ( beacon_period_strategy == WEIGHTED_MEAN_PERIOD )
			{
				millis_t sum, num;
				sum = num = 0;
				for ( Protocol_vector_iterator it = protocols.begin(); it != protocols.end(); ++it )
				{
					sum = it->get_protocol_settings_ref()->get_proposed_beacon_period() * it->get_protocol_settings_ref()->get_proposed_beacon_period_weight() + sum;
					num = it->get_protocol_settings_ref()->get_proposed_beacon_period_weight() + num;
				}
				millis_t weighted_mean = sum / num;
				beacon_period = weighted_mean;
			}
			if ( beacon_period_old != beacon_period )
			{
				for ( Protocol_vector_iterator pit = protocols.begin(); pit != protocols.end(); ++pit )
				{
					if ( pit->get_protocol_settings_ref()->get_events_flag() & ProtocolSettings::BEACON_PERIOD_UPDATE )
					{
						pit->get_event_notifier_callback()( ProtocolSettings::BEACON_PERIOD_UPDATE, radio().id(), 0, NULL );
					}
				}
			}
			return beacon_period;
		}
		// --------------------------------------------------------------------
		void set_beacon_period( millis_t _bp )
		{
			beacon_period =_bp;
		}
		// --------------------------------------------------------------------
		int8_t get_transmission_power_dB()
		{
			int8_t old_transmission_power_dB = transmission_power_dB;
			if ( transmission_power_dB_strategy == FIXED_TRANSM )
			{
				return transmission_power_dB;
			}
			else if ( transmission_power_dB_strategy == LEAST_TRANSM )
			{
				int8_t least = 128;
				for ( Protocol_vector_iterator pit = protocols.begin(); pit != protocols.end(); ++pit )
				{
					if ( pit->get_protocol_settings_ref()->get_proposed_transmission_power_dB() < least )
					{
						least = pit->get_protocol_settings_ref()->get_proposed_transmission_power_dB();
					}
				}
				transmission_power_dB = least;
			}
			else if ( transmission_power_dB_strategy == MEAN_TRANSM )
			{
				int8_t sum, num;
				sum = num = 0;
				for ( Protocol_vector_iterator pit = protocols.begin(); pit != protocols.end(); ++pit )
				{
					sum = pit->get_protocol_settings_ref()->get_proposed_transmission_power_dB() + sum;
					num = num + 1;
				}
				int8_t mean = sum / num;
				transmission_power_dB = mean;
			}
			else if ( transmission_power_dB_strategy == WEIGHTED_MEAN_TRANSM )
			{
				int8_t sum, num;
				sum = num = 0;
				for ( Protocol_vector_iterator pit = protocols.begin(); pit != protocols.end(); ++pit )
				{
					sum = pit->get_protocol_settings_ref()->get_proposed_transmission_power_dB() * pit->get_protocol_settings_ref()->get_proposed_transmission_power_dB_weight() + sum;
					num = pit->get_protocol_settings_ref()->get_proposed_transmission_power_dB_weight() + num;
				}
				int8_t weighted_mean = sum / num;
				transmission_power_dB = weighted_mean;
			}
			if ( old_transmission_power_dB != transmission_power_dB )
			{
				for ( Protocol_vector_iterator pit = protocols.begin(); pit != protocols.end(); ++pit )
				{
					if ( pit->get_protocol_settings_ref()->get_events_flag() & ProtocolSettings::TRANS_DB_UPDATE )
					{
						pit->get_event_notifier_callback()( ProtocolSettings::TRANS_DB_UPDATE, radio().id(), 0, NULL );
					}
				}
			}
			return transmission_power_dB;
		}
		// --------------------------------------------------------------------
		void set_transmission_power_dB( int8_t _tp_dB )
		{
			 transmission_power_dB = _tp_dB;
		}
		// --------------------------------------------------------------------
		uint8_t get_channel()
		{
			return channel;
		}
		// --------------------------------------------------------------------
		void set_channel( uint8_t _ch )
		{
			channel =_ch;
		}
		// --------------------------------------------------------------------
		uint8_t get_protocol_max_payload_size()
		{
			return protocol_max_payload_size;
		}
		// --------------------------------------------------------------------
		void set_transmission_power_dB_strategy( uint8_t _tp_dB_s )
		{
			if ( _tp_dB_s > TP_DB_STRATEGY_NUM_VALUES )
			{
				transmission_power_dB_strategy = FIXED_TRANSM;
			}
			else
			{
				transmission_power_dB_strategy = _tp_dB_s;
			}
		}
		// --------------------------------------------------------------------
		uint8_t get_transmission_power_dB_strategy()
		{
			return transmission_power_dB_strategy;
		}
		// --------------------------------------------------------------------
		void set_beacon_period_strategy( uint8_t _bp_s )
		{
			if ( _bp_s > BP_STRATEGY_NUM_VALUES )
			{
				beacon_period_strategy = FIXED_TRANSM;
			}
			else
			{
				beacon_period_strategy = _bp_s;
			}
		}
		// --------------------------------------------------------------------
		uint8_t get_beacon_period_strategy()
		{
			return beacon_period_strategy;
		}
		// --------------------------------------------------------------------
		void set_protocol_max_payload_size_strategy( uint8_t _mpps_s )
		{
			if ( _mpps_s > PP_STRATEGY_NUM_VALUES )
			{
				protocol_max_payload_size_strategy = FIXED_PAYLOAD_SIZE;
			}
			else
			{
				protocol_max_payload_size_strategy = _mpps_s;
			}
		}
		// --------------------------------------------------------------------
		uint8_t get_protocol_max_payload_size_strategy()
		{
			return protocol_max_payload_size_strategy;
		}
		// --------------------------------------------------------------------
		millis_t get_relax_millis()
		{
			return relax_millis;
		}
		// --------------------------------------------------------------------
		void set_relax_millis( millis_t _rm )
		{
			relax_millis = _rm;
		}
		// --------------------------------------------------------------------
		millis_t get_nb_daemon_period()
		{
			return nb_daemon_period;
		}
		// --------------------------------------------------------------------
		void set_nb_daemon_period( millis_t _nbdp )
		{
			relax_millis = _nbdp;
		}
		// --------------------------------------------------------------------
		void init( Radio& _radio, Timer& _timer, Debug& _debug, Clock& _clock )
		{
			radio_ = &_radio;
			timer_ = &_timer;
			debug_ = &_debug;
			clock_ = &_clock;
		}
		// --------------------------------------------------------------------
		Radio& radio()
		{
			return *radio_;
		}
		// --------------------------------------------------------------------
		Clock& clock()
		{
			return *clock_;
		}
		// --------------------------------------------------------------------
		Timer& timer()
		{
			return *timer_;
		}
		// --------------------------------------------------------------------
		Debug& debug()
		{
			return *debug_;
		}
		// --------------------------------------------------------------------
		enum error_codes
		{
			SUCCESS,
			PROT_NUM_IN_USE,
			PROT_LIST_FULL,
			INV_PROT_ID,
			NO_PAYLOAD_SPACE,
			PAYLOAD_SIZE_OUT_OF_BOUNDS,
			ERROR_CODES_NUM_VALUES
		};
		enum neighbor_discovery_status
		{
			ACTIVE_STATUS,
			WAITING_STATUS,
			NB_STATUS_NUM_VALUES
		};
		enum transmission_power_dB_strategies
		{
			FIXED_TRANSM,
			LEAST_TRANSM,
			MEAN_TRANSM,
			WEIGHTED_MEAN_TRANSM,
			TP_DB_STRATEGY_NUM_VALUES
		};
		enum beacon_period_strategies
		{
			FIXED_PERIOD,
			LEAST_PERIOD,
			MEAN_PERIOD,
			WEIGHTED_MEAN_PERIOD,
			BP_STRATEGY_NUM_VALUES
		};
		enum protocol_payload_strategies
		{
			FIXED_PAYLOAD_SIZE,
			DYNAMIC_PAYLOAD_SIZE,
			PP_STRATEGY_NUM_VALUES
		};
		enum protocol_ids
		{
			NB_PROTOCOL_ID,
			TRACKING_PROTOCOL_ID
		};
		enum message_ids
		{
			NB_MESSAGE = 12
		};
	private:
		uint32_t recv_callback_id_;
        uint8_t status;
        millis_t beacon_period;
        uint8_t channel;
        int8_t transmission_power_dB;
        Protocol_vector protocols;
        size_t protocol_max_payload_size;
        uint8_t transmission_power_dB_strategy;
        uint8_t protocol_max_payload_size_strategy;
        uint8_t beacon_period_strategy;
        millis_t relax_millis;
        millis_t nb_daemon_period;
        Radio * radio_;
        Clock * clock_;
        Timer * timer_;
        Debug * debug_;
    };
}

#endif
