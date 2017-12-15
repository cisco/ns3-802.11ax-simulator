/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Cisco and/or its affiliates
 *
 * This file also contains code from the following file(s) from ns-3.26:
 * Filename : aarf-wifi-manager.h
 * Copyright (c) 2005,2006 INRIA
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *
 * Filename : ideal-wifi-manager.h
 * Copyright (c) 2006 INRIA
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
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
 * Authors: Balamurugan Ramachandran
 *          Ramachandra Murthy
 *          Bibek Sahu
 *          Mukesh Taneja
 */

/*
 * This file is for OFDMA/802.11ax type of systems. It is not
 * fully compliant to IEEE 802.11ax standards.
 */

#ifndef RRM_WIFI_MANAGER_H_
#define RRM_WIFI_MANAGER_H_

#include <stdint.h>
#include <vector>
#include "ns3/traced-value.h"
#include "ns3/he-bitmap.h"
#include "wifi-mode.h"
#include "wifi-remote-station-manager.h"
#include "mac-low.h"
#include <iostream>
#include <fstream>
#include "tlv.h"

namespace ns3 {

enum RateControlAlgorithm
{
   ARF   = 0,
   AARF  = 1,
   IDEAL = 2
};

typedef struct
{
  uint32_t  bufLen;  // Buffer in 256 octets
  Time      time;    // time stamp
} BufferStats;

/**
 * \brief hold per-remote-station state for RRM Wifi manager.
 *
 * This struct extends from WifiRemoteStation struct to hold additional
 * information required by the RRM Wifi manager
 */
struct RRMWifiRemoteStation : public WifiRemoteStation
{
  uint16_t m_aid;
  MacLowTransmissionListener *lt;
  WifiTxVector      dataTxVector;

  //Buffer stats of stations (UL)
  BufferStats       ulBufferStat;
  EventId           ulBSStaleTimer;
  uint32_t          m_timer;
  uint32_t          m_success;
  uint32_t          m_failed;
  bool              m_recovery;
  uint32_t          m_retry;
  uint32_t          m_timerTimeout;
  uint32_t          m_successThreshold;
  int32_t           m_mcsVal;
  double            m_lastSnrObserved;  //!< SNR of most recently reported packet sent to the remote station
  double            m_lastSnrCached;    //!< SNR most recently used to select a rate
  uint32_t          m_arfSuccessThreshold;
  uint32_t          m_arfFailureThreshold;
};

class RRMWifiManager : public WifiRemoteStationManager
{
public:
  static TypeId GetTypeId (void);
  RRMWifiManager ();
  virtual ~RRMWifiManager ();

  virtual void SetupPhy (Ptr<WifiPhy> phy);
  /**
   * \return the TXVECTOR to use to send the MU RTS prior to the
   *         transmission of the data packet itself.
   */
  virtual WifiTxVector GetMuRtsTxVector (void);

  virtual void SetupAidQueue (uint16_t aid, Mac48Address mac, MacLowTransmissionListener *lt, AcIndex ac);
  
  void SetSiMin (Time siMin);
  void SetSiMax (Time siMax);
  Time GetSiMin (void) const;
  Time GetSiMax (void) const;

  int tlvWriteTypeLen(TlvBuffer* buf,short type,short len);
  int tlvEncode1Byte(TlvBuffer* buf,short type,char val);
  int tlvEncode2Bytes(TlvBuffer* buf,short type,short val);
  int tlvEncode4Bytes(TlvBuffer* buf,short type,int val);
  int tlvEncodeString(TlvBuffer* buf,short type,char* val);
  int tlvEncode2ByteIntArray(TlvBuffer* buf,short type,short* array,short count);

  int tlvReadType(TlvBuffer* message,short* type);
  int tlvReadLen(TlvBuffer* message,short* len);
  int tlvReadLen1(TlvBuffer* message,short* len);

  int tlvDecode1Byte(TlvBuffer* message,char* val);
  int tlvDecode2Bytes(TlvBuffer* message,short* val);
  int tlvDecode4Bytes(TlvBuffer* message,int* val);
  int tlvDecodeString(TlvBuffer* message,char* val);
  int tlvDecode2ByteIntArray(TlvBuffer* message,short** array,short *count);

