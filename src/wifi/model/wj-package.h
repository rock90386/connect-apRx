#ifndef WJ_PACKAGE_H
#define WJ_PACKAGE_H

#include <list>
#include "ns3/nstime.h"
#include "wifi-tx-vector.h"
#include "wifi-phy-header.h"
#include "wifi-psdu.h"

#include "serializer.h"
#include <string.h>

namespace ns3 
{
	class WifiPsdu;
  class WifiPpdu;

  class WjPackage : public SimpleRefCount<WjPackage>
{
	public:

  friend class WifiPpdu;

  WjPackage ();

  WjPackage (Ptr<WifiPpdu>);

  virtual ~WjPackage ();

  uint8_t* PsduPacketSerialize (Ptr<WifiPpdu>, size_t&);

  std::string serialize(void);

  int deserialize(std::string &str);

  WifiTxVector ConstructTxVector(void);

  Time GetPpduDuration(void);

  uint16_t GetFrequency(void);

  /**
   * \brief Print the PPDU contents.
   * \param os output stream in which the data should be printed.
   */
  void Print (std::ostream &os);
  
/****** member of psdu *****/
  bool m_isSingle;                                //!< true for an S-MPDU
  uint32_t m_size;
  std::vector<struct Pkt_Info> mpdu_pkt_info_vec;



private:
  Time ppduDuration;
  uint16_t m_frequency;

  WifiTxVector wjTxVector;

  WifiMode m_mode;               /**< The DATARATE parameter in Table 15-4.
                                 It is the value that will be passed
                                 to PMD_RATE.request */
  uint8_t  m_txPowerLevel;       /**< The TXPWR_LEVEL parameter in Table 15-4.
                                 It is the value that will be passed
                                 to PMD_TXPWRLVL.request */
  WifiPreamble m_preamble;       /**< preamble */
  uint16_t m_channelWidth;       /**< channel width in MHz */
  uint16_t m_guardInterval;      /**< guard interval duration in nanoseconds */
  uint8_t  m_nTx;                /**< number of TX antennas */
  uint8_t  m_nss;                /**< number of spatial streams */
  uint8_t  m_ness;               /**< number of spatial streams in beamforming */
  bool     m_aggregation;        /**< Flag whether the PSDU contains A-MPDU. */
  bool     m_stbc;               /**< STBC used or not */
  uint8_t  m_bssColor;           /**< BSS color */

  bool     m_modeInitialized;         /**< Internal initialization flag */
  

};

/**
 * \brief Stream insertion operator.
 *
 * \param os the stream
 * \param ppdu the PPDU
 * \returns a reference to the stream
 */
std::ostream& operator<< (std::ostream& os, WjPackage &ppdu);

} //namespace ns3

#endif /* WJ_PACKAGE_H */

