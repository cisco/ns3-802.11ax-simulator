/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This file also contains code from the following file(s) from ns-3.26:
 * Filename : wifi-phy.h
 * Copyright (c) 2005,2006 INRIA
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Sébastien Deronne <sebastien.deronne@gmail.com>
 *
 * Copyright (c) 2017 Cisco and/or its affiliates
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

#ifndef HE_WIFI_PHY_H
#define HE_WIFI_PHY_H

#include "wifi-phy.h"

namespace ns3 {

class HEWifiChannel;

/**
 * \brief 802.11 PHY layer model
 * \ingroup wifi
 *
 * This PHY implements a model of 802.11a. The model
 * implemented here is based on the model described
 * in "Yet Another Network Simulator",
 * (http://cutebugs.net/files/wns2-HE.pdf).
 *
 *
 * This PHY model depends on a channel loss and delay
 * model as provided by the ns3::PropagationLossModel
 * and ns3::PropagationDelayModel classes, both of which are
 * members of the ns3::HEWifiChannel class.
 */
class HEWifiPhy : public WifiPhy
{
public:
  static TypeId GetTypeId (void);

  HEWifiPhy ();
  virtual ~HEWifiPhy ();

  /**
   * Set the HEWifiChannel this HEWifiPhy is to be connected to.
   *
   * \param channel the HEWifiChannel this HEWifiPhy is to be connected to
   */
  void SetChannel (Ptr<HEWifiChannel> channel);

  /**
   * Starting receiving the plcp of a packet (i.e. the first bit of the preamble has arrived).
   *
   * \param packet the arriving packet
   * \param rxPowerDbm the receive power in dBm
   * \param txVector the TXVECTOR of the arriving packet
   * \param preamble the preamble of the arriving packet
   * \param mpdutype the type of the MPDU as defined in WifiPhy::mpduType.
   * \param rxDuration the duration needed for the reception of the packet
   */
  void StartReceivePreambleAndHeader (Ptr<Packet> packet,
                                      double rxPowerDbm,
                                      WifiTxVector txVector,
                                      WifiPreamble preamble,
                                      enum mpduType mpdutype,
                                      Time rxDuration);
  /**
   * Starting receiving the payload of a packet (i.e. the first bit of the packet has arrived).
   *
   * \param packet the arriving packet
   * \param txVector the TXVECTOR of the arriving packet
   * \param preamble the preamble of the arriving packet
   * \param mpdutype the type of the MPDU as defined in WifiPhy::mpduType.
   * \param event the corresponding event of the first time the packet arrives
   */
  void StartReceivePacket (Ptr<Packet> packet,
                           WifiTxVector txVector,
                           WifiPreamble preamble,
                           enum mpduType mpdutype,
                           Ptr<InterferenceHelper::Event> event);

  virtual void SetReceiveOkCallback (WifiPhy::RxOkCallback callback);
  virtual void SetReceiveErrorCallback (WifiPhy::RxErrorCallback callback);
  virtual void SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, WifiPreamble preamble);
  virtual void SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, WifiPreamble preamble, enum mpduType mpdutype);
  virtual void RegisterListener (WifiPhyListener *listener);
  virtual void UnregisterListener (WifiPhyListener *listener);
  virtual void SetSleepMode (void);
  virtual void ResumeFromSleep (void);
  virtual Ptr<WifiChannel> GetChannel (void) const;

protected:
  // Inherited
  virtual void DoDispose (void);
  virtual bool DoChannelSwitch (uint16_t id);
  virtual bool DoFrequencySwitch (uint32_t frequency);

private:
  /**
   * The last bit of the packet has arrived.
   *
   * \param packet the packet that the last bit has arrived
   * \param preamble the preamble of the arriving packet
   * \param mpdutype the type of the MPDU as defined in WifiPhy::mpduType.
   * \param event the corresponding event of the first time the packet arrives
   */
  void EndReceive (Ptr<Packet> packet, enum WifiPreamble preamble, enum mpduType mpdutype, Ptr<InterferenceHelper::Event> event);

  Ptr<HEWifiChannel> m_channel;        //!< HEWifiChannel that this HEWifiPhy is connected to
};

} //namespace ns3

#endif /* HE_WIFI_PHY_H */
