/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 University of New Brunswick and the Wiselib project
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

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <cmath>

#include "wiselib-ext-iface.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/wifi-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/packet.h"
#include "ns3/node-container.h"
#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/wifi-net-device.h"

NS_LOG_COMPONENT_DEFINE ("WiselibExtIface");

namespace wiselib {

WiselibExtIface::WiselibExtIface ()
  : m_readCallbackIndex (0)
{
  Init ();
}

WiselibExtIface::~WiselibExtIface ()
{
}

void
WiselibExtIface::Debug (const char *msg)
{
  LogComponentEnable ("WiselibExtIface", ns3::LOG_DEBUG);
  NS_LOG_DEBUG (msg);
}

void 
WiselibExtIface::SendWiselibMessage( node_id_t dest, size_t len, block_data_t *data, node_id_t local )
{
  uint32_t addr = dest;
  ns3::Ipv4Address ipv4Addr(addr);// although we use ipv4 address to differentiate the packet type here, we send packet only in mac layer

  if (ipv4Addr == ns3::Ipv4Address::GetBroadcast ())
    {
      // broadcast
      ns3::Address addr = nodes.Get (local)->GetDevice (0)->GetAddress ();
      std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> >::iterator it = addDevMap.end (); 
      it = addDevMap.find (addr);
      if (it != addDevMap.end ()) 
        {
          ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet> (data, len);
          it->second->Send (packet, it->second->GetBroadcast (), 0);
        }
    }
  else
    {
      // unicast
      ns3::Address addr = nodes.Get (local)->GetDevice (0)->GetAddress ();
      std::map<ns3::Address, ns3::Ptr<ns3::NetDevice> >::iterator it = addDevMap.end (); 
      it = addDevMap.find (addr);
      if (it != addDevMap.end ()) 
        {
          // find dest node's mac address
          ns3::Address dst = nodes.Get (dest)->GetDevice (0)->GetAddress ();
          ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet> (data, len);
          it->second->Send (packet, dst, 0);
        }

    }
}

WiselibExtIface::node_id_t
WiselibExtIface::EnableRadio ()
{
  ns3::NodeContainer node;
  node.Create (1); 
  nodes.Add (node.Get (0)); 
  ns3::NetDeviceContainer device = wifi.Install (wifiPhy, wifiMac, node);

  ns3::MobilityHelper mobility;
  ns3::Ptr<ns3::ListPositionAllocator> positionAlloc = ns3::CreateObject<ns3::ListPositionAllocator> ();
  positionAlloc->Add (ns3::Vector (0.0, 0.0, 0.0)); // default position
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (node);

  addDevMap.insert (std::pair<ns3::Address, ns3::Ptr<ns3::NetDevice> >
                        (device.Get (0)->GetAddress (), device.Get (0)));      
  
  node_id_t id = nodes.Get (nodes.GetN () - 1)->GetId ();

  std::map <node_id_t, ns3::Ptr<ns3::Node> >::iterator it = nodeMap.end ();
  it = nodeMap.find (id);
  if (it == nodeMap.end ())
    {
      nodeMap.insert(std::pair<node_id_t, ns3::Ptr<ns3::Node> >(id, nodes.Get (nodes.GetN () -1) ) );
    }

  return id;
}


void
WiselibExtIface::Init ()
{
  std::string phyMode ("DsssRate11Mbps");	
  double rss = -80;  // -dBm

  // disable fragmentation for frames below 2200 bytes
  ns3::Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", ns3::StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  ns3::Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", ns3::StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  ns3::Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", 
                      ns3::StringValue (phyMode));

  // The below set of helpers will help us to put together the wifi NICs we want
  wifi.SetStandard (ns3::WIFI_PHY_STANDARD_80211b);

  wifiPhy =  ns3::YansWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", ns3::DoubleValue (0) ); 
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (ns3::YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

  ns3::YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",ns3::DoubleValue (rss));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  wifiMac = ns3::NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",ns3::StringValue (phyMode),
                                "ControlMode",ns3::StringValue (phyMode));

  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
}

double
WiselibExtIface::GetNs3Time ()
{
  return ns3::Simulator::Now ().GetSeconds ();
}

