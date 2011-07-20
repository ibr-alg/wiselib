#ifndef __ROOMBA_MOVEMENT_H__
#define __ROOMBA_MOVEMENT_H__

#include <stdint.h>

#include "util/standalone_math.h"
#include "util/serialization/serialization.h"

#include "external_interface/external_interface.h"

#include "external_interface/pc/roomba.h"
#include "external_interface/pc/roomba_motion.h"
#include "external_interface/pc/roomba_event_sensor.h"

#include "algorithms/roomba_message_delivery/roomba_movement_message.h"

template< 	typename OsModel_P,
		typename Roomba_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Debug_P = typename OsModel_P::Debug,
		typename Clock_P = typename OsModel_P::Clock,
		typename Timer_P = typename OsModel_P::Timer,
		typename Rand_P = typename OsModel_P::Rand
		>
class RoombaMovement {
public:
	typedef OsModel_P Os;
	typedef Roomba_P Roomba;
	typedef Radio_P Radio;
	typedef Debug_P Debug;
	typedef Clock_P Clock;
	typedef Timer_P Timer;
	typedef Rand_P Rand;

	typedef RoombaMovement<Os, Roomba, Radio, Debug, Clock, Timer, Rand> self_type;
	typedef self_type* self_pointer_t;

	typedef typename RoombaMovementMessage<Os, Radio>::self_type msg_t;
	typedef wiselib::RoombaMotion<Roomba, typename Roomba::Math> RoombaMotion;
	typedef wiselib::RoombaEventSensor<Os, Roomba> RoombaEventSensor;

	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::message_id_t message_id_t;

	enum MovementPattern {
		RANDOM_WALK,
		LINE
	};

	static const uint32_t
		RANDOM_WALK_MIN_MOVE = 3000,
		RANDOM_WALK_MAX_MOVE = 8000,
		RANDOM_WALK_MIN_ROTATE = 1000,
		RANDOM_WALK_MAX_ROTATE = 3000;

	void init( Roomba& roomba, Radio* radio, Debug& debug, Clock& clock, Timer& timer, Rand& rand ) {
		radio_ = radio;
		debug_ = &debug;
		timer_ = &timer;
		rand_ = &rand;
		clock_ = &clock;
		roomba_ = &roomba;

		roomba_motion_.init( *roomba_ );
		roomba_event_sensor_.init(*roomba_);

		init();
	}


	void init()
	{
		movement_pattern_ = RANDOM_WALK;
		rand_->srand( clock_->microseconds( clock_->time() ) );

		if( radio_ ) {
			radio_->enable_radio();
			radio_->template reg_recv_callback<self_type, &self_type::on_receive>( this  );
		}

		roomba_event_sensor_.template reg_event_callback<self_type, &self_type::on_event>( this );

		speed_ = 0;
		duration_ = 10000;

		stop();
	}

	void inline start( int16_t speed = 300 )
	{
		if( is_stopped() )
		{
			speed_ = speed;
			stopped_ = false;
			next_action_ = MOVE;
			perform_action();
		}
	}

	void inline stop()
	{
		stop_movement();
		stopped_ = true;
	}

	bool inline is_stopped()
	{
		return stopped_;
	}

	void inline set_movement_pattern_to_line( uint32_t duration=10000 )
	{
		movement_pattern_ = LINE;
		duration_ = duration;
	}

	void inline set_movement_pattern_to_random_walk()
	{
		movement_pattern_ = RANDOM_WALK;
	}

protected:
	enum ActionType {
		NONE = 0,
		MOVE,
		MOVE_BACKWARD,
		ROTATE,
		STOP,
	};

	void turn( int16_t speed ) {
		roomba_motion_.turn( speed );
	}

	void move_callback( void* userdata ) {
		if( cur_speed_ != dest_speed_ ) {
			if( dest_speed_ - cur_speed_ > 0 ) {
				cur_speed_ = ( cur_speed_ + 50 > dest_speed_ ) ? dest_speed_ : cur_speed_ + 50;
			} else {
				cur_speed_ = ( cur_speed_ - 50 < dest_speed_ ) ? dest_speed_ :cur_speed_ - 50;
			}

		//	debug_->debug( "Changing speed to %d.\n", cur_speed_ );
			roomba_motion_.move( cur_speed_ );

			timer_->template set_timer<self_type, &self_type::move_callback>( 100, this, 0 );
		}

		if( cur_speed_ == dest_speed_ )
			changing_speed_ = false;
	}
		
		void distance() {
			return roomba_motion_.distance();
		}

