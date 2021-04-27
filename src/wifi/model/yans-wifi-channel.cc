/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
 * Author: Mathieu Lacage, <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/mobility-model.h"
#include "yans-wifi-channel.h"
#include "yans-wifi-phy.h"
#include "wifi-utils.h"
#include "wifi-ppdu.h"
#include "wifi-psdu.h"
#include "wifi-mac-queue-item.h"

#include "ns3/fd-net-device.h"
#include "ns3/fd-net-device-module.h"

#include "ns3/packet.h"
#include <iostream>
#include "ns3/string.h"



namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("YansWifiChannel");

NS_OBJECT_ENSURE_REGISTERED (YansWifiChannel);

TypeId
YansWifiChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::YansWifiChannel")
    .SetParent<Channel> ()
    .SetGroupName ("Wifi")
    .AddConstructor<YansWifiChannel> ()
    .AddAttribute ("PropagationLossModel", "A pointer to the propagation loss model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&YansWifiChannel::m_loss),
                   MakePointerChecker<PropagationLossModel> ())
    .AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&YansWifiChannel::m_delay),
                   MakePointerChecker<PropagationDelayModel> ())
  ;
  return tid;
}

YansWifiChannel::YansWifiChannel ()
{
  SetFdNetdevice();
  // SetCallBack();
  NS_LOG_FUNCTION (this);
}

void
YansWifiChannel::SetFdNetdevice()
{
  EmuFdNetDeviceHelper emu;
  emu.SetDeviceName ("eno1");
  emu.SetAttribute ("EncapsulationMode", StringValue ("Dix"));

  NodeContainer vnode;
  vnode.Create(1);
  NetDeviceContainer fdDevices;
  // Ipv4InterfaceContainer fdInterface;

  // Ipv4AddressHelper fdaddress;

  // fdaddress.SetBase ("192.168.14.0", "255.255.255.0", "0.0.0.3");
  
  // fdDevices = emu.Install (sender->GetDevice()->GetNode());

  fdDevices = emu.Install (vnode.Get(0), MakeCallback(&YansWifiChannel::WjCallReceive, this));
  // fdDevices = emu.Install (vnode.Get(0));

  dev = fdDevices.Get (0)->GetObject<FdNetDevice> ();
  dev->SetAddress (Mac48Address ("00:00:00:00:00:08"));
  //NS_LOG_INFO ("Assign IP Addresses.");
  //fdaddress.NewAddress ();  // burn the 10.1.1.1 address so that 10.1.1.2 is next
  //fdInterface = fdaddress.Assign (fdDevices);
              
  // std::cout << "shit" << std::endl;
  //dev->Send(copy, dstNetDevice->GetAddress(), 0);

  // Ptr<FdNetDevice> fdnet = sender->GetDevice()->GetObject<FdNetDevice> ();
  // Ptr<FdNetDevice> fdnet = (sender->GetDevice()->GetNode()->GetDevice(1)); 
  //std::cout << sender->GetDevice()->GetNode() << std::endl;
  // fdnet->Send(copy, dstNetDevice->GetAddress(), 0);

}

YansWifiChannel::~YansWifiChannel ()
{
  NS_LOG_FUNCTION (this);
  m_phyList.clear ();
}

void
YansWifiChannel::SetPropagationLossModel (const Ptr<PropagationLossModel> loss)
{
  NS_LOG_FUNCTION (this << loss);
  m_loss = loss;
}

void
YansWifiChannel::SetPropagationDelayModel (const Ptr<PropagationDelayModel> delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_delay = delay;
}

