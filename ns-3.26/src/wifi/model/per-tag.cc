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
 * Authors: Bibek Sahu
 *          Balamurugan Ramachandran
 *          Ramachandra Murthy
 *          Mukesh Taneja
 */
 
/* 
 * This file is for OFDMA/802.11ax type of systems. It is not
 * fully compliant to IEEE 802.11ax standards.
 */

#include "per-tag.h"
#include "ns3/tag.h"
#include "ns3/double.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PerTag);

TypeId
PerTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PerTag")
    .SetParent<Tag> ()
    .SetGroupName ("Wifi")
    .AddConstructor<PerTag> ()
    .AddAttribute ("Per", "The per of the last packet received",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&PerTag::Get),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
PerTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

PerTag::PerTag ()
  : m_per (0)
{
}

PerTag::PerTag (double per)
  : m_per (per)
{
}

uint32_t
PerTag::GetSerializedSize (void) const
{
  return sizeof (double);
}

void
PerTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_per);
}

void
PerTag::Deserialize (TagBuffer i)
{
  m_per = i.ReadDouble ();
}

void
PerTag::Print (std::ostream &os) const
{
  os << "Per=" << m_per;
}

void
PerTag::Set (double per)
{
  m_per = per;
}

double
PerTag::Get (void) const
{
  return m_per;
}

}
