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

#ifndef COAP_SERVICE_H_
#define COAP_SERVICE_H_

#include "util.h"

#define DEBUG_COAP_SERVICE

#ifdef DEBUG_COAP_SERVICE
#define DBG_COAP_S(...) debug_->debug( __VA_ARGS__)
#else
#define DBG_COAP_S(X)
#endif

namespace wiselib
{

template<typename Os_P, typename CoapRadio_P, typename String_T, typename Value_P>
class CoapService
{
public:

	typedef Os_P Os;
	typedef String_T string_t;
	typedef CoapRadio_P Radio;
	typedef Value_P value_t;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef typename Radio::coap_packet_t coap_packet_t;
	typedef typename Os::Debug Debug;
	typedef delegate1<void, coap_message_t&> coapreceiver_delegate_t;
	typedef CoapService<Os, Radio, string_t, value_t> self_type;
	typedef delegate1<void, value_t> status_listener_delegate_t;
	typedef vector_static<Os, status_listener_delegate_t, COAP_MAX_OBSERVERS> status_listener_vector_t;
	typedef typename status_listener_vector_t::iterator listener_iterator_t;

	~CoapService() { }

	CoapService(string_t path, Radio& radio) :
		path_(path),
		radio_(&radio),
		handle_subresources_(false),
		request_callback_(coapreceiver_delegate_t()),
		radio_reg_id_(0)
	{

	}

	void register_at_radio() {
		radio_reg_id_ = radio_->template reg_resource_callback<self_type, &self_type::handle_request >( path_, this );
	}

	void handle_request(coap_message_t &msg) {

		coap_packet_t & packet = msg.message();

		int path_compare = path_cmp<string_t>( path_, packet.uri_path() );
		if( (handle_subresources_ || path_compare == EQUAL) && request_callback_ && request_callback_.obj_ptr() != NULL )
		{
			request_callback_(msg);
		}
	}

	inline string_t path()
	{
		return path_;
	}

	inline Radio* radio()
	{
		return radio_;
	}

	value_t status()
	{
		return status_;
	}

	void set_status(value_t newStatus)
	{
		status_ = newStatus;
		for (listener_iterator_t it = status_listeners_.begin(); it != status_listeners_.end(); it++)
		{
			(*it)(status_);
		}
	}

	inline bool handles_subresources()
	{
		return handle_subresources_;
	}

	void set_handle_subresources(bool handle_subresources)
	{
		handle_subresources_ = handle_subresources;
	}

	template < class T, void (T::*TMethod)( typename self_type::value_t value) >
	bool add_status_listener( T *callback)
	{


		if (status_listeners_.max_size() == status_listeners_.size())
		{
			DBG_COAP_S("COAP_SERVICE: MAX STATUS LISTENERS REACHED. WON'T ADD");
			return false;
		}
		else
		{
			for (listener_iterator_t it = status_listeners_.begin();
					it != status_listeners_.end(); it++)
			{
				// look if already registered
				if (*it && (*it).obj_ptr() == callback )
				{
					return true;
				}
			}
			status_listener_delegate_t delegate = status_listener_delegate_t::template from_method<T, TMethod>( callback );
			status_listeners_.push_back(delegate);
			DBG_COAP_S("COAP_SERVICE: Added status listener.");
			return true;
		}
	}

	void shutdown()
	{
		radio_->unreg_resource_callback(radio_reg_id_);
	}

	template < class T, void (T::*TMethod)( typename self_type::coap_message_t & ) >
	void set_request_callback( T *callback )
	{
		request_callback_ = coapreceiver_delegate_t::template from_method<T, TMethod>( callback );
	}

private:
	Radio *radio_;
	Debug *debug_;
	value_t status_;
	string_t path_;
	int radio_reg_id_;
	bool handle_subresources_;						// if set, parent also receives all subresource requests
	coapreceiver_delegate_t request_callback_;
	status_listener_vector_t status_listeners_;

};

}

#endif /* COAP_SERVICE_H_ */