double 
WiselibExtIface::GetPositionX (node_id_t id)
{
  std::map <node_id_t, ns3::Ptr<ns3::Node> >::iterator it = nodeMap.end ();
  it = nodeMap.find (id);
  if (it != nodeMap.end ())
    {
      ns3::Ptr<ns3::Object> object = (*it).second;
      ns3::Ptr<ns3::MobilityModel> model = object->GetObject<ns3::MobilityModel> ();
      return (model->GetPosition ()).x;        
    }

  return -1;
}

double 
WiselibExtIface::GetPositionY (node_id_t id)
{
  std::map <node_id_t, ns3::Ptr<ns3::Node> >::iterator it = nodeMap.end ();
  it = nodeMap.find (id);
  if (it != nodeMap.end ())
    {
      ns3::Ptr<ns3::Object> object = (*it).second;
      ns3::Ptr<ns3::MobilityModel> model = object->GetObject<ns3::MobilityModel> ();
      return (model->GetPosition ()).y;        
    }

  return -1;
}

double 
WiselibExtIface::GetPositionZ (node_id_t id)
{
  std::map <node_id_t,ns3::Ptr<ns3::Node> >::iterator it = nodeMap.end ();
  it = nodeMap.find (id);
  if (it != nodeMap.end ())
    {
      ns3::Ptr<ns3::Object> object = (*it).second;
      ns3::Ptr<ns3::MobilityModel> model = object->GetObject<ns3::MobilityModel> ();
      return (model->GetPosition ()).z;        
    }

  return -1;
}

double 
WiselibExtIface::GetDistance (node_id_t src, node_id_t dst)
{
  ns3::Ptr<ns3::MobilityModel> srcModel;
  ns3::Ptr<ns3::MobilityModel> dstModel;

  std::map <node_id_t, ns3::Ptr<ns3::Node> >::iterator it = nodeMap.end ();
  it = nodeMap.find (src);
  if (it != nodeMap.end ())
    {
      ns3::Ptr<ns3::Object> object = (*it).second;
      srcModel = object->GetObject<ns3::MobilityModel> ();
    }
  else
    return -1;

  it = nodeMap.find (dst);
  if (it != nodeMap.end ())
    {
      ns3::Ptr<ns3::Object> object = (*it).second;
      dstModel = object->GetObject<ns3::MobilityModel> ();
    }
  else 
    return -1;

  double distance = ns3::CalculateDistance (srcModel->GetPosition (), dstModel->GetPosition ());

  return distance;
}

void 
WiselibExtIface::SetPosition (double x, double y, double z, node_id_t id)
{
  std::map <node_id_t, ns3::Ptr<ns3::Node> >::iterator it = nodeMap.end ();
  it = nodeMap.find (id);
  if (it != nodeMap.end ())
    {
      // update node position
      ns3::Ptr<ns3::Object> object = (*it).second;
      ns3::Ptr<ns3::MobilityModel> model = object->GetObject<ns3::MobilityModel> ();
      ns3::Vector pos(x,y,z);
      model->SetPosition (pos);
    }
}

void 
WiselibExtIface::Receive (size_t size, block_data_t* data, int index)
{
  std::map <int, EventImplExt*>::iterator it = m_readCallbackMap.end (); 
  it = m_readCallbackMap.find (index);
  if ( it != m_readCallbackMap.end ())
    {
      it->second->ReadCallback (sizeof(data), data);
    }
}

void 
WiselibExtIface::UnregReadCallback (int index)
{
  std::map <int, EventImplExt*>::iterator it = m_readCallbackMap.end (); 
  it = m_readCallbackMap.find (index);
  if ( it != m_readCallbackMap.end ())
    {
      m_readCallbackMap.erase (it);
    }
}

void 
ExtendedDataClass::SetRss (double rss)
{
  m_rss = rss;
}
        
double 
ExtendedDataClass::GetRss () const
{
  return ExtendedDataClass::m_rss;
}

void 
TxPowerClass::set_ratio(int ratio)
{
  this->SetTxPowerStart ( RatioToDb (static_cast<double>(ratio) ) );
  this->SetTxPowerEnd ( RatioToDb(static_cast<double>(ratio) ) );
}

TxPowerClass 
TxPowerClass::from_ratio(int ratio)
{
  TxPowerClass txpower ( RatioToDb (static_cast<double>(ratio) ) );
  return txpower;
}

