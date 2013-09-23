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

#ifndef STATES_SERVICE_H_
#define STATES_SERVICE_H_

#include "radio/coap/coap_service.h"
#include "radio/coap/coap_high_level_states.h"

#define DEBUG_HLS

#ifdef DEBUG_HLS
#define DBG_HLS(...) debug_->debug( __VA_ARGS__)
#else
#define DBG_HLS(...)
#endif

namespace wiselib
{

/**
 * This namespace is just to circumvent c++ templates' lack to specialize funtions in class scope.
 * C++03 Standard says in 14.7.3 p18: "In an explicit specialization declaration for a member of a
 * class template or a member template that appears in namespace scope, the member template and some
 * of its enclosing class templates may remain unspecialized, except that the declaration shall not
 * explicitly specialize a class member template if its enclosing class templates are not explicitly
 * specialized as well."
 */
namespace specialized
{
	// TODO hand over debug_ object
	template<typename T>
	number_state<T> create_state_from_option(OpaqueData hl_data)
	{
		return NULL;
	}
	template<>
	number_state<const char*> create_state_from_option(OpaqueData hl_data)
	{
		uint8_t * option = hl_data.value();
		uint8_t type = option[0];

		if ( type != HighLevelCreationType::STRING )
		{
			//DBG_HLS("HLS: Error! Only STRING states supported!\n");
			number_state<const char*> undefined = {"","undefined"};
			return undefined;
		}

		// TODO how to parse string without length? Possible error in specification!
		char* value = (char*) option+1;
		char* value_copy = new char[128];
		memcpy(value_copy, value, 128);

		char* name = (char*) option+129;
		char* name_copy = new char[128];
		memcpy(name_copy, name, 128);

		//DBG_HLS("Created integer state:\"%s\" lower: %d upper: %d \n", name, lower, upper);
		number_state<const char*> state = {value_copy,name_copy};
		return state;
	}
	// --------------------------------------------------------------------------
	template<>
	number_state<uint16_t> create_state_from_option(OpaqueData hl_data)
	{
		uint8_t * option = hl_data.value();
		uint8_t type = option[0];

		if ( type != HighLevelCreationType::INTEGER )
		{
			//DBG_HLS("HLS: Error! Only INTEGER states supported!\n");
			number_state<uint16_t> undefined = {0,0,"undefined"};
			return undefined;
		}

		uint16_t lower = 0 | (option[1]<<8) | option[2];
		uint16_t upper = 0 | (option[3]<<8) | option[4];

		char* name = (char*) option+5;
		char* name_copy = new char[hl_data.length()-5+1];
		memcpy(name_copy, name, hl_data.length()-5);

		//DBG_HLS("Created integer state:\"%s\" lower: %d upper: %d \n", name, lower, upper);
		number_state<uint16_t> state = {lower,upper,name_copy};
		return state;
	}
	// --------------------------------------------------------------------------
	template<>
	number_state<float> create_state_from_option(OpaqueData hl_data)
	{
		uint8_t * option = hl_data.value();
		uint8_t type = option[0];

		if ( type != HighLevelCreationType::FLOAT )
		{
			//DBG_HLS("HLS: Error! Only FLOAT states supported!\n");
			number_state<float> undefined = {0,0,"undefined"};
			return undefined;
		}

		// FIXME implement proper float representation
		float lower = (float) (0 | (option[1]<<8) | option[2]);
		float upper = (float) (0 | (option[3]<<8) | option[4]);
		char* name = (char*) option+5;
		char* name_copy = new char[hl_data.length()-5+1];
		memcpy(name_copy, name, hl_data.length()-5);

		//DBG_HLS("Created float state:\"%s\" lower: %f upper: %f \n",name, lower, upper);
		number_state<float> state = {lower,upper,name_copy};
		return state;
	}
}


template<typename Os_P, typename CoapRadio_P, typename String_T, typename Value_T>
class HLStatesService
{
public:

	typedef CoapRadio_P Radio;
	typedef Os_P Os;
	typedef String_T string_t;
	typedef Value_T value_t;
	typedef typename Os::size_t size_t;
	typedef typename Os::Debug Debug;
	typedef typename Os::Timer Timer;
	typedef typename Os::Rand Rand;
	typedef HLStatesService<Os, Radio, string_t, value_t> self_type;
	typedef CoapService<Os, Radio, string_t, value_t> coap_service_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::ReceivedMessage coap_message_t;
	typedef typename Radio::coap_packet_t coap_packet_t;
	typedef delegate1<void, coap_message_t&> coapreceiver_delegate_t;

	typedef number_state_resource<Os, value_t> state_resource_t;
	typedef vector_static<Os, state_resource_t, COAP_MAX_STATE_RESOURCES> state_resources_vector_t;
	typedef vector_static<Os, coap_service_t*, COAP_MAX_STATE_RESOURCES> state_resources_services_vector_t;

