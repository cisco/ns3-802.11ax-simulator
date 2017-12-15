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

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include <map>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "he-bitmap.h"

#define LOWER_FREQ_2_4GHZ 2401

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HEBitMap");

NS_OBJECT_ENSURE_REGISTERED (HEBitMap);

TypeId
HEBitMap::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::HEBitMap")
    .SetParent<Object> ()
    .SetGroupName ("Propagation")
    .AddConstructor<HEBitMap> ()
    .AddAttribute ("DataBitMap",
                   "8 bit BitMap of MU PPDU RU distribution",
                   IntegerValue (0),
                   MakeIntegerAccessor (&HEBitMap::m_DataBitMap),
                   MakeIntegerChecker<int>())
    .AddAttribute ("TriggerBitMap",
                   "8 bit BitMap of RUInfo in trigger frame",
                   IntegerValue (0),
                   MakeIntegerAccessor (&HEBitMap::m_TriggerBitMap),
                   MakeIntegerChecker<int>());
  return tid;
}

HEBitMap::HEBitMap ()
{
  NS_LOG_FUNCTION (this);
  HEMuPPDUConstuctTable(&m_RUDistTable);
}

HEBitMap::~HEBitMap ()
{
  NS_LOG_FUNCTION (this);
}

double
HEBitMap::GetDataRate(uint32_t mcsVal, uint32_t chanW)
{
  double dataRate = 0.0;
  if (chanW == 2)
  {
    switch(mcsVal){
      case 0:
        dataRate = 900000;
        break;
      case 1:
        dataRate = 1800000;
        break;
      case 2:
        dataRate = 2600000;
        break;
      case 3:
        dataRate = 3500000;
        break;
      case 4:
        dataRate = 5300000;
        break;
      case 5:
        dataRate = 7100000;
        break;
      case 6:
        dataRate = 7900000;
        break;
      case 7:
        dataRate = 8800000;
        break;
      case 8:
        dataRate = 10600000;
        break;
      case 9:
        dataRate = 11800000;
        break;
      default:
        break;
    }
  }
  else if (chanW == 4)
  {
    switch(mcsVal){
      case 0:
        dataRate = 1800000;
        break;
      case 1:
        dataRate = 3500000;
        break;
      case 2:
        dataRate = 5300000;
        break;
      case 3:
        dataRate = 7100000;
        break;
      case 4:
        dataRate = 10600000;
        break;
      case 5:
        dataRate = 14100000;
        break;
      case 6:
        dataRate = 15900000;
        break;
      case 7:
        dataRate = 17600000;
        break;
      case 8:
        dataRate = 21200000;
        break;
      case 9:
        dataRate = 23500000;
        break;
      default:
        break;
    }
  }
  else if (chanW == 8)
  {
    switch(mcsVal){
      case 0:
        dataRate = 3800000;
        break;
      case 1:
        dataRate = 7500000;
        break;
      case 2:
        dataRate = 11300000;
        break;
      case 3:
        dataRate = 15000000;
        break;
      case 4:
        dataRate = 22500000;
        break;
      case 5:
        dataRate = 30000000;
        break;
      case 6:
        dataRate = 33800000;
        break;
      case 7:
        dataRate = 37500000;
        break;
      case 8:
        dataRate = 45000000;
        break;
      case 9:
        dataRate = 50000000;
        break;
      default:
        break;
    }
  }
  else if (chanW == 20)
  {
    switch(mcsVal){
      case 0:
        dataRate = 8600000;
        break;
      case 1:
        dataRate = 17200000;
        break;
      case 2:
        dataRate = 25800000;
        break;
      case 3:
        dataRate = 34400000;
        break;
      case 4:
        dataRate = 51600000;
        break;
      case 5:
        dataRate = 68800000;
        break;
      case 6:
        dataRate = 77400000;
        break;
      case 7:
        dataRate = 86000000;
        break;
      case 8:
        dataRate = 103200000;
        break;
      case 9:
        dataRate = 114700000;
        break;
      case 10:
        dataRate = 129000000;
        break;
      case 11:
        dataRate = 143400000;
        break;
      default:
        break;
    }
  }
  else if (chanW == 40)
  {
    switch(mcsVal){
      case 0:
        dataRate = 17200000;
        break;
      case 1:
        dataRate = 34400000;
        break;
      case 2:
        dataRate = 51600000;
        break;
      case 3:
        dataRate = 68800000;
        break;
      case 4:
        dataRate = 103200000;
        break;
      case 5:
        dataRate = 137600000;
        break;
      case 6:
        dataRate = 154900000;
        break;
      case 7:
        dataRate = 172100000;
        break;
      case 8:
        dataRate = 206500000;
        break;
      case 9:
        dataRate = 229400000;
        break;
      case 10:
        dataRate = 258100000;
        break;
      case 11:
        dataRate = 286800000;
        break;
      default:
        break;
    }
  }
  else if (chanW == 80)
  {
    switch(mcsVal){
      case 0:
        dataRate = 36000000;
        break;
      case 1:
        dataRate = 72100000;
        break;
      case 2:
        dataRate = 108100000;
        break;
      case 3:
        dataRate = 144100000;
        break;
      case 4:
        dataRate = 216200000;
        break;
      case 5:
        dataRate = 288200000;
        break;
      case 6:
        dataRate = 324300000;
        break;
      case 7:
        dataRate = 360300000;
        break;
      case 8:
        dataRate = 432400000;
        break;
      case 9:
        dataRate = 480400000;
        break;
      case 10:
        dataRate = 540400000;
        break;
      case 11:
        dataRate = 600400000;
        break;
      default:
        break;
    }
  }
  else if (chanW == 160)
  {
    switch(mcsVal){
      case 0:
        dataRate = 72100000;
        break;
      case 1:
        dataRate = 144100000;
        break;
      case 2:
        dataRate = 216200000;
        break;
      case 3:
        dataRate = 288200000;
        break;
      case 4:
        dataRate = 432400000;
        break;
      case 5:
        dataRate = 576500000;
        break;
      case 6:
        dataRate = 648500000;
        break;
      case 7:
        dataRate = 720600000;
        break;
      case 8:
        dataRate = 864700000;
        break;
      case 9:
        dataRate = 960700000;
        break;
      case 10:
        dataRate = 1020800000;
        break;
      case 11:
        dataRate = 1134200000;
        break;
      default:
        break;
    }
  }
  return dataRate;
}

