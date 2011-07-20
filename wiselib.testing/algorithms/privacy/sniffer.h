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

#include "util/delegates/delegate.hpp"

namespace wiselib
{
template<	typename Os_P,
			typename Radio_P,
			typename Debug_P,
			typename PrivacyMessage_P
>
	class SnifferType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef PrivacyMessage_P PrivacyMessage;
		typedef SnifferType<Os, Radio, Debug,PrivacyMessage> self_type;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Radio::TxPower TxPower;
		typedef delegate3<void, node_id_t, size_t, block_data_t*> event_notifier_delegate_t;
		struct callback_element{ event_notifier_delegate_t callback; uint16_t callback_id; };
		// -----------------------------------------------------------------------
		void init( Radio& radio, Debug& debug )
		{
			radio_ = &radio;
			debug_ = &debug;
		}
		// -----------------------------------------------------------------------
		SnifferType()
			:radio_callback_id_  	( 0 )
		{}
		// -----------------------------------------------------------------------
		~SnifferType()
		{}
		// -----------------------------------------------------------------------
		void enable( void )
		{	
			radio().enable_radio();
			radio_callback_id_ = radio().template reg_recv_callback<self_type, &self_type::radio_receive>( this );
			
		}
		void disable( void )
		{
			radio().unreg_recv_callback( radio_callback_id_ );
			radio().disable();
		}
		// -----------------------------------------------------------------------
		void radio_receive( node_id_t from, size_t len, block_data_t *data )
		{	
			
			PrivacyMessage *message = ( PrivacyMessage* )data;
			
			debug().debug("Sniffed a  message:\n");
			debug().debug("msg_id: %i\n",message->msg_id());
			debug().debug("Id of the sender :%x\n",message->request_id());
			debug().debug("Payload lenght: %i\n",message->payload_size());
			
			
			
		}

		// -----------------------------------------------------------------------
		
		Radio& radio()
		{
			return *radio_;
		}
		// -----------------------------------------------------------------------
		Debug& debug()
		{
			return *debug_;
		}
		private:
		Radio * radio_;
		Debug * debug_;
		enum MessageIds
		{
			PRIVACY_DECRYPTION_REQUEST_ID = 100,
			PRIVACY_ENCRYPTION_REQUEST_ID = 110,
			PRIVACY_RANDOMIZE_REQUEST_ID = 120,
			PRIVACY_DECRYPTION_REPLY_ID = 130,
			PRIVACY_ENCRYPTION_REPLY_ID = 140,
			PRIVACY_RANDOMIZE_REPLY_ID = 150,
			PRIVACY_UNREGISTER = 160
		};
		uint32_t radio_callback_id_;
		uint32_t uart_callback_id_;

   	};
}