void
YansWifiChannel::Send (Ptr<YansWifiPhy> sender, Ptr<const WifiPpdu> ppdu, double txPowerDbm) const
{
  NS_LOG_FUNCTION (this << sender << ppdu << txPowerDbm);
  Ptr<MobilityModel> senderMobility = sender->GetMobility ();
  NS_ASSERT (senderMobility != 0);
  for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++)
    {
      if (sender == (*i))
        {
          //For now don't account for inter channel interference nor channel bonding
          if ((*i)->GetChannelNumber () != sender->GetChannelNumber ())
            {
              continue;
            }

          Ptr<MobilityModel> receiverMobility = (*i)->GetMobility ()->GetObject<MobilityModel> ();
          Time delay = m_delay->GetDelay (senderMobility, receiverMobility);
          double rxPowerDbm = m_loss->CalcRxPower (txPowerDbm, senderMobility, receiverMobility);
          NS_LOG_DEBUG ("propagation: txPower=" << txPowerDbm << "dbm, rxPower=" << rxPowerDbm << "dbm, " <<
                        "distance=" << senderMobility->GetDistanceFrom (receiverMobility) << "m, delay=" << delay);

          Ptr<WifiPpdu> copy = Copy (ppdu);
          //copy->Print (std::cout);
          
          WjPackage wjPackage (copy);
          size_t t_size = 0;
          uint8_t *bbuf = Total_Serialize (copy, wjPackage, rxPowerDbm, t_size);
          //Ptr<WifiPpdu> pppppppppppppdu= Total_Deserialize (bbuf);

          Ptr<Packet> pkt = Create<Packet> (bbuf, (int)t_size);



          /*
          Ptr<NetDevice> dstNetDevice = (*i)->GetDevice ();
          uint32_t dstNode;
          if (dstNetDevice == 0)
            {
              dstNode = 0xffffffff;
            }
          else
            {
              dstNode = dstNetDevice->GetNode ()->GetId ();
            }
          */
          /*
          Simulator::ScheduleWithContext (dstNode,
                                          delay, &YansWifiChannel::Receive,
                                          (*i), copy, rxPowerDbm);
          */
          /*
          Simulator::ScheduleWithContext (dstNode,
                                          delay, &YansWifiChannel::Receive,
                                          (*i), pppppppppppppdu, rxPowerDbm);
          */

          Ptr<NetDevice> dstNetDevice = (*i)->GetDevice ();

/*          std::cout << "Time in send" << Simulator::Now() << std::endl;
          std::cout << "Node id " << dstNetDevice->GetNode ()->GetId () << std::endl;
          std::cout << "sender = " << sender << std::endl;
          std::cout << "Receiver = " << *i << std::endl;*/

          //copy->Print(std::cout);
          //std::cout << "sender node id "<< sender->GetDevice()->GetNode() << std::endl;
          dev->Send(pkt, dstNetDevice->GetAddress(), 0);      
        }
    }
}


uint8_t*
YansWifiChannel::Total_Serialize (Ptr<WifiPpdu> ppdu, WjPackage wjpackage, double rxPowerDbm, size_t &total_size) const
{
  
  // Serialize all of packet in msdu
  size_t serialize_size = 0; // total size of total packet
  uint8_t *psduBuffer = wjpackage.PsduPacketSerialize (ppdu, serialize_size);


  // Serialize wjpackage
  OutStream os;
  os << wjpackage;
  std::string serializestr = os.str();
  int packageBufferLen = serializestr.size();
  
  uint8_t *packageBuffer = (uint8_t*)malloc(sizeof(uint8_t)*packageBufferLen);
  std::copy (serializestr.begin(), serializestr.end(), packageBuffer);

  // Serialize buffer size , size of this serialization = 8
  OutStream oss;
  int s_size = (int)serialize_size;
  oss << packageBufferLen << s_size;
  std::string serialstr = oss.str();
  uint8_t *sizeBuffer = (uint8_t*)malloc(sizeof(uint8_t)*8);
  std::copy (serialstr.begin(), serialstr.end(), sizeBuffer);


  // Serialize buffer size , size of this serialization = 8
  OutStream rxpos;
  oss << rxPowerDbm;
  std::string serstr = rxpos.str();
  uint8_t *rxpbuf = (uint8_t*)malloc(sizeof(uint8_t)*8);
  std::copy (serstr.begin(), serstr.end(), rxpbuf);


  total_size = packageBufferLen + serialize_size + 8 + 8;
  // std::cout << "packageBufferLen : " << packageBufferLen << std::endl;
  // std::cout << "serialize_size " << serialize_size << std::endl;
  // std::cout << "total_size " << total_size << std::endl;

  uint8_t *total_buffer = (uint8_t*)malloc(sizeof(uint8_t)*total_size);

  std::copy (rxpbuf, rxpbuf+8, total_buffer);
  std::copy (sizeBuffer, sizeBuffer+8, total_buffer+8);
  std::copy (packageBuffer, packageBuffer+packageBufferLen, total_buffer+16);
  std::copy (psduBuffer, psduBuffer+serialize_size, total_buffer+16+packageBufferLen);

  return total_buffer;

}