void HEBitMap::HEMuPPDUConstuctTable(struct MuPPDUBitMapTable *RU)
{
  /* BitMap Table
  an array RU->table[39][9] corresponding to 39 rows (bitmaps) and 9 RUs for a 20 Mhz Band
  if RU[i][j] = {0	-	Free
    1	-	RU of 26 subcarriers
    2	-	RU of 52 subcarriers
    3   - 	RU of 106 subcarriers
    4   - 	RU of 242 subcarriers
    5   - 	RU of 484 subcarriers
    6   - 	RU of 996 subcarriers
    7   - 	RU of 2x996 subcarriers}
	(i,j = row and column index)
*/
	
  unsigned char row=0, column=0;

  for (row = 0; row < 39; row++)
  {
    for (column = 0; column < 9; column++)
    {
      if (row < 16)
      {
        if (column == 4)
          RU->table[row][column] = 1;
        else if (column == 7 || column == 8)
          RU->table[row][column] = (row%2) + 1;
        else if (column == 5 || column == 6)
          RU->table[row][column] = floor((row%4)/2) + 1;
        else if (column == 2 || column == 3)
          RU->table[row][column] = floor((row%8)/4) + 1;
        else // column = 0 & 1
          RU->table[row][column] = floor(row/8) + 1;
      }
    else if (row == 16 || row == 17)
    {
      if (column == 4)
        RU->table[row][column] = 0;
      else if (column < 4)
        row == 16 ? RU->table[row][column] = 2 : RU->table[row][column] = 3;
      else // column > 4
        row == 16 ? RU->table[row][column] = 3 : RU->table[row][column] = 2;
    }
    else if (row < 26)
    {
      if (column == 4)
        RU->table[row][column] = 1;
      else if (((column < 4) && ((row-18) > 3)) || ((column > 4) && ((row-18) < 4)))
        RU->table[row][column] = 3;
      else if (column == 2 || column == 3 || column == 7 || column == 8)
        RU->table[row][column] = (row-18)%2 + 1;
      else
        RU->table[row][column] = floor(((row-18)%4)/2) + 1;
    }
    else if (row == 26 || row == 27)
    {
      if (column == 4)
        RU->table[row][column] = 0;
      else
        row == 26 ? RU->table[row][column] = 3 : RU->table[row][column] = 2;
    }
    else if (row == 28 || row == 29 || row == 30)
      RU->table[row][column] = 104 + (row-28);
    else if (row == 31 || row == 32 || row == 38)
      RU->table[row][column] = 255;
    else if (row == 33)
    {
      if (column == 4)
        RU->table[row][column] = 1;
      else
        RU->table[row][column] = 3;
    }
    else if (row > 33 && row < 38)
      RU->table[row][column] = 3 + (row - 33);		
    }
  } 
}

