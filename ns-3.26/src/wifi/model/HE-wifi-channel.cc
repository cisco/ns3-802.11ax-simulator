/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This file also contains code from the following file(s) from ns-3.26:
 * Filename : wifi-channel.cc
 * Copyright (c) 2006,2007 INRIA
 * Authors: Mathieu Lacage, <mathieu.lacage@sophia.inria.fr>
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

#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "HE-wifi-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HEWifiChannel");

NS_OBJECT_ENSURE_REGISTERED (HEWifiChannel);

TypeId
HEWifiChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::HEWifiChannel")
    .SetParent<WifiChannel> ()
    .SetGroupName ("Wifi")
    .AddConstructor<HEWifiChannel> ()
    .AddAttribute ("PropagationLossModel", "A pointer to the propagation loss model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&HEWifiChannel::m_loss),
                   MakePointerChecker<PropagationLossModel> ())
    .AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&HEWifiChannel::m_delay),
                   MakePointerChecker<PropagationDelayModel> ())
  ;
  return tid;
}

HEWifiChannel::HEWifiChannel ()
{
}

HEWifiChannel::~HEWifiChannel ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_phyList.clear ();
}

void
HEWifiChannel::SetPropagationLossModel (Ptr<PropagationLossModel> loss)
{
  m_loss = loss;
}

void
HEWifiChannel::SetPropagationDelayModel (Ptr<PropagationDelayModel> delay)
{
  m_delay = delay;
}

void
HEWifiChannel::Send (Ptr<HEWifiPhy> sender, Ptr<const Packet> packet, double txPowerDbm,
                       WifiTxVector txVector, WifiPreamble preamble, enum mpduType mpdutype, Time duration) const
{
  Ptr<MobilityModel> senderMobility = sender->GetMobility ()->GetObject<MobilityModel> ();
  NS_ASSERT (senderMobility != 0);
  uint32_t j = 0;
  for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++, j++)
    {
      if (sender != (*i))
        {
          //For now don't account for inter channel interference
          if ((*i)->GetChannelNumber () != sender->GetChannelNumber ())
            {
              continue;
            }

          Ptr<MobilityModel> receiverMobility = (*i)->GetMobility ()->GetObject<MobilityModel> ();
          Time delay = m_delay->GetDelay (senderMobility, receiverMobility);
 
          double rxPowerDbm = m_loss->CalcRxPower (txPowerDbm, senderMobility, receiverMobility, txVector.GetRu(), (*i)->GetChannelNumber());
          NS_LOG_DEBUG ("propagation: txPower=" << txPowerDbm << "dbm, rxPower=" << rxPowerDbm << "dbm, " <<
                        "distance=" << senderMobility->GetDistanceFrom (receiverMobility) << "m, delay=" << delay);
          Ptr<Packet> copy = packet->Copy ();
          Ptr<Object> dstNetDevice = m_phyList[j]->GetDevice ();
          uint32_t dstNode;
          if (dstNetDevice == 0)
            {
              dstNode = 0xffffffff;
            }
          else
            {
              dstNode = dstNetDevice->GetObject<NetDevice> ()->GetNode ()->GetId ();
            }

          struct HeParameters parameters;
          parameters.rxPowerDbm = rxPowerDbm;
          parameters.type = mpdutype;
          parameters.duration = duration;
          parameters.txVector = txVector;
          parameters.preamble = preamble;

          Simulator::ScheduleWithContext (dstNode,
                                          delay, &HEWifiChannel::Receive, this,
                                          j, copy, parameters);
        }
    }
}

void
HEWifiChannel::Receive (uint32_t i, Ptr<Packet> packet, struct HeParameters parameters) const
{
  m_phyList[i]->StartReceivePreambleAndHeader (packet, parameters.rxPowerDbm, parameters.txVector, parameters.preamble, parameters.type, parameters.duration);
}

uint32_t
HEWifiChannel::GetNDevices (void) const
{
  return m_phyList.size ();
}

Ptr<NetDevice>
HEWifiChannel::GetDevice (uint32_t i) const
{
  return m_phyList[i]->GetDevice ()->GetObject<NetDevice> ();
}

void
HEWifiChannel::Add (Ptr<HEWifiPhy> phy)
{
  m_phyList.push_back (phy);
}

int64_t
HEWifiChannel::AssignStreams (int64_t stream)
{
  int64_t currentStream = stream;
  currentStream += m_loss->AssignStreams (stream);
  return (currentStream - stream);
}

} //namespace ns3
