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

#include <ns3/simple-ref-count.h>
#include <vector>

#ifndef HEBITMAP_H
#define HEBITMAP_H

namespace ns3 {

struct MuPPDUBitMapTable{
  unsigned char table[39][9];
};

struct RUInfo{
  int type;
  int index;
};

class RUData : public SimpleRefCount <RUData>
{
public:
  RUData();
  
  ~RUData();

  void SetBitMap(uint8_t bitMap);
  uint8_t GetBitMap(void);
  void SetCentralFrequency(double freq);
  double GetCentralFrequency(void);
  void SetChannelWidth(double chanWidth);
  double GetChannelWidth(void);
  void SetNumberOfMimoUsers(uint8_t mimoUsers);
  uint8_t GetNumberOfMimoUsers(void);
  
private:
  uint8_t m_bitMap;
  double  m_centralFrequency;
  double  m_channelWidth;
  uint8_t m_mimoUsers;
};

typedef struct heMcsInfo_ {
  uint32_t mcs;
  uint32_t bufValue;
} heMcsInfo_t;

typedef std::vector<uint8_t> mapIndexVector;
typedef std::vector<Ptr<RUData>> ruVector;
typedef std::vector<double> bufIndexVector;
typedef std::vector<uint32_t> ruTypeVector;

class HEBitMap : public Object
{
public:
  static TypeId GetTypeId (void);

  HEBitMap();

  ~HEBitMap();

  void HEMuPPDUConstuctTable(struct MuPPDUBitMapTable *Table);

  unsigned char GetIndexFromBitMap(int BitMapValue);

  void GetRUDistFromBitMap(unsigned char *row, int BitMapValue);

  RUInfo GetRUInfoFromTriggerBitMap(int BitMapValue);
  
  RUData GetRUDataFromBitMap(uint8_t BitMapValue, int channelNumber);
  
  uint8_t GetBitMapFromRUInfo(struct RUInfo RU);

  double GetRUOffset(int RUtype, int RUindex, int channelNumber);

  void CalculateFcFromRUDist(double *Fc, unsigned char *RUrow);

  double GetCentralFrequencyFromChannelNumber2_4GHz20MHz(int channelNumber);

  double GetCentralFrequencyFromChannelNumber(int channelNumber);

  ruVector GetRuVectorFromRuBitMap(uint8_t BitMap);

  double GetDataRate(uint32_t mcsVal, uint32_t chanW);

  mapIndexVector GetMapVectorFromUserCount(int numUsers, bool isMimo, int numMimoUsers);
 
  static bool SortRUData(Ptr<RUData> ru1, Ptr<RUData> ru2);

  void GetBitMapAll(std::vector<uint32_t> &bitmapPtr);
  void GetBitMap20(std::vector<uint32_t> &bitmapPtr);

  int m_DataBitMap;
  RUInfo m_RU;
  int m_TriggerBitMap;

private:
  MuPPDUBitMapTable m_RUDistTable;
};

}
#endif // HEBITMAP_H