  int tlvAppend2Bytes(TlvBuffer* buf,short val);
  int tlvAppend1Byte(TlvBuffer* buf,char val);
  int tlvDecodeAppended2Bytes(TlvBuffer* message,short* val);
  void print_bytes(const void *object, size_t size);

  int tlvEncodeAllStats(TlvBuffer* message,short type,AllStats_t* clientArray,short count);
  int tlvEncodeResults(TlvBuffer* message,short type,RRMClientResponse_t* respArray,short count);

  int tlvDecodeAllStats(TlvBuffer* message,AllStats_t** clientArray,short* count);
  int tlvDecodeResults(TlvBuffer* message,RRMClientResponse_t** respArray,short* count);
  int SendDLStations(bool isEmptyBufReq);
  int SendULStations(bool isEmptyBufReq);
  int SendAllInfo(bool isEmptyReq);
  int GetRUMCS(short station_id,short *mcs,short *ru);
  int HandleDLMCSInfoRequest(TlvBuffer* message);
  int HandleULMCSInfoRequest(TlvBuffer* message);
  int GetBufferDepth(short stationId,bool isDL,short* bufferDepth);
  int GetWaitingTime(short stationId,bool isDL,short* waitingTime);
  int HandleBufferDepthRequest(TlvBuffer* message,bool isDL);
  int HandleWaitingTimeRequest(TlvBuffer* message,bool isDL);
  bool HandleRRMResults(TlvBuffer* message, bool isDownlink);
  void PrintCurrentTime(void);
  int ProcessTlvMessage(TlvBuffer* message, bool isDownlink);
  int initSocket(int* socketfd);

private:
  class Dcf;
  friend class Dcf;
  //overriden from base class
  virtual void DoInitialize (void);
  virtual WifiRemoteStation* DoCreateStation (void) const;
  virtual void DoReportRxOk (WifiRemoteStation *station,
                             double rxSnr, WifiMode txMode);
  virtual void DoReportRtsFailed (WifiRemoteStation *station);
  virtual void DoReportDataFailed (WifiRemoteStation *station);
  virtual void DoReportRtsOk (WifiRemoteStation *station,
                              double ctsSnr, WifiMode ctsMode, double rtsSnr);
  virtual void DoReportDataOk (WifiRemoteStation *station,
                               double ackSnr, WifiMode ackMode, double dataSnr);
  virtual void DoReportAmpduTxStatus (WifiRemoteStation *station, uint32_t nSuccessfulMpdus, uint32_t nFailedMpdus, double rxSnr, double dataSnr);
  virtual void DoReportFinalRtsFailed (WifiRemoteStation *station);
  virtual void DoReportFinalDataFailed (WifiRemoteStation *station);
  virtual bool DoNeedDataRetransmission (WifiRemoteStation *station, Ptr<const Packet> packet, bool normally);
  virtual WifiTxVector DoGetDataTxVector (WifiRemoteStation *station);
  virtual WifiTxVector DoGetDataTxVector (WifiRemoteStation *station, uint32_t bitMap, uint32_t aid, uint32_t mcs);
  virtual WifiTxVector DoGetRtsTxVector (WifiRemoteStation *station);
  virtual bool IsLowLatency (void) const;
  virtual void SetupDcfManager (DcfManager *manager);
  virtual void PrepareForQueue (Mac48Address address, const WifiMacHeader *header,
                        Ptr<const Packet> packet);
  /**
   * CSMA/CA definitions
   */
  /* dcf notifications forwarded here */
   /**
   * Notify the DCF that access has been granted.
   */
  void NotifyAccessGranted (void);
  /**
   * Notify the DCF that internal collision has occurred.
   */
  void NotifyInternalCollision (void);
  /**
   * Notify the DCF that collision has occurred.
   */
  void NotifyCollision (void);
  /**
   * When a channel switching occurs, enqueued packets are removed.
   */
  void NotifyChannelSwitching (void);
  /**
   * When sleep operation occurs, if there is a pending packet transmission,
   * it will be reinserted to the front of the queue.
   */
  void NotifySleep (void);
  /**
   * When wake up operation occurs, channel access will be restarted
   */
  void NotifyWakeUp (void);
  /**
   * Request access from DCF manager if needed.
   */
  void StartAccessIfNeeded (void);
  /**
   * Round Robin Scheduler
   */
  bool ScheduleStations();
  /**
   * Round Robin Scheduler Downlink
   */
  bool SampleDLScheduler();
  /**
    * Round Robin Scheduler Uplink
    */
  bool SampleULScheduler();
  /**
   *
   */
  int EstablishRRMServerConnection();
  bool CallAlgoPlugin(bool isDownlink);
 
