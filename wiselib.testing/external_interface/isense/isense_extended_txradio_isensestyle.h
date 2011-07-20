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
#ifndef CONNECTOR_ISENSE_EXTENDEDISENSESTYLETXRADIOMODEL_H
#define CONNECTOR_ISENSE_EXTENDEDISENSESTYLETXRADIOMODEL_H

#include "external_interface/isense/isense_types.h"
#include "util/delegates/delegate.hpp"
#include <isense/os.h>
#include <isense/radio.h>
#include <isense/hardware_radio.h>
#include <isense/dispatcher.h>
#include <isense/time.h>

namespace wiselib
{

   template<typename OsModel_P>
   class iSenseExtendediSenseStyleTxRadioModel
   : public isense::Receiver
   {
   public:
      class ExtendedData
      {
         public:
            ExtendedData(){}

            uint16 get_rssi() const
			{ return rssi_; };

			void set_rssi( uint16 rssi )
			{ rssi_ = rssi; };

			uint16 get_lqi() const
			{ return lqi_; };

			void set_lqi( uint16 lqi )
			{ lqi_ = lqi; };

		 private:
			uint16 rssi_;
			uint16 lqi_;
      };
      // --------------------------------------------------------------------
      typedef OsModel_P OsModel;

      typedef iSenseExtendediSenseStyleTxRadioModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

#ifdef ISENSE_RADIO_ADDR_TYPE
      typedef ISENSE_RADIO_ADDR_TYPE node_id_t;
#else
      typedef uint16 node_id_t;
#endif
      typedef uint8  block_data_t;
      typedef uint8  size_t;
      typedef uint8  message_id_t;

      typedef delegate3<void, node_id_t, size_t, block_data_t*> isense_radio_delegate_t;
      typedef delegate5<void, node_id_t, size_t, block_data_t*,uint8,uint8> extended_radio_delegate_iss_t;
      typedef isense_radio_delegate_t radio_delegate_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };
      // --------------------------------------------------------------------
      enum { MAX_INTERNAL_RECEIVERS = 10 };
      enum { MAX_EXTENDED_RECEIVERS = MAX_INTERNAL_RECEIVERS };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = 0xffff, ///< All nodes in communication range
         NULL_NODE_ID      = 0       ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = 116    ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      class TxPower;
      // --------------------------------------------------------------------
      iSenseExtendediSenseStyleTxRadioModel( isense::Os& os )
         : os_(os)
      {
         os_.dispatcher().add_receiver( this );
      }
      // --------------------------------------------------------------------
      int
      send( node_id_t id, size_t len, block_data_t *data )
      {
         os().radio().send( id, len, data, 0, 0 );
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int enable_radio()
      {
         os().radio().enable();
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int disable_radio()
      {
         os().radio().disable();
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      node_id_t id()
      {
         return os().id();
      }
      // --------------------------------------------------------------------
      //---------- From concept VariablePowerRadio ------------
      int set_power(TxPower p);
      //-------------------------------------------------------
      TxPower power();
      //-------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
      int reg_recv_callback( T *obj_pnt )
      {
         for ( int i = 0; i < MAX_INTERNAL_RECEIVERS; i++ )
         {
            if ( !isense_radio_callbacks_[i] )
            {
               isense_radio_callbacks_[i] =
                  isense_radio_delegate_t::template from_method<T, TMethod>( obj_pnt );
               return i;
            }
         }
         return -1;
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*,uint8,uint8)>
      int reg_recv_callback( T *obj_pnt )
      {
         for ( int i = 0; i < MAX_EXTENDED_RECEIVERS; i++ )
         {
            if ( !isense_ext_radio_callbacks_[i] )
            {
               isense_ext_radio_callbacks_[i]=
            	 extended_radio_delegate_iss_t::template from_method<T, TMethod>( obj_pnt );
               return i;
            }
         }
         return -1;
      }
      // --------------------------------------------------------------------
      int unreg_recv_callback( int idx )
      {
         return ERR_NOTIMPL;
      }
      // --------------------------------------------------------------------
#ifdef ISENSE_RADIO_ADDR_TYPE
      virtual void receive( uint8 len, const uint8 *buf,
         ISENSE_RADIO_ADDR_TYPE src_addr, ISENSE_RADIO_ADDR_TYPE dest_addr,
         uint16 signal_strength, uint16 signal_quality, uint8 sequence_no,
         uint8 interface, isense::Time rx_time )
#else
      virtual void receive( uint8 len, const uint8 *buf,
                              uint16 src_addr, uint16 dest_addr,
                              uint16 lqi, uint8 seq_no, uint8 interface )
#endif
      {
         for ( int i = 0; i < MAX_INTERNAL_RECEIVERS; i++ )
         {
            if ( isense_radio_callbacks_[i] )
               isense_radio_callbacks_[i]( src_addr, len, const_cast<uint8*>(buf) );
         }

         for ( int i = 0; i < MAX_EXTENDED_RECEIVERS; i++ )
         {
            if ( isense_ext_radio_callbacks_[i] )
               isense_ext_radio_callbacks_[i]( src_addr, len, const_cast<uint8*>(buf), (uint8)signal_strength, (uint8)signal_quality );
         }
      }

   private:
      isense::Os& os()
      { return os_; }
      // --------------------------------------------------------------------
      isense::Os& os_;
      isense_radio_delegate_t isense_radio_callbacks_[MAX_INTERNAL_RECEIVERS];
      extended_radio_delegate_iss_t isense_ext_radio_callbacks_[MAX_EXTENDED_RECEIVERS];
   };
   // --------------------------------------------------------------------
   /** \brief iSense Implementation of \ref txpower_concept "TxPower Concept"
   *  \ingroup txpower_concept
   *
   *  iSense implementation of the \ref txpower_concept "TxPower Concept" ...
   */
   template<typename OsModel_P>
   class iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower {
   public:
	   TxPower();
		TxPower(TxPower const &);

	   TxPower &operator=(TxPower const &);
	   bool operator==(TxPower) const;
	   bool operator!=(TxPower) const;
	   bool operator<=(TxPower) const;
	   bool operator>=(TxPower) const;
	   bool operator<(TxPower) const;
	   bool operator>(TxPower) const;
	   TxPower operator++();
	   TxPower operator++(int);
	   TxPower operator--();
	   TxPower operator--(int);

	   static TxPower from_ratio(int);
	   void set_ratio(int);
	   int to_ratio() const;
	   static TxPower from_dB(int);
	   void set_dB(int);
	   int to_dB() const;

	   static TxPower const MIN;
	   static TxPower const MAX;

   private:
	   TxPower(int);

	   int value;

	   friend class iSenseExtendediSenseStyleTxRadioModel<OsModel_P>;
   };

   template<typename OsModel_P>
   typename  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower const  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::MIN=-30;

   template<typename OsModel_P>
   typename  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower const  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::MAX=0;

   template<typename OsModel_P>
   inline  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::TxPower(int v):value(v){}

   template<typename OsModel_P>
   inline  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::TxPower():value(0){}

   template<typename OsModel_P>
   inline typename iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower &iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator=(TxPower const &p) {
	   value=p.value;
	   return *this;
   }

   template<typename OsModel_P>
   inline iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::TxPower(TxPower const &power):value(power.value){}

   template<typename OsModel_P>
   inline bool  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator==(TxPower p) const {
	   return value==p.value;
   }

   template<typename OsModel_P>
   inline bool  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator!=(TxPower p) const {
	   return value!=p.value;
   }

   template<typename OsModel_P>
   inline bool  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator<=(TxPower p) const {
	   return value<=p.value;
   }

   template<typename OsModel_P>
   inline bool  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator>=(TxPower p) const {
	   return value>=p.value;
   }

   template<typename OsModel_P>
   inline bool  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator<(TxPower p) const {
	   return value<p.value;
   }

   template<typename OsModel_P>
   inline bool  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator>(TxPower p) const {
	   return value>p.value;
   }

   template<typename OsModel_P>
   typename  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator++(){
	   value+=6;
	   return *this;
   }

   template<typename OsModel_P>
   typename  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator++(int){
	   TxPower p=*this;
	   value+=6;
	   return p;
   }

   template<typename OsModel_P>
   typename  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator--(){
	   value-=6;
	   return *this;
   }

   template<typename OsModel_P>
   typename  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::operator--(int){
	   TxPower p=*this;
	   value-=6;
	   return p;
   }

   template<typename OsModel_P>
   typename  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::from_ratio(int ratio){
	   if(ratio<=1)
		   return MIN;
	   else if(ratio<=4)
		   return TxPower(-24);
	   else if(ratio<=16)
		   return TxPower(-18);
	   else if(ratio<=63)
		   return TxPower(-12);
	   else if(ratio<=251)
		   return TxPower(-6);
	   else
		   return MAX;
   }

   template<typename OsModel_P>
   void  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::set_ratio(int ratio){
	   if(ratio<=1)
		   value=-30;
	   else if(ratio<=4)
		   value=-24;
	   else if(ratio<=16)
		   value=-18;
	   else if(ratio<=63)
		   value=-12;
	   else if(ratio<=251)
		   value=-6;
	   else
		   value=0;
   }

   template<typename OsModel_P>
   int  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::to_ratio() const {
	   switch(value){
	   case 0:
		   return 1000;
	   case -6:
		   return 251;
	   case -12:
		   return 63;
	   case -18:
		   return 16;
	   case -24:
		   return 4;
	   default:
		   return 1;
	   }
   }

   template<typename OsModel_P>
   typename  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::from_dB(int db){
	   if(db<=-30)
		   return MIN;
	   else if(db<=-24)
		   return TxPower(-24);
	   else if(db<=-18)
		   return TxPower(-18);
	   else if(db<=-12)
		   return TxPower(-12);
	   else if(db<=-6)
		   return TxPower(-6);
	   else
		   return MAX;
   }

   template<typename OsModel_P>
   void  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::set_dB(int db){
	   if(db<=-30)
		   value=-30;
	   else if(db<=-24)
		   value=-24;
	   else if(db<=-18)
		   value=-18;
	   else if(db<=-12)
		   value=-12;
	   else if(db<=-6)
		   value=-6;
	   else
		   value=0;
//Another way: value=-(((-db)/6)*6);
   }

   template<typename OsModel_P>
   inline int  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower::to_dB() const {
	   return value;
   }
   // --------------------------------------------------------------------
   template<typename OsModel_P>
   int  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::set_power(TxPower p){
      os().radio().hardware_radio().set_power(p.value);
      return SUCCESS;
   }

   template<typename OsModel_P>
   typename  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::TxPower  iSenseExtendediSenseStyleTxRadioModel<OsModel_P>::power(){
 	  return TxPower(os().radio().hardware_radio().power());
   }
   //-------------------------------------------------------
}

#endif