unsigned char HEBitMap::GetIndexFromBitMap(int BitMap)
{
  unsigned char index=0;
  if (BitMap < 16)
    index = BitMap;
  else if (floor(BitMap/8) > 1 && floor(BitMap/8) < 12)
    index = 14 + floor(BitMap/8);
  else if (floor(BitMap/16) == 6)
   index = 26;
  else if (BitMap > 111 && BitMap < 116)
    index = 26 + (BitMap - 111);
  else if (floor(BitMap/4) == 29 || floor(BitMap/4) == 31)
    (floor(BitMap/4) == 29) ? index = 31 : index =32;
  else if (floor(BitMap/64) == 2)
    index = 33;
  else if (floor(BitMap/8) > 23 && floor(BitMap/8) < 28)
    index = 34 + (floor(BitMap/8) - 24);
  else
    index = 38;

  return index;
}

void HEBitMap::GetRUDistFromBitMap(unsigned char *RUrow, int BitMap)
{
  unsigned char index = GetIndexFromBitMap(BitMap);
  unsigned char i;
  for (i=0; i<9; i++)
  {
    RUrow[i] = m_RUDistTable.table[index][i];
  }
}

void
HEBitMap::GetBitMap20(std::vector<uint32_t> &bitmapPtr) 
{
  struct RUInfo RU;
  uint8_t bitmap = 0, i = 0;

  RU.type = 1;
  RU.index = 0;

  for(i = 0; i < 9; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU);
      bitmapPtr.push_back(bitmap);
  }

  RU.type = 2;
  RU.index = 0;

  for(i = 0; i < 4; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU);
      bitmapPtr.push_back(bitmap);
  }

  RU.type = 3;
  RU.index = 0;

  for(i = 0; i < 2; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU);
      bitmapPtr.push_back(bitmap);
  }

  RU.type = 4;
  RU.index = 0;

  for(i = 0; i < 1; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU);
      bitmapPtr.push_back(bitmap);
  }
}

void
HEBitMap::GetBitMapAll(std::vector<uint32_t> &bitmapPtr) 
{
  struct RUInfo RU;
  uint8_t bitmap = 0, i = 0;

  RU.type = 1;
  RU.index = 0;

  for(i = 0; i < 37; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU);
      bitmapPtr.push_back(bitmap);
  }
  for(i = 0; i < 37; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU)+1;
      bitmapPtr.push_back(bitmap);
  }

  RU.type = 2;
  RU.index = 0;

  for(i = 0; i < 16; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU);
      bitmapPtr.push_back(bitmap);
  }
  for(i = 0; i < 16; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU)+1;
      bitmapPtr.push_back(bitmap);
  }

  RU.type = 3;
  RU.index = 0;

  for(i = 0; i < 8; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU);
      bitmapPtr.push_back(bitmap);
  }
  for(i = 0; i < 8; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU)+1;
      bitmapPtr.push_back(bitmap);
  }

  RU.type = 4;
  RU.index = 0;

  for(i = 0; i < 4; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU);
      bitmapPtr.push_back(bitmap);
  }
  for(i = 0; i < 4; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU)+1;
      bitmapPtr.push_back(bitmap);
  }

  RU.type = 5;
  RU.index = 0;

  for(i = 0; i < 2; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU);
      bitmapPtr.push_back(bitmap);
  }
  for(i = 0; i < 2; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU)+1;
      bitmapPtr.push_back(bitmap);
  }

  RU.type = 6;
  RU.index = 0;

  for(i = 0; i < 1; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU);
      bitmapPtr.push_back(bitmap);
  }
  for(i = 0; i < 1; i ++) {
      RU.index = i;
      bitmap = GetBitMapFromRUInfo(RU)+1;
      bitmapPtr.push_back(bitmap);
  }

  RU.type = 7;
  RU.index = 0;

  bitmap = GetBitMapFromRUInfo(RU);
  bitmapPtr.push_back(bitmap);

}

uint8_t HEBitMap::GetBitMapFromRUInfo(struct RUInfo RU){
  uint8_t BitMap=0;
  if(RU.type == 1)
    BitMap = RU.index;
  else if (RU.type == 2)
    BitMap = 37+RU.index;
  else if (RU.type == 3)
    BitMap = 53+RU.index;
  else if (RU.type == 4)
    BitMap = 61+RU.index;
  else if (RU.type == 5)
    BitMap = 65+RU.index;
  else if (RU.type == 6)
    BitMap = 67+RU.index;
  else if (RU.type == 7){
    BitMap = 137;
    goto End;
  }
  BitMap = 2*BitMap;

End:
  return BitMap;
}

