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

enum
{
  AM_WISELIB_GLUE = 6
};

configuration TinyosWiselibGlueRadioC
{}

implementation
{
   components TinyosWiselibGlueRadioP as App;
   components ActiveMessageC;
   components new AMSenderC(AM_WISELIB_GLUE);
   components new AMReceiverC(AM_WISELIB_GLUE);
#ifdef __CC2420_H__
  components CC2420ActiveMessageC;
  App -> CC2420ActiveMessageC.CC2420Packet;
#elif defined(TOSSIM)
  components TossimActiveMessageC;
  App.TossimPacket -> TossimActiveMessageC;
#elif defined(PLATFORM_GNODE)
  components PacketMetadataC;
  App.PacketMetadata -> PacketMetadataC;
#elif defined(PLATFORM_INGA)
  components ReadLqiC;
  App.ReadLqi -> ReadLqiC;
#endif

   App.Packet -> AMSenderC;
   App.AMPacket -> AMSenderC;
   App.AMControl -> ActiveMessageC;
   App.AMSend -> AMSenderC;
   App.Receive -> AMReceiverC;
}