	// --------------------------------------------------------------------------
	HLStatesService(coap_service_t& service, value_t start_value = value_t()) :
		num_(start_value),
		status_(num_),
		service_(&service),
		radio_reg_id_(-1)
	{
		rand_->srand( service_->radio()->id() );
	}
	// --------------------------------------------------------------------------
	void register_at_radio()
	{
		radio_reg_id_ = service_->radio()->template reg_resource_callback<self_type, &self_type::handle_request >( service_->path(), this );
	}
	// --------------------------------------------------------------------------
	void shutdown()
	{
		if ( radio_reg_id_ != -1 )
			service_->radio().unreg_resource_callback( radio_reg_id_ );
	}
	// --------------------------------------------------------------------------
	void dummy(coap_message_t& msg)
	{}
	// --------------------------------------------------------------------------
	void handle_request(coap_message_t& msg)
	{
		coap_packet_t & packet = msg.message();
		OpaqueData hl_data;

		if ( packet.get_option(COAP_OPT_HL_STATE, hl_data) == coap_packet_t::SUCCESS )
		{
			switch ( packet.code() ) {
				case COAP_CODE_GET:
					handle_get_request(msg);
					break;
				case COAP_CODE_POST:
					handle_post_request(msg);
					break;
				case COAP_CODE_PUT:
					service_->radio()->reply( msg, (uint8_t*) 0, 0, COAP_CODE_METHOD_NOT_ALLOWED );
					break;
				case COAP_CODE_DELETE:
					handle_delete_request(msg);
					break;
				default:
					break;
			}
		} else {
			request_callback_(msg);
		}
	}
	// --------------------------------------------------------------------------
	void handle_delete_request(coap_message_t& msg)
	{
		coap_packet_t & packet = msg.message();
		OpaqueData hl_data;

		if (	packet.uri_path() != service_->path() &&
				packet.get_option(COAP_OPT_HL_STATE, hl_data) == coap_packet_t::SUCCESS )
		{
			state_resource_t* resource;
			resource = find_state_resource( packet.uri_path() );
			if ( resource != NULL )
			{
				DBG_HLS("DELETE Request\n");
				service_->radio()->reply( msg, (uint8_t*) 0, 0, COAP_CODE_DELETED );
			}
		}
	}
	// --------------------------------------------------------------------------
	void handle_get_request(coap_message_t& msg)
	{
		coap_packet_t & packet = msg.message();
		OpaqueData hl_data;

		if ( packet.get_option(COAP_OPT_HL_STATE, hl_data) == coap_packet_t::SUCCESS )
		{

			state_resource_t* resource;
			if ( packet.uri_path() != service_->path() )
			{
				resource = find_state_resource( packet.uri_path() );
			}

			uint8_t type;
			size_t length;
			hl_data.get(&type, length);

			switch (type) {
				case HighLevelQueryType::STATE:
					if ( resource != NULL)
					{
						const char* curr_state = resource->get_state( service_->status() );
						service_->radio()->reply( msg, (uint8_t*) curr_state, strlen(curr_state) );
					}
					break;
				case HighLevelQueryType::STATE_NUMBER:
					// TODO state number
					DBG_HLS("Aksing for Number");
					break;
				case HighLevelQueryType::STATE_DESCRIPTION:
					char* json;
					if ( packet.uri_path() == service_->path() )
					{
						json = resources_to_json();
					}
					else
					{
						if ( resource == NULL )
						{
							// no such resource found
							char * error_description = NULL;
							int len = 0;
							char error_description_str[COAP_ERROR_STRING_LEN];
							len = sprintf(error_description_str, "Resource \"%s\" not found.", packet.uri_path().c_str() );
							DBG_HLS("Couldn't find %s \n", packet.uri_path().c_str());
							error_description = error_description_str;
							CoapContentType ctype = COAP_CONTENT_TYPE_TEXT_PLAIN;
							service_->radio()->reply( msg, (uint8_t*) error_description, len, COAP_CODE_NOT_FOUND, ctype );

							return;
						}
						else
						{
							json = resource->to_json();
						}

					}
					DBG_HLS("HLS: Message= %s\n",json);
					// TODO Content Type
					service_->radio()->reply( msg, (uint8_t*) json, strlen(json) );

					break;
				default:
					// Query Type not supported -> send Bad_Option
					service_->radio()->reply( msg, (uint8_t*) 0, 0, COAP_CODE_BAD_OPTION );
					break;
			}
		}
	}
	// --------------------------------------------------------------------------
	/**
	 * \brief Handles POST requests with High-Level-States options. Registers a new sub-resource
	 *  under a random 5 char URI.
	 */
	void handle_post_request(coap_message_t& msg)
	{
		coap_packet_t & packet = msg.message();
		list_static<Os, OpaqueData, COAP_MAX_HL_STATES> hl_list;

		if ( packet.template get_options< list_static<Os, OpaqueData, COAP_MAX_HL_STATES> > (COAP_OPT_HL_STATE, hl_list) == coap_packet_t::SUCCESS )
		{

			typename list_static<Os, OpaqueData, COAP_MAX_HL_STATES>::iterator it = hl_list.begin();

			number_state_resource<Os, value_t> hls_res;
			hls_res.type = HighLevelCreationType::INTEGER;
			hls_res.path = gen_random(5); // TODO make sure this is unique
			string_t full_path = service_->path().append("/").append(hls_res.path);
			int radio_reg_id_ = service_->radio()->template reg_resource_callback<self_type, &self_type::dummy >( full_path, this );
			DBG_HLS("Creating new High Level State Resource at path: %s \n", full_path.c_str());

			for(; it != hl_list.end(); ++it)
			{
				OpaqueData hl_data = (*it);
				uint8_t * option = hl_data.value();
				uint8_t type = option[0];

				number_state<value_t> state = create_state_from_option(*it);
				if ( strcmp(state.name, "undefined") )
				{
					// Types don't match...send 4.02
					service_->radio()->reply( msg, (uint8_t*) 0, 0, COAP_CODE_BAD_OPTION );
					return;
				}
				if ( !already_mapped(state) )
				{
					hls_res.states.push_back(state);
				}
			}

			state_resources_.push_back(hls_res);

			coap_packet_t repl;
			repl.set_option(COAP_OPT_LOCATION_PATH, hls_res.path);
			service_->radio()->reply( msg, 0, 0, COAP_CODE_CREATED, COAP_CONTENT_TYPE_APPLICATION_JSON, repl );
		}
	}
	// --------------------------------------------------------------------------
	/**
	 * \brief Converts all State-Resources to a description containing paths and mappings in JSON
	 * 			format
	 * @return a char array (max length 1024bytes) containing the JSON
	 */
	char* resources_to_json()
	{
		string_t delim = ",";
		string_t res = "{res:{r:[";
		string_t footer = "]}}";
		char* buffer = new char[1024];

		strcpy(buffer, res.c_str());

		for ( size_t i=0; i<state_resources_.size(); i++)
		{
			string_t json = string_t(state_resources_.at(i).to_json());
			strcat(buffer, json.c_str());

			if ( i < (state_resources_.size()-1) )
			{
				strcat(buffer, delim.c_str());
			}
		}
		strcat(buffer, footer.c_str());

		return buffer;
	}
	// --------------------------------------------------------------------------
	void set_status(value_t newStatus)
	{
		status_ = newStatus;
	}
	// --------------------------------------------------------------------------
	template <class T, void (T::*TMethod)( typename self_type::coap_message_t & ) >
	void set_request_callback( T *callback )
	{
		request_callback_ = coapreceiver_delegate_t::template from_method<T, TMethod>( callback );
	}

private:
	Rand *rand_;
	Debug *debug_;
	coap_service_t *service_;
	value_t num_;
	value_t status_;
	int radio_reg_id_;
	coapreceiver_delegate_t request_callback_;
	state_resources_vector_t state_resources_;
	state_resources_services_vector_t state_resources_services_;