  typedef std::vector <RRMWifiRemoteStation *> AxStations;
  typedef std::vector<RRMWifiRemoteStation *> ServingStations;
  /**
   *
   */
  bool StartTranmission (bool isDownlink, ServingStations servingStaions);
  /**
   *
   */
  void PrepareHeMuMpdu(ServingStations servingStations);
  /**
   *
   */
  uint32_t FectchAxStationIndexFromMac(uint8_t mac[6], uint8_t trafficType);
  /**
   * make the UL buffer status invalid if the stale timer expired
   */
  Time GetAcStaleTime (uint ac);
  void StaleBSData(RRMWifiRemoteStation *st);
  /**
   * To get the rx drops
   */
  void RxDrop (Ptr<const Packet> p);
  /**
   * Rate control support
   */
  void rateControlDataSuccess(RRMWifiRemoteStation *st);
  void rateControlDataFailed(RRMWifiRemoteStation *st);
  uint32_t rateControlIdeal(RRMWifiRemoteStation *st);

  /**
   * Return the minimum SNR needed to successfully transmit
   * data with this WifiTxVector at the specified BER.
   *
   * \param txVector WifiTxVector (containing valid mode, width, and nss)
   *
   * \return the minimum SNR for the given WifiTxVector
   */
  double GetSnrThreshold (WifiTxVector txVector) const;
  /**
   * Adds a pair of WifiTxVector and the minimum SNR for that given vector
   * to the list.
   *
   * \param txVector the WifiTxVector storing mode, channel width, and nss
   * \param snr the minimum SNR for the given txVector
   */
  void AddSnrThreshold (WifiTxVector txVector, double snr);

  /**
   * This is a pointer to the WifiPhy associated with this
   * WifiRemoteStationManager. Through this pointer the
   * station manager can determine PHY characteristics, such as the
   * set of all transmission rates that may be supported (the
   * "DeviceRateSet").
   */
  Ptr<WifiPhy> m_wifiPhy;
  Dcf *m_dcf;
  RandomStream *m_rng;
  DcfManager *m_dcfManager;
  Ptr<HEBitMap> m_ruTable;
  /**
   * A vector of WifiRemoteStations
   */
  AxStations m_axStations;                  //!< Information for each known stations
  WifiMode m_dataMode; //!< Wifi mode for unicast DATA frames
  WifiMode m_ctlMode;  //!< Wifi mode for RTS frames

  /**
   * for Enhanced Round Robin Scheduling in UL
   */
  Time        m_siMin;                      // Minimum Scheduling interval
  Time        m_siMax;                      // Maximum Scheduling interval


 /**
  * Socket Communication between ns3 and RRM Server
  */
  int m_sockId; //Socket to communicate betweeen ns3 and RRM Server Module

  uint32_t m_minTimerThreshold;
  uint32_t m_minSuccessThreshold;
  double m_successK;
  uint32_t m_maxSuccessThreshold;
  double m_timerK;
  uint32_t m_rateControlSelector;

  /**
   * A vector of <snr, WifiTxVector> pair holding the minimum SNR for the
   * WifiTxVector
   */
  typedef std::vector<std::pair<double, WifiTxVector> > Thresholds;

  double m_ber;             //!< The maximum Bit Error Rate acceptable at any transmission mode
  Thresholds m_thresholds;  //!< List of WifiTxVector and the minimum SNR pair
  bool m_SchedulerPluginEnabled;
};

}

#endif /* RRM_WIFI_MANAGER_H_ */
