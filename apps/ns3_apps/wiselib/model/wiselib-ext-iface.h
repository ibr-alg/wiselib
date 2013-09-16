/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Georgia Tech Research Corporation
 * Copyright (c) 2010 Adrian Sai-wah Tam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Dizhi Zhou (dizhi.zhou@gmail.com)
 */

#ifndef WISELIB_EXT_IFACE_H
#define WISELIB_EXT_IFACE_H

#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include <climits>
#include "ns3/simulator.h"
#include "ns3/event-id.h"

#include "ns3/node-container.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/node.h"
#include "ns3/net-device-container.h"
#include "ns3/address.h"
#include "ns3/packet.h"
#include "ns3/event-impl.h"
#include "ns3/type-traits.h"
#include "ns3/make-event.h"
#include "ns3/wifi-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/vector.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-phy.h"

namespace wiselib {

class WiselibExtIface;
class ExtendedDataClass;

class EventImplExt : public ns3::EventImpl
{
public:
   EventImplExt () { }
   virtual ~EventImplExt () { }

   virtual void RecvCallback (uint32_t , size_t, unsigned char*) { };
   virtual void ExtendedDataRecvCallback (uint32_t , size_t, unsigned char*, ExtendedDataClass*) { };
   virtual void ReadCallback (size_t, unsigned char*) { };
};

template <typename MEM, typename OBJ>
EventImplExt * MakeRecvCallback (MEM mem_ptr, OBJ obj)
{
  class EventMemberImpl0 : public EventImplExt
  {
public:
    EventMemberImpl0 (OBJ obj, MEM function)
      : m_obj (obj),
        m_function (function)
    {
    }
    virtual ~EventMemberImpl0 ()
    {
    }
  
    // call user-defined callback
    virtual void RecvCallback (uint32_t from, size_t len, unsigned char* data)
    {
      (ns3::EventMemberImplObjTraits<OBJ>::GetReference (m_obj).*m_function)(from, len, data);
    }

private:
    
    virtual void Notify (void)
    {
    }
    
    OBJ m_obj;
    MEM m_function;
  } *ev = new EventMemberImpl0 (obj, mem_ptr);
  return ev;
}

template <typename MEM, typename OBJ>
EventImplExt * MakeExtendedDataRecvCallback (MEM mem_ptr, OBJ obj)
{
  class EventMemberImpl0 : public EventImplExt
  {
public:
    EventMemberImpl0 (OBJ obj, MEM function)
      : m_obj (obj),
        m_function (function)
    {
    }
    virtual ~EventMemberImpl0 ()
    {
    }
  
    // call user-defined callback
    virtual void ExtendedDataRecvCallback (uint32_t from, size_t len, unsigned char* data, ExtendedDataClass *extData)
    {
      (ns3::EventMemberImplObjTraits<OBJ>::GetReference (m_obj).*m_function)(from, len, data, extData);
    }

private:
    
    virtual void Notify (void)
    {
    }
    
    OBJ m_obj;
    MEM m_function;
  } *ev = new EventMemberImpl0 (obj, mem_ptr);
  return ev;
}


template <typename MEM, typename OBJ>
EventImplExt * MakeReadCallback (MEM mem_ptr, OBJ obj)
{
  // zero argument version
  class EventMemberImpl0 : public EventImplExt
  {
public:
    EventMemberImpl0 (OBJ obj, MEM function)
      : m_obj (obj),
        m_function (function)
    {
    }
    virtual ~EventMemberImpl0 ()
    {
    }
  
    // call use-defined callback
    virtual void ReadCallback (size_t len, unsigned char* data)
    {
      (ns3::EventMemberImplObjTraits<OBJ>::GetReference (m_obj).*m_function)(len, data);
    }

private:
    
    virtual void Notify (void)
    {
    }
    
    OBJ m_obj;
    MEM m_function;
  } *ev = new EventMemberImpl0 (obj, mem_ptr);
  return ev;
}

class ExtendedDataClass 
{
  public:
    ExtendedDataClass () 
      : m_rss (-80)
      {
      }

    virtual ~ExtendedDataClass ()
      {
      }

    void SetRss (double rss);
    double GetRss () const;

  private:
    double m_rss; // the received signal strength (RSS) in dBm      
};

class TxPowerClass
{
  public:
    TxPowerClass () 
      : m_txPowerStart(16.0206),
        m_txPowerEnd (16.0206)
      {
      }

    virtual ~TxPowerClass ()
      {
      }

    void SetTxPowerStart (double dbm);
    void SetTxPowerEnd (double dbm);
    double GetTxPowerStart () const;
    double GetTxPowerEnd () const;
   