	void move( int16_t speed ) {
		dest_speed_ = speed;
		changing_speed_ = true;

		move_callback( 0 );
	}

	void stop_movement() {
		roomba_motion_.move( 0 );
		cur_speed_ = 0;
		dest_speed_ = 0;
		changing_speed_ = false;
	}

	void perform_action()
	{
		if( !is_stopped() )
		{
			uint32_t duration;
			int8_t direction;

			if( movement_pattern_ == RANDOM_WALK )
			{
				switch( next_action_ )
				{
				case MOVE:
					duration = RANDOM_WALK_MIN_MOVE + uint32_t( double( (*rand_)() )/Rand::RANDOM_MAX*(RANDOM_WALK_MAX_MOVE-RANDOM_WALK_MIN_MOVE) );
					debug_->debug( "Moving for %ims.\n", duration );
					move( speed_ );
					next_action_ = STOP;

					break;

				case STOP:
					duration = 500;
					debug_->debug( "Stopping.\n", duration );
					move( 0 );
					next_action_ = ROTATE;

					break;

				case ROTATE:
					duration = RANDOM_WALK_MIN_ROTATE + uint32_t( double( (*rand_)() )/(Rand::RANDOM_MAX)*(RANDOM_WALK_MAX_ROTATE-RANDOM_WALK_MIN_ROTATE));
					debug_->debug( "Turning for %ims.\n", duration );
					direction = ( ( (*rand_)() <= Rand::RANDOM_MAX/2 ) ? 1 : -1 );
					turn( speed_ * direction );
					next_action_ = MOVE;

					break;

				default:
					debug_->debug( "Encountered unknown action. Stopped.\n" );
					stop();
				}
			} else {
				switch( next_action_ )
				{
				case MOVE:
					duration = duration_;
					debug_->debug( "Moving for %ims.\n", duration );
					move( speed_ );
					next_action_ = MOVE_BACKWARD;
					break;

				case MOVE_BACKWARD:
					duration = duration_;
					debug_->debug( "Moving backward for %ims.\n", duration );
					move( -speed_ );
					next_action_ = MOVE;
					break;

				default:
					debug_->debug( "Encountered unknown action. Stopped.\n" );
					stop();
				}
			}

			if( !is_stopped() ) {
				timer_->template set_timer<self_type, &self_type::on_timer>( duration, this, (void*)0 );
				debug_->debug( "-> Movement Timer was set\n" );
			}
		}
		else
		{
			stop();
		}
	}

	/*
	 * Timer callback.
	 */
	void on_timer( void* userdata ) {
		if( changing_speed_ )
		{
			timer_->template set_timer<self_type, &self_type::on_timer>( 500, this, (void*)0 );
		} else {
			perform_action();
		}
	}

	/*
	 * Receive callback.
	 *
	 * Starts/stops movement on incoming start/stop-message.
	 */
	void on_receive( node_id_t id, size_t size, block_data_t* data) {
		message_id_t msg_id = wiselib::read<Os, block_data_t, message_id_t>( data );

		if( msg_id == msg_t::ROOMBA_MOVEMENT_MESSAGE_ID )
		{
			msg_t* msg = (msg_t*)data;

			switch( msg->msg_type() )
			{
			case msg_t::STOP_MOVEMENT:
				debug_->debug( "Received stop\n");
				stop();
				break;
			case msg_t::START_MOVEMENT:
				debug_->debug( "Received start\n");
				start();
				break;
			default:
				debug_->debug( "Received RoombaMovementMessage with unknown type %d\n", msg->msg_type() );
			}
		}
	}


	void on_event( uint8_t event ) {
		if( event == RoombaEventSensor::EVENT_WALL )
		{
			int16_t distance = (int16_t) roomba_motion_.distance();
				debug_->debug( "Roomba hit wall! Moved distance is %d\n", distance );
				if( movement_pattern_ == RANDOM_WALK )
				{
					stop_movement();
					move( -speed_ );
					next_action_ = STOP;
				} else {
					stop();
				}
			}
		}

		Radio* radio_;
		Roomba* roomba_;
		RoombaMotion roomba_motion_;
		RoombaEventSensor roomba_event_sensor_;
		typename Debug::self_pointer_t debug_;
		typename Timer::self_pointer_t timer_;
		typename Rand::self_pointer_t rand_;
		typename Clock::self_pointer_t clock_;

		ActionType next_action_;
		bool stopped_;
		int16_t speed_;
		uint32_t duration_;
		MovementPattern movement_pattern_;

		int16_t cur_speed_;
		int16_t dest_speed_;
		bool changing_speed_;
};

#endif
