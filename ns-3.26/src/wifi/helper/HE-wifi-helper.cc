/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This file also contains code from the following file(s) from ns-3.26:
 * Filename : wifi-helper.cc
 * Copyright (c) 2008 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Mirko Banchi <mk.banchi@gmail.com>
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

#include "ns3/trace-helper.h"
#include "HE-wifi-helper.h"
#include "ns3/error-rate-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/HE-wifi-channel.h"
#include "ns3/HE-wifi-phy.h"
#include "ns3/wifi-net-device.h"
#include "ns3/names.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HEWifiHelper");

HEWifiChannelHelper::HEWifiChannelHelper ()
{
}

HEWifiChannelHelper
HEWifiChannelHelper::Default (void)
{
  HEWifiChannelHelper helper;
  helper.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  helper.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
  return helper;
}

void
HEWifiChannelHelper::AddPropagationLoss (std::string type,
                                           std::string n0, const AttributeValue &v0,
                                           std::string n1, const AttributeValue &v1,
                                           std::string n2, const AttributeValue &v2,
                                           std::string n3, const AttributeValue &v3,
                                           std::string n4, const AttributeValue &v4,
                                           std::string n5, const AttributeValue &v5,
                                           std::string n6, const AttributeValue &v6,
                                           std::string n7, const AttributeValue &v7)
{
  ObjectFactory factory;
  factory.SetTypeId (type);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_propagationLoss.push_back (factory);
}

void
HEWifiChannelHelper::SetPropagationDelay (std::string type,
                                            std::string n0, const AttributeValue &v0,
                                            std::string n1, const AttributeValue &v1,
                                            std::string n2, const AttributeValue &v2,
                                            std::string n3, const AttributeValue &v3,
                                            std::string n4, const AttributeValue &v4,
                                            std::string n5, const AttributeValue &v5,
                                            std::string n6, const AttributeValue &v6,
                                            std::string n7, const AttributeValue &v7)
{
  ObjectFactory factory;
  factory.SetTypeId (type);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_propagationDelay = factory;
}

Ptr<HEWifiChannel>
HEWifiChannelHelper::Create (void) const
{
  Ptr<HEWifiChannel> channel = CreateObject<HEWifiChannel> ();
  Ptr<PropagationLossModel> prev = 0;
  for (std::vector<ObjectFactory>::const_iterator i = m_propagationLoss.begin (); i != m_propagationLoss.end (); ++i)
    {
      Ptr<PropagationLossModel> cur = (*i).Create<PropagationLossModel> ();
      if (prev != 0)
        {
          prev->SetNext (cur);
        }
      if (m_propagationLoss.begin () == i)
        {
          channel->SetPropagationLossModel (cur);
        }
      prev = cur;
    }
  Ptr<PropagationDelayModel> delay = m_propagationDelay.Create<PropagationDelayModel> ();
  channel->SetPropagationDelayModel (delay);
  return channel;
}

int64_t
HEWifiChannelHelper::AssignStreams (Ptr<HEWifiChannel> c, int64_t stream)
{
  return c->AssignStreams (stream);
}

HEWifiPhyHelper::HEWifiPhyHelper ()
  : m_channel (0)
{
  m_phy.SetTypeId ("ns3::HEWifiPhy");
}

HEWifiPhyHelper
HEWifiPhyHelper::Default (void)
{
  HEWifiPhyHelper helper;
  helper.SetErrorRateModel ("ns3::NistErrorRateModel");
  return helper;
}

void
HEWifiPhyHelper::SetChannel (Ptr<HEWifiChannel> channel)
{
  m_channel = channel;
}

void
HEWifiPhyHelper::SetChannel (std::string channelName)
{
  Ptr<HEWifiChannel> channel = Names::Find<HEWifiChannel> (channelName);
  m_channel = channel;
}

Ptr<WifiPhy>
HEWifiPhyHelper::Create (Ptr<Node> node, Ptr<NetDevice> device) const
{
  Ptr<HEWifiPhy> phy = m_phy.Create<HEWifiPhy> ();
  Ptr<ErrorRateModel> error = m_errorRateModel.Create<ErrorRateModel> ();
  phy->SetErrorRateModel (error);
  phy->SetChannel (m_channel);
  phy->SetDevice (device);
  return phy;
}

} //namespace ns3