    // APIs to Wiselib user
    void set_ratio(int ratio);  // set tx power start and end to the same value by input ratio
    TxPowerClass from_ratio(int); // return TxPowerClass object based on input ratio
    int to_ratio() const;  // return ratio based on the txpower start in this class
    TxPowerClass from_dB(int db); // return TxPowerclass object based on input dbm
    void set_dB(int);   // set txpower start and end to the same value 
    int to_dB() const;  // return txpower start
    
  private:
     TxPowerClass (double pw) 
      : m_txPowerStart(pw),
        m_txPowerEnd (pw)
      {
      }
   
    double DbToRatio (double db) const;
    double RatioToDb (double ratio) const;

  private:

    double m_txPowerStart; // Minimum available transmission level (dbm).
    double m_txPowerEnd;  // Maximum available transmission level (dbm).
};

class WiselibExtIface
{
public:
  
  typedef uint32_t node_id_t;    // we use unsigned because NS-3 does not have node id smaller than zero
  typedef unsigned char block_data_t;
  typedef long size_t;
  typedef uint8_t message_id_t;

  WiselibExtIface ();

  virtual ~WiselibExtIface ();

  void Debug (const char *msg);

  template<typename T, void (T::*TMethod)(void*)>
  bool SetTimeout( uint32_t millis, T *obj_pnt, void *userdata )
    {

      Ns3Time delay = ns3::MilliSeconds(millis);

      // Note:
      //   1, user does not define the typename for Simulator::Schedule. In this case, the compiler will deduct the typename
      //   2, delegate in Wiselib is not used here. In NS-3, Simulator::Schedule provides a similar solution to realize delegate.
      //      more details about delegate in NS-3 can be found in src/core/model/make-event.h
      m_timerFacetEvent = ns3::Simulator::Schedule (delay, TMethod, obj_pnt, userdata);
      // Q: must I cancel the event when it is expired? 

      //(obj_pnt->*TMethod) (userdata); // call member function TMethod now
      return true;
    }

  // register receive callback method for all sink nodes
  template<typename T, void (T::*TMethod)( node_id_t, size_t, block_data_t* )>
  bool RegRecvCallback( T *obj_pnt, node_id_t local ) 
    {
      ns3::Address addr = nodes.Get (local)->GetDevice (0)->GetAddress ();
      std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> >::iterator it = addDevMap.end (); 
      it = addDevMap.find (addr);
      if (it != addDevMap.end ()) 
        {
          // regeister NS-3 callback
          it->second->SetReceiveCallback (ns3::MakeCallback 
                                  (&WiselibExtIface::DoRegRecvCallback, this));

          // store function and member which will be called in NS-3 callback
          EventImplExt *event = MakeRecvCallback(TMethod, obj_pnt);
          m_recvCallbackMap.insert(std::pair<node_id_t,EventImplExt*>(local, event));
        }

      return true;
    }

  bool DoRegRecvCallback (ns3::Ptr<ns3::NetDevice> device, ns3::Ptr<const ns3::Packet> packet,
                          uint16_t protocol, const ns3::Address &from) 
    {
      uint8_t buffer[packet->GetSize ()]; 
      packet->CopyData (buffer, sizeof(buffer)); 
      // call ns3::Node::ReceiveFromDevice method ?
      std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> >::iterator it = addDevMap.end (); 
      it = addDevMap.find (device->GetAddress ());
      if (it != addDevMap.end ()) 
        {
          node_id_t sendId = (addDevMap.find (from))->second->GetNode ()->GetId ();
          node_id_t recvId = it->second->GetNode ()->GetId ();

          std::map <node_id_t, EventImplExt*>::iterator itMap = m_recvCallbackMap.end ();
          itMap = m_recvCallbackMap.find (recvId);       
          if (itMap != m_recvCallbackMap.end ())
            {
              itMap->second->RecvCallback (sendId, sizeof(buffer), buffer);
            }
          else
            std::cout << "Unknow receiver node id " << std::endl;
        }
      else 
        std::cout << "Unknow node id " << std::endl;
      return true;
    }
  
