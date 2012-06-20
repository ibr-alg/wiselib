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
#ifndef __ALGORITHMS_6LOWPAN_SIMPLE_ROUTING_H__
#define __ALGORITHMS_6LOWPAN_SIMPLE_ROUTING_H__


namespace wiselib
{
	template<typename OsModel_P,
		typename Radio_P,
		typename Radio_Link_Layer_P,
		typename Debug_P,
		typename Timer_P>
	class SimpleQueryableRouting
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef Radio_Link_Layer_P Radio_Link_Layer;
		typedef typename Radio::node_id_t node_id_t;
		
		typedef SimpleQueryableRouting<OsModel, Radio, Radio_Link_Layer, Debug, Timer> self_type;

		
		// -----------------------------------------------------------------
		SimpleQueryableRouting()
			{
				is_working = false;
			}
			
		void init( Timer& timer )
		{
			timer_ = &timer;
		}

		// -----------------------------------------------------------------

		void find( node_id_t ip_destination, Radio* ipv6 )
		{
			callback_radio_ = ipv6;
			requested_destination_ = ip_destination;
			is_working = true;
			timer().template set_timer<self_type, &self_type::call_it_back>( 2000, this, 0);
		}
		// --------------------------------------------------------------------
		void call_it_back( void* )
		{
			is_working = false;
			callback_radio_->route_estabilished( requested_destination_, requested_destination_ );
		}
		
		bool is_working;
		
	 private:
	 	typename Timer::self_pointer_t timer_;
		node_id_t requested_destination_;
		Radio* callback_radio_;
		
		Timer& timer()
		{
			return *timer_;
		}
		
	};

}
#endif