RUData 
HEBitMap::GetRUDataFromBitMap(uint8_t TriggerBitMap, int channelNum)
{
  int BitMap = 0;
  RUData RU;
  RU.SetBitMap(TriggerBitMap);
  if (TriggerBitMap == 137)
  {
    RU.SetChannelWidth(160);
    RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(channelNum)+GetRUOffset(7, 0, channelNum));
    RU.SetNumberOfMimoUsers(0);
    return RU;
  }
  if (TriggerBitMap%2 == 1 && ((channelNum == 50 || channelNum == 114)))
  {
    int virtualChanN = channelNum;
    if (channelNum == 50)
      virtualChanN = 58;
    else if (channelNum == 114)
      virtualChanN =122;
    BitMap = floor(TriggerBitMap/2);
    if (BitMap >= 0 && BitMap < 37)
    {
      RU.SetChannelWidth(2);
      RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(virtualChanN)+GetRUOffset(1, BitMap, virtualChanN));
      RU.SetNumberOfMimoUsers(0);
    }
    else if (BitMap > 36 && BitMap <= 52)
    {
      RU.SetChannelWidth(4);
      RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(virtualChanN)+GetRUOffset(2, BitMap-37, virtualChanN));
      RU.SetNumberOfMimoUsers(0);
    }
    else if (BitMap > 52 && BitMap <= 60)
    {
      RU.SetChannelWidth(8);
      RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(virtualChanN)+GetRUOffset(3, BitMap-53, virtualChanN));
      RU.SetNumberOfMimoUsers(0);
    }
    else if (BitMap > 60 && BitMap <= 64)
    {
      RU.SetChannelWidth(20);
      RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(virtualChanN)+GetRUOffset(4, BitMap-61, virtualChanN));
      RU.SetNumberOfMimoUsers(0);
    }
    else if (BitMap > 64 && BitMap <= 66)
    {
      RU.SetChannelWidth(40);
      RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(virtualChanN)+GetRUOffset(5, BitMap-65, virtualChanN));
      RU.SetNumberOfMimoUsers(0);
    }
    else if (BitMap == 67 || BitMap == 68)
    {
      RU.SetChannelWidth(80);
      RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(virtualChanN)+GetRUOffset(6, BitMap-67, virtualChanN));
      RU.SetNumberOfMimoUsers(0);
    }
    return RU;
  }
  BitMap = floor(TriggerBitMap/2);
  if (BitMap >= 0 && BitMap < 37)
  {
    RU.SetChannelWidth(2);
    RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(channelNum)+GetRUOffset(1, BitMap, channelNum));
    RU.SetNumberOfMimoUsers(0);
  }
  else if (BitMap > 36 && BitMap <= 52)
  {
    RU.SetChannelWidth(4);
    RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(channelNum)+GetRUOffset(2, BitMap-37, channelNum));
    RU.SetNumberOfMimoUsers(0);
  }
  else if (BitMap > 52 && BitMap <= 60)
  {
    RU.SetChannelWidth(8);
    RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(channelNum)+GetRUOffset(3, BitMap-53, channelNum));
    RU.SetNumberOfMimoUsers(0);
  }
  else if (BitMap > 60 && BitMap <= 64)
  {
    RU.SetChannelWidth(20);
    RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(channelNum)+GetRUOffset(4, BitMap-61, channelNum));
    RU.SetNumberOfMimoUsers(0);
  }
  else if (BitMap > 64 && BitMap <= 66)
  {
    RU.SetChannelWidth(40);
    RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(channelNum)+GetRUOffset(5, BitMap-65, channelNum));
    RU.SetNumberOfMimoUsers(0);
  }
  else if (BitMap == 67 || BitMap == 68)
  {
    RU.SetChannelWidth(80);
    RU.SetCentralFrequency(GetCentralFrequencyFromChannelNumber(channelNum)+GetRUOffset(6, BitMap-67, channelNum));
    RU.SetNumberOfMimoUsers(0);
  }
  else
  {
    NS_LOG_ERROR("Not a valid bitmap");
  }

  return RU;
}

