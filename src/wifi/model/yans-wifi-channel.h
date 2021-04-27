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

#ifndef YANS_WIFI_CHANNEL_H
#define YANS_WIFI_CHANNEL_H

#include "ns3/channel.h"
#include "wj-package.h"

#include "serializer.h"
#include <string.h>

#include "ns3/fd-net-device-module.h"

namespace ns3 {

class NetDevice;
class PropagationLossModel;
class PropagationDelayModel;
class YansWifiPhy;
class Packet;
class Time;
class WifiPpdu;

/**
 * \brief a channel to interconnect ns3::YansWifiPhy objects.
 * \ingroup wifi
 *
 * This class is expected to be used in tandem with the ns3::YansWifiPhy
 * class and supports an ns3::PropagationLossModel and an
 * ns3::PropagationDelayModel.  By default, no propagation models are set;
 * it is the caller's responsibility to set them before using the channel.
 */
class YansWifiChannel : public Channel
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  YansWifiChannel ();
  virtual ~YansWifiChannel ();

  void SetFdNetdevice(void);

  //inherited from Channel.
  virtual std::size_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (std::size_t i) const;

  /**
   * Adds the given YansWifiPhy to the PHY list
   *
   * \param phy the YansWifiPhy to be added to the PHY list
   */
  void Add (Ptr<YansWifiPhy> phy);

  /**
   * \param loss the new propagation loss model.
   */
  void SetPropagationLossModel (const Ptr<PropagationLossModel> loss);
  /**
   * \param delay the new propagation delay model.
   */
  void SetPropagationDelayModel (const Ptr<PropagationDelayModel> delay);

  /**
   * \param sender the phy object from which the packet is originating.
   * \param ppdu the PPDU to send
   * \param txPowerDbm the tx power associated to the packet, in dBm
   *
   * This method should not be invoked by normal users. It is
   * currently invoked only from YansWifiPhy::StartTx.  The channel
   * attempts to deliver the PPDU to all other YansWifiPhy objects
   * on the channel (except for the sender).
   */
  void Send (Ptr<YansWifiPhy> sender, Ptr<const WifiPpdu> ppdu, double txPowerDbm) const;

  uint8_t* Total_Serialize (Ptr<WifiPpdu> ppdu, WjPackage wjppdu, double rxPowerDbm, size_t &total_size) const;

  Ptr<WifiPpdu> Total_Deserialize (uint8_t *total_buffer, double &rxPower) const;

  void WjCallReceive(Ptr<Packet> pkt);
  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   *
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);


private:
  /**
   * A vector of pointers to YansWifiPhy.
   */
  typedef std::vector<Ptr<YansWifiPhy> > PhyList;

  /**
   * This method is scheduled by Send for each associated YansWifiPhy.
   * The method then calls the corresponding YansWifiPhy that the first
   * bit of the PPDU has arrived.
   *
   * \param receiver the device to which the packet is destined
   * \param ppdu the PPDU being sent
   * \param txPowerDbm the tx power associated to the packet being sent (dBm)
   */
  static void Receive (Ptr<YansWifiPhy> receiver, Ptr<WifiPpdu> ppdu, double txPowerDbm);

  PhyList m_phyList;                   //!< List of YansWifiPhys connected to this YansWifiChannel
  Ptr<PropagationLossModel> m_loss;    //!< Propagation loss model
  Ptr<PropagationDelayModel> m_delay;  //!< Propagation delay model

  Ptr<FdNetDevice> dev;
};

} //namespace ns3

#endif /* YANS_WIFI_CHANNEL_H */