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
#include "tinyos_wiselib_glue.h"


typedef nx_struct wiselib_radio_msg
{
   nx_uint8_t data[TOSH_DATA_LENGTH];
} wiselib_radio_msg_t;
// --------------------------------------------------------------------------
module TinyosWiselibGlueRadioP
{
   uses
   {
      interface Packet;
      interface AMPacket;
      interface AMSend;
      interface Receive;
      interface SplitControl as AMControl;
#ifdef __CC2420_H__
      interface CC2420Packet;
#elif defined(TOSSIM)
      interface TossimPacket;
#elif defined(PLATFORM_GNODE)
	  interface PacketMetadata;
#endif
   }
}
// --------------------------------------------------------------------------
implementation
{
   message_t packet;
   bool busy = FALSE;
   // -----------------------------------------------------------------------
   void tinyos_init_radio_module() @C() @spontaneous()
   {
      call AMControl.start();
   }
   // -----------------------------------------------------------------------
   uint16_t tinyos_get_nodeid() @C() @spontaneous()
   {
      return TOS_NODE_ID;
   }
   // -----------------------------------------------------------------------
   event void AMControl.startDone(error_t err)
   {
      if (err == SUCCESS) {
         dbg("AMControl", "AM Start Done for %i\n", TOS_NODE_ID);
      }
      else {
         call AMControl.start();
      }
   }
   // -----------------------------------------------------------------------
   event void AMControl.stopDone(error_t err)
   {
      dbg("AMControl", "AM STOP Done for %i\n", TOS_NODE_ID);
   }
   // -----------------------------------------------------------------------
   int tinyos_send_message( uint16_t id, uint8_t len, const uint8_t *data ) @C() @spontaneous()
   {
      if (busy)
      {
         dbg("AMControl", "Cannot send - still busy!\n");
         return TINYOS_RADIO_BUSY;
      }

      {
         wiselib_radio_msg_t *wrm =
            (wiselib_radio_msg_t*)call Packet.getPayload(&packet, sizeof(wiselib_radio_msg_t));
         if (wrm == NULL)
         {
            dbg("AMControl", "Packet.getPayload FAILED\n");
            return TINYOS_RADIO_ERR_UNSPEC;
         }

         if ( len > TOSH_DATA_LENGTH - 1 )
         {
            dbg("AMControl", "Packet too large! Send FAILED.\n");
            return TINYOS_RADIO_PACKET_TOO_LARGE;
         }
         memcpy( wrm->data, data, len );
      }

      if (call AMSend.send(id, &packet, len) == SUCCESS)
      {
         dbg("AMControl", "Sent %u bytes at %u!\n", len, TOS_NODE_ID);
         busy = TRUE;
      }
      else
      {
         dbg("AMControl", "call to send FAILED!\n");
         return TINYOS_RADIO_ERR_UNSPEC;
      }

      return TINYOS_RADIO_SUCCESS;
   }
   // -----------------------------------------------------------------------
   event void AMSend.sendDone(message_t* msg, error_t err)
   {
      if (&packet == msg)
      {
         dbg("AMControl", "Send Done at %u\n", TOS_NODE_ID);
         busy = FALSE;
      }
   }
   // -----------------------------------------------------------------------
#ifdef __CC2420_H__
   uint8_t getLinkQuality(message_t *msg)
   {
      return (uint8_t) call CC2420Packet.getLqi(msg);
   }
#elif defined(TOSSIM)
   uint8_t getLinkQuality(message_t *msg)
   {
      return (uint8_t) call TossimPacket.strength(msg);
   }
#elif defined(PLATFORM_GNODE)
	uint8_t getLinkQuality(message_t *msg)
	{
		return (uint8_t) call PacketMetadata.getLqi(msg);
	}
#else
  #error Radio chip not supported! Currently link quality works only \
         for motes with CC2420 and TOSSIM.
#endif
   // -----------------------------------------------------------------------
   event message_t* Receive.receive(message_t* msg, void* payload, uint8_t len)
   {
      am_addr_t addr = call AMPacket.source(msg);
      dbg("AMControl", "Received Message from %u with size %u\n", addr, len );

      tinyos_external_receive( addr, len, payload, getLinkQuality(msg) );

      return msg;
   }

}
