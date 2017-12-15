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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/gnuplot.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleInfra");

Ptr<UniformRandomVariable> mobilityRandom = CreateObject<UniformRandomVariable> ();
Ptr<UniformRandomVariable> pktSizeRandom = CreateObject<UniformRandomVariable> ();
Ptr<UniformRandomVariable> pktSilenceBusyIntervalVoip = CreateObject<UniformRandomVariable> ();
Ptr<UniformRandomVariable> initStaTimeRandom = CreateObject<UniformRandomVariable> ();
Ptr<UniformRandomVariable> globalRv = CreateObject<UniformRandomVariable> ();
Ptr<UniformRandomVariable> locationRv = CreateObject<UniformRandomVariable> ();

#define  VOIP_BUSY_INTERVAL_20MS_START  8
#define  VOIP_BUSY_INTERVAL_20MS_END    12 
#define  _ENABLE_DEBUG_   0 

enum clientPosition_t {
    NEAR = 1,
    NORMAL,
    FAR
};


uint8_t  pktMonitor = 0;
uint32_t numPackets = 1000000;
uint32_t voipPacketSize = 38; // bytes
uint32_t dataPacketSize = 200; // bytes
double simulatorBeginRangeStart = 2; //Seconds
double simulatorBeginRangeEnd = 2; //Seconds
//double simulatorMLClientTrafficStart = 12; //Seconds
double voipInterval = 0.02; // seconds
double dataInterval = 0.000008; // seconds
//double dataInterval = 0.1; // seconds
double videoInterval = 0.035; // seconds
uint8_t clPosition = NORMAL;

double simulatorDuration = 5; //Seconds
uint32_t  nVoipStas = 0;
uint32_t  nVideoStas = 0;
uint32_t  nDataStas = 0;
bool  downlink = false;

std::vector<Ptr<Socket>> recvSockPtr;
std::vector<Ptr<Socket>> sendSockPtr;
std::vector<Ptr<Socket>> dataRecvSockPtr;
std::vector<Ptr<Socket>> dataSendSockPtr;
std::vector<Ptr<Socket>> videoRecvSockPtr;
std::vector<Ptr<Socket>> videoSendSockPtr;


typedef struct pktStats_ {
   uint32_t pktSent;
   uint32_t pktRecv;
   uint64_t worstLatency;
   uint64_t avgLatency;
   uint64_t bestLatency;
   double worstJitter;
   double avgJitter;
   double avgPer;
   uint64_t numJitter;
   uint64_t numPer;
   uint64_t numPkt;
   uint32_t typeOfClientTraffic;
   uint32_t lastLatency;
   uint32_t currentPktCount;
   uint32_t maxPktCount;
   uint32_t lastRecvPktNumber;
   uint32_t intervalRandom;
   uint32_t startTime;
   double   aggDistance;
   double   avgPktSize;
   double   dropPer;
   double   dropBeforeQueue;
   double   dropAfterQueue;
   double   dropTotal;
   std::vector<double > coordinates;
   std::vector<uint64_t> latencyStats;
   std::vector<uint64_t> jitterStats;
   std::vector<double > perStats;
} pktStats_t;

std::vector<pktStats_t> pktStats;
std::vector<pktStats_t> dataPktStats;
std::vector<pktStats_t> videoPktStats;

void bufferToData64(uint64_t *data, unsigned char *recvBuffer, uint32_t start) 
{
  *data = recvBuffer[start + 7];
  *data <<= 8;
  *data |= recvBuffer[start + 6];
  *data <<= 8;
  *data |= recvBuffer[start + 5];
  *data <<= 8;
  *data |= recvBuffer[start + 4];
  *data <<= 8;
  *data |= recvBuffer[start + 3];
  *data <<= 8;
  *data |= recvBuffer[start + 2];
  *data <<= 8;
  *data |= recvBuffer[start + 1];
  *data <<= 8;
  *data |= recvBuffer[start];
}

void bufferToData32(uint32_t *data, unsigned char *recvBuffer, uint32_t start) 
{
  *data |= recvBuffer[start + 3];
  *data <<= 8;
  *data |= recvBuffer[start + 2];
  *data <<= 8;
  *data |= recvBuffer[start + 1];
  *data <<= 8;
  *data |= recvBuffer[start];
}

void dataToBuffer64(unsigned char *buffer, uint64_t data, uint32_t start)
{
  buffer[start] = (data >> 0) & 0xff;
  buffer[start + 1] = (data >> 8) & 0xff;
  buffer[start + 2] = (data >> 16) & 0xff;
  buffer[start + 3] = (data >> 24) & 0xff;
  buffer[start + 4] = (data >> 32) & 0xff;
  buffer[start + 5] = (data >> 40) & 0xff;
  buffer[start + 6] = (data >> 48) & 0xff;
  buffer[start + 7] = (data >> 56) & 0xff;
}

void dataToBuffer32(unsigned char *buffer, uint32_t data, uint32_t start)
{
  buffer[start] = (data >> 0) & 0xff;
  buffer[start + 1] = (data >> 8) & 0xff;
  buffer[start + 2] = (data >> 16) & 0xff;
  buffer[start + 3] = (data >> 24) & 0xff;
}

enum clientTraffic {
   CLIENT_TRAFFIC_TYPE_VOICE = 1,
   CLIENT_TRAFFIC_TYPE_VIDEO,
   CLIENT_TRAFFIC_TYPE_BESTEFFORT
};

std::string ClientTrafficTypeToStr(uint32_t trafficType) 
{
   switch(trafficType) {
       case CLIENT_TRAFFIC_TYPE_VOICE:
          return "Voice";
       case CLIENT_TRAFFIC_TYPE_VIDEO:
          return "Video";
       case CLIENT_TRAFFIC_TYPE_BESTEFFORT:
          return "Best Effort";
       default:
          return "Unknown";
   }
   return "Unknown";
}

void DataReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> m_receivedPacket;
  uint32_t recvBytes;
  uint64_t startTime, latency, jitter = 0;
  double per = 0.0;
  unsigned char recvBuffer[1500];
  Time time_now;
  uint32_t recvPktNumber = 0;

  while ((m_receivedPacket = socket->Recv ()))
    {
        for (std::vector<Ptr<Socket>>::size_type rit = 0; rit < dataRecvSockPtr.size(); rit++) {
            if(socket == dataRecvSockPtr[rit]) {
                memset(recvBuffer, '\0', sizeof(recvBuffer));
                recvBytes = m_receivedPacket->CopyData(recvBuffer, 16);
 
                NS_ASSERT(recvBytes);
                bufferToData64(&startTime, recvBuffer, 0);  //Decode from 0th byte
                bufferToData32(&recvPktNumber, recvBuffer, 8);  //Decode from 8th byte

                time_now = Now(); 
                latency = time_now.GetNanoSeconds() - startTime;
                dataPktStats[rit].latencyStats.push_back(latency);
           
                PerTag tag;
                m_receivedPacket->RemovePacketTag (tag);
                per = tag.Get(); //m_receivedPacket->m_per;
                dataPktStats[rit].perStats.push_back(per);
                if((!dataPktStats[rit].avgPer) && (per)) {
                    dataPktStats[rit].avgPer = per;
                    dataPktStats[rit].numPer = 1;
                } else {
                    dataPktStats[rit].avgPer = ( (dataPktStats[rit].avgPer * (dataPktStats[rit].numPer)) + per) / (dataPktStats[rit].numPer + 1);
                    dataPktStats[rit].numPer ++;
                }

                #if _ENABLE_DEBUG_ 
                NS_LOG_UNCOND("### Packet Number Received : " << recvPktNumber << "  now @ : " << Now() << " Latency (millisecond) : " << latency / 1000000.0); 
                #endif

                dataPktStats[rit].pktRecv ++;
                if(dataPktStats[rit].pktRecv > pktMonitor) {

                    if(!dataPktStats[rit].lastLatency) {
                        dataPktStats[rit].lastLatency = latency;
                        dataPktStats[rit].worstJitter = 0; 
                        dataPktStats[rit].lastRecvPktNumber = recvPktNumber;
                    } else {
                        jitter = (dataPktStats[rit].lastLatency > latency) ? (dataPktStats[rit].lastLatency - latency) : (latency - dataPktStats[rit].lastLatency);
                        dataPktStats[rit].jitterStats.push_back(jitter);
                        
                        #if _ENABLE_DEBUG_ 
                        NS_LOG_UNCOND("#### Jitter with respect to present packet number : " << recvPktNumber << " and previous packet number : " << \
                            dataPktStats[rit].lastRecvPktNumber << " in (milli seconds) : " << jitter / 1000000.0); 
                        #endif
                        dataPktStats[rit].lastRecvPktNumber = recvPktNumber;
                        dataPktStats[rit].lastLatency = latency; 
                        if(dataPktStats[rit].worstJitter < jitter) {
                            dataPktStats[rit].worstJitter = jitter;
                        }
                    }
                    if((!dataPktStats[rit].avgJitter) && (jitter)) {
                        dataPktStats[rit].avgJitter = jitter;               
                        dataPktStats[rit].numJitter = 1; 
                    } else { 
                        dataPktStats[rit].avgJitter = ( (dataPktStats[rit].avgJitter * (dataPktStats[rit].numJitter)) + jitter) / (dataPktStats[rit].numJitter + 1); 
                        dataPktStats[rit].numJitter ++; 
                    }


                    if(dataPktStats[rit].worstLatency < latency) {
                        dataPktStats[rit].worstLatency = latency;
                    }
                    if(!dataPktStats[rit].avgLatency) {
                        dataPktStats[rit].avgLatency = latency;                
                    } else { 
                        dataPktStats[rit].avgLatency = ( (dataPktStats[rit].avgLatency * (dataPktStats[rit].pktRecv - 1 - pktMonitor)) + latency) / (dataPktStats[rit].pktRecv - pktMonitor);
                    }
                    if( (!dataPktStats[rit].bestLatency) || (dataPktStats[rit].bestLatency > latency) ) {
                        dataPktStats[rit].bestLatency = latency;
                    }
                }
            }
        }
    }
}

void ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> m_receivedPacket;
  uint32_t recvBytes;
  uint64_t startTime, latency, jitter = 0;
  double per = 0.0;
  unsigned char recvBuffer[1500];
  Time time_now;
  uint32_t recvPktNumber = 0;

  while ((m_receivedPacket = socket->Recv ()))
    {
        for (std::vector<Ptr<Socket>>::size_type rit = 0; rit < recvSockPtr.size(); rit++) {
            if(socket == recvSockPtr[rit]) {
                memset(recvBuffer, '\0', sizeof(recvBuffer));
                recvBytes = m_receivedPacket->CopyData(recvBuffer, 16);
 
                NS_ASSERT(recvBytes);
                bufferToData64(&startTime, recvBuffer, 0);  //Decode from 0th byte
                bufferToData32(&recvPktNumber, recvBuffer, 8);  //Decode from 8th byte

                time_now = Now(); 
                latency = time_now.GetNanoSeconds() - startTime;
                pktStats[rit].latencyStats.push_back(latency);

                PerTag tag;
                m_receivedPacket->RemovePacketTag (tag);
                per = tag.Get(); //m_receivedPacket->m_per;
                pktStats[rit].perStats.push_back(per);
                if((!pktStats[rit].avgPer) && (per)) {
                    pktStats[rit].avgPer = per;
                    pktStats[rit].numPer = 1;
                } else {
                    pktStats[rit].avgPer = ( (pktStats[rit].avgPer * (pktStats[rit].numPer)) + per) / (pktStats[rit].numPer + 1);
                    pktStats[rit].numPer ++;
                }
           
                #if _ENABLE_DEBUG_ 
                NS_LOG_UNCOND("### Packet Number Received : " << recvPktNumber << "  now @ : " << Now() << " Latency (millisecond) : " << latency / 1000000.0); 
                #endif

                pktStats[rit].pktRecv ++;
                if(pktStats[rit].pktRecv > pktMonitor) {

                    if(!pktStats[rit].lastLatency) {
                        pktStats[rit].lastLatency = latency;
                        pktStats[rit].worstJitter = 0; 
                        pktStats[rit].lastRecvPktNumber = recvPktNumber;
                    } else {
                        jitter = (pktStats[rit].lastLatency > latency) ? (pktStats[rit].lastLatency - latency) : (latency - pktStats[rit].lastLatency);
                        pktStats[rit].jitterStats.push_back(jitter);
                        
                        #if _ENABLE_DEBUG_ 
                        NS_LOG_UNCOND("#### Jitter with respect to present packet number : " << recvPktNumber << " and previous packet number : " << \
                            pktStats[rit].lastRecvPktNumber << " in (milli seconds) : " << jitter / 1000000.0); 
                        #endif
                        pktStats[rit].lastRecvPktNumber = recvPktNumber;
                        pktStats[rit].lastLatency = latency; 
                        if(pktStats[rit].worstJitter < jitter) {
                            pktStats[rit].worstJitter = jitter;
                        }
                    }
                    if((!pktStats[rit].avgJitter) && (jitter)) {
                        pktStats[rit].avgJitter = jitter;               
                        pktStats[rit].numJitter = 1; 
                    } else { 
                        pktStats[rit].avgJitter = ( (pktStats[rit].avgJitter * (pktStats[rit].numJitter)) + jitter) / (pktStats[rit].numJitter + 1); 
                        pktStats[rit].numJitter ++; 
                    }


                    if(pktStats[rit].worstLatency < latency) {
                        pktStats[rit].worstLatency = latency;
                    }
                    if(!pktStats[rit].avgLatency) {
                        pktStats[rit].avgLatency = latency;                
                    } else { 
                        pktStats[rit].avgLatency = ( (pktStats[rit].avgLatency * (pktStats[rit].pktRecv - 1 - pktMonitor)) + latency) / (pktStats[rit].pktRecv - pktMonitor);
                    }
                    if( (!pktStats[rit].bestLatency) || (pktStats[rit].bestLatency > latency) ) {
                        pktStats[rit].bestLatency = latency;
                    }
                }
            }
        }
    }
}

static void GenerateVoiceTraffic (uint32_t pktSize, Time pktInterval, uint32_t staIndex)
{
  uint32_t randPacketSize;
  Time duration_time;
  uint64_t duration_now;

  std::vector<Ptr<Socket>>::size_type rit = staIndex;
  
  if(rit < recvSockPtr.size()) { 
      if(pktStats[rit].currentPktCount < pktStats[rit].maxPktCount) {
           sendSockPtr[rit]->SetIpTos (192);  // AF11
           pktStats[rit].typeOfClientTraffic = CLIENT_TRAFFIC_TYPE_VOICE;

           //  sendSockPtr[rit]->SetPriority (6);  // Interactive
           //randPacketSize = pktSizeRandom->GetValue(200, 1000);
           randPacketSize = pktSizeRandom->GetValue(pktSize, pktSize);

           duration_time = Now();
           duration_now = (double)duration_time.GetNanoSeconds ();

           pktStats[rit].currentPktCount ++;
           uint8_t buffer[randPacketSize];
           dataToBuffer64(buffer, duration_now, 0);    // Encode from 0th byte
           dataToBuffer32(buffer, pktStats[rit].currentPktCount, 8); // Encode from 8th byte

           sendSockPtr[rit]->Send (buffer, randPacketSize, 0);

           pktStats[rit].pktSent ++;
           #if _ENABLE_DEBUG_ 
           NS_LOG_UNCOND("\n ");
           NS_LOG_UNCOND("### Enqueuing the packet now : " << Now() << " Packet Count : " << pktStats[rit].currentPktCount);
           #endif
      } else {
           sendSockPtr[rit]->Close();
      }
  }

  if(pktStats[rit].intervalRandom) {
     if (pktStats[rit].pktSent == 1){
       Simulator::Schedule (MilliSeconds(20.0)+ Seconds(1.0), &GenerateVoiceTraffic, 
                        pktSize, MilliSeconds(20.0), staIndex);
     }
     else{
       Simulator::Schedule (MilliSeconds(20.0), &GenerateVoiceTraffic, 
                        pktSize, MilliSeconds(20.0), staIndex);
     }
       pktStats[rit].intervalRandom --;
  } else {
       pktStats[rit].intervalRandom = pktSilenceBusyIntervalVoip->GetValue(VOIP_BUSY_INTERVAL_20MS_START, VOIP_BUSY_INTERVAL_20MS_END);
     if (pktStats[rit].pktSent == 1){
       Simulator::Schedule (MilliSeconds(160.0)+ Seconds(1.0), &GenerateVoiceTraffic, 
                           pktSize, MilliSeconds(160.0), staIndex);
     }
     else {
       Simulator::Schedule (MilliSeconds(160.0), &GenerateVoiceTraffic, 
                           pktSize, MilliSeconds(160.0), staIndex);
     }
  } 

}

const std::string currentDateTime() 
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

static void PrintRunningTime()
{
   NS_LOG_UNCOND("Running time now Seconds : " << Now().GetSeconds() << " Current system time : " << currentDateTime());
   Simulator::Schedule (Seconds(1.0), &PrintRunningTime);
}

static void GenerateDataTraffic (uint32_t pktSize, Time pktInterval, uint32_t staIndex)
{
  uint32_t randPacketSize;
  Time duration_time;
  uint64_t duration_now;

  std::vector<Ptr<Socket>>::size_type rit = staIndex;
  
  if(rit < dataRecvSockPtr.size()) { 
      if(dataPktStats[rit].currentPktCount < dataPktStats[rit].maxPktCount) {
           dataSendSockPtr[rit]->SetIpTos (0);  // AF11
           dataPktStats[rit].typeOfClientTraffic = CLIENT_TRAFFIC_TYPE_BESTEFFORT;

           randPacketSize = pktSizeRandom->GetValue(pktSize, pktSize);

           duration_time = Now();
           duration_now = (double)duration_time.GetNanoSeconds ();

           dataPktStats[rit].currentPktCount ++;
           uint8_t buffer[randPacketSize];
           dataToBuffer64(buffer, duration_now, 0);    // Encode from 0th byte
           dataToBuffer32(buffer, dataPktStats[rit].currentPktCount, 8); // Encode from 8th byte

           dataSendSockPtr[rit]->Send (buffer, randPacketSize, 0);

           dataPktStats[rit].pktSent ++;
           #if _ENABLE_DEBUG_ 
           NS_LOG_UNCOND("\n ");
           NS_LOG_UNCOND("### Enqueuing the packet now : " << Now() << " Packet Count : " << dataPktStats[rit].currentPktCount);
           #endif
      } else {
           dataSendSockPtr[rit]->Close();
      }
  }

    if (dataPktStats[rit].pktSent == 1){
       Simulator::Schedule (pktInterval+Seconds(1.0), &GenerateDataTraffic, 
                        pktSize, pktInterval, staIndex);
    }
    else {
       Simulator::Schedule (pktInterval, &GenerateDataTraffic, 
                        pktSize, pktInterval, staIndex);
    }
}

void VideoReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> m_receivedPacket;
  uint32_t recvBytes;
  uint64_t startTime, latency, jitter = 0;
  double per = 0.0;
  unsigned char recvBuffer[1500];
  Time time_now;
  uint32_t recvPktNumber = 0;

  while ((m_receivedPacket = socket->Recv ()))
    {
        for (std::vector<Ptr<Socket>>::size_type rit = 0; rit < videoRecvSockPtr.size(); rit++) {
            if(socket == videoRecvSockPtr[rit]) {
//                NS_LOG_UNCOND("Video packet received for video sta : " << videoPktStats[rit].coordinates[0]);
                memset(recvBuffer, '\0', sizeof(recvBuffer));
                recvBytes = m_receivedPacket->CopyData(recvBuffer, 16);
 
                NS_ASSERT(recvBytes);
                bufferToData64(&startTime, recvBuffer, 0);  //Decode from 0th byte
                bufferToData32(&recvPktNumber, recvBuffer, 8);  //Decode from 8th byte

                time_now = Now(); 
                latency = time_now.GetNanoSeconds() - startTime;
                videoPktStats[rit].latencyStats.push_back(latency);
           
                PerTag tag;
                m_receivedPacket->RemovePacketTag (tag);
                per = tag.Get(); //m_receivedPacket->m_per;
                videoPktStats[rit].perStats.push_back(per);
                if((!videoPktStats[rit].avgPer) && (per)) {
                    videoPktStats[rit].avgPer = per;
                    videoPktStats[rit].numPer = 1;
                } else {
                    videoPktStats[rit].avgPer = ( (videoPktStats[rit].avgPer * (videoPktStats[rit].numPer)) + per) / (videoPktStats[rit].numPer + 1);
                    videoPktStats[rit].numPer ++;
                }

                #if _ENABLE_DEBUG_ 
                NS_LOG_UNCOND("### Packet Number Received : " << recvPktNumber << "  now @ : " << Now() << " Latency (millisecond) : " << latency / 1000000.0); 
                #endif

                videoPktStats[rit].pktRecv ++;
                if(videoPktStats[rit].pktRecv > pktMonitor) {

                    if(!videoPktStats[rit].lastLatency) {
                        videoPktStats[rit].lastLatency = latency;
                        videoPktStats[rit].worstJitter = 0; 
                        videoPktStats[rit].lastRecvPktNumber = recvPktNumber;
                    } else {
                        jitter = (videoPktStats[rit].lastLatency > latency) ? (videoPktStats[rit].lastLatency - latency) : (latency - videoPktStats[rit].lastLatency);
                        videoPktStats[rit].jitterStats.push_back(jitter);
                        
                        #if _ENABLE_DEBUG_ 
                        NS_LOG_UNCOND("#### Jitter with respect to present packet number : " << recvPktNumber << " and previous packet number : " << \
                            videoPktStats[rit].lastRecvPktNumber << " in (milli seconds) : " << jitter / 1000000.0); 
                        #endif
                        videoPktStats[rit].lastRecvPktNumber = recvPktNumber;
                        videoPktStats[rit].lastLatency = latency; 
                        if(videoPktStats[rit].worstJitter < jitter) {
                            videoPktStats[rit].worstJitter = jitter;
                        }
                    }
                    if((!videoPktStats[rit].avgJitter) && (jitter)) {
                        videoPktStats[rit].avgJitter = jitter;               
                        videoPktStats[rit].numJitter = 1; 
                    } else { 
                        videoPktStats[rit].avgJitter = ( (videoPktStats[rit].avgJitter * (videoPktStats[rit].numJitter)) + jitter) / (videoPktStats[rit].numJitter + 1); 
                        videoPktStats[rit].numJitter ++; 
                    }


                    if(videoPktStats[rit].worstLatency < latency) {
                        videoPktStats[rit].worstLatency = latency;
                    }
                    if(!videoPktStats[rit].avgLatency) {
                        videoPktStats[rit].avgLatency = latency;                
                    } else { 
                        videoPktStats[rit].avgLatency = ( (videoPktStats[rit].avgLatency * (videoPktStats[rit].pktRecv - 1 - pktMonitor)) + latency) / (videoPktStats[rit].pktRecv - pktMonitor);
                    }
                    if( (!videoPktStats[rit].bestLatency) || (videoPktStats[rit].bestLatency > latency) ) {
                        videoPktStats[rit].bestLatency = latency;
                    }
                }
            }
        }
    }
}

void GetWeibullParameterFromVideoClass(double *scale, double *shape, uint8_t videoClass)
{
  if (videoClass == 1)
  {
    *scale = 6950;
    *shape = 0.8099;
  }
  else if (videoClass == 2)
  {
    *scale = 13900;
    *shape = 0.8099;
  }
  else if (videoClass == 3)
  {
    *scale = 20850;
    *shape = 0.8099;
  }
  else if (videoClass == 4)
  {
    *scale = 27800;
    *shape = 0.8099;
  }
  else if (videoClass == 5)
  {
    *scale = 34750;
    *shape = 0.8099;
  }
  else if (videoClass == 6)
  {
    *scale = 54210;
    *shape = 0.8099;
  }
  return;
}

void GenerateVideoFrameTraffic (int32_t frameSize, uint32_t staIndex)
{
  std::vector<Ptr<Socket>>::size_type rit = staIndex;
  uint8_t buffer[frameSize];
  Time duration_time;
  uint64_t duration_now;

  videoPktStats[rit].pktSent ++;
  //NS_LOG_UNCOND("Sending frame for STA : " << staIndex << " Size : " << frameSize << " Now : " << Now());
  if((!videoPktStats[rit].avgPktSize) && (frameSize)) {
    videoPktStats[rit].avgPktSize = frameSize;
    videoPktStats[rit].numPkt = 1;
  } else {
    videoPktStats[rit].avgPktSize = ( (videoPktStats[rit].avgPktSize * (videoPktStats[rit].numPkt)) + frameSize) / (videoPktStats[rit].numPkt + 1);
    videoPktStats[rit].numPkt ++;
  }
  videoSendSockPtr[rit]->SetIpTos (136);  // AF41
  videoPktStats[rit].typeOfClientTraffic = CLIENT_TRAFFIC_TYPE_VIDEO;
  videoPktStats[rit].currentPktCount ++;
  duration_time = Now();
  duration_now = (double)duration_time.GetNanoSeconds ();
  dataToBuffer64(buffer, duration_now, 0);    // Encode from 0th byte
  dataToBuffer32(buffer, videoPktStats[rit].currentPktCount, 8); // Encode from 8th byte
  videoSendSockPtr[rit]->Send (buffer, frameSize, 0);
}

static void GenerateVideoTraffic (Time pktInterval, uint32_t staIndex, uint8_t videoClass)
{
/*************Packet Size**************/
  int32_t pktSize = 0;
  double scale = 0.0;          //lambda
  double shape = 0.0;          // k
  GetWeibullParameterFromVideoClass(&scale, &shape, videoClass);
  Ptr<WeibullRandomVariable> wRv = CreateObject<WeibullRandomVariable> ();
  wRv->SetAttribute ("Scale", DoubleValue (scale));
  wRv->SetAttribute ("Shape", DoubleValue (shape));

/************InterPacket Interval*************/
  double interval = 0.0;
  double alpha = 0.2463;
  double beta = (1/60.227);
  Ptr<GammaRandomVariable> gRv = CreateObject<GammaRandomVariable> ();
  gRv->SetAttribute ("Alpha", DoubleValue (alpha));
  gRv->SetAttribute ("Beta", DoubleValue (beta));
  interval = gRv->GetValue() + 0.035;       // 0.035s interval corresponds to ~ 30fps
  //NS_LOG_UNCOND("Inter Packet Interval : " << interval << " ms");

  std::vector<Ptr<Socket>>::size_type rit = staIndex;
  
  if(rit < videoRecvSockPtr.size()) { 
      if(videoPktStats[rit].currentPktCount < videoPktStats[rit].maxPktCount) {
           pktSize = (uint32_t)(wRv->GetValue());
           if(videoPktStats[rit].pktSent == 0 || pktSize < 150)
             pktSize = 150;
           if(pktSize == 500)
             pktSize++;
           pktInterval = Seconds(interval);
           
           //  sendSockPtr[rit]->SetPriority (6);  // Interactive

           Time interFrameInterval = MicroSeconds(400);
           int32_t frameSize = 0;
           uint32_t count=1;
           //NS_LOG_UNCOND("Parent Packet size : " << pktSize << " for Sta : " << staIndex << " Now : " << Now());
           while(pktSize > 0)
           {
             if (pktSize > 500){
               frameSize = 500;
             }
             else {
               frameSize = pktSize;
             }
             Simulator::Schedule (count*interFrameInterval, &GenerateVideoFrameTraffic,
                                    frameSize, staIndex);
             pktSize = pktSize - 500;
             count++;
           }
           #if _ENABLE_DEBUG_ 
           NS_LOG_UNCOND("\n ");
           NS_LOG_UNCOND("### Enqueuing the packet now : " << Now() << " Packet Count : " << videoPktStats[rit].currentPktCount);
           #endif
      } else {
           videoSendSockPtr[rit]->Close();
      }
  }

    if (videoPktStats[rit].pktSent == 1){
       Simulator::Schedule (pktInterval+Seconds(1.0), &GenerateVideoTraffic, 
                        pktInterval, staIndex, videoClass);
    }
    else {
       Simulator::Schedule (pktInterval, &GenerateVideoTraffic, 
                       pktInterval, staIndex, videoClass);
    }
}

static void
RxDrop (uint32_t index, Ptr<const Packet> p)
{
  if (index > 0 && index < nVoipStas+1)
    pktStats[index-1].dropPer++;
  else if (index > nVoipStas && index < nVoipStas+nDataStas+1)
    dataPktStats[index-nVoipStas-1].dropPer++;
  else if (index > nVoipStas+nDataStas && index < nVoipStas+nDataStas+nVideoStas+1)
    videoPktStats[index-nVoipStas-nDataStas-1].dropPer++;
}

/* For 550 bytes, 
    InterPacketInterval 0.0001   =  44 Mbps
    			0.001  = 4.4 
    			0.01  = 0.4
    			0.1 = 0.04

   For 200 bytes, 50 packets * 200 bytes * 8 = 80 kbps = 0.08Mbps
*/

Ptr<Socket>
SetupPacketReceive (Ptr<Node> node, int port, Callback<void, Ptr<Socket> > recvF )
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), port);
  sink->Bind (local);
  sink->SetRecvCallback (recvF);
  return sink;
}

Ptr<Socket>
SetupPacketSend (Ptr<Node> node, Ipv4Address addr, int port)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> source = Socket::CreateSocket (node, tid);
  InetSocketAddress remote = InetSocketAddress (addr, port);
  source->SetAllowBroadcast (true);
  source->Connect (remote);
  return source;
}

