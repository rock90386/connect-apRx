/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 Orange Labs
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
 * Author: Rediet <getachew.redieteab@orange.com>
 */

#include "ns3/log.h"
#include "ns3/packet.h"
#include "wj-package.h"
#include "wifi-phy.h"
#include "wifi-utils.h"
#include "wifi-ppdu.h"



namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WjPackage");

WjPackage::WjPackage ()
{}

WjPackage::WjPackage (Ptr<WifiPpdu> ppdu)
{
  ppduDuration = ppdu->GetTxDuration();
  // wj is friend class of ppdu, so can access private member directly.
  m_frequency = ppdu->m_frequency;

  WifiTxVector wjTxVector = ppdu->GetTxVector ();

  // WifiMode, need to get uid
  m_mode = wjTxVector.GetMode();

  // WifiPreamble, need to convert enum to int
  m_preamble = wjTxVector.GetPreambleType();
  
  m_txPowerLevel = wjTxVector.GetTxPowerLevel();
  m_channelWidth = wjTxVector.GetChannelWidth();       
  m_guardInterval = wjTxVector.GetGuardInterval();     
  m_nTx = wjTxVector.GetNTx();   
  m_nss = wjTxVector.GetNss();     
  m_ness = wjTxVector.GetNess();         
  m_aggregation = wjTxVector.IsAggregation();   
  m_stbc = wjTxVector.IsStbc();        
  m_bssColor = wjTxVector.GetBssColor();    
  m_modeInitialized = wjTxVector.GetModeInitialized();     

  // psdu member
  m_isSingle = ppdu->m_psdu->m_isSingle;
  m_size = ppdu->m_psdu->m_size;
  
}


WjPackage::~WjPackage ()
{
}

uint8_t*
WjPackage::PsduPacketSerialize (Ptr<WifiPpdu> ppdu, size_t &serialize_size)
{
  Ptr<WifiPsdu> psdu = Copy(ppdu->m_psdu);
  uint8_t *b = psdu->WjPacketHeaderSerialize (serialize_size);
  mpdu_pkt_info_vec.assign(psdu->pkt_Info_vec.begin(), psdu->pkt_Info_vec.end());
  // std::cout << "mpdu_pkt_info_vec " << mpdu_pkt_info_vec.size() << std::endl;
  return b;
}

std::string 
WjPackage::serialize()
{
    uint32_t m_uid = m_mode.GetUid(); // WifiMode uid
    int preambleId = (int)m_preamble;
    double duration = ppduDuration.GetDouble();

    std::vector<int> boolvec;
    boolvec.push_back((int)m_isSingle);
    boolvec.push_back((int)m_aggregation);
    boolvec.push_back((int)m_stbc);
    boolvec.push_back((int)m_modeInitialized);
    /*
    int i;
    for (i=0;i<(int)boolvec.size();i++)
      std::cout << boolvec.at(i) << std::endl;
*/
    OutStream os;
    os << m_txPowerLevel << m_channelWidth << m_guardInterval\
    << m_nTx << m_nss << m_ness << boolvec\
    << m_bssColor << m_size\
    << mpdu_pkt_info_vec << m_uid << preambleId << m_frequency << duration;

    return os.str();
}

int 
WjPackage::deserialize(std::string &str)
{
    InStream is(str);
    uint32_t m_uid;
    int preambleId;
    double duration;
    std::vector<int> boolvec;

    is >> m_txPowerLevel >> m_channelWidth >> m_guardInterval\
    >> m_nTx >> m_nss >> m_ness >>boolvec\
    >> m_bssColor >> m_size\
    >> mpdu_pkt_info_vec >> m_uid >> preambleId >> m_frequency >> duration;

    m_isSingle = (bool)boolvec.at(0);
    m_aggregation = (bool)boolvec.at(1);
    m_stbc = (bool)boolvec.at(2);
    m_modeInitialized = (bool)boolvec.at(3);

    Time tmp (duration);
    ppduDuration = tmp;
    WifiMode tmpMode (m_uid);
    m_mode = tmpMode; 
    m_preamble = WifiPreamble(preambleId); // enum

    return is.size();
}

WifiTxVector 
WjPackage::ConstructTxVector()
{
  WifiTxVector wjTxVector (m_mode, m_txPowerLevel, m_preamble, m_guardInterval, 
                          m_nTx, m_nss, m_ness, m_channelWidth, m_aggregation, 
                          m_stbc, m_bssColor, m_modeInitialized);
  return wjTxVector;
}

Time
WjPackage::GetPpduDuration()
{
  return ppduDuration;
}

uint16_t
WjPackage::GetFrequency()
{
  return m_frequency;
}

void
WjPackage::Print (std::ostream& os)
{
  os << "preamble=" << ", modulation=" << ", truncatedTx=" << ", PSDU=";
}

std::ostream & operator << (std::ostream &os, WjPackage &ppdu)
{
  ppdu.Print (os);
  return os;
}

} //namespace ns3