RUInfo HEBitMap::GetRUInfoFromTriggerBitMap(int TriggerBitMap)
{
  int BitMap = 0;
  struct RUInfo RU;
  if (TriggerBitMap == 137)
  {
    RU.type = 7;
    RU.index = 0;
    return RU;
  }
  BitMap = floor(TriggerBitMap/2);
  if (BitMap >= 0 && BitMap < 37)
  {
    RU.type = 1;
    RU.index = BitMap;
  }
  else if (BitMap > 36 && BitMap < 53)
  {
    RU.type = 2;
    RU.index = BitMap-37;
  }
  else if (BitMap > 52 && BitMap < 61)
  {
    RU.type = 3;
    RU.index = BitMap-53;
  }
  else if (BitMap > 60 && BitMap < 65)
  {
    RU.type = 4;
    RU.index = BitMap-61;
  }
  else if (BitMap == 65 || BitMap == 66)
  {
    RU.type = 5;
    RU.index = BitMap-65;
  }
  else if (BitMap == 67)
  {
    RU.type = 6;
    RU.index = BitMap-67;
  }
  else
  {
    RU.type = 0;
    RU.index = -1;
  }

  return RU;
}

bool
HEBitMap::SortRUData(Ptr<RUData> ru1, Ptr<RUData> ru2)
{
  return ru1->GetChannelWidth() > ru2->GetChannelWidth();
}

ruVector
HEBitMap::GetRuVectorFromRuBitMap(uint8_t BitMap)
{
  ruVector ruVec;
  RUInfo ruInfo;
  double chanWidth = 0.0;
  uint8_t numMimoUsers = 1;
  unsigned char ruDist[9];
  GetRUDistFromBitMap(ruDist, BitMap);
  int index = 0;
  bool flag = true;
  if (BitMap == 200)
  {
    Ptr<RUData> ruData = Create<RUData> ();
    ruData->SetBitMap(130);
    ruData->SetCentralFrequency(GetCentralFrequencyFromChannelNumber(38)+GetRUOffset(ruInfo.type, ruInfo.index, 38));
    ruData->SetChannelWidth(40);
    ruData->SetNumberOfMimoUsers(numMimoUsers);
    ruVec.push_back(ruData);
    goto End;
  }
  else if (BitMap == 208)
  {
    Ptr<RUData> ruData = Create<RUData> ();
    ruData->SetBitMap(134);
    ruData->SetCentralFrequency(GetCentralFrequencyFromChannelNumber(42)+GetRUOffset(ruInfo.type, ruInfo.index, 42));
    ruData->SetChannelWidth(80);
    ruData->SetNumberOfMimoUsers(numMimoUsers);
    ruVec.push_back(ruData);
    goto End;
  }
  else if (BitMap == 216)
  {
    Ptr<RUData> ruData = Create<RUData> ();
    ruData->SetBitMap(137);
    ruData->SetCentralFrequency(GetCentralFrequencyFromChannelNumber(50)+GetRUOffset(ruInfo.type, ruInfo.index, 50));
    ruData->SetChannelWidth(160);
    ruData->SetNumberOfMimoUsers(numMimoUsers);
    ruVec.push_back(ruData);
    goto End;
  }
  for (index = 0;index < 9 && flag;)
  {
    ruInfo.type = ruDist[index];
    if (ruInfo.type == 1){
      ruInfo.index = index;
      chanWidth = 2.0;
      index++;
    }
    else if (ruInfo.type == 2){
      int i = (index > 4) ? ( 2 + (index - 5)/2 ) : ( index/2 );
      ruInfo.index = i;
      chanWidth = 4.0;
      index = index+2;
    }
    else if (ruInfo.type == 3){
      ruInfo.index = (index > 4) ? 1 : 0;
      chanWidth = 8.0;
      index = index+4;
    }
    else if (ruInfo.type == 4){
      ruInfo.index = 0;
      chanWidth = 20.0;
      index = index+9;
    }
    else{
    index++;
    flag = false;
    }
      
    if(flag){
      Ptr<RUData> ruData = Create<RUData> ();
      ruData->SetBitMap(GetBitMapFromRUInfo(ruInfo));
      ruData->SetCentralFrequency(GetCentralFrequencyFromChannelNumber(1)+GetRUOffset(ruInfo.type, ruInfo.index, 1));
      ruData->SetChannelWidth(chanWidth);
      ruData->SetNumberOfMimoUsers(numMimoUsers);
      ruVec.push_back(ruData);
    }
    flag = true;
  }
End:
  std::sort(ruVec.begin(),ruVec.end(),SortRUData);
  return ruVec;
}