int main (int argc, char *argv[])
{
  std::string phyMode ("HeMcs0");
  bool verbose = false;
  NetDeviceContainer devices;
  pktStats_t  addPktStats = { 0 };
  double base = 1000000.0;
  double throughput = 0.0;
  uint32_t latency_percentile = 0, jitter_percentile = 0, delayJitterCounter = 0, delayLatencyCounter = 0;
  uint32_t latency_99_percentile = 0, jitter_99_percentile = 0;
  uint32_t nNearStasVo = 0, nNormalStasVo = 0, nEdgeStasVo = 0;
  uint32_t nNearStasVi = 0, nNormalStasVi = 0, nEdgeStasVi = 0;
  uint32_t nNearStasFb = 0, nNormalStasFb = 0, nEdgeStasFb = 0;
  uint32_t runNumber=0;
  double  aggregateThroughput = 0.0;

  CommandLine cmd;

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("voipInterval", "interval (seconds) between packets", voipInterval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("runNumber", "the index of the run when running from python script", runNumber);

  Config::SetDefault ("ns3::WifiNetDevice::Mtu", UintegerValue (800));

  cmd.Parse (argc, argv);

  // Convert to time object
  Time interPacketInterval = Seconds (voipInterval);
  Time dataPacketInterval = Seconds (dataInterval);
  Time videoPacketInterval = Seconds (videoInterval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", 
                      StringValue (phyMode));
  Config::SetDefault ("ns3::RRMWifiManager::SchedulerPlugin",
                      BooleanValue (false));
  NodeContainer NodeC;
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ax_2_4GHZ);
  //wifi.SetStandard (WIFI_PHY_STANDARD_80211ax_5GHZ);

  HEWifiPhyHelper wifiPhy =  HEWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  //wifiPhy.Set ("RxGain", DoubleValue (0) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");

  HEWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  //wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  wifiChannel.AddPropagationLoss ("ns3::Enterprise11axPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  //wifiPhy.Set ("ChannelNumber", UintegerValue(42));

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  // Setup the rest of the mac
  Ssid ssid = Ssid ("wifi-default");
  NetDeviceContainer apDevice, staDevice;
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  double apX = 0, apY = 0, apZ = 0;         // x, y, z coordinates of AP initialization
  std::ifstream file("scratch/scenario.txt");
  std::string str; 

  while (std::getline(file, str))
  {
    // Process one line
    if (str[0] == '#')
    {
	//Comments , ignore it
      continue;
    }
    std::string staType, trafficClass, macStr;
    char dummy;
    double X = 0, Y = 0, Z = 0;
    std::stringstream ss(str);
    ss >> staType >> macStr >> trafficClass >> X >> dummy >> Y >> dummy >> Z;
    std::cout << staType << " " << macStr << " " << trafficClass << " " << X <<","<< Y<<","<< Z << std::endl;

    Ptr<Node> node = CreateObject<Node> ();
    NodeC.Add(node);
    if (staType == "AP")
    {
      // setup ap.
      wifiMac.SetType ("ns3::ApWifiMac",
                       "Ssid", SsidValue (ssid));
      wifi.SetRemoteStationManager ("ns3::RRMWifiManager",
                                    "DataMode",StringValue (phyMode),
                                    "ControlMode",StringValue (phyMode),
				    "RateControl", UintegerValue(AARF));
      apDevice = wifi.Install (wifiPhy, wifiMac, node, Mac48Address (macStr.c_str()));
      devices.Add (apDevice);
      apX = X; apY = Y; apZ = Z;
      positionAlloc->Add (Vector (X, Y, Z));
    }
    else
    {
      wifiMac.SetType ("ns3::StaWifiMac",
                       "Ssid", SsidValue (ssid));
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode",StringValue (phyMode),
                                    "ControlMode",StringValue (phyMode));
      staDevice = wifi.Install (wifiPhy, wifiMac, node, Mac48Address (macStr.c_str()));
      devices.Add (staDevice);
      positionAlloc->Add (Vector (X, Y, Z));

      memset(&addPktStats, '\0', sizeof(addPktStats));
      addPktStats.aggDistance = std::sqrt(std::pow((apX-X),2)+std::pow((apY-Y),2)+std::pow((apZ-Z),2));   //distance from AP to STA
      addPktStats.coordinates.push_back(X);
      addPktStats.coordinates.push_back(Y);
      addPktStats.coordinates.push_back(Z);
      addPktStats.maxPktCount = numPackets;

      addPktStats.dropPer = 0;
      if (trafficClass == "AC_VO")
      {
        nVoipStas++;
        pktStats.push_back(addPktStats);
      }
      else if (trafficClass == "AC_VI")
      {
        nVideoStas++;
        videoPktStats.push_back(addPktStats);
      }
      else
      {
        nDataStas++;
        dataPktStats.push_back(addPktStats);
      }
    }
  }

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (NodeC);

  InternetStackHelper internet;
  internet.Install (NodeC);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ipIndex = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink;
  Ptr<Socket> source;

  for(uint32_t i = 0; i < nVoipStas; i ++) {
      uint32_t port, staId;

      port = 4000 + i;
      staId = i + 1;

      if(downlink == true) {    
	  // Downlink
	  recvSink = SetupPacketReceive(NodeC.Get (staId), port, MakeCallback (&ReceivePacket));
	  source = SetupPacketSend(NodeC.Get (0), ipIndex.GetAddress(staId), port);
      } else {
          // Uplink
	  recvSink = SetupPacketReceive(NodeC.Get (0), port, MakeCallback (&ReceivePacket));
	  source = SetupPacketSend(NodeC.Get (staId), ipIndex.GetAddress(0), port);
      }
      recvSockPtr.push_back(recvSink);
      sendSockPtr.push_back(source);
  }

  for(uint32_t i = nVoipStas + 1; i <= nDataStas + nVoipStas; i ++) {
      uint32_t port, staId;

      port = 5000 + i;
      staId = i;

      if(downlink == true) {    
	  // Downlink
	  recvSink = SetupPacketReceive(NodeC.Get (staId), port, MakeCallback (&DataReceivePacket));
	  source = SetupPacketSend(NodeC.Get (0), ipIndex.GetAddress(staId), port);
      } else {
          // Uplink
	  recvSink = SetupPacketReceive(NodeC.Get (0), port, MakeCallback (&DataReceivePacket));
	  source = SetupPacketSend(NodeC.Get (staId), ipIndex.GetAddress(0), port);
      }
      dataRecvSockPtr.push_back(recvSink);
      dataSendSockPtr.push_back(source);
  }

  for(uint32_t i = nDataStas + nVoipStas + 1; i <= nVideoStas + nDataStas + nVoipStas; i ++) {
      uint32_t port, staId;

      port = 6000 + i;
      staId = i;
      if(downlink == true) {
	  // Downlink
	  recvSink = SetupPacketReceive(NodeC.Get (staId), port, MakeCallback (&VideoReceivePacket));
	  source = SetupPacketSend(NodeC.Get (0), ipIndex.GetAddress(staId), port);
      } else {
          // Uplink
	  recvSink = SetupPacketReceive(NodeC.Get (0), port, MakeCallback (&VideoReceivePacket));
	  source = SetupPacketSend(NodeC.Get (staId), ipIndex.GetAddress(0), port);
      }
      videoRecvSockPtr.push_back(recvSink);
      videoSendSockPtr.push_back(source);
  }

  std::ofstream resultsLog;
  resultsLog.open("resultsFile.txt", std::ios_base::app);
  resultsLog << (nVoipStas+nDataStas+nVideoStas) << std::endl;
  resultsLog << nVoipStas << ", " << nDataStas << ", " << nVideoStas << std::endl;
  resultsLog << nNearStasVo <<  ", " << nNormalStasVo <<  ", " << nEdgeStasVo << std::endl;
  resultsLog << nNearStasFb <<  ", " << nNormalStasFb <<  ", " << nEdgeStasFb << std::endl;
  resultsLog << nNearStasVi <<  ", " << nNormalStasVi <<  ", " << nEdgeStasVi << std::endl;

  // Tracing
  wifiPhy.EnablePcap ("wifi-simple-infra-" + std::to_string(runNumber) + "-run", devices);

  // Setting the algorithm
  //apDevice.Get(0)->GetObject<WifiNetDevice>()->GetRemoteStationManager()->GetObject<RRMWifiManager>()->SetAlgoDownlink(dlScheduler);
  //apDevice.Get(0)->GetObject<WifiNetDevice>()->GetRemoteStationManager()->GetObject<RRMWifiManager>()->SetAlgoSubDownlink(subDlScheduler);
  //apDevice.Get(0)->GetObject<WifiNetDevice>()->GetRemoteStationManager()->GetObject<RRMWifiManager>()->SetAlgoUplink(ulScheduler);

  double randomStaTime = initStaTimeRandom->GetValue(simulatorBeginRangeStart, simulatorBeginRangeEnd);

    for(uint32_t staIndex = 0; staIndex < nVoipStas; staIndex ++ ) {
         pktStats[staIndex].startTime = randomStaTime + 1.0;
         Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                    Seconds (randomStaTime), &GenerateVoiceTraffic, 
                                    voipPacketSize, interPacketInterval, staIndex);
    }

    for(uint32_t staIndex = 0; staIndex < nDataStas; staIndex ++ ) {
         dataPktStats[staIndex].startTime = randomStaTime + 1.0;
         Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                    Seconds (randomStaTime), &GenerateDataTraffic, 
                                    dataPacketSize, dataPacketInterval, staIndex);
    }

    uint8_t videoClass = 2;   /* Video class */
    for(uint32_t staIndex = 0; staIndex < nVideoStas; staIndex ++ ) {
         videoPktStats[staIndex].startTime = randomStaTime + 1.0;
         Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                    Seconds (randomStaTime), &GenerateVideoTraffic, 
                                    videoPacketInterval, staIndex, videoClass);
    }
  Simulator::Schedule (Seconds(1.0), &PrintRunningTime);

  for(uint32_t index = 1; index <= nVideoStas + nDataStas + nVoipStas; index ++) {
      devices.Get(index)->GetObject<WifiNetDevice>()->GetPhy()->TraceConnectWithoutContext("PhyRxDrop", MakeBoundCallback (&RxDrop, index));
  }
  
  Simulator::Stop (Seconds (simulatorDuration));
  Simulator::Run ();

  std::ofstream throughput_voip("throughputVoip.txt");
  std::ofstream throughput_fullBuffer("throughputFullBuffer.txt");
  std::ofstream throughput_video("throughputVideo.txt");
  std::ofstream throughputBarPlot("throughputBarPlot");
  std::ofstream ruliBarPlot("ruliBarPlot");

  std::ofstream xlocation_var("xlocation.txt");
  std::ofstream ylocation_var("ylocation.txt");

  std::ofstream table_create("tableData.txt");
  std::ofstream table_create_combined("tableDataCombined.txt");

  std::ofstream drop_voip;
  std::ofstream drop_fullBuffer;
  std::ofstream drop_video;
  drop_voip.open ("dropVoip.txt");
  drop_fullBuffer.open ("dropFullBuffer.txt");
  drop_video.open ("dropVideo.txt");
  std::ofstream drop_voip_per;
  std::ofstream drop_fullBuffer_per;
  std::ofstream drop_video_per;
  drop_voip_per.open ("dropVoipPer.txt");
  drop_fullBuffer_per.open ("dropFullBufferPer.txt");
  drop_video_per.open ("dropVideoPer.txt");
  std::ofstream lat_voip;
  std::ofstream lat_voip_97;
  std::ofstream lat_voip_99;
  std::ofstream lat_video;
  std::ofstream lat_video_97;
  std::ofstream lat_video_99;
  std::ofstream lat_fullBuffer;
  lat_voip.open ("LatVoipWorst.txt");
  lat_voip_97.open ("LatVoipWorst97.txt");
  lat_voip_99.open ("LatVoipWorst99.txt");
  lat_fullBuffer.open ("LatFullBufferAvg.txt");
  lat_video.open ("LatVideoWorst.txt");
  lat_video_97.open ("LatVideoWorst97.txt");
  lat_video_99.open ("LatVideoWorst99.txt");


  std::ofstream jit_voip;
  std::ofstream jit_voip_97;
  std::ofstream jit_voip_99;
  std::ofstream jit_video;
  std::ofstream jit_video_97;
  std::ofstream jit_video_99;
  std::ofstream jit_fullBuffer;
  jit_voip.open ("JitVoipWorst.txt");
  jit_voip_97.open ("JitVoipWorst97.txt");
  jit_voip_99.open ("JitVoipWorst99.txt");
  jit_fullBuffer.open ("JitFullBufferAvg.txt");
  jit_video.open ("JitVideoWorst.txt");
  jit_video_97.open ("JitVideoWorst97.txt");
  jit_video_99.open ("JitVideoWorst99.txt");

  NS_LOG_UNCOND("\n---------------------------------------------------------------Voip Client STATS ------------------------------------------  ");

  xlocation_var << apX;
  xlocation_var << ",";
  ylocation_var << apY; 
  ylocation_var << ",";

  std::ofstream histfile_latency;
  std::ofstream histfile_jitter;
  histfile_latency.open ("histfileLatencyVoip.txt");
  histfile_jitter.open ("histfileJitterVoip.txt");
  uint32_t numPktRecvd, numPktDropped;
  for (std::vector<pktStats_t>::size_type rit = 0; rit < pktStats.size(); rit++) {
      for(uint32_t delay_index = 0; delay_index < pktStats[rit].jitterStats.size(); delay_index ++) {
          histfile_jitter << pktStats[rit].jitterStats[delay_index]/1000000.0 << ",";
      }
      for(uint32_t delay_index = 0; delay_index < pktStats[rit].latencyStats.size(); delay_index ++) {
          histfile_latency << pktStats[rit].latencyStats[delay_index]/1000000.0 << ",";
      }
      std::sort(pktStats[rit].latencyStats.begin(), pktStats[rit].latencyStats.end());
      std::sort(pktStats[rit].jitterStats.begin(), pktStats[rit].jitterStats.end());

      double worst_lat = 0.0, avg_lat = 0.0;
      double lat97Per = 0.0, lat99Per = 0.0;
      double worst_jit = 0.0, avg_jit = 0.0;
      double jit97Per = 0.0, jit99Per = 0.0;
      delayJitterCounter = 0;
      for(uint32_t delay_index = 0; delay_index < pktStats[rit].jitterStats.size(); delay_index ++) {
          // Number of packets less than 10ms
          if(pktStats[rit].jitterStats[delay_index] < 10000000) {
              delayJitterCounter ++;
          }
      }
      if(delayJitterCounter) {
          delayJitterCounter = (100 * delayJitterCounter) / pktStats[rit].jitterStats.size();
      }

      delayLatencyCounter = 0;
      for(uint32_t delay_index = 0; delay_index < pktStats[rit].latencyStats.size(); delay_index ++) {
          // Number of packets less than 60ms
          if(pktStats[rit].latencyStats[delay_index] < 60000000) {
              delayLatencyCounter ++;
          }
      }
      if(delayLatencyCounter) {
          delayLatencyCounter = (100 * delayLatencyCounter) / pktStats[rit].latencyStats.size();
      }

      //Find the 97th percentile
      latency_percentile = (pktStats[rit].latencyStats.size() * 97) / 100;
      jitter_percentile = (pktStats[rit].jitterStats.size() * 97) / 100;
      //Find the 99th percentile
      latency_99_percentile = (pktStats[rit].latencyStats.size() * 99) / 100;
      jitter_99_percentile = (pktStats[rit].jitterStats.size() * 99) / 100;

#if _ENABLE_DEBUG_ 
      NS_LOG_UNCOND("### jitter_percentile : " << jitter_percentile << " value : " << pktStats[rit].jitterStats[jitter_percentile - 1]);
      for(uint32_t print_index = 0; print_index < pktStats[rit].jitterStats.size(); print_index ++) {
          NS_LOG_UNCOND("### latency : " << pktStats[rit].jitterStats[print_index]);
      }
#endif

      if (pktStats[rit].pktSent <= pktStats[rit].pktRecv)
        numPktDropped = 0;
      else
        numPktDropped = pktStats[rit].pktSent - pktStats[rit].pktRecv;
      numPktRecvd = pktStats[rit].pktRecv;
      NS_LOG_UNCOND(" Stats for Client  " << rit + 1 << " : " << " Packets Sent : " << pktStats[rit].pktSent << " Packets Recv : " << pktStats[rit].pktRecv << \
          " Packets Drop : " << numPktDropped); 
      NS_LOG_UNCOND("      Client Traffic Type : " << ClientTrafficTypeToStr(pktStats[rit].typeOfClientTraffic) << ", Inter Packet Interval during Active Period (milli sec) : " << voipInterval * 1000 << \
          " Packet Size : " << voipPacketSize); 
      NS_LOG_UNCOND("      Latency (ms) : " << " Worst latency : " << (pktStats[rit].worstLatency/base) << " avg latency : " << (pktStats[rit].avgLatency/base) << \
          " best latency : " << (pktStats[rit].bestLatency/base));
      worst_lat = pktStats[rit].worstLatency/base;
      avg_lat = pktStats[rit].avgLatency/base;
      if(latency_percentile && pktStats[rit].latencyStats[latency_percentile - 1]) {
          NS_LOG_UNCOND("      Latency (ms) : " << " 97th percentile latency : " << pktStats[rit].latencyStats[latency_percentile - 1] / base);
          lat97Per = pktStats[rit].latencyStats[latency_percentile - 1] / base ;
      } else {
          NS_LOG_UNCOND("      Latency (ms) : " << " 97th percentile latency : " << 0); 
      }
      if(latency_99_percentile && pktStats[rit].latencyStats[latency_99_percentile - 1]) {
          NS_LOG_UNCOND("      Latency (ms) : " << " 99th percentile latency : " << pktStats[rit].latencyStats[latency_99_percentile - 1] / base);
          lat99Per = pktStats[rit].latencyStats[latency_percentile - 1] / base ;
      } else {
          NS_LOG_UNCOND("      Latency (ms) : " << " 99th percentile latency : " << 0); 
      }
      NS_LOG_UNCOND("      % packets meet jitter bound (10ms) : " << delayJitterCounter << ", % packets meet latency bound (60ms) : " << delayLatencyCounter);
      NS_LOG_UNCOND("      Simulator Start Time : " << pktStats[rit].startTime << ", Simulator Duration : " << simulatorDuration);

      throughput = (pktStats[rit].pktRecv * voipPacketSize * 8) / (simulatorDuration - pktStats[rit].startTime);
      throughput = throughput / 1000;
      if(pktStats[rit].pktRecv > 2) {
       if(pktStats[rit].worstJitter && pktStats[rit].avgJitter && pktStats[rit].jitterStats[jitter_percentile - 1]) {
          NS_LOG_UNCOND("      Worst Jitter (milli sec) : " << pktStats[rit].worstJitter / base << " Avg Jitter (milli sec) : " << pktStats[rit].avgJitter / base << \
                           ", 97th percentile jitter : " << pktStats[rit].jitterStats[jitter_percentile - 1] / base);
          worst_jit = pktStats[rit].worstJitter / base;
          avg_jit = pktStats[rit].avgJitter / base;
          jit97Per = pktStats[rit].jitterStats[jitter_percentile - 1] / base;
       }
      }
      if(jitter_99_percentile && pktStats[rit].jitterStats[jitter_99_percentile - 1]) {
         NS_LOG_UNCOND("      99th percentile jitter : " << pktStats[rit].jitterStats[jitter_99_percentile - 1] / base);
         jit99Per = pktStats[rit].jitterStats[jitter_99_percentile - 1] / base;
      }
      throughput_voip << throughput << ",";
      drop_voip << numPktRecvd << "," << numPktDropped << ",";
      pktStats[rit].dropTotal = numPktDropped;
      if ((numPktRecvd+numPktDropped) == 0)
        drop_voip_per << 0 << ",";
      else
        drop_voip_per << ((double)numPktDropped/(numPktRecvd+numPktDropped)*100) << ",";
      lat_voip << worst_lat << ",";
      lat_voip_97 << lat97Per << ",";
      lat_voip_99 << lat99Per << ",";
      jit_voip << worst_jit << ",";
      jit_voip_97 << jit97Per << ",";
      jit_voip_99 << jit99Per << ",";
      NS_LOG_UNCOND("      Throughput : " << throughput << " kbps" ",   Distance from AP (mts) : " << pktStats[rit].aggDistance);
      aggregateThroughput += throughput;
      double xCo, yCo, zCo;
      xCo = pktStats[rit].coordinates[0];
      yCo = pktStats[rit].coordinates[1];
      zCo = pktStats[rit].coordinates[2];
      NS_LOG_UNCOND("      AP Location : (" << apX << ", " << apY << ", " << apZ << ")" << "                     Station Location : (" << xCo << ", " << yCo << ", " << zCo << ")");
      xlocation_var << xCo;
      xlocation_var << ",";
      ylocation_var << yCo;
      ylocation_var << ",";
      if (pktStats[rit].pktRecv)
      NS_LOG_UNCOND("      Average Per : " << pktStats[rit].avgPer);
      NS_LOG_UNCOND("--------------------------------------------------------------------------------------------------------------------------------");
      resultsLog << pktStats[rit].pktSent << ", " << pktStats[rit].pktRecv << ", " << voipPacketSize << ", " << worst_lat << ", " << avg_lat << ", " << lat97Per << ", " << lat99Per << ", " << worst_jit << ", " << avg_jit << ", " << jit97Per << ", " << jit99Per << ", " << throughput << ", " << pktStats[rit].aggDistance << ", " << pktStats[rit].avgPer << std::endl;
      histfile_latency << '\n' << '\n';
      histfile_jitter << '\n' << '\n';
  }
  histfile_latency.close();
  histfile_jitter.close();

  NS_LOG_UNCOND("\n---------------------------------------------------------- Data  Client STATS (full buffer)--------------------------------------  ");

  histfile_latency.open ("histfileLatencyFullBuffer.txt");
  histfile_jitter.open ("histfileJitterFullBuffer.txt");
  for (std::vector<pktStats_t>::size_type rit = 0; rit < dataPktStats.size(); rit++) {
      for(uint32_t delay_index = 0; delay_index < dataPktStats[rit].jitterStats.size(); delay_index ++) {
          histfile_jitter << dataPktStats[rit].jitterStats[delay_index]/1000000.0 << ",";
      }
      for(uint32_t delay_index = 0; delay_index < dataPktStats[rit].latencyStats.size(); delay_index ++) {
          histfile_latency << dataPktStats[rit].latencyStats[delay_index]/1000000.0 << ",";
      }
      std::sort(dataPktStats[rit].latencyStats.begin(), dataPktStats[rit].latencyStats.end());
      std::sort(dataPktStats[rit].jitterStats.begin(), dataPktStats[rit].jitterStats.end());

      double worst_lat = 0, avg_lat = 0;
      double lat97Per = 0, lat99Per = 0;
      double worst_jit = 0, avg_jit = 0;
      double jit97Per = 0, jit99Per = 0;
      delayJitterCounter = 0;
      for(uint32_t delay_index = 0; delay_index < dataPktStats[rit].jitterStats.size(); delay_index ++) {
          // Number of packets less than 10ms
          if(dataPktStats[rit].jitterStats[delay_index] < 10000000) {
              delayJitterCounter ++;
          }
      }
      if(delayJitterCounter) {
          delayJitterCounter = (100 * delayJitterCounter) / dataPktStats[rit].jitterStats.size();
      }

      delayLatencyCounter = 0;
      for(uint32_t delay_index = 0; delay_index < dataPktStats[rit].latencyStats.size(); delay_index ++) {
          // Number of packets less than 60ms
          if(dataPktStats[rit].latencyStats[delay_index] < 60000000) {
              delayLatencyCounter ++;
          }
      }
      if(delayLatencyCounter) {
          delayLatencyCounter = (100 * delayLatencyCounter) / dataPktStats[rit].latencyStats.size();
      }

      //Find the 97th percentile
      latency_percentile = (dataPktStats[rit].latencyStats.size() * 97) / 100;
      jitter_percentile = (dataPktStats[rit].jitterStats.size() * 97) / 100;
      //Find the 99th percentile
      latency_99_percentile = (dataPktStats[rit].latencyStats.size() * 99) / 100;
      jitter_99_percentile = (dataPktStats[rit].jitterStats.size() * 99) / 100;

#if _ENABLE_DEBUG_ 
      NS_LOG_UNCOND("### jitter_percentile : " << jitter_percentile << " value : " << dataPktStats[rit].jitterStats[jitter_percentile - 1]);
      for(uint32_t print_index = 0; print_index < dataPktStats[rit].jitterStats.size(); print_index ++) {
          NS_LOG_UNCOND("### latency : " << dataPktStats[rit].jitterStats[print_index]);
      }
#endif

      if (dataPktStats[rit].pktSent <= dataPktStats[rit].pktRecv)
        numPktDropped = 0;
      else
        numPktDropped = dataPktStats[rit].pktSent - dataPktStats[rit].pktRecv;
      numPktRecvd = dataPktStats[rit].pktRecv;
      NS_LOG_UNCOND(" Stats for Client  " << rit + 1 << " : " << " Packets Sent : " << dataPktStats[rit].pktSent << " Packets Recv : " << dataPktStats[rit].pktRecv << \
          " Packets Drop : " << numPktDropped); 
      NS_LOG_UNCOND("      Client Traffic Type : " << ClientTrafficTypeToStr(dataPktStats[rit].typeOfClientTraffic) << ", Packet Size : " << dataPacketSize); 
      NS_LOG_UNCOND("      Latency (ms) : " << " Worst latency : " << (dataPktStats[rit].worstLatency/base) << " avg latency : " << (dataPktStats[rit].avgLatency/base) << \
          " best latency : " << (dataPktStats[rit].bestLatency/base));
      worst_lat = dataPktStats[rit].worstLatency/base;
      avg_lat = dataPktStats[rit].avgLatency/base;
      if(latency_percentile && dataPktStats[rit].latencyStats[latency_percentile - 1]) {
          NS_LOG_UNCOND("      Latency (ms) : " << " 97th percentile latency : " << dataPktStats[rit].latencyStats[latency_percentile - 1] / base);
          lat97Per = dataPktStats[rit].latencyStats[latency_percentile - 1] / base;
      }
      if(latency_99_percentile && dataPktStats[rit].latencyStats[latency_99_percentile - 1]) {
          NS_LOG_UNCOND("      Latency (ms) : " << " 99th percentile latency : " << dataPktStats[rit].latencyStats[latency_99_percentile - 1] / base);
          lat99Per = dataPktStats[rit].latencyStats[latency_99_percentile - 1] / base;
      }

      throughput = (dataPktStats[rit].pktRecv * dataPacketSize * 8) / (simulatorDuration - dataPktStats[rit].startTime);
      NS_LOG_UNCOND("      Simulator Start Time : " << dataPktStats[rit].startTime << ", Simulator Duration : " << simulatorDuration);
      throughput = throughput / 1000;
      if(dataPktStats[rit].pktRecv > 2) {
       if(dataPktStats[rit].worstJitter && dataPktStats[rit].avgJitter && dataPktStats[rit].jitterStats[jitter_percentile - 1]) {
          NS_LOG_UNCOND("      Worst Jitter (milli sec) : " << dataPktStats[rit].worstJitter / base << " Avg Jitter (milli sec) : " << dataPktStats[rit].avgJitter / base << \
                           " 97th percentile jitter : " << dataPktStats[rit].jitterStats[jitter_percentile - 1] / base);
          worst_jit = dataPktStats[rit].worstJitter / base;
          avg_jit = dataPktStats[rit].avgJitter / base;
          jit97Per = dataPktStats[rit].jitterStats[jitter_percentile - 1] / base;
       }
      }
      if(jitter_99_percentile && dataPktStats[rit].jitterStats[jitter_99_percentile - 1]) {
      NS_LOG_UNCOND("      99th percentile jitter : " << dataPktStats[rit].jitterStats[jitter_99_percentile - 1] / base);
        jit99Per = dataPktStats[rit].jitterStats[jitter_99_percentile - 1] / base;
      }
      throughput_fullBuffer << throughput/1000 << ",";
      drop_fullBuffer << numPktRecvd << "," << numPktDropped << ",";
      dataPktStats[rit].dropTotal = numPktDropped;
      if ((numPktRecvd+numPktDropped) == 0)
        drop_fullBuffer_per << 0 << ",";
      else
        drop_fullBuffer_per << ((double)numPktDropped/(numPktRecvd+numPktDropped)*100) << ",";
      lat_fullBuffer << avg_lat << ",";
      jit_fullBuffer << avg_jit << ",";
      NS_LOG_UNCOND("      Throughput : " << throughput << " kbps" ",   Distance from AP (mts) : " << dataPktStats[rit].aggDistance);
      aggregateThroughput += throughput;
      double xCo, yCo, zCo;
      xCo = dataPktStats[rit].coordinates[0];
      yCo = dataPktStats[rit].coordinates[1];
      zCo = dataPktStats[rit].coordinates[2];
      NS_LOG_UNCOND("      AP Location : (" << apX << ", " << apY << ", " << apZ << ")" << "                     Station Location : (" << xCo << ", " << yCo << ", " << zCo << ")");
      xlocation_var << xCo;
      xlocation_var << ",";
      ylocation_var << yCo;
      ylocation_var << ",";
      if (dataPktStats[rit].pktRecv)
      NS_LOG_UNCOND("      Average Per : " << dataPktStats[rit].avgPer);
      NS_LOG_UNCOND("--------------------------------------------------------------------------------------------------------------------------------");
      resultsLog << dataPktStats[rit].pktSent << ", " << dataPktStats[rit].pktRecv << ", " << dataPacketSize << ", " << worst_lat << ", " << avg_lat << ", " << lat97Per << ", " << lat99Per << ", " << worst_jit << ", " << avg_jit << ", " << jit97Per << ", " << jit99Per << ", " << throughput << ", " << dataPktStats[rit].aggDistance << ", " << dataPktStats[rit].avgPer << std::endl;
      histfile_latency << '\n' << '\n';
      histfile_jitter << '\n' << '\n';
}
  histfile_latency.close();
  histfile_jitter.close();

  NS_LOG_UNCOND("\n---------------------------------------------------------- Video  Client STATS -------------------------------------------------  ");

  histfile_latency.open ("histfileLatencyVideo.txt");
  histfile_jitter.open ("histfileJitterVideo.txt");
  for (std::vector<pktStats_t>::size_type rit = 0; rit < videoPktStats.size(); rit++) {
      for(uint32_t delay_index = 0; delay_index < videoPktStats[rit].jitterStats.size(); delay_index ++) {
          histfile_jitter << videoPktStats[rit].jitterStats[delay_index]/1000000.0 << ",";
      }
      for(uint32_t delay_index = 0; delay_index < videoPktStats[rit].latencyStats.size(); delay_index ++) {
          histfile_latency << videoPktStats[rit].latencyStats[delay_index]/1000000.0 << ",";
      }
      std::sort(videoPktStats[rit].latencyStats.begin(), videoPktStats[rit].latencyStats.end());
      std::sort(videoPktStats[rit].jitterStats.begin(), videoPktStats[rit].jitterStats.end());

      double worst_lat = 0, avg_lat = 0;
      double lat97Per = 0, lat99Per = 0;
      double worst_jit = 0, avg_jit = 0;
      double jit97Per = 0, jit99Per = 0;
      delayJitterCounter = 0;
      for(uint32_t delay_index = 0; delay_index < videoPktStats[rit].jitterStats.size(); delay_index ++) {
          // Number of packets less than 10ms
          if(videoPktStats[rit].jitterStats[delay_index] < 10000000) {
              delayJitterCounter ++;
          }
      }
      if(delayJitterCounter) {
          delayJitterCounter = (100 * delayJitterCounter) / videoPktStats[rit].jitterStats.size();
      }

      delayLatencyCounter = 0;
      for(uint32_t delay_index = 0; delay_index < videoPktStats[rit].latencyStats.size(); delay_index ++) {
          // Number of packets less than 60ms
          if(videoPktStats[rit].latencyStats[delay_index] < 60000000) {
              delayLatencyCounter ++;
          }
      }
      if(delayLatencyCounter) {
          delayLatencyCounter = (100 * delayLatencyCounter) / videoPktStats[rit].latencyStats.size();
      }

      //Find the 97th percentile
      latency_percentile = (videoPktStats[rit].latencyStats.size() * 97) / 100;
      jitter_percentile = (videoPktStats[rit].jitterStats.size() * 97) / 100;
      //Find the 99th percentile
      latency_99_percentile = (videoPktStats[rit].latencyStats.size() * 99) / 100;
      jitter_99_percentile = (videoPktStats[rit].jitterStats.size() * 99) / 100;

#if _ENABLE_DEBUG_ 
      NS_LOG_UNCOND("### jitter_percentile : " << jitter_percentile << " value : " << videoPktStats[rit].jitterStats[jitter_percentile - 1]);
      for(uint32_t print_index = 0; print_index < videoPktStats[rit].jitterStats.size(); print_index ++) {
          NS_LOG_UNCOND("### latency : " << videoPktStats[rit].jitterStats[print_index]);
      }
#endif

      if (videoPktStats[rit].pktSent <= videoPktStats[rit].pktRecv)
        numPktDropped = 0;
      else
        numPktDropped = videoPktStats[rit].pktSent - videoPktStats[rit].pktRecv;
      numPktRecvd = videoPktStats[rit].pktRecv;
      NS_LOG_UNCOND(" Stats for Client  " << rit + 1 << " : " << " Packets Sent : " << videoPktStats[rit].pktSent << " Packets Recv : " << videoPktStats[rit].pktRecv << \
          " Packets Drop : " << numPktDropped); 
      NS_LOG_UNCOND("      Client Traffic Type : " << ClientTrafficTypeToStr(videoPktStats[rit].typeOfClientTraffic) << " Avg Frame Size : " << videoPktStats[rit].avgPktSize); 
      NS_LOG_UNCOND("      Latency (ms) : " << " Worst latency : " << (videoPktStats[rit].worstLatency/base) << " avg latency : " << (videoPktStats[rit].avgLatency/base) << \
          " best latency : " << (videoPktStats[rit].bestLatency/base));
      worst_lat = videoPktStats[rit].worstLatency/base;
      avg_lat = videoPktStats[rit].avgLatency/base;
      if(latency_percentile && videoPktStats[rit].latencyStats[latency_percentile - 1]) {
          NS_LOG_UNCOND("      Latency (ms) : " << " 97th percentile latency : " << videoPktStats[rit].latencyStats[latency_percentile - 1] / base);
          lat97Per = videoPktStats[rit].latencyStats[latency_percentile - 1] / base;
      }
      if(latency_99_percentile && videoPktStats[rit].latencyStats[latency_99_percentile - 1]) {
          NS_LOG_UNCOND("      Latency (ms) : " << " 99th percentile latency : " << videoPktStats[rit].latencyStats[latency_99_percentile - 1] / base);
          lat99Per = videoPktStats[rit].latencyStats[latency_99_percentile - 1] / base;
      }

      throughput = (videoPktStats[rit].pktRecv * videoPktStats[rit].avgPktSize * 8) / (simulatorDuration - videoPktStats[rit].startTime);
      NS_LOG_UNCOND("      Simulator Start Time : " << videoPktStats[rit].startTime << ", Simulator Duration : " << simulatorDuration);
      throughput = throughput / 1000;

      if(videoPktStats[rit].pktRecv > 2) {
        if(videoPktStats[rit].worstJitter && videoPktStats[rit].avgJitter && videoPktStats[rit].jitterStats[jitter_percentile - 1]) {
          NS_LOG_UNCOND("      Worst Jitter (milli sec) : " << videoPktStats[rit].worstJitter / base << " Avg Jitter (milli sec) : " << videoPktStats[rit].avgJitter / base << \
                           " 97th percentile jitter : " << videoPktStats[rit].jitterStats[jitter_percentile - 1] / base);
          worst_jit = videoPktStats[rit].worstJitter / base;
          avg_jit = videoPktStats[rit].avgJitter / base;
          jit97Per = videoPktStats[rit].jitterStats[jitter_percentile - 1] / base;
        }
      }
      if(jitter_99_percentile && videoPktStats[rit].jitterStats[jitter_99_percentile - 1]) {
      NS_LOG_UNCOND("      99th percentile jitter : " << videoPktStats[rit].jitterStats[jitter_99_percentile - 1] / base);
        jit99Per = videoPktStats[rit].jitterStats[jitter_99_percentile - 1]/base;
      }
      throughput_video << throughput/1000 << ",";
      drop_video << numPktRecvd << "," << numPktDropped << ",";
      videoPktStats[rit].dropTotal = numPktDropped;
      if ((numPktRecvd+numPktDropped) == 0)
        drop_video_per << 0 << ",";
      else
        drop_video_per << ((double)numPktDropped/(numPktRecvd+numPktDropped)*100) << ",";
      lat_video << worst_lat << ",";
      lat_video_97 << lat97Per << ",";
      lat_video_99 << lat99Per << ",";
      jit_video << worst_jit << ",";
      jit_video_97 << jit97Per << ",";
      jit_video_99 << jit99Per << ",";
      NS_LOG_UNCOND("      Throughput : " << throughput << " kbps" ",   Distance from AP (mts) : " << videoPktStats[rit].aggDistance);
      aggregateThroughput += throughput;
      double xCo, yCo, zCo;
      xCo = videoPktStats[rit].coordinates[0];
      yCo = videoPktStats[rit].coordinates[1];
      zCo = videoPktStats[rit].coordinates[2];
      NS_LOG_UNCOND("      AP Location : (" << apX << ", " << apY << ", " << apZ << ")" << "                     Station Location : (" << xCo << ", " << yCo << ", " << zCo << ")");
      xlocation_var << xCo;
      xlocation_var << ",";
      ylocation_var << yCo;
      ylocation_var << ",";
      if (videoPktStats[rit].pktRecv)
      NS_LOG_UNCOND("      Average Per : " << videoPktStats[rit].avgPer);
      NS_LOG_UNCOND("--------------------------------------------------------------------------------------------------------------------------------");
      resultsLog << videoPktStats[rit].pktSent << ", " << videoPktStats[rit].pktRecv << ", " << voipPacketSize << ", " << worst_lat << ", " << avg_lat << ", " << lat97Per << ", " << lat99Per << ", " << worst_jit << ", " << avg_jit << ", " << jit97Per << ", " << jit99Per << ", " << throughput << ", " << videoPktStats[rit].aggDistance << ", " << videoPktStats[rit].avgPer << std::endl;
      histfile_latency << '\n' << '\n';
      histfile_jitter << '\n' << '\n';
}
  resultsLog << std::endl;
  histfile_latency.close();
  histfile_jitter.close();

  throughput_voip.close();
  throughput_fullBuffer.close();
  throughput_video.close();
  drop_voip.close();
  drop_fullBuffer.close();
  drop_video.close();
  drop_voip_per.close();
  drop_fullBuffer_per.close();
  drop_video_per.close();
  lat_voip.close();
  lat_voip_97.close();
  lat_voip_99.close();
  lat_fullBuffer.close();
  lat_video.close();
  lat_video_97.close();
  lat_video_99.close();
  jit_voip.close();
  jit_voip_97.close();
  jit_voip_99.close();
  jit_fullBuffer.close();
  jit_video.close();
  jit_video_97.close();
  jit_video_99.close();
  xlocation_var.close();
  ylocation_var.close();

  double highLoadingAxResourceUL = 0;
  double usedAxResourceUL = 0;
  double totalAxResourceUL = 0;
  double highLoadingAxResourceDL = 0;
  double usedAxResourceDL = 0;
  double totalAxResourceDL = 0;
  for (uint32_t index = 0; index < nVideoStas + nDataStas + nVoipStas ; index++)
  {
    highLoadingAxResourceUL = highLoadingAxResourceUL + devices.Get(index+1)->GetObject<WifiNetDevice>()->GetMac()->GetObject<StaWifiMac>()->GetHighLoadingAxResourceUL();
    usedAxResourceUL = usedAxResourceUL + devices.Get(index+1)->GetObject<WifiNetDevice>()->GetMac()->GetObject<StaWifiMac>()->GetUsedAxResourceUL();
  }
  //totalAxResourceUL = apDevice.Get(0)->GetObject<WifiNetDevice>()->GetRemoteStationManager()->GetObject<RRMWifiManager>()->GetTotalAxResourceULTime().GetMilliSeconds()*242;
  //totalAxResourceDL = apDevice.Get(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<ApWifiMac>()->GetMacLow()->GetTotalAxResourceDL();
  highLoadingAxResourceDL = apDevice.Get(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<ApWifiMac>()->GetMacLow()->GetHighLoadingAxResourceDL();
  usedAxResourceDL = apDevice.Get(0)->GetObject<WifiNetDevice>()->GetMac()->GetObject<ApWifiMac>()->GetMacLow()->GetUsedAxResourceDL();
  NS_LOG_UNCOND("\n\n-------------------DownLink-------------------");
  NS_LOG_UNCOND("(DS or Edge Load/Total Load) : " << highLoadingAxResourceDL/usedAxResourceDL*100 << " percent and Time Duration : " << (Now()-Seconds(2.0)));
  NS_LOG_UNCOND(" Total Resource Available : " << totalAxResourceDL << " Used Resource : " << usedAxResourceDL << " Unused Resource : " << totalAxResourceDL-usedAxResourceDL);
  NS_LOG_UNCOND("\n--------------------UpLink--------------------");
  NS_LOG_UNCOND("(DS or Edge Load/Total Load) : " << highLoadingAxResourceUL/usedAxResourceUL*100 << " percent and Time Duration : " << (Now()-Seconds(2.0)));
  NS_LOG_UNCOND(" Total Resource Available : " << totalAxResourceUL << " Used Resource : " << usedAxResourceUL << " Unused Resource : " << totalAxResourceUL-usedAxResourceUL);
  NS_LOG_UNCOND("\n--------------------Throughput--------------------");
  NS_LOG_UNCOND(" Aggegate Throughput in AP : " << aggregateThroughput << " kbps");
  NS_LOG_UNCOND("\n--------------------Queue Drop Stats--------------------");
  double afterQueueDrop = 0;
  double beforeQueueDrop = 0;

  if (downlink == false){
    for (uint32_t index = 0; index < nVideoStas + nDataStas + nVoipStas ; index++)
    {
      for(uint32_t queueIndex = 0; queueIndex<4; queueIndex++){
        afterQueueDrop = devices.Get(index+1)->GetObject<WifiNetDevice>()->GetMac()->GetObject<StaWifiMac>()->GetAfterQueueDrop(queueIndex);
        beforeQueueDrop = devices.Get(index+1)->GetObject<WifiNetDevice>()->GetMac()->GetObject<StaWifiMac>()->GetBeforeQueueDrop(queueIndex);
        if(index < nVoipStas){
          pktStats[index].dropAfterQueue += afterQueueDrop;
          pktStats[index].dropBeforeQueue += beforeQueueDrop;
        }
        else if(index >= nVoipStas && index < nDataStas+nVoipStas){
          dataPktStats[index].dropAfterQueue += afterQueueDrop;
          dataPktStats[index].dropBeforeQueue += beforeQueueDrop;
        }
        else if(index >= nDataStas+nVoipStas && index < nVideoStas+nDataStas+nVoipStas){
          videoPktStats[index].dropAfterQueue += afterQueueDrop;
          videoPktStats[index].dropBeforeQueue += beforeQueueDrop;
        }
        //NS_LOG_UNCOND("For StaId : " << index+1 << " and Queue : " << queueIndex << " after queueDrop : " << afterQueueDrop);
        //NS_LOG_UNCOND("For StaId : " << index+1 << " and Queue : " << queueIndex << " before queueDrop : " << beforeQueueDrop);
      }
    }
  }
  std::ifstream qDropLog("queueDrops.txt");
  char inStr[255];
  while(qDropLog) {
    qDropLog.getline(inStr, 255);  // delim defaults to '\n'
    std::vector<uint32_t> vect;
    std::stringstream ss(inStr);
    uint32_t i;
    while (ss >> i)
    {
        vect.push_back(i);
        if (ss.peek() == ',')
            ss.ignore();
    }
    if (vect.size() == 3){
      if(vect.at(0) > 1 && vect.at(0) <= nVoipStas+1){
        pktStats[vect.at(0)-2].dropAfterQueue += vect.at(1);
        pktStats[vect.at(0)-2].dropBeforeQueue += vect.at(2);
      }
      else if (vect.at(0) > nVoipStas+1 && vect.at(0) <= nDataStas+nVoipStas+1){
        dataPktStats[vect.at(0)-2-nVoipStas].dropAfterQueue += vect.at(1);
        dataPktStats[vect.at(0)-2-nVoipStas].dropBeforeQueue += vect.at(2);
      }
      else if (vect.at(0) > nDataStas+nVoipStas+1 && vect.at(0) <= nVideoStas+nDataStas+nVoipStas+1){
        videoPktStats[vect.at(0)-2-nVoipStas-nDataStas].dropAfterQueue += vect.at(1);
        videoPktStats[vect.at(0)-2-nVoipStas-nDataStas].dropBeforeQueue += vect.at(2);
      }
    }
  }
  std::ofstream allDropVoip("DropsAllVoip.txt");
  std::ofstream allDropFullBuffer("DropsAllFullBuffer.txt");
  std::ofstream allDropVideo("DropsAllVideo.txt");
  for(uint32_t staIndex = 0; staIndex < nVoipStas; staIndex ++ ) {
    if( (pktStats[staIndex].dropTotal - pktStats[staIndex].dropAfterQueue) < pktStats[staIndex].dropBeforeQueue) {
        pktStats[staIndex].dropBeforeQueue = pktStats[staIndex].dropTotal - pktStats[staIndex].dropAfterQueue;
    }
    allDropVoip << pktStats[staIndex].dropTotal << "," << pktStats[staIndex].dropAfterQueue << "," << pktStats[staIndex].dropBeforeQueue << ",";
  }

  for(uint32_t staIndex = 0; staIndex < nDataStas; staIndex ++ ) {
    if( (dataPktStats[staIndex].dropTotal - dataPktStats[staIndex].dropAfterQueue) < dataPktStats[staIndex].dropBeforeQueue) {
        dataPktStats[staIndex].dropBeforeQueue = dataPktStats[staIndex].dropTotal - dataPktStats[staIndex].dropAfterQueue;
    }
    allDropFullBuffer << dataPktStats[staIndex].dropTotal << "," << dataPktStats[staIndex].dropAfterQueue << "," << dataPktStats[staIndex].dropBeforeQueue << ",";
  }

  for(uint32_t staIndex = 0; staIndex < nVideoStas; staIndex ++ ) {
    if( (videoPktStats[staIndex].dropTotal - videoPktStats[staIndex].dropAfterQueue) < videoPktStats[staIndex].dropBeforeQueue) {
        videoPktStats[staIndex].dropBeforeQueue = videoPktStats[staIndex].dropTotal - videoPktStats[staIndex].dropAfterQueue;
    }
    allDropVideo << videoPktStats[staIndex].dropTotal << "," << videoPktStats[staIndex].dropAfterQueue << "," << videoPktStats[staIndex].dropBeforeQueue << ",";
  }
  allDropVoip.close();
  allDropFullBuffer.close();
  allDropVideo.close();

  /*for(uint32_t index = 0; index < nVoipStas; index ++) {
     NS_LOG_UNCOND("Drop Per for Voip Sta " << index+1 << " is : " << pktStats[index].dropPer);
  }

  for(uint32_t index = 0; index < nDataStas; index ++) {
     NS_LOG_UNCOND("Drop Per for Data Sta " << index+1 << " is : " << dataPktStats[index].dropPer);
  }

  for(uint32_t index = 0; index < nVideoStas; index ++) {
     NS_LOG_UNCOND("Drop Per for Video Sta " << index+1 << " is : " << videoPktStats[index].dropPer);
  }*/

  std::string location("Varying");
  if(downlink == true) { 
    table_create << nVoipStas << "," << nVideoStas << "," << nDataStas<< "," << "Downlink" << "," << location << "," << "17dBm" << "," << highLoadingAxResourceDL/usedAxResourceDL*100 << ",";
  } else {
    table_create << nVoipStas << "," << nVideoStas << "," << nDataStas<< "," << "Uplink" << "," << location << "," << "17dBm" << "," << highLoadingAxResourceDL/usedAxResourceDL*100 << ","; 
  }
  table_create << highLoadingAxResourceUL/usedAxResourceUL*100 << "," << aggregateThroughput/1000 <<",";
  table_create << simulatorDuration <<",";

  table_create_combined << nVoipStas << "," << nVideoStas << "," << nDataStas<< "," << location << "," << simulatorDuration;
  throughputBarPlot << aggregateThroughput/1000;

  if(downlink == true) {
     ruliBarPlot << highLoadingAxResourceDL/usedAxResourceDL*100;
  } else {
     ruliBarPlot << highLoadingAxResourceUL/usedAxResourceUL*100;
  }

  Simulator::Destroy ();

  return 0;
}