int 
TxPowerClass::to_ratio() const
{
  double ratio = DbToRatio(this->GetTxPowerStart ());
  return static_cast<int>(ratio);
}

TxPowerClass
TxPowerClass::from_dB(int db)
{
  TxPowerClass txpower( static_cast<double>(db) );
  return txpower;
}

void 
TxPowerClass::set_dB(int db)
{
  this->SetTxPowerStart ( static_cast<double>(db) );
  this->SetTxPowerEnd ( static_cast<double>(db) );
}

int 
TxPowerClass::to_dB() const
{
  return static_cast<int>(this->GetTxPowerStart ()); 
}

double
TxPowerClass::DbToRatio (double dB) const
{
  double ratio = std::pow (10.0, dB / 10.0);
  return ratio;
}

double
TxPowerClass::RatioToDb (double ratio) const
{
  return 10.0 * std::log10 (ratio);
}

void 
TxPowerClass::SetTxPowerStart (double dbm )
{
  m_txPowerStart = dbm;
}

void 
TxPowerClass::SetTxPowerEnd (double dbm)
{
  m_txPowerEnd = dbm;
}

double 
TxPowerClass::GetTxPowerStart () const
{
  return m_txPowerStart;
}

double 
TxPowerClass::GetTxPowerEnd () const
{
  return m_txPowerEnd;
}

void 
WiselibExtIface::SetTxPower (TxPowerClass &txpower, node_id_t id )
{
  // find wifi phy objective for node id
  std::map <node_id_t, ns3::Ptr<ns3::Node> >::iterator it = nodeMap.end ();
  it = nodeMap.find (id);
  if (it != nodeMap.end ())
    {
      ns3::Ptr<ns3::NetDevice> dev = (*it).second->GetDevice (0);
      ns3::Ptr<ns3::WifiNetDevice> wifiDev = ns3::DynamicCast<ns3::WifiNetDevice> (dev);
      ns3::Ptr<ns3::WifiPhy> phy = wifiDev->GetPhy ();
      ns3::Ptr<ns3::YansWifiPhy> wifiPhy = ns3::DynamicCast<ns3::YansWifiPhy> (phy);

      // set tx power 
      double start = txpower.GetTxPowerStart ();
      wifiPhy->SetTxPowerStart (start);
      double end = txpower.GetTxPowerEnd ();
      wifiPhy->SetTxPowerEnd (end);

      // store tx power info to this node (because there aren't GetTxPowerStart and GetTxPowerEnd methods in WifiPhy)
      std::map <node_id_t, std::pair<double, double> >::iterator itMap = nodeTxPowerMap.end ();
      itMap = nodeTxPowerMap.find (id);
      if ( itMap == nodeTxPowerMap.end ())
        {
          std::pair<double, double> txPowerInfo;
          txPowerInfo = std::make_pair (start, end);
          nodeTxPowerMap.insert (std::pair<node_id_t, std::pair<double, double> > (id, txPowerInfo ) );
        }
    }

}

TxPowerClass 
WiselibExtIface::GetTxPower (node_id_t id)
{
  TxPowerClass txpower;

  // find wifi phy objective for node id
  std::map <node_id_t, ns3::Ptr<ns3::Node> >::iterator it = nodeMap.end ();
  it = nodeMap.find (id);
  if (it != nodeMap.end ())
    {
      ns3::Ptr<ns3::NetDevice> dev = (*it).second->GetDevice (0);
      ns3::Ptr<ns3::WifiNetDevice> wifiDev = ns3::DynamicCast<ns3::WifiNetDevice> (dev);
      ns3::Ptr<ns3::WifiPhy> phy = wifiDev->GetPhy ();
      ns3::Ptr<ns3::YansWifiPhy> wifiPhy = ns3::DynamicCast<ns3::YansWifiPhy> (phy);

      // set tx power 
      std::map <node_id_t, std::pair<double, double> >::iterator itMap = nodeTxPowerMap.end ();
      itMap = nodeTxPowerMap.find (id);
      if ( itMap != nodeTxPowerMap.end ())
        {
          double start = (*itMap).second.first;
          double end = (*itMap).second.second;

          txpower.SetTxPowerStart (start);
          txpower.SetTxPowerEnd (end);
        }
    }

  return txpower;
}

}