  // extended data radio facet support
  template<typename T, void (T::*TMethod)( node_id_t, size_t, block_data_t*, ExtendedDataClass* )>
  bool RegExtendedDataRecvCallback( T *obj_pnt, node_id_t local ) 
    {
      ns3::Address addr = nodes.Get (local)->GetDevice (0)->GetAddress ();
      std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> >::iterator it = addDevMap.end (); 
      it = addDevMap.find (addr);
      if (it != addDevMap.end ()) 
        {
          // regeister NS-3 callback
          it->second->SetReceiveCallback (ns3::MakeCallback 
                                  (&WiselibExtIface::DoRegExtendedDataRecvCallback, this));

          // store function and member which will be called in NS-3 callback
          EventImplExt *event = MakeExtendedDataRecvCallback(TMethod, obj_pnt);
          m_recvCallbackMap.insert(std::pair<node_id_t,EventImplExt*>(local, event));
        }

      return true;
    }

  bool DoRegExtendedDataRecvCallback (ns3::Ptr<ns3::NetDevice> device, ns3::Ptr<const ns3::Packet> packet,
                          uint16_t protocol, const ns3::Address &from) 
    {
      uint8_t buffer[packet->GetSize ()]; 
      packet->CopyData (buffer, sizeof(buffer)); 
      // call ns3::Node::ReceiveFromDevice method ?
      std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> >::iterator it = addDevMap.end (); 
      it = addDevMap.find (device->GetAddress ());
      if (it != addDevMap.end ()) 
        {
          node_id_t sendId = (addDevMap.find (from))->second->GetNode ()->GetId ();
          node_id_t recvId = it->second->GetNode ()->GetId ();

          std::map <node_id_t, EventImplExt*>::iterator itMap = m_recvCallbackMap.end ();
          itMap = m_recvCallbackMap.find (recvId);       
          if (itMap != m_recvCallbackMap.end ())
            {
              // any other way to calculate rx power in run-time?             
              ExtendedDataClass extData;
              extData.SetRss (-80); // currently, we use fix rss loss model
              itMap->second->ExtendedDataRecvCallback (sendId, sizeof(buffer), buffer, &extData);
            }
          else
            std::cout << "Unknow receiver node id " << std::endl;
        }
      else 
        std::cout << "Unknow node id " << std::endl;

      return true;
    }



  // define default phy (802.11b), mac (adhoc) and wifi helper instance
  void Init ();

  // create 1 node and install phy, mac on it
  // return the node id to user
  node_id_t EnableRadio ();

  // send mac layer packet
  void SendWiselibMessage( node_id_t dest, size_t len, block_data_t *data, node_id_t local );

  // clock facet support
  double GetNs3Time ();

  // position facet support
  double GetPositionX (node_id_t id);
  double GetPositionY (node_id_t id);
  double GetPositionZ (node_id_t id);
  // set position support in position facet
  void SetPosition (double x, double y, double z, node_id_t id);

  // distance facet support
  double GetDistance (node_id_t src, node_id_t dst);

  // Serial communication facet
  template<typename T, void (T::*TMethod)(size_t, block_data_t* )>
  int RegReadCallback( T *obj_pnt )
    {
      EventImplExt *event = MakeReadCallback(TMethod, obj_pnt);
      int index = m_readCallbackIndex; // unique identifer of this read callback  

      // overflow??
      if ( m_readCallbackIndex > INT_MAX || m_readCallbackIndex < INT_MIN)
        return -1;

      m_readCallbackIndex++;
      m_readCallbackMap.insert(std::pair<int, EventImplExt*>(index, event));
      return index;
    }

   void UnregReadCallback (int index);

   void Receive (size_t size, block_data_t* data, int index);

   // tx power radio facet
   void SetTxPower (TxPowerClass &txpower, node_id_t id );
   TxPowerClass GetTxPower (node_id_t id);
   
private:
  typedef ns3::EventId Ns3EventId;
  typedef ns3::Time Ns3Time;

  Ns3EventId m_timerFacetEvent;
  int m_readCallbackIndex;                         // unique identifer of read callbacks

  // wifi parameters
  ns3::NodeContainer nodes;                        // note: we use the node index in index as the node id
  ns3::YansWifiPhyHelper wifiPhy;
  ns3::NqosWifiMacHelper wifiMac;
  ns3::WifiHelper wifi;

  std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> > addDevMap;  // Address --> NetDevice --> Node --> node id
  std::map <node_id_t, EventImplExt*> m_recvCallbackMap; // store the mem and obj of user-defined receive callbacks
  std::map <node_id_t, ns3::Ptr<ns3::Node> > nodeMap; // store the node id and node object in ns3
  std::map <node_id_t, std::pair<double, double> > nodeTxPowerMap; // store the txpower info of the node 
  std::map <int, EventImplExt*> m_readCallbackMap; // store the mem and obj of user-defined read callbacks
};

}

#endif /* WISELIB_EXT_IFACE_H */

