/*
 *      Author: Juan Farré, UPC
 */

#ifndef ALGORITHMS_TOPOLOGY_ADJUST_POWER_SEQUENTIAL_TRY_H_
#define ALGORITHMS_TOPOLOGY_ADJUST_POWER_SEQUENTIAL_TRY_H_

#include <stdint.h>

#include "external_interface/external_interface.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/algorithm.h"

#include "algorithms/topology/adjust_power/sequential_try_types.h"
#include "algorithms/topology/adjust_power/sequential_try_ping_message.h"
#include "algorithms/topology/adjust_power/sequential_try_pong_message.h"

namespace wiselib {

#define MAX_NEIGH_DEF 32
#define SEND_DELAY 500
#define POWER_SET_DELAY 500
#define DELTA_DEF 10000
#define PING_TASK 0
#define PONG_TASK 1
#define RESTORE_POWER_TASK 2
#define CHECK_TASK 3
#define END_PONG_TASK 4
#define SEND_TASK 5
#define END_PING_TASK 6

template<class OsModel_P, class Radio_P = typename OsModel_P::TxRadio,
		class Neigh_P = vector_static<OsModel_P, typename Radio_P::node_id_t,
				MAX_NEIGH_DEF> , class Timer_P = typename OsModel_P::Timer>
class SequentialTry {
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Neigh_P Neighbors;
	typedef Timer_P Timer;
#ifdef DEBUG
	typedef typename OsModel_P::Debug Debug;
#endif

	typedef SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P> self_type;

	typedef typename OsModel::Os Os;

	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::TxPower TxPower;
	typedef typename Timer::millis_t millis_t;

	SequentialTry();

	void enable();
	void disable();

	TxPower power() const;

#ifdef DEBUG
	void init(Radio &r, Timer &t, Debug &d) {
		radio = &r;
		timer = &t;
		debug = &d;
	}
#else
	void init(Radio &r, Timer &t) {
		radio = &r;
		timer = &t;
	}
#endif

	void set_neighbors(Neighbors &);
	Neighbors &neighbors();
	void set_delta(millis_t);
	millis_t delta();

	template<class T, void(T::*TMethod)()>
	void reg_listener_callback(T *);

	template<void(*TMethod)()>
	void reg_listener_callback();

	void unreg_listener_callback();

	static void set_default_delta(millis_t);
	static millis_t default_delta();

private:
	typedef delegate0<void> delegate_t;

	void notify_listeners();

	void start_ping();
	void send_ping();
	void send_pong();
	void receive(node_id_t from, size_t len, block_data_t *data);
	void timer_callback(void *);

	SequentialTryPongMessage<OsModel, Radio> pong_msg;
	Radio *radio;
	Timer *timer;
#ifdef DEBUG
	Debug *debug;
#endif
	Neighbors replied;
	TxPower power_;
	delegate_t callback_;
	Neighbors *neigh;
	millis_t delta_;
	bool enabled;
	bool initialized;
	bool wait;
	bool waiting;

