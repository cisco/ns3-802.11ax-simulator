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

#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/enum.h"
#include "ns3/mobility-model.h"
#include <cmath>
#include "ns3/he-bitmap.h"

#include "enterprise-11ax-propagation-loss-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Enterprise11axPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (Enterprise11axPropagationLossModel);

TypeId
Enterprise11axPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Enterprise11axPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Propagation")
    .AddConstructor<Enterprise11axPropagationLossModel> ()
    .AddAttribute ("Frequency",
                   "The propagation frequency in Hz",
                   DoubleValue (2160e6),
                   MakeDoubleAccessor (&Enterprise11axPropagationLossModel::m_frequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("BaseFreq",
                   "2.4/5 in Ghz",
                   DoubleValue (2.4),
                   MakeDoubleAccessor (&Enterprise11axPropagationLossModel::m_baseFreq),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("IndoorWallLoss",
                   "Indoor Wall Loss in dB",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&Enterprise11axPropagationLossModel::m_indoorWallLoss),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ShadowingEnabled",
                   "Enable shadowing",
                   BooleanValue (1),
                   MakeBooleanAccessor (&Enterprise11axPropagationLossModel::m_shadowing),
                   MakeBooleanChecker ())
    .AddAttribute ("ShadowingStandardDeviation",
                   "Standard Deviation for shadowing Loss",
                   DoubleValue (5.0),
                   MakeDoubleAccessor (&Enterprise11axPropagationLossModel::m_shadowingStandardDeviation),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RUBitMap",
                   "BitMap that describes unique ID of RU",
                   IntegerValue (0),
                   MakeIntegerAccessor (&Enterprise11axPropagationLossModel::m_bitMap),
                   MakeIntegerChecker<int> ())
    .AddAttribute ("ChannelNumber",
                   "Channel Number",
                   IntegerValue (1),
                   MakeIntegerAccessor (&Enterprise11axPropagationLossModel::m_channelNumber),
                   MakeIntegerChecker<int> ());
  return tid;
}

Enterprise11axPropagationLossModel::Enterprise11axPropagationLossModel ()
  : PropagationLossModel ()
{
  NS_LOG_DEBUG ("In Enterprise model");
}

Enterprise11axPropagationLossModel::~Enterprise11axPropagationLossModel ()
{
}

double
Enterprise11axPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double loss = 0.0;
  double fGhz = m_frequency / 1e9;
  double shadowingLoss = 0.0;
  double dist = ((a->GetDistanceFrom (b)) > 1 ? (a->GetDistanceFrom (b)) : 1);
  uint32_t W = dist/20;

  if (m_baseFreq == 2.4)
  {
    loss = 40.05 + 20* std::log10(fGhz/m_baseFreq) + 20* std::log10(dist < 12 ? dist : 12) + floor(dist/12)*35* std::log10(dist/12) + m_indoorWallLoss*W;
  }
  else if (m_baseFreq == 5)
  {
    loss = 40.05 + 20* std::log10(fGhz/m_baseFreq) + 20* std::log10(dist < 12 ? dist : 12) + floor(dist/12)*35* std::log10(dist/12) + m_indoorWallLoss*W;
  }
  
  if (m_shadowing == 1)
  {
    shadowingLoss = (-1)*10*std::log10((1/(dist*std::sqrt(2*M_PI)*m_shadowingStandardDeviation))*std::exp((-1)*(std::pow(std::log(dist),2)/(2*std::pow(m_shadowingStandardDeviation,2)))));
  }
  loss = loss + shadowingLoss;

  return loss;
}

double
Enterprise11axPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                        Ptr<MobilityModel> a,
                        Ptr<MobilityModel> b,
                        int BitMap, int ChannelNumber)
{
  SetBitMap(BitMap);
  SetChannelNumber(ChannelNumber);
  m_frequency = CalculateFcFromBitMap();
  double rxPowerDbm = (txPowerDbm - GetLoss (a, b));
  return rxPowerDbm;
}

double
Enterprise11axPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                        Ptr<MobilityModel> a,
                        Ptr<MobilityModel> b) const
{
  double rxPowerDbm = (txPowerDbm - GetLoss (a, b));
  return rxPowerDbm;
}

int64_t
Enterprise11axPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

void
Enterprise11axPropagationLossModel::SetBitMap(int BitMap)
{
  m_bitMap = BitMap;
}

void
Enterprise11axPropagationLossModel::SetChannelNumber(int ChannelNumber)
{
  m_channelNumber = ChannelNumber;
}

double
Enterprise11axPropagationLossModel::CalculateFcFromBitMap(void)
{
  double fc = 0.0;
  HEBitMap BitMap;
  double offset = 0.0;
  fc = BitMap.GetCentralFrequencyFromChannelNumber(m_channelNumber);
  RUInfo RU = {0};
  RU = BitMap.GetRUInfoFromTriggerBitMap(m_bitMap);
  offset = BitMap.GetRUOffset(RU.type, RU.index, m_channelNumber);
  fc = fc + offset;
  return fc;
}

} // namespace ns3
