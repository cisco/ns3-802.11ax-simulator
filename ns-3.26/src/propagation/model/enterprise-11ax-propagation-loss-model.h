/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "ns3/he-bitmap.h"

#ifndef ENTERPRISE_11AX_PROPAGATION_LOSS_MODEL_H
#define ENTERPRISE_11AX_PROPAGATION_LOSS_MODEL_H

#include <ns3/propagation-loss-model.h>

namespace ns3 {

class Enterprise11axPropagationLossModel : public PropagationLossModel
{

public:
  static TypeId GetTypeId (void);

  Enterprise11axPropagationLossModel ();
  virtual ~Enterprise11axPropagationLossModel ();

  double GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  void SetBitMap(int BitMap);
  void SetChannelNumber(int ChannelNumber);
  double CalculateFcFromBitMap(void);

private:
  Enterprise11axPropagationLossModel (const Enterprise11axPropagationLossModel &);
  Enterprise11axPropagationLossModel & operator = (const Enterprise11axPropagationLossModel &);

  virtual double DoCalcRxPower (double txPowerDbm,
                                Ptr<MobilityModel> a,
                                Ptr<MobilityModel> b) const;
  virtual double DoCalcRxPower (double txPowerDbm,
                                Ptr<MobilityModel> a,
                                Ptr<MobilityModel> b,
                                int BitMap, int ChannelNumber);
  virtual int64_t DoAssignStreams (int64_t stream);

  HEBitMap BitMap;
  double m_frequency;
  double m_baseFreq;
  double m_indoorWallLoss;
  bool m_shadowing;
  double m_shadowingStandardDeviation;
  int m_bitMap;
  int m_channelNumber;
};

} // namespace ns3

#endif // ENTERPRISE_11AX_PROPAGATION_LOSS_MODEL_H