double HEBitMap::GetRUOffset(int RUtype, int RUindex, int channelNumber)
{
  double offset = 0.0;
  int RUoff26[9] = {-108, -82, -55, -29, 0, 29, 55, 82, 108};
  int RUoff52[4] = {-95, -42, 42, 95};
  int RUoff106[2] = {-69, 69};
  double subcarrierSpacing = (double)(20e6/256);
  if (channelNumber%4 == 0 || (channelNumber >= 1 && channelNumber <= 11))
  {
    // 20Mhz case
    if (RUtype == 1)
      offset = RUoff26[RUindex]*subcarrierSpacing;
    else if (RUtype == 2)
      offset = RUoff52[RUindex]*subcarrierSpacing;
    else if (RUtype == 3)
      offset = RUoff106[RUindex]*subcarrierSpacing;
    else if (RUtype == 4)
      offset = 0;
  }
  else if (channelNumber == 50 || channelNumber == 114)
  {
    // 160Mhz case :: taken care in caller function by changing channel number for 2nd half 80Mhz
  }
  else if (channelNumber == 42 || channelNumber == 58 || channelNumber == 106 || channelNumber == 122)
  {
    // 80Mhz case
    if (RUtype == 1){
      if(RUindex/9 == 0){
        offset = (RUoff26[RUindex]-256-128)*subcarrierSpacing;
      }
      else if(RUindex/9 == 1){
        offset = (RUoff26[RUindex-9]-128)*subcarrierSpacing;
      }
      else if(RUindex == 18)
      {
        offset = 0;
      }
      else if((RUindex >= 19)&&(RUindex <= 27)){
        offset = (RUoff26[RUindex-19]+128)*subcarrierSpacing;
      }
      else if((RUindex >= 19)&&(RUindex <= 27)){
        offset = (RUoff26[RUindex-28]+128+256)*subcarrierSpacing;
      }
    }
    else if (RUtype == 2){
      if(RUindex/4 == 0){
        offset = (RUoff52[RUindex]-256-128)*subcarrierSpacing;
      }
      else if(RUindex/4 == 1){
        offset = (RUoff52[RUindex-4]-128)*subcarrierSpacing;
      }
      else if(RUindex/4 == 2){
        offset = (RUoff52[RUindex-8]+128)*subcarrierSpacing;
      }
      else if(RUindex/4 == 3){
        offset = (RUoff52[RUindex-12]+128+256)*subcarrierSpacing;
      }
    }
    else if (RUtype == 3){
      if(RUindex/2 == 0){
        offset = (RUoff106[RUindex]-128-256)*subcarrierSpacing;
      }
      else if(RUindex/2 == 1){
        offset = (RUoff106[RUindex-2]-128)*subcarrierSpacing;
      }
      else if(RUindex/2 == 2){
        offset = (RUoff106[RUindex-4]+128)*subcarrierSpacing;
      }
      else if(RUindex/2 == 3){
        offset = (RUoff106[RUindex-6]+128+256)*subcarrierSpacing;
      }
    }
    else if (RUtype == 4){
      if(RUindex == 0){
        offset = (-128-256)*subcarrierSpacing;
      }
      else if(RUindex == 1){
        offset = (-128)*subcarrierSpacing;
      }
      else if(RUindex == 2){
        offset = (128)*subcarrierSpacing;
      }
      else if(RUindex == 3){
        offset = (128+256)*subcarrierSpacing;
      }
    }
    else if (RUtype == 5){
      if(RUindex == 0){
        offset = (-256)*subcarrierSpacing;
      }
      else if(RUindex == 1){
        offset = (256)*subcarrierSpacing;
      }
    }
    else if (RUtype == 6){
      offset = 0;
    }
  }
  else
  {
    // 40Mhz case
    if (RUtype == 1){
      if(RUindex/9 == 0){
        offset = (RUoff26[RUindex]-128)*subcarrierSpacing;
      }
      else if(RUindex/9 == 1){
        offset = (RUoff26[RUindex-9]+128)*subcarrierSpacing;
      }
    }
    else if (RUtype == 2){
      if(RUindex/4 == 0){
        offset = (RUoff52[RUindex]-128)*subcarrierSpacing;
      }
      else if(RUindex/4 == 1){
        offset = (RUoff52[RUindex-4]+128)*subcarrierSpacing;
      }
    }
    else if (RUtype == 3){
      if(RUindex/2 == 0){
        offset = (RUoff106[RUindex]-128)*subcarrierSpacing;
      }
      else if(RUindex/2 == 1){
        offset = (RUoff106[RUindex-2]+128)*subcarrierSpacing;
      }
    }
    else if (RUtype == 4){
      if(RUindex == 0){
        offset = (-128)*subcarrierSpacing;
      }
      else if(RUindex == 1){
        offset = (-128)*subcarrierSpacing;
      }
    }
    else if (RUtype == 5){
      offset = 0;
    }
  }

  return offset;
}