	static uint8_t const ping_task;
	static uint8_t const pong_task;
	static uint8_t const send_task;
	static uint8_t const restore_power_task;
	static uint8_t const check_task;
	static uint8_t const end_ping_task;
	static uint8_t const end_pong_task;
	static millis_t delta_def;
};

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
uint8_t const SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::ping_task(
		PING_TASK);

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
uint8_t const SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::pong_task(
		PONG_TASK);

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
uint8_t const SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::send_task(
		SEND_TASK);

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
uint8_t const
		SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::restore_power_task(
				RESTORE_POWER_TASK);

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
uint8_t const SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::check_task(
		CHECK_TASK);

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
uint8_t const
		SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::end_ping_task(
				END_PING_TASK);

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
uint8_t const
		SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::end_pong_task(
				END_PONG_TASK);

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
typename SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::millis_t
		SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::delta_def(
				DELTA_DEF);

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::set_default_delta(
		millis_t millis) {
	delta_def = millis;
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
typename SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::millis_t SequentialTry<
		OsModel_P, Radio_P, Neigh_P, Timer_P>::default_delta() {
	return delta_def;
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::set_neighbors(
		Neighbors &n) {
	neigh = &n;
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
typename SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::Neighbors &SequentialTry<
		OsModel_P, Radio_P, Neigh_P, Timer_P>::neighbors() {
	return *neigh;
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::set_delta(
		millis_t millis) {
	delta_ = millis;
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
typename SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::millis_t SequentialTry<
		OsModel_P, Radio_P, Neigh_P, Timer_P>::delta() {
	return delta_;
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
typename SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::TxPower SequentialTry<
		OsModel_P, Radio_P, Neigh_P, Timer_P>::power() const {
	return power_;
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
template<class T, void(T::*TMethod)()>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::reg_listener_callback(
		T *obj_pnt) {
	callback_ = delegate_t::from_method<T, TMethod>(obj_pnt);
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
template<void(*TMethod)()>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::reg_listener_callback() {
	callback_ = delegate_t::from_function<TMethod>();
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::unreg_listener_callback() {
	callback_ = delegate_t();
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::notify_listeners() {
	if (callback_)
		callback_();
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::send_ping() {
	radio->set_power(power_);
	wait = true;
	timer->template set_timer<self_type, &self_type::timer_callback> (
			POWER_SET_DELAY, this, const_cast<uint8_t *> (&send_task));
#ifdef DEBUG
	debug->debug("SequentialTry set power to %i\n", power_.to_dB());
#endif
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::start_ping() {
#ifdef DEBUG
	debug->debug("SequentialTry ready to send ping\n");
#endif
	if (wait)
		waiting = true;
	else
		send_ping();
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::send_pong() {
	radio->send(Radio::BROADCAST_ADDRESS, pong_msg.buffer_size(),
			pong_msg.buf());
	timer->template set_timer<self_type, &self_type::timer_callback> (
			SEND_DELAY, this, const_cast<uint8_t *> (&end_pong_task));
	wait = true;
#ifdef DEBUG
	debug->debug("SequentialTry sending pong\n");
#endif
	pong_msg.set_neighbor_number(0);
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::timer_callback(
		void *userdata) {
	typedef typename Neighbors::iterator Iter;
	if (!enabled)
		return;
	switch (*static_cast<uint8_t const *> (userdata)) {
	case PING_TASK:
		start_ping();
		break;
	case SEND_TASK: {
		SequentialTryPingMessage<OsModel, Radio> msg;
		msg.set_msg_id(SequentialTryPingMsgId);
		radio->send(Radio::BROADCAST_ADDRESS, msg.buffer_size(), msg.buf());
		timer->template set_timer<self_type, &self_type::timer_callback> (
				SEND_DELAY, this, const_cast<uint8_t *> (&restore_power_task));
#ifdef DEBUG
		debug->debug("SequentialTry sent ping at %i dB\n", power_.to_dB());
#endif
	}
		break;
	case RESTORE_POWER_TASK:
		radio->set_power(TxPower::MAX);
		timer->template set_timer<self_type, &self_type::timer_callback> (
				POWER_SET_DELAY, this, const_cast<uint8_t *> (&end_ping_task));
		timer->template set_timer<self_type, &self_type::timer_callback> (
				delta() + 2 * POWER_SET_DELAY + 2 * SEND_DELAY, this,
				const_cast<uint8_t *> (&check_task));
#ifdef DEBUG
		debug->debug("SequentialTry restored max power\n");
#endif
		break;
	case END_PING_TASK:
#ifdef DEBUG
		debug->debug("SequentialTry finished sending ping\n");
#endif
		wait = false;
		if (waiting) {
			send_pong();
			waiting = false;
		}
		break;
	case CHECK_TASK:
		for (Iter i = neigh->begin(); i != neigh->end(); ++i) {
			if (find(replied.begin(), replied.end(), *i) == replied.end()) {
				++power_;
				if (power_ == TxPower::MAX)
					break;
				start_ping();
				return;
			}
		}
#ifdef DEBUG
		debug->debug("SequentialTry found final power: %i\n", power_.to_dB());
#endif
		notify_listeners();
		break;
	case PONG_TASK:
		if (pong_msg.neighbor_number() != 0) {
#ifdef DEBUG
			debug->debug("SequentialTry ready to send pong\n");
#endif
			if (wait)
				waiting = true;
			else
				send_pong();
		}
		timer->template set_timer<self_type, &self_type::timer_callback> (
				delta(), this, const_cast<uint8_t *> (&pong_task));
		break;
	case END_PONG_TASK:
#ifdef DEBUG
		debug->debug("SequentialTry finished sending pong\n");
#endif
		wait = false;
		if (waiting) {
			send_ping();
			waiting = false;
		}
		break;
	}
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::receive(
		node_id_t from, size_t len, block_data_t *data) {
	if (len == 0)
		return;
	switch (*data) {
	case SequentialTryPingMsgId:
#ifdef DEBUG
		debug->debug("SequentialTry receives ping from %i\n", from);
#endif
		pong_msg.set_neighbor(pong_msg.neighbor_number(), from);
		pong_msg.set_neighbor_number(pong_msg.neighbor_number() + 1);
		break;
	case SequentialTryPongMsgId: {
		SequentialTryPongMessage<OsModel, Radio>
				*msg =
						reinterpret_cast<SequentialTryPongMessage<OsModel,
								Radio> *> (data);
		for (size_t i = 0; i < msg->neighbor_number(); ++i)
			if (msg->neighbor(i) == radio->id()) {
				if (find(replied.begin(), replied.end(), from) == replied.end())
					replied.push_back(from);
#ifdef DEBUG
				debug->debug("SequentialTry gets reply from %i\n", from);
#endif
				break;
			}
	}
		break;
	}
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::enable() {
	if (enabled || !radio)
		return;
	if (!initialized) {
		if (TxPower::MIN == TxPower::MAX) {
#ifdef DEBUG
			debug->debug("SequentialTry: Only one power lever. Nothing to do.\n");
#endif
			notify_listeners();
		}
		radio->template reg_recv_callback<self_type, &self_type::receive> (this);
		initialized = true;
#ifdef DEBUG
		debug->debug("SequentialTry Boots for %i\n", radio->id());
#endif
	}
	if (neigh) {
		timer->template set_timer<self_type, &self_type::timer_callback> (
				delta(), this, const_cast<uint8_t *> (&pong_task));
		radio->set_power(TxPower::MAX);
		pong_msg.set_neighbor_number(0);
		replied.clear();
		power_ = TxPower::MIN;
		enabled = true;
		wait = false;
		waiting = false;
		timer->template set_timer<self_type, &self_type::timer_callback> (
				delta(), this, const_cast<uint8_t *> (&ping_task));
#ifdef DEBUG
		debug->debug("Starting SequentialTry\n");
#endif
	}
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
void SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::disable() {
	enabled = false;
#ifdef DEBUG
	debug->debug("SequentialTry disabled\n");
#endif
}

template<class OsModel_P, class Radio_P, class Neigh_P, class Timer_P>
SequentialTry<OsModel_P, Radio_P, Neigh_P, Timer_P>::SequentialTry() :
	radio(0), timer(0), neigh(0), delta_(delta_def), enabled(false),
			initialized(false), wait(false), waiting(false) {
	pong_msg.set_msg_id(SequentialTryPongMsgId);
}

}

#endif /* ALGORITHMS_TOPOLOGY_ADJUST_POWER_SEQUENTIAL_TRY_H_ */