	// --------------------------------------------------------------------------
	/**
	 * \brief Find a State-Resource in the list of all registered State-Resources by a given URI-path
	 * @param uri_path
	 * @return A pointer to the found state resource or NULL if none
	 */
	state_resource_t* find_state_resource(string_t uri_path)
	{
		for ( size_t i=0; i<state_resources_.size(); i++)
		{
			state_resource_t curr = state_resources_.at(i);
			string_t full_path = string_t( service_->path() ).append("/").append(curr.path);
			if ( full_path == uri_path )
			{
				return &(state_resources_.at(i));
			}
		}
		return NULL;
	}
	// --------------------------------------------------------------------------
	/**
	 * \brief Generates a random alphanumeric string with a given length.
	 * @param len length of random string
	 * @return Null terminated char array containing the generated string
	 */
	char * gen_random(const size_t len)
	{
		char* s = new char[len+1];
		static const char alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		for (size_t i = 0; i < len; ++i)
		{
			uint32_t ran = (*rand_)( sizeof(alphanum)-1 );
			s[i] = (char) alphanum[ran];
		}

		s[len] = 0;
		return s;
	}
	// --------------------------------------------------------------------------
	number_state<value_t> create_state_from_option(OpaqueData hl_data)
	{
		return specialized::create_state_from_option<value_t>(hl_data);
	}
	// --------------------------------------------------------------------------
	/**
	 * \brief Checks if a value from a given state is already mapped by another state in the list
	 */
	bool already_mapped(number_state<value_t> state)
	{
		// TODO check if parts of this state are already mapped by another in the list
		return false;
	}

};

}

#endif /* STATES_SERVICE_H_ */