Ptr<WifiPpdu>
YansWifiChannel::Total_Deserialize (uint8_t *total_buffer, double &rxPower) const
{
  // Deserialize rxPowerDbm
  uint8_t *rxpbuf = (uint8_t*)malloc(sizeof(uint8_t)*8);
  std::copy (total_buffer, total_buffer+8, rxpbuf);
  std::string serstr(rxpbuf, rxpbuf+8);
  InStream rxpis(serstr);
  double rxp;
  rxpis >> rxp;
  rxPower = rxp;

  // Deserialize size buffer
  uint8_t *sizeBuffer = (uint8_t*)malloc(sizeof(uint8_t)*8);
  std::copy (total_buffer+8, total_buffer+16, sizeBuffer);
  std::string serialstr(sizeBuffer, sizeBuffer+8);
  InStream iss(serialstr);
  int packageBufferLen;
  int s_size;
  iss >> packageBufferLen >> s_size;

  // std::cout << "DpackageBufferLen : " << packageBufferLen << std::endl;
  // std::cout << "Dserialize_size " << s_size << std::endl;
  
  // Deserialize wjPackage buffer
  uint8_t *packageBuffer = (uint8_t*)malloc(sizeof(uint8_t)*packageBufferLen);
  std::copy(total_buffer+16, total_buffer+16+packageBufferLen, packageBuffer);
  WjPackage wjPackage;
  std::string serializestr(packageBuffer, packageBuffer+packageBufferLen);
  InStream is(serializestr);
  is >> wjPackage;
  
  // Deserialize packet buffer
  int i;
  int len = 0;
  int tmp = 0;
  std::vector<Ptr<WifiMacQueueItem>> mpdu_list;

  uint8_t *psduBuffer = (uint8_t*)malloc(sizeof(uint8_t)*s_size);
  std::copy(total_buffer+16+packageBufferLen, total_buffer+16+packageBufferLen+s_size, psduBuffer);

  
  for (i = 0 ; i < (int)wjPackage.mpdu_pkt_info_vec.size() ; i++)
  {
    len = (int)wjPackage.mpdu_pkt_info_vec.at(i).pkt_len;
    uint8_t *per_pkt_buf = (uint8_t*)malloc(len);

    std::copy (psduBuffer+tmp, psduBuffer+tmp+len, per_pkt_buf);
    Ptr<Packet> packet = Create<Packet> (reinterpret_cast<const uint8_t *> (per_pkt_buf), len);

    WifiMacHeader header;
    packet->RemoveHeader(header);
    
    Ptr<WifiMacQueueItem> mpdu = Create<WifiMacQueueItem> \
                                  (packet, header, wjPackage.mpdu_pkt_info_vec.at(i).m_timestamp);
    
    mpdu_list.push_back(mpdu);
    tmp = len;
    free (per_pkt_buf);  
  }


  Ptr<WifiPsdu> psdu = Create<WifiPsdu>(mpdu_list, wjPackage.m_isSingle);
  //std::cout << "size " <<psdu->GetNMpdus() <<std::endl;
  //std::cout << "bool " <<psdu->IsSingle() <<std::endl;
  Ptr<WifiPpdu> ppdu = Create<WifiPpdu> (psdu, wjPackage.ConstructTxVector(),\
      wjPackage.GetPpduDuration(), wjPackage.GetFrequency());
  
  free (sizeBuffer);
  free (packageBuffer);
  free (psduBuffer);
  free (total_buffer);


    return ppdu;  
  
}

void
YansWifiChannel::WjCallReceive(Ptr<Packet> pkt)
{
  /*
  for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++)
    Receive ((*i), packet, -35.1725, NanoSeconds (108000));
  */
  //std::cout << "WjCallReceive" << std::endl;
  size_t len =  (size_t) pkt->GetSize ();
  uint8_t *bbuf = (uint8_t*)malloc (len);
  pkt->CopyData (bbuf, len);
  double rxPowerDbm;
  Ptr<WifiPpdu> pppppppppppppdu= Total_Deserialize (bbuf, rxPowerDbm);
  //std::cout << "receiver node id "<< m_phyList[0]->GetDevice()->GetNode() << std::endl;
  uint32_t dstNode = m_phyList[0]->GetDevice()->GetNode()->GetId ();
  Simulator::ScheduleWithContext (dstNode,
                                Seconds(0), &YansWifiChannel::Receive,
                                m_phyList[0], pppppppppppppdu, rxPowerDbm);
  //Receive (m_phyList[0], pppppppppppppdu, rxPowerDbm);

}


void
YansWifiChannel::Receive (Ptr<YansWifiPhy> phy, Ptr<WifiPpdu> ppdu, double rxPowerDbm)
{
  NS_LOG_FUNCTION (phy << ppdu << rxPowerDbm << Simulator::Now());
  //std::cout << "Receive" << Simulator::Now() << std::endl;
  // Do no further processing if signal is too weak
  // Current implementation assumes constant rx power over the PPDU duration
  if ((rxPowerDbm + phy->GetRxGain ()) < phy->GetRxSensitivity ())
    {
      NS_LOG_INFO ("Received signal too weak to process: " << rxPowerDbm << " dBm");
      return;
    }
  phy->StartReceivePreamble (ppdu, DbmToW (rxPowerDbm + phy->GetRxGain ()));
}

std::size_t
YansWifiChannel::GetNDevices (void) const
{
  return m_phyList.size ();
}

Ptr<NetDevice>
YansWifiChannel::GetDevice (std::size_t i) const
{
  return m_phyList[i]->GetDevice ()->GetObject<NetDevice> ();
}

void
YansWifiChannel::Add (Ptr<YansWifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  m_phyList.push_back (phy);
}

int64_t
YansWifiChannel::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  int64_t currentStream = stream;
  currentStream += m_loss->AssignStreams (stream);
  return (currentStream - stream);
}

} //namespace ns3