void HEBitMap::CalculateFcFromRUDist(double *Fc, unsigned char *RUrow)
{
  int index=0, iterator=0, count=0;
  int RUtype = 0, RUindex = 0;
  double frequency = 0.0, centralFreq = 2.412e9, offset = 0.0;
  int chanN = 1;
  for (index = 0;index < 9;)
  {
    count=0;
    if (RUrow[index] == 0)
    {
      Fc[index] = 0.0;
      index++;
    }
    else if (RUrow[index] == 255 || RUrow[index] == 104 || RUrow[index] == 105 || RUrow[index] == 106)
    {
      Fc[index] = (-1)*RUrow[index];
      index++;
    }
    else
    {
      if (RUrow[index] == 1)
        count = 1;
      else if (RUrow[index] == 2)
        count = 2;
      else if (RUrow[index] == 3)
        count = 4;
      else if (RUrow[index] == 4 || RUrow[index] == 5 || RUrow[index] == 6 || RUrow[index] == 7)
        count = 9;
		    		
      RUtype = RUrow[index];
			
      if (RUtype == 1)
        RUindex = index;
      else if (RUtype == 2)
      {
        if (index == 0)
          RUindex = 0;
        else if (index == 2)
          RUindex = 1;
        else if (index == 5)
          RUindex = 2;
        else if (index == 7)
          RUindex = 3;
      }
      else if (RUtype == 3)
      {
        if (index == 0)
          RUindex = 0;
        else if (index == 5)
          RUindex = 1;
      }
      else if (RUtype == 4)
        RUindex = 0;
			
      offset = GetRUOffset(RUtype, RUindex, chanN);
      frequency = centralFreq + offset;
      for (iterator = index;iterator < index+count; iterator++){
        Fc[iterator] = frequency;
      }
      index = index+count;
    }
  }
}

double HEBitMap::GetCentralFrequencyFromChannelNumber2_4GHz20MHz(int channelNumber)
{
  return (LOWER_FREQ_2_4GHZ + 6 + channelNumber*5)*1e6;
}

double HEBitMap::GetCentralFrequencyFromChannelNumber(int channelNumber)
{
  double fc = 0;
  switch (channelNumber){
    case 1:
      fc  = 2412*1e6;
      break;
    case 2:
      fc  = 2417*1e6;
      break;
    case 3:
      fc  = 2422*1e6;
      break;
    case 4:
      fc  = 2427*1e6;
      break;
    case 5:
      fc  = 2432*1e6;
      break;
    case 6:
      fc  = 2437*1e6;
      break;
    case 7:
      fc  = 2442*1e6;
      break;
    case 8:
      fc  = 2447*1e6;
      break;
    case 9:
      fc  = 2452*1e6;
      break;
    case 10:
      fc  = 2457*1e6;
      break;
    case 11:
      fc  = 2462*1e6;
      break;
    case 36:
      fc  = 5180*1e6;
      break;
    case 38:
      fc  = 5190*1e6;
      break;
    case 40:
      fc  = 5200*1e6;
      break;
    case 42:
      fc  = 5210*1e6;
      break;
    case 44:
      fc  = 5220*1e6;
      break;
    case 46:
      fc  = 5230*1e6;
      break;
    case 48:
      fc  = 5240*1e6;
      break;
    case 50:
      fc  = 5250*1e6;
      break;
    case 52:
      fc  = 5260*1e6;
      break;
    case 54:
      fc  = 5270*1e6;
      break;
    case 56:
      fc  = 5280*1e6;
      break;
    case 58:
      fc  = 5290*1e6;
      break;
    case 60:
      fc  = 5300*1e6;
      break;
    case 62:
      fc  = 5310*1e6;
      break;
    case 64:
      fc  = 5320*1e6;
      break;
    case 100:
      fc  = 5500*1e6;
      break;
    case 102:
      fc  = 5510*1e6;
      break;
    case 104:
      fc  = 5520*1e6;
      break;
    case 106:
      fc  = 5530*1e6;
      break;
    case 108:
      fc  = 5540*1e6;
      break;
    case 110:
      fc  = 5550*1e6;
      break;
    case 112:
      fc  = 5560*1e6;
      break;
    case 114:
      fc  = 5570*1e6;
      break;
    case 116:
      fc  = 5580*1e6;
      break;
    case 118:
      fc  = 5590*1e6;
      break;
    case 120:
      fc  = 5600*1e6;
      break;
    case 122:
      fc  = 5610*1e6;
      break;
    case 124:
      fc  = 5620*1e6;
      break;
    case 126:
      fc  = 5630*1e6;
      break;
    case 128:
      fc  = 5640*1e6;
      break;
    case 132:
      fc  = 5660*1e6;
      break;
    case 134:
      fc  = 5670*1e6;
      break;
    case 136:
      fc  = 5680*1e6;
      break;
    case 138:
      fc  = 5690*1e6;
      break;
    case 140:
      fc  = 5700*1e6;
      break;
    case 142:
      fc  = 5710*1e6;
      break;
    case 144:
      fc  = 5720*1e6;
      break;
    default:
      fc  = 2412*1e6;
  }
  return fc;
}

