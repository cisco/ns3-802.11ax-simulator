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

#ifndef PER_TAG_H
#define PER_TAG_H

#include "ns3/packet.h"

namespace ns3 {

class Tag;

class PerTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Create a SnrTag with the default snr 0
   */
  PerTag ();

  /**
   * Create a SnrTag with the given snr value
   * \param snr the given SNR value
   */
  PerTag (double snr);

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  /**
   * Set the SNR to the given value.   *
   * \param snr the value of the snr to set
   */
  void Set (double per);
  /**
   * Return the SNR value.
   *
   * \return the SNR value
   */
  double Get (void) const;


private:
  double m_per;  //!< SNR value
};

}
#endif /* SNR_TAG_H */