mapIndexVector 
HEBitMap::GetMapVectorFromUserCount(int numUsers, bool isMimo, int numMimoUsers)
{
  mapIndexVector mapVector;
  if (!isMimo){
    if (numUsers == 9)
    {
      mapVector.push_back(0);
      return mapVector;
    } 
    else if (numUsers == 8)
    {
      mapVector.push_back(1);
      mapVector.push_back(2);
      mapVector.push_back(4);
      mapVector.push_back(8);
      return mapVector;
    }
    else if (numUsers == 7)
    {
      mapVector.push_back(3);
      mapVector.push_back(5);
      mapVector.push_back(6);
      mapVector.push_back(9);
      mapVector.push_back(10);
      mapVector.push_back(12);
      return mapVector;
    }
    else if (numUsers == 6)
    {
      mapVector.push_back(7);
      mapVector.push_back(11);
      mapVector.push_back(13);
      mapVector.push_back(14);
      mapVector.push_back(32);
      mapVector.push_back(64);
      return mapVector;
    }
    else if (numUsers == 5)
    {
      mapVector.push_back(15);
      mapVector.push_back(40);
      mapVector.push_back(48);
      mapVector.push_back(72);
      mapVector.push_back(80);
      return mapVector;
    }
    else if (numUsers == 4)
    {
      mapVector.push_back(56);
      mapVector.push_back(88);
      mapVector.push_back(112);
      return mapVector;
    }
    else if (numUsers == 3)
    {
      mapVector.push_back(16);
      mapVector.push_back(24);
      mapVector.push_back(128);
      return mapVector;
    }
    else if (numUsers == 2)
    {
      mapVector.push_back(96);
      return mapVector;
    }
    else if (numUsers == 1)
    {
      mapVector.push_back(192);
      return mapVector;
    }
  }
  else{
    int numSlots = numUsers - numMimoUsers + 1;
    if (numSlots == 6)
    {
      mapVector.push_back(32+numMimoUsers);
      mapVector.push_back(64+numMimoUsers);
      return mapVector;
    }
    else if (numSlots == 5)
    {
      mapVector.push_back(40+numMimoUsers);
      mapVector.push_back(48+numMimoUsers);
      mapVector.push_back(72+numMimoUsers);
      mapVector.push_back(80+numMimoUsers);
      return mapVector;
    }
    else if (numSlots == 4)
    {
      mapVector.push_back(56+numMimoUsers);
      mapVector.push_back(88+numMimoUsers);
      mapVector.push_back(112);
      return mapVector;
    }
    else if (numSlots == 3)
    {
      mapVector.push_back(16+numMimoUsers);
      mapVector.push_back(24+numMimoUsers);
      //mapVector.push_back(128);   Bibek : 2 mimos y2y1y0z2z1z0 need to be taken care of
      return mapVector;
    }
    else if (numSlots == 2)
    {
      //mapVector.push_back(96);    Bibek : 2 mimos y1y0z1z0 need to be taken care of
      return mapVector;
    }
    else if (numSlots == 1)
    {
      mapVector.push_back(192+numMimoUsers);
      return mapVector;
    }
  }
  return mapVector;
}

RUData::RUData()
{
  NS_LOG_FUNCTION (this);
}

RUData::~RUData()
{
  NS_LOG_FUNCTION (this);
}

void
RUData::SetBitMap(uint8_t bitMap)
{
  m_bitMap = bitMap;
}

uint8_t 
RUData::GetBitMap(void )
{
  return m_bitMap;
}

void
RUData::SetCentralFrequency(double freq)
{
  m_centralFrequency = freq;
}

double
RUData::GetCentralFrequency(void )
{
  return m_centralFrequency;
}

void
RUData::SetChannelWidth(double chanWidth)
{
  m_channelWidth = chanWidth;
}

double
RUData::GetChannelWidth(void )
{
  return m_channelWidth;
}

void
RUData::SetNumberOfMimoUsers(uint8_t num)
{
  m_mimoUsers = num;
}

uint8_t
RUData::GetNumberOfMimoUsers(void )
{
  return m_mimoUsers;
}


}
