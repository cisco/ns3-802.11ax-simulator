/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Cisco and/or its affiliates
 *
 * This file also contains code from the following file(s) from ns-3.26:
 * Filename : aarf-wifi-manager.cc
 * Copyright (c) 2004,2005,2006 INRIA
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *
 * Filename : ideal-wifi-manager.cc
 * Copyright (c) 2006 INRIA
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
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

#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/type-id.h"
#include "ns3/string.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/he-bitmap.h"
#include "snr-tag.h"
#include "wifi-mac-queue.h"
#include "mac-tx-middle.h"
#include "wifi-mac-trailer.h"
#include "wifi-mac.h"
#include "random-stream.h"
#include "ns3/dcf.h"
#include "dcf-manager.h"
#include "rrm-wifi-manager.h"
#include "regular-wifi-mac.h"
#include "ampdu-tag.h"
#include <algorithm>
#include <cstring>
#include <cstdint>

#include <unistd.h>                                                      
#include <stdio.h>                                                       
#include <netdb.h>                                                       
#include <sys/types.h>                                                   
#include <sys/socket.h>                                                  
#include <netinet/in.h>                                                  
#include <sstream>                                                       
#include <iomanip>                                                       
#include <strings.h>                                                     
#include <stdlib.h>                                                      
#include <string>                                                        
#include <time.h>                                                        
#include "tlv.h"

#define PORTNUM 8888
#define SERVERIP "localhost"
int sd = 0 ;
#define AC_VO 3
#define AC_VI 2
#define AC_BK 1
#define AC_BE 0

#define RRM_MAC_COPY(dst,src) \
  dst[0] = src[0]; \
  dst[1] = src[1]; \
  dst[2] = src[2]; \
  dst[3] = src[3]; \
  dst[4] = src[4]; \
  dst[5] = src[5];



namespace ns3 {


/// To avoid using the cache before a valid value has been cached
static const double CACHE_INITIAL_VALUE = -100;

NS_LOG_COMPONENT_DEFINE ("RRMWifiManager");

NS_OBJECT_ENSURE_REGISTERED (RRMWifiManager);

class RRMWifiManager::Dcf : public DcfState
{
public:
  Dcf (RRMWifiManager * rrmManager)
    : m_RRMWifiManager (rrmManager)
  {
  }
  virtual bool IsEdca (void) const
  {
    return false;
  }
private:
  virtual void DoNotifyAccessGranted (void)
  {
    m_RRMWifiManager->NotifyAccessGranted ();
  }
  virtual void DoNotifyInternalCollision (void)
  {
    m_RRMWifiManager->NotifyInternalCollision ();
  }
  virtual void DoNotifyCollision (void)
  {
    m_RRMWifiManager->NotifyCollision ();
  }
  virtual void DoNotifyChannelSwitching (void)
  {
    m_RRMWifiManager->NotifyChannelSwitching ();
  }
  virtual void DoNotifySleep (void)
  {
    m_RRMWifiManager->NotifySleep ();
  }
  virtual void DoNotifyWakeUp (void)
  {
    m_RRMWifiManager->NotifyWakeUp ();
  }

  RRMWifiManager *m_RRMWifiManager;
};

int 
RRMWifiManager::SendDLStations(bool isEmptyBufReq){
    short stationids[] = {1,2,3,56,123};
    TlvBuffer* message = (TlvBuffer*)malloc(sizeof(TlvBuffer));          
    memset(message,0x00,sizeof(TlvBuffer));                              
                                                                                 
    memset(message->data,0x00,BUFFER_DATA_MAX_SIZE);
    tlvEncode2ByteIntArray(message,TYPE_11AX_DL_STATION_MAC_RESP,stationids,5);
    write(sd,(void*)message,sizeof(TlvBuffer));
    free(message);
    return 1;
}
int 
RRMWifiManager::SendULStations(bool isEmptyBufReq){
    short stationids[] = {1,10,6,123};
    TlvBuffer* message = (TlvBuffer*)malloc(sizeof(TlvBuffer));          
    memset(message,0x00,sizeof(TlvBuffer));                              
                                                                                 
    memset(message->data,0x00,BUFFER_DATA_MAX_SIZE);
    tlvEncode2ByteIntArray(message,TYPE_11AX_UL_STATION_MAC_RESP,stationids,4);
    write(sd,(void*)message,sizeof(TlvBuffer));
    free(message);

    return 1;
}

int 
RRMWifiManager::SendAllInfo(bool isEmptyReq)
{
  uint16_t lastServedStation = 3, totalAxStations = m_axStations.size(), i;
  AllStats_t clientArray[100];
  uint32_t index = 0;
  Ptr<WifiMacQueue> queue;
  WifiMacHeader hdr;
  //uint32_t t_ac = 0;

  memset(&clientArray, '\0', sizeof(clientArray));
  for (i = lastServedStation + 1; i < totalAxStations; i+=4)
  {
      m_axStations[i]->m_state->m_address.CopyTo(clientArray[index].macStr);
      uint32_t mcs = 0, mcsVal;
      for (uint32_t ac  = 0; ac < AC_BE_NQOS; ac++)
        {
	  //Downlink buffer informtion
	  clientArray[index].bufferDepthDL[ac] = m_axStations[i+ac]->lt->GetQueue ()->GetBytes ();
          clientArray[index].WaitingTimeDL[ac] = std::round(Now().GetMilliSeconds() - m_axStations[i+ac]->lt->PeekFirstPacket(&hdr).GetMilliSeconds());
          clientArray[index].ThroughputDL[ac] = m_axStations[i+ac]->lt->GetQueue ()->GetThroughput();
	  //Uplink buffer information
          clientArray[index].bufferDepthUL[ac] = m_axStations[i+ac]->ulBufferStat.bufLen;
          clientArray[index].WaitingTimeUL[ac] = std::round(Now().GetMilliSeconds() - m_axStations[i+ac]->ulBufferStat.time.GetMilliSeconds());
          mcsVal = rateControlIdeal(m_axStations[i+ac]);
          if ( mcsVal != 0xff)
          mcs = mcsVal;
        }
        clientArray[index].mcsVal = mcs;
        //std::cout << "For station with mac " << +clientArray[index].macStr[4] << +clientArray[index].macStr[5] << " mcs : " << clientArray[index].mcsVal << std::endl;
      index++;
  }

  TlvBuffer* message = (TlvBuffer*)malloc(sizeof(TlvBuffer));
  memset(message, 0x00, sizeof(TlvBuffer));

  memset(message->data, 0x00, BUFFER_DATA_MAX_SIZE);

  if (index*sizeof(AllStats_t) > BUFFER_DATA_MAX_SIZE)
    {
      // memory not enough
      NS_ASSERT_MSG(false, "Memory is not enough for all client info");
    }

  tlvEncodeAllStats(message, TYPE_11AX_ALL_STATS_RESP, clientArray, index); //last is number of clients
  write(sd, (void*)message, sizeof(TlvBuffer));
  free(message);
  return 1;
}


int 
RRMWifiManager::GetRUMCS(short station_id,short *mcs,short *ru){
   /*To be Implemented:Right now sending dummy MCS values and RU values*/
   if(station_id < 10){
	*mcs = 10;
	*ru = 40;
        return 1;
   }
   else if(station_id < 20){
	*mcs = 20;
	*ru = 50;
        return 1;
   }
   else{
	*mcs = 30;
	*ru = 60;
	return 1;
   }
}
int 
RRMWifiManager::HandleDLMCSInfoRequest(TlvBuffer* message){
    char flagRU = false;
    short len,respLen;  
    short stationCount;
    short stationId;
    short mcs,ru;
    short i;   

    TlvBuffer* mcsInfoResp = (TlvBuffer*)malloc(sizeof(TlvBuffer));
    memset(mcsInfoResp,0x00,sizeof(TlvBuffer));

    memset(mcsInfoResp->data,0x00,BUFFER_DATA_MAX_SIZE);

    tlvReadLen(message,&len);
   tlvDecode1Byte(message,&flagRU);
    stationCount = (len-1)/sizeof(short); 
    if(flagRU == false){
	for(i = 0 ;i<stationCount;i++)
	{
	    if(i == 0)
	    {
		//Set the Type and Len
		respLen = 3*sizeof(short)*stationCount;//(StationId,MCS,RU)triplets
		tlvWriteTypeLen(mcsInfoResp,TYPE_11AX_DL_MCS_INFO_RESP,respLen);
	    }
	    tlvDecodeAppended2Bytes(message,&stationId);
            GetRUMCS(stationId,&mcs,&ru);
	    tlvAppend2Bytes(mcsInfoResp,stationId);
            tlvAppend2Bytes(mcsInfoResp,mcs);              
            tlvAppend2Bytes(mcsInfoResp,ru);    
	}
	write(sd,(void*)mcsInfoResp,sizeof(TlvBuffer));
	free(mcsInfoResp);
        return 1;
    }else{
	return 1;	
   }
}

int 
RRMWifiManager::HandleULMCSInfoRequest(TlvBuffer* message){
    char flagRU = false;
    short len,respLen;  
    short stationCount;
    short stationId;
    short mcs,ru;
    short i;   

    TlvBuffer* mcsInfoResp = (TlvBuffer*)malloc(sizeof(TlvBuffer));
    memset(mcsInfoResp,0x00,sizeof(TlvBuffer));

    memset(mcsInfoResp->data,0x00,BUFFER_DATA_MAX_SIZE);

    tlvReadLen(message,&len);
    tlvDecode1Byte(message,&flagRU);
    stationCount = (len-1)/sizeof(short); 
    if(flagRU == false){
	for(i = 0 ;i<stationCount;i++)
	{
	    if(i == 0)
	    {
		//Set the Type and Len
		respLen = 3*sizeof(short)*stationCount;//(StationId,MCS,RU)triplets
		tlvWriteTypeLen(mcsInfoResp,TYPE_11AX_UL_MCS_INFO_RESP,respLen);
	    }
	    tlvDecodeAppended2Bytes(message,&stationId);
            GetRUMCS(stationId,&mcs,&ru);
	    tlvAppend2Bytes(mcsInfoResp,stationId);
            tlvAppend2Bytes(mcsInfoResp,mcs);              
            tlvAppend2Bytes(mcsInfoResp,ru);    
	}
	write(sd,(void*)mcsInfoResp,sizeof(TlvBuffer));
	free(mcsInfoResp);
        return 1;
    }else{
	return 1;	
   }
}
int 
RRMWifiManager::GetBufferDepth(short stationId,bool isDL,short* bufferDepth){
    if(isDL){
	if(stationId <10)
	{
	    *bufferDepth = 2000;
            return 1;
        }else if(stationId <20){
	    *bufferDepth = 3000;
	     return 1;
	}else{
   	    *bufferDepth = 4000;
             return 1;
	}
    }
    else{
	if(stationId <10)
	{
	    *bufferDepth = 200;
            return 1;
        }else if(stationId <20){
	    *bufferDepth = 300;
	     return 1;
	}else{
   	    *bufferDepth = 400;
             return 1;
	}
    }
	return 1;
}
int 
RRMWifiManager::GetWaitingTime(short stationId,bool isDL,short* waitingTime){
    if(isDL){
	if(stationId <10)
	{
	    *waitingTime = 34;
            return 1;
        }else if(stationId <20){
	    *waitingTime = 37;
	     return 1;
	}else{
   	    *waitingTime = 40;
             return 1;
	}
    }
    else{
	if(stationId <10)
	{
	    *waitingTime = 230;
            return 1;
        }else if(stationId <20){
	    *waitingTime = 340;
	     return 1;
	}else{
   	    *waitingTime = 460;
             return 1;
	}
    }
	return 1;
}
int 
RRMWifiManager::HandleBufferDepthRequest(TlvBuffer* message,bool isDL){
    short len,respLen;  
    short stationCount;
    short stationId;
    short i;   
    short bufferDepth;

    printf("The buffer depth request\n");
    print_bytes(message->data,BUFFER_DATA_MAX_SIZE);

    TlvBuffer* bufferDepthResp = (TlvBuffer*)malloc(sizeof(TlvBuffer));
    memset(bufferDepthResp,0x00,sizeof(TlvBuffer));

    memset(bufferDepthResp->data,0x00,BUFFER_DATA_MAX_SIZE);

    tlvReadLen1(message,&len);
    stationCount = (len-1)/sizeof(short); 
    for(i = 0;i<stationCount;i++)
    {
	if(i == 0)
	{
	    //Set the Type and Len
	    respLen = 2*sizeof(short)*stationCount;//(StationId,BufferDepth)pairs
	    if(isDL)
	        tlvWriteTypeLen(bufferDepthResp,TYPE_11AX_DL_BUFFER_DEPTH_RESP,respLen);
	    else
		tlvWriteTypeLen(bufferDepthResp,TYPE_11AX_UL_BUFFER_DEPTH_RESP,respLen);
        }
	tlvDecodeAppended2Bytes(message,&stationId);
	GetBufferDepth(stationId,isDL,&bufferDepth);
	tlvAppend2Bytes(bufferDepthResp,stationId);
        tlvAppend2Bytes(bufferDepthResp,bufferDepth);
   }
   printf("The buffer depth response is \n");
   print_bytes(bufferDepthResp->data,BUFFER_DATA_MAX_SIZE);
   write(sd,(void*)bufferDepthResp,sizeof(TlvBuffer));
   free(bufferDepthResp);
   return 1;
}

int 
RRMWifiManager::HandleWaitingTimeRequest(TlvBuffer* message,bool isDL){
    short len,respLen;  
    short stationCount;
    short stationId;
    short i;   
    short waitingTime;

    TlvBuffer* waitingTimeResp = (TlvBuffer*)malloc(sizeof(TlvBuffer));
    memset(waitingTimeResp,0x00,sizeof(TlvBuffer));

    memset(waitingTimeResp->data,0x00,BUFFER_DATA_MAX_SIZE);

    tlvReadLen1(message,&len);
    stationCount = (len-1)/sizeof(short); 
    for(i = 0;i<stationCount;i++)
    {
	if(i == 0)
	{
	    //Set the Type and Len
	    respLen = 2*sizeof(short)*stationCount;//(StationId,BufferDepth)pairs
	    if(isDL)
	        tlvWriteTypeLen(waitingTimeResp,TYPE_11AX_DL_WAITING_TIME_RESP,respLen);
	    else
		tlvWriteTypeLen(waitingTimeResp,TYPE_11AX_UL_WAITING_TIME_RESP,respLen);
        }
	tlvDecodeAppended2Bytes(message,&stationId);
	GetWaitingTime(stationId,isDL,&waitingTime);
	tlvAppend2Bytes(waitingTimeResp,stationId);
        tlvAppend2Bytes(waitingTimeResp,waitingTime);
   }              
   printf("The waitingTimeResponse is\n");
   print_bytes(waitingTimeResp->data,100);
   write(sd,(void*)waitingTimeResp,sizeof(TlvBuffer));
   free(waitingTimeResp);
   return 1;
}

void
RRMWifiManager::PrintCurrentTime()
{ 
  time_t tt;
  
  struct tm * ti;
  time (&tt);
  ti = localtime(&tt);
  std::cout << "Current Day, Date and Time is = " << asctime(ti) << std::endl;
}

bool 
RRMWifiManager::HandleRRMResults(TlvBuffer* message, bool isDownlink)
{
	short i;
	short count;
	RRMClientResponse_t* respArray = 0;
        uint32_t axStationIndex = 0;
        WifiTxVector txVector;
        ServingStations servingStations;

        servingStations.clear();
	tlvDecodeResults(message,&respArray,&count);
	for(i = 0;i<count;i++)
	{
#if 0
		printf("\n\nRRM Results of Client[%d]:\n",i+1);
		print_bytes(respArray[i].macStr,6);
		printf("Traffic Type:%d\n",respArray[i].trafficType);
		printf("RU Bit Map:%d\n",respArray[i].ruBitMap);
		printf("MCS Value:%d\n",respArray[i].mcsValue);
		printf("Channel Width : %d\n",respArray[i].chanW);
                PrintCurrentTime();
#endif
                axStationIndex = FectchAxStationIndexFromMac(respArray[i].macStr, respArray[i].trafficType);
                //printf("AX Index : %d\n", axStationIndex);
                txVector = DoGetDataTxVector (m_axStations[axStationIndex], respArray[i].ruBitMap, m_axStations[axStationIndex]->m_aid, respArray[i].mcsValue);
                txVector.SetRu(respArray[i].ruBitMap);
                txVector.SetChannelWidth(respArray[i].chanW);
                txVector.SetAid(m_axStations[axStationIndex]->m_aid);
                m_axStations[axStationIndex]->dataTxVector = txVector;
                servingStations.push_back(m_axStations[axStationIndex]);
	}

	if(respArray)
	  {
            free(respArray);
	  }
        if(count > 0) {
            return StartTranmission(isDownlink, servingStations);
        }
        return false;
}

int 
RRMWifiManager::ProcessTlvMessage(TlvBuffer* message, bool isDownlink)
{
    char isEmptyReq = false;
    short type;
    char isEndAlgo = false;

    if(message == NULL) {
        printf("The tlv message is Null\n");
        return 1;
    }

    //print_bytes(message->data,100);
    while(message->read_offset <= message->len){
        tlvReadType(message,&type);
	if(message->read_offset > message->len) {//Indicates that data is complete
                return 0;
        } 
        //printf("The type Receievd is %d\n",type);
        switch(type){
	    case TYPE_11AX_ALL_STATS_REQ:
		tlvDecode1Byte(message,&isEmptyReq);
		SendAllInfo(isEmptyReq);
		break;
	    case TYPE_11AX_RRM_RESULTS_RESP:
		HandleRRMResults(message, isDownlink);
		return 1;      // End the State machine.
		break;
            case TYPE_11AX_DL_STATION_MAC_REQ:
                tlvDecode1Byte(message,&isEmptyReq);
                SendDLStations(isEmptyReq);
                break;
            case TYPE_11AX_UL_STATION_MAC_REQ:
                tlvDecode1Byte(message,&isEmptyReq);
                SendULStations(isEmptyReq);
	        break;
            case TYPE_11AX_DL_MCS_INFO_REQ:
        	HandleDLMCSInfoRequest(message);
	        break;
            case TYPE_11AX_UL_MCS_INFO_REQ:
                HandleULMCSInfoRequest(message);
	        break;
            case TYPE_11AX_DL_BUFFER_DEPTH_REQ:
		HandleBufferDepthRequest(message,true);
                break;
            case TYPE_11AX_UL_BUFFER_DEPTH_REQ:
		HandleBufferDepthRequest(message,false);
                break;
            case TYPE_11AX_DL_WAITING_TIME_REQ:
                HandleWaitingTimeRequest(message,true);
		break;
            case TYPE_11AX_UL_WAITING_TIME_REQ:
		HandleWaitingTimeRequest(message,false);
                break;
	    case TYPE_11AX_ALGO_END:
		printf("Returning from the switch case\n");
		tlvDecode1Byte(message,&isEndAlgo);
		return 1;
            default:
                break;
        }
    }
    return 0;
}

void
RRMWifiManager::NotifyAccessGranted (void)
{
  bool isBroadcastServed = false;
  NS_LOG_FUNCTION("Got DCF Access ");
  for (uint8_t  tid = 0; tid < 8; tid++)
    {
      RRMWifiRemoteStation *station = m_axStations[QosUtilsMapTidToAc(tid)];
      MacLowTransmissionListener *listener = station->lt;
      if (listener->NeedsAccess())
        {
	  WifiTxVector txVector = DoGetDataTxVector(station);
	  txVector.SetRu(0xff);
          //If there is something in the broadcast (aid == 0) queue, we have to send it on priority
	  listener->NotifyAccessGranted(txVector, MilliSeconds(1));
          isBroadcastServed = true;
          break;
        }
    }
  if (isBroadcastServed == false)
    {
      if (ScheduleStations() == false)
	{
          /*
           * If none clients are scheduled, we can attempt to schedule once again
           */
          ScheduleStations ();
        }
    }

   // Lets ask for medium aggressively as far as possible
   Simulator::Schedule (MicroSeconds(1), &RRMWifiManager::StartAccessIfNeeded, this);
   //StartAccessIfNeeded();

}

void
RRMWifiManager::NotifyInternalCollision (void)
{
  NS_LOG_FUNCTION (this);
  NotifyCollision ();
}

void
RRMWifiManager::NotifyCollision (void)
{
  NS_LOG_FUNCTION (this);
  m_dcf->StartBackoffNow (m_rng->GetNext (1, m_dcf->GetCw ()));
  StartAccessIfNeeded();
}

void
RRMWifiManager::NotifyChannelSwitching (void)
{
  NS_LOG_FUNCTION (this);
}

void
RRMWifiManager::NotifySleep (void)
{
  NS_LOG_FUNCTION (this);
}

void
RRMWifiManager::NotifyWakeUp (void)
{
  NS_LOG_FUNCTION (this);
}

void
RRMWifiManager::SetupDcfManager (DcfManager *manager)
{
  NS_LOG_FUNCTION (this << manager);
  m_dcfManager = manager;
  //These params are same as Becon
  // TBD:  For mixed mode (along with legacy, then we need to think through these values)
  m_dcf->SetCwMin (15);
  m_dcf->SetCwMax (1023);
  m_dcf->SetAifsn (2);
  //This limit is same as Max for Video AC
  m_dcf->SetTxopLimit (MicroSeconds (6016));
  m_dcfManager->Add (m_dcf);
}

void
RRMWifiManager::StartAccessIfNeeded (void)
{
  NS_LOG_FUNCTION (this);
  if (!m_dcf->IsAccessRequested ())
    {
      m_dcf->ResetCw ();
      m_dcfManager->RequestAccess (m_dcf);
    }
}

TypeId
RRMWifiManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RRMWifiManager")
    .SetParent<ns3::WifiRemoteStationManager> ()
    .SetGroupName ("Wifi")
    .AddConstructor<RRMWifiManager> ()
    .AddAttribute ("DataMode", "The transmission mode to use for every data packet transmission",
                   StringValue ("DsssRate1Mbps"),
                   MakeWifiModeAccessor (&RRMWifiManager::m_dataMode),
                   MakeWifiModeChecker ())
    .AddAttribute ("ControlMode", "The transmission mode to use for every RTS packet transmission.",
                   StringValue ("DsssRate1Mbps"),
                   MakeWifiModeAccessor (&RRMWifiManager::m_ctlMode),
                   MakeWifiModeChecker ())
    .AddAttribute ("SIMin", "When this timeout expires, the RTS/CTS handshake has failed.",
                   TimeValue (Seconds (0.1)),
                   MakeTimeAccessor (&RRMWifiManager::SetSiMin,
                                     &RRMWifiManager::GetSiMin),
                   MakeTimeChecker ())
    .AddAttribute ("SIMax", "When this timeout expires, the RTS/CTS handshake has failed.",
                   TimeValue (Seconds (0.1)),
                   MakeTimeAccessor (&RRMWifiManager::SetSiMax,
                                     &RRMWifiManager::GetSiMax),
                   MakeTimeChecker ())
    .AddAttribute ("RateControl", "Rate Control Algorithm selector.",
                   UintegerValue (ARF),
                   MakeUintegerAccessor (&RRMWifiManager::m_rateControlSelector),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SuccessK", "Multiplication factor for the success threshold in the AARF algorithm.",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&RRMWifiManager::m_successK),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TimerK",
                   "Multiplication factor for the timer threshold in the AARF algorithm.",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&RRMWifiManager::m_timerK),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxSuccessThreshold",
                   "Maximum value of the success threshold in the AARF algorithm.",
                   UintegerValue (30),
                   MakeUintegerAccessor (&RRMWifiManager::m_maxSuccessThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MinTimerThreshold",
                   "The minimum value for the 'timer' threshold in the AARF algorithm.",
                   UintegerValue (5),
                   MakeUintegerAccessor (&RRMWifiManager::m_minTimerThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MinSuccessThreshold",
                   "The minimum value for the success threshold in the AARF algorithm.",
                   UintegerValue (20),
                   MakeUintegerAccessor (&RRMWifiManager::m_minSuccessThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("BerThreshold",
                   "The maximum Bit Error Rate acceptable at any transmission mode",
                   DoubleValue (1e-5),
                   MakeDoubleAccessor (&RRMWifiManager::m_ber),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SchedulerPlugin",
                   "This Boolean attribute is set to enable a separate Scheduler Plugin",
                   BooleanValue (false),
                   MakeBooleanAccessor (&RRMWifiManager::m_SchedulerPluginEnabled),
                   MakeBooleanChecker ())
  ;
  return tid;
}

RRMWifiManager::RRMWifiManager ()
{
  NS_LOG_FUNCTION (this);
  m_dcf = new RRMWifiManager::Dcf (this);
  m_rng = new RealRandomStream ();
  m_ruTable = CreateObject<HEBitMap> ();
  m_sockId = socket(AF_INET, SOCK_STREAM, 0);
  EstablishRRMServerConnection();
}

RRMWifiManager::~RRMWifiManager ()
{
  NS_LOG_FUNCTION (this);
  close(m_sockId);
}

void
RRMWifiManager::DoInitialize ()
{
  m_wifiPhy->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RRMWifiManager::RxDrop, this));
  WifiMode mode;
  WifiTxVector txVector;
  uint32_t nModes = GetPhy ()->GetNModes ();
  txVector.SetChannelWidth (GetPhy ()->GetChannelWidth ());
  txVector.SetNss (1);
  for (uint32_t i = 0; i < nModes; i++)
    {
      mode = GetPhy ()->GetMode (i);
      txVector.SetMode (mode);
      NS_LOG_DEBUG ("Initialize, adding mode = " << mode.GetUniqueName ());
      AddSnrThreshold (txVector, GetPhy ()->CalculateSnr (txVector, m_ber));
    }
  nModes = GetPhy ()->GetNMcs ();
  for (uint32_t i = 0; i < nModes; i++)
    {
      mode = GetPhy ()->GetMcs (i);
      if (mode.GetModulationClass () == WIFI_MOD_CLASS_HE)
        {
          txVector.SetMode (mode);
          AddSnrThreshold (txVector, GetPhy ()->CalculateSnr (txVector, m_ber));
        }
    }
}

void
RRMWifiManager::SetupPhy (Ptr<WifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  m_wifiPhy = phy;
  WifiRemoteStationManager::SetupPhy (phy);
}

void
RRMWifiManager::SetupAidQueue (uint16_t aid, Mac48Address mac, MacLowTransmissionListener *lt, enum AcIndex ac)
{
  for (uint8_t tid = 0; tid < 8; tid++)
    {
      if (QosUtilsMapTidToAc (tid) == ac && !(tid % 2))
	{
	  RRMWifiRemoteStation *station = (RRMWifiRemoteStation *)Lookup(mac, tid);
	  station->m_aid = aid;
	  station->lt = lt;
	  station->ulBufferStat.bufLen = -1;
	  station->ulBufferStat.time = Now();
          station->m_successThreshold = m_minSuccessThreshold;
          station->m_timerTimeout = m_minTimerThreshold;
          station->m_success = 0;
          station->m_failed = 0;
          station->m_recovery = false;
          station->m_retry = 0;
          station->m_timer = 0;
          station->m_mcsVal = 0xff;
          station->m_arfSuccessThreshold = 4;
          station->m_arfFailureThreshold = 4;
	  m_axStations.push_back(station);
	}
    }
}

WifiRemoteStation *
RRMWifiManager::DoCreateStation (void) const
{
  NS_LOG_FUNCTION (this);
  RRMWifiRemoteStation *station = new RRMWifiRemoteStation ();
  station->m_aid = 0;
  station->m_lastSnrObserved = 0.0;
  station->m_lastSnrCached = CACHE_INITIAL_VALUE;
  return station;
}

Time
RRMWifiManager::GetAcStaleTime(uint ac)
{
  if (ac == AC_VO)
    {
      return MilliSeconds(30);
    }
  else if (ac == AC_VI)
    {
      return MilliSeconds(100);
    }
  else
    {
      return MilliSeconds(400);
    }
}
void
RRMWifiManager::StaleBSData (RRMWifiRemoteStation *st)
{
  st->ulBufferStat.bufLen = -1;
  st->ulBSStaleTimer = Simulator::Schedule (NanoSeconds(st->ulBSStaleTimer.GetTs()), &RRMWifiManager::StaleBSData, this, st);
}

void
RRMWifiManager::DoReportRxOk (WifiRemoteStation *station,
                                double rxSnr, WifiMode txMode)
{
  RRMWifiRemoteStation *st = (RRMWifiRemoteStation *)station;
  if (st->m_state->m_heSupported && st->m_state->m_isTxopLimitValid)
    {
      for (uint ac  = 0; ac < AC_BE_NQOS; ac++)
        {
	  RRMWifiRemoteStation *sta = (RRMWifiRemoteStation *)Lookup(st->m_state->m_address, ac*2);
	  if(st->m_state->m_Qsize[ac])
	    {
	      sta->ulBSStaleTimer.Cancel();
	      sta->ulBSStaleTimer = Simulator::Schedule (GetAcStaleTime(ac), &RRMWifiManager::StaleBSData, this, sta);
	    }
	  sta->ulBufferStat.bufLen = st->m_state->m_Qsize[ac]*256;
	  sta->ulBufferStat.time = Now();
	  //Timer to make this data stale after a timeout
	  st->m_state->m_isTxopLimitValid = false;
        }
      uint ac;
      for ( ac  = 0; ac < AC_BE_NQOS&&!st->m_state->m_Qsize[ac]; ac++);
      if (ac == AC_BE_NQOS)
	{
	  st->ulBSStaleTimer = Simulator::Schedule (MilliSeconds(30), &RRMWifiManager::StaleBSData, this, st);
	}
      //If we are here, we have successfully received UL Data frame.
      st->m_lastSnrObserved = rxSnr;
      rateControlDataSuccess(st);
    }
}

void
RRMWifiManager::DoReportRtsFailed (WifiRemoteStation *station)
{
}

void
RRMWifiManager::DoReportDataFailed (WifiRemoteStation *station)
{
  RRMWifiRemoteStation *st = (RRMWifiRemoteStation *)station;
  rateControlDataFailed(st);
}

void
RRMWifiManager::DoReportRtsOk (WifiRemoteStation *st,
                                 double ctsSnr, WifiMode ctsMode, double rtsSnr)
{
  RRMWifiRemoteStation *sta = (RRMWifiRemoteStation *)st;
  NS_LOG_FUNCTION (this << sta << ctsSnr << ctsMode.GetUniqueName () << rtsSnr);
  sta->m_lastSnrObserved = rtsSnr;
}

void
RRMWifiManager::DoReportDataOk (WifiRemoteStation *station,
                                  double ackSnr, WifiMode ackMode, double dataSnr)
{
  NS_LOG_FUNCTION (this << station << ackSnr << ackMode.GetUniqueName () << dataSnr);
  RRMWifiRemoteStation *st = (RRMWifiRemoteStation *)station;
  st->m_lastSnrObserved = dataSnr;
  rateControlDataSuccess(st);
}

void
RRMWifiManager::DoReportAmpduTxStatus (WifiRemoteStation *station, uint32_t nSuccessfulMpdus, uint32_t nFailedMpdus, double rxSnr, double dataSnr)
{
  NS_LOG_FUNCTION (this << station << nSuccessfulMpdus << nFailedMpdus << rxSnr << dataSnr);
  RRMWifiRemoteStation *st = (RRMWifiRemoteStation *)station;
  st->m_lastSnrObserved = dataSnr;

  if (nSuccessfulMpdus == 0)
    {
      rateControlDataFailed(st);
    }
  else if (nFailedMpdus == 0)
    {
      rateControlDataSuccess(st);
    }
}


void
RRMWifiManager::DoReportFinalRtsFailed (WifiRemoteStation *station)
{
}

void
RRMWifiManager::DoReportFinalDataFailed (WifiRemoteStation *station)
{
  RRMWifiRemoteStation *st = (RRMWifiRemoteStation *)station;
  rateControlDataFailed(st);
}

bool
RRMWifiManager::DoNeedDataRetransmission (WifiRemoteStation *station, Ptr<const Packet> packet, bool normally)
{
  return normally;
}

WifiTxVector
RRMWifiManager::DoGetDataTxVector (WifiRemoteStation *station, uint32_t bitMap, uint32_t aid, uint32_t mcs)
{
  NS_LOG_FUNCTION (this << station);
  WifiTxVector dataTxVector;
  WifiMode   t_dataMode;
  RRMWifiRemoteStation *st = (RRMWifiRemoteStation *)station;

  if (bitMap <= 121 && mcs > 9) {
    mcs = 9;
  }
  t_dataMode = m_wifiPhy->GetHeMcs(mcs);
  //t_dataMode = m_wifiPhy->GetHeMcs(st->m_mcsVal);
  dataTxVector = WifiTxVector (t_dataMode, GetDefaultTxPowerLevel (), GetLongRetryCount (st), GetShortGuardInterval (st), 1, 0, GetChannelWidth (st), GetAggregation (st), false);
  return dataTxVector;
}

WifiTxVector
RRMWifiManager::DoGetDataTxVector (WifiRemoteStation *station)
{
  NS_LOG_FUNCTION (this << station);
  WifiTxVector dataTxVector;
  RRMWifiRemoteStation *st = (RRMWifiRemoteStation *)station;

  //For all other Non HE stations we are using Constant rate algorithm
  dataTxVector = WifiTxVector (m_dataMode, GetDefaultTxPowerLevel (), GetLongRetryCount (st), GetShortGuardInterval (st), 1, 0, GetChannelWidth (st), GetAggregation (st), false);
  return dataTxVector;
}

WifiTxVector
RRMWifiManager::DoGetRtsTxVector (WifiRemoteStation *st)
{
  //For all other Non HE stations we are using Constant rate algorithm.
  return WifiTxVector (m_ctlMode, GetDefaultTxPowerLevel (), GetShortRetryCount (st), GetShortGuardInterval (st), 1, 0, GetChannelWidth (st), GetAggregation (st), false);
}

WifiTxVector
RRMWifiManager::GetMuRtsTxVector (void)
{
  WifiTxVector muRtsVector = WifiTxVector (m_ctlMode, GetDefaultTxPowerLevel (), 0, false, 1, 0, m_wifiPhy->GetChannelWidth (), false, false);
  muRtsVector.SetRu(0);
  muRtsVector.SetAid(0);
  return muRtsVector;
}

bool
RRMWifiManager::IsLowLatency (void) const
{
  return true;
}

void
RRMWifiManager::PrepareForQueue (Mac48Address address, const WifiMacHeader *header, Ptr<const Packet> packet)
{
  WifiRemoteStationManager::PrepareForQueue (address, header, packet);
  //Request access for next round
  StartAccessIfNeeded();
}

void
RRMWifiManager::PrepareHeMuMpdu(ServingStations servingStations)
{
  for(ServingStations::iterator s = servingStations.begin(); s != servingStations.end(); s++)
  {
      // Dequeue one packet from the queue and add it to the HE MU MPDU
      (*s)->lt->NotifyAccessGranted((*s)->dataTxVector, MicroSeconds(5472*3));
  }
}

bool
RRMWifiManager::ScheduleStations(void)
{
  NS_LOG_FUNCTION(this);
  static bool nextScheduleUplink = false;
  bool isScheduled = false;
  if (m_SchedulerPluginEnabled)
    {
      isScheduled = CallAlgoPlugin(!nextScheduleUplink);
    }
  else
    {
      /**
       * Following sample resource allocation methods are for 20MHZ only. it just allocates 26 tone resource unit
       * to each station.
       */
      if (nextScheduleUplink)
	{
	  isScheduled = SampleULScheduler();
	}
      else
	{
	  isScheduled = SampleDLScheduler();
	}
    }
  nextScheduleUplink = !nextScheduleUplink;
  return isScheduled;
}

/* 
 * This is a "sample" method for resource allocation
 */
bool
RRMWifiManager::SampleDLScheduler(void)
{
  NS_LOG_FUNCTION(this);
  uint16_t stationsPerRound = 9;
  uint16_t currListOfStations = 0;
  static uint16_t lastServedStation = 3;
  int16_t i;
  uint16_t  totalAxStations = m_axStations.size();
  ServingStations servingStaions;
  WifiTxVector txVector;
  struct RUInfo                     ruI = {0,0};
  uint32_t bitMap = 0;
  Ptr<UniformRandomVariable> mcsRandom = CreateObject<UniformRandomVariable> ();

  for (i = lastServedStation + 1; i < totalAxStations; i++)
    {
      if(m_axStations[i]->lt->NeedsAccess())
	{
	      //Populate ruMap
              ruI.type = 1;
              bitMap = m_ruTable->GetBitMapFromRUInfo(ruI); 
              ruI.index ++;

              txVector = DoGetDataTxVector (m_axStations[i]);

              //XXX: Seleting 26 tone RU for 9 stations
              txVector.SetRu(bitMap);
	      txVector.SetAid(m_axStations[i]->m_aid);
              txVector.SetChannelWidth(2);
	      m_axStations[i]->dataTxVector = txVector;
	      servingStaions.push_back (m_axStations[i]);
	      currListOfStations++;

	}
      if(i == totalAxStations - 1)
	{
	  i = 3;
	}
      if ( i == lastServedStation || currListOfStations == stationsPerRound)
	{
	  break;
	}
    }
  if (currListOfStations)
    {
      lastServedStation = i;
      return StartTranmission(true, servingStaions);
    }
  return false;
}

/* 
 * This is a "sample" method for resource allocation
 */
bool
RRMWifiManager::SampleULScheduler()
{
  NS_LOG_FUNCTION(this);
  uint16_t stationsPerRound = 9;
  uint16_t currListOfStations = 0;
  static uint16_t lastServedStation = 3;
  uint16_t i;
  uint16_t  totalAxStations = m_axStations.size();
  ServingStations servingStaions;
  struct RUInfo                     ruI = {0,0};
  std::vector<uint16_t>  selectedAid = {0}; 
  std::vector<uint16_t>::iterator it;

  for (i = lastServedStation + 1; i < totalAxStations;i++)
    {
      it = find (selectedAid.begin(), selectedAid.end(), m_axStations[i]->m_aid);

      if(it == selectedAid.end())
      { 
         //Populate ruMap
         WifiTxVector txVector = DoGetDataTxVector (m_axStations[i]);

         //XXX: Seleting 26 tone RU for 9 stations
         ruI.type = 1;
         txVector.SetRu(m_ruTable->GetBitMapFromRUInfo(ruI));
         ruI.index ++;

         txVector.SetAid(m_axStations[i]->m_aid);
         selectedAid.push_back(m_axStations[i]->m_aid);
         m_axStations[i]->dataTxVector = txVector;
         servingStaions.push_back (m_axStations[i]);
         currListOfStations++;
      }

      if(i == totalAxStations - 1)
      {
         i = 3;
      }
      if ( i == lastServedStation || currListOfStations == stationsPerRound)
      {
         break;
      }
    }
  if (currListOfStations)
    {
      lastServedStation = i;
      return StartTranmission(false, servingStaions);
    }
  return false;
}

void
RRMWifiManager::SetSiMin (Time siMin)
{
  m_siMin = siMin;
}

void
RRMWifiManager::SetSiMax (Time siMax)
{
  m_siMax = siMax;
}

Time
RRMWifiManager::GetSiMin () const
{
  return m_siMin;
}

Time
RRMWifiManager::GetSiMax () const
{
  return m_siMax;
}

bool
RRMWifiManager::StartTranmission (bool isDownlink, ServingStations servingStations)
{

    NS_LOG_FUNCTION (this);
    staRuMap  staMapTmp;
    RuInfo   ruInfoTx;

    staMapTmp.clear();
    if (servingStations.empty())
      {
        NS_LOG_DEBUG("Empty station map passed");
        return false;
      }

    for(ServingStations::iterator it=servingStations.begin(); it != servingStations.end(); it++)
      {
  	  uint16_t staAid = (*it)->m_aid;
          ruInfoTx.index = (*it)->dataTxVector.GetRu();
  	  // We are going to serve these AIDs in this iteration
  	  ruInfoTx.m_aid = staAid;
  	  ruInfoTx.mcs = (*it)->dataTxVector.GetMode().GetMcsValue();
  	  staMapTmp.insert (std::make_pair (staAid, ruInfoTx));
      }

    if(isDownlink == false)
      {
 	GetMac()->GetObject <RegularWifiMac> ()->GetMacLow ()->SendBasicTrigger(staMapTmp, MicroSeconds(5472));
      }
    else
      {
        //Send MU RTS and start CTS timer
	GetMac()->GetObject <RegularWifiMac> ()->GetMacLow ()->SetOfdmaHeTransmit (true);
	PrepareHeMuMpdu(servingStations);
	GetMac()->GetObject <RegularWifiMac> ()->GetMacLow ()->SendMuRtsForPacket (MicroSeconds(5472*3));
      }
    return true;
}

uint32_t
RRMWifiManager::FectchAxStationIndexFromMac(uint8_t mac[6], uint8_t trafficType)
{
  uint32_t   i, axIndex = 0;
  uint32_t t_ac = 0;
  uint8_t macAddr[6];
 
  memset(macAddr, '\0', sizeof(macAddr));
  for (i = 0; i < m_axStations.size(); i++)
  {
      t_ac = QosUtilsMapTidToAc(m_axStations[i]->m_tid);
      m_axStations[i]->m_state->m_address.CopyTo(macAddr);
      if(memcmp(macAddr, mac, 6) == 0) { 
          if(trafficType == AC_VO) {
              if(t_ac == AC_VO) {
                  axIndex = i;
                  break; 
              }
          } else if(trafficType == AC_VI) {
              if(t_ac == AC_VI) { 
                  axIndex = i; 
                  break; 
              }
          } else if(trafficType == AC_BE){
              if(t_ac == AC_BE) { 
                  axIndex = i; 
                  break; 
              }
          }
      } 
  }
  return axIndex;
}

int 
RRMWifiManager::tlvWriteTypeLen(TlvBuffer* buf,short type,short len){
    short tmp;
    
    tmp = htons(type);
    memcpy((buf->data+buf->write_offset),&tmp,sizeof(short)); //Writing the type to the Buffer
    buf->write_offset += sizeof(short);

    tmp = htons(len);
    memcpy((buf->data+buf->write_offset),&tmp,sizeof(short)); //Writing the len to the Buffer
    buf->write_offset += sizeof(short);
   
    buf->len += 2*sizeof(short);
   return 1;
}

int 
RRMWifiManager::tlvEncode1Byte(TlvBuffer* buf,short type,char val){
    tlvWriteTypeLen(buf,type,1);

    memcpy((buf->data+buf->write_offset),&val,sizeof(char));
    buf->write_offset += sizeof(char);

    buf->len += 1;
   return 1;

}

int 
RRMWifiManager::tlvAppend1Byte(TlvBuffer* buf,char val)
{
    memcpy((buf->data+buf->write_offset),&val,sizeof(char));
    buf->write_offset += sizeof(char);

    buf->len += 1;
   return 1;

}

int 
RRMWifiManager::tlvEncode2Bytes(TlvBuffer* buf,short type,short val){

    short tmp_val = htons(val);
    tlvWriteTypeLen(buf,type,2);

    memcpy((buf->data+buf->write_offset),&tmp_val,sizeof(short));
    buf->write_offset += sizeof(short);

    buf->len +=  2;
   return 1;

}
int 
RRMWifiManager::tlvAppend2Bytes(TlvBuffer* buf,short val){

    short tmp_val = htons(val);

    memcpy((buf->data+buf->write_offset),&tmp_val,sizeof(short));
    buf->write_offset += sizeof(short);

    buf->len +=  2;
   return 1;

}

int 
RRMWifiManager::tlvEncode4Bytes(TlvBuffer* buf,short type,int val){

    int tmp_val = htonl(val);
    tlvWriteTypeLen(buf,type,4);

    memcpy((buf->data+buf->write_offset),&tmp_val,sizeof(int));
    buf->write_offset += sizeof(int);

    buf->len +=  4;
   return 1;

}
int 
RRMWifiManager::tlvEncodeString(TlvBuffer* buf,short type,char* val){
    tlvWriteTypeLen(buf,type,strlen(val));

    memcpy((buf->data+buf->write_offset),val,strlen(val));
    buf->write_offset += strlen(val);

    buf->len +=  strlen(val);
   return 1;

}
int 
RRMWifiManager::tlvEncode2ByteIntArray(TlvBuffer* buf,short type,short* array,short count){
    int i;
    short tmp_val;
    tlvWriteTypeLen(buf,type,2*count);

    for(i = 0;i<count;i++){
        tmp_val = htons(array[i]);
        memcpy((buf->data+buf->write_offset),&tmp_val,sizeof(short));
        buf->write_offset += sizeof(short);
    }
    buf->len +=  2*count;
    return 1;

}

int 
RRMWifiManager::tlvEncodeAllStats(TlvBuffer* buf,short type,AllStats_t* clientArray,short count)
{
   short i;
   short len ;
   len = count*sizeof(AllStats_t);
   tlvWriteTypeLen(buf,type,len);

   for(i = 0;i<count;i++){
        memcpy((buf->data+buf->write_offset),(void*)(&clientArray[i]),sizeof(AllStats_t));
        buf->write_offset += sizeof(AllStats_t);
   }
   buf->len += count*sizeof(AllStats_t);
   return 1;
}

int 
RRMWifiManager::tlvEncodeResults(TlvBuffer* buf,short type,RRMClientResponse_t* respArray,short count)
{
   short i;
   short len ;
   len = count*sizeof(RRMClientResponse_t);
   tlvWriteTypeLen(buf,type,len);

   for(i = 0;i<count;i++){
        memcpy((buf->data+buf->write_offset),(void*)(&respArray[i]),sizeof(RRMClientResponse_t));
        buf->write_offset += sizeof(RRMClientResponse_t);
   }
   buf->len += count*sizeof(RRMClientResponse_t);
   return 1;
}


int 
RRMWifiManager::tlvReadType(TlvBuffer* message,short* type){
    
    *type = ntohs(*(short*)(message->data+message->read_offset));
    message->read_offset += sizeof(short);
    return 1;

}
int 
RRMWifiManager::tlvReadLen(TlvBuffer* message,short* len){

    *len = ntohs(*(short*)(message->data+message->read_offset));
    return 1;
}
int 
RRMWifiManager::tlvReadLen1(TlvBuffer* message,short* len){

    *len = ntohs(*(short*)(message->data+message->read_offset));
    message->read_offset += sizeof(short);
    return 1;
}

int 
RRMWifiManager::tlvDecode1Byte(TlvBuffer* message,char* val)
{
    short len ;
    len = ntohs(*(short*)(message->data+message->read_offset));
	len = len*1;//To escape compiler warning
    message->read_offset += sizeof(short);
    *val = *(short*)(message->data+message->read_offset);
    message->read_offset += 1;
   return 1;

}
int 
RRMWifiManager::tlvDecode2Bytes(TlvBuffer* message,short* val)
{
    short len ;
    len = ntohs(*(short*)(message->data+message->read_offset));
    len = len*1;//To escape warnings
    message->read_offset += sizeof(short);
    *val = ntohs(*(short*)(message->data+message->read_offset));
    message->read_offset += 2;
   return 1;

}
int 
RRMWifiManager::tlvDecodeAppended2Bytes(TlvBuffer* message,short* val)
{
    *val = ntohs(*(short*)(message->data+message->read_offset));
    message->read_offset += 2;
    return 1;

}

int 
RRMWifiManager::tlvDecode4Bytes(TlvBuffer* message,int* val)
{
    short len ;
    len = ntohs(*(short*)(message->data+message->read_offset));
    len = len*1; //To escape compiler warning
    message->read_offset += sizeof(short);
    *val = ntohl(*(int*)(message->data+message->read_offset));
     message->read_offset += 4;
   return 1;

}
int 
RRMWifiManager::tlvDecodeString(TlvBuffer* message,char* val)
{
    short len ;
    len = ntohs(*(short*)(message->data+message->read_offset));
    message->read_offset += sizeof(short);
    if(len > 0){
        memcpy(val,(message->data+message->read_offset),len);
        val[len] = '\0';
        message->read_offset += len;
    }
   return 1;

}
int 
RRMWifiManager::tlvDecode2ByteIntArray(TlvBuffer* message,short** array,short *count){
  return 1;
    short len;
    short i;
    len = ntohs(*(short*)(message->data+message->read_offset));
    message->read_offset += sizeof(short); 
    *count = len/2; //Number of elements
    
    *array = (short*)malloc(sizeof(short)*(*count));
    memset(*array,0x00,sizeof(short)*(len/2));

    for(i = 0;i<*count;i++){
        (*array)[i] = ntohs(*(short*)(message->data+message->read_offset));          
        message->read_offset += 2;
    }
   return 1;

}
int 
RRMWifiManager::tlvDecodeAllStats(TlvBuffer* message,AllStats_t** clientArray,short* count)
{
  return 1;
        short len;
        len = ntohs(*(short*)(message->data+message->read_offset));
        message->read_offset += sizeof(short);
       *count = len/(sizeof(AllStats_t));

        *clientArray = (AllStats_t*)malloc(len);
        memset(*clientArray,0x00,len);
         memcpy((void*)*clientArray,(message->data+message->read_offset),len);
        message->read_offset += len;

	return 1;
}
int 
RRMWifiManager::tlvDecodeResults(TlvBuffer* message,RRMClientResponse_t** respArray,short* count)
{
        short len;
        len = ntohs(*(short*)(message->data+message->read_offset));
        message->read_offset += sizeof(short);
       *count = len/(sizeof(RRMClientResponse_t));

        *respArray = (RRMClientResponse_t*)malloc(len);
        memset(*respArray,0x00,len);
         memcpy((void*)*respArray,(message->data+message->read_offset),len);
        message->read_offset += len;

	return 1;
}

void 
RRMWifiManager::print_bytes(const void *object, size_t size)
{
//    const unsigned char * const bytes =  (char *)(object);
    const unsigned char * const bytes = static_cast<const unsigned char *>(object);
    size_t i;

    printf("[ ");
    for(i = 0; i < size; i++)
    {
        printf("%02x ", bytes[i]);
    }
    printf("]\n");

}
int 
RRMWifiManager::EstablishRRMServerConnection()
{
	struct sockaddr_in rrmServer ;
	if(m_sockId < 0)
	{
		std::cout<<"The socket has not been created\n";
	  return -1;
	}
	struct hostent *tempServer;
  tempServer = gethostbyname("localhost");
  bcopy((char *)tempServer->h_addr, (char *)&rrmServer.sin_addr.s_addr, tempServer->h_length);

	 //Server info
    rrmServer.sin_family       = AF_INET;
    rrmServer.sin_port         = htons(PORTNUM);

	if (connect(m_sockId, (struct sockaddr *)&rrmServer , sizeof(rrmServer)) < 0)
  {
  	NS_LOG_DEBUG("Connection to the RRM Server Failed\n");
    return -1;
  }

	return 1;
}

bool 
RRMWifiManager::CallAlgoPlugin(bool isDownlink)
{
    int max_sd = 0;
    fd_set readfds;
    TlvBuffer recvBuf;
    int  ret = 0;
    TlvBuffer* message = NULL;

    message = (TlvBuffer*)malloc(sizeof(TlvBuffer));
    memset(message,0x00,sizeof(TlvBuffer));
    
    //Setting the Indicator to activate 11ax RRM
    tlvEncode1Byte(message,TYPE_11AX_ACTIVATE_RRM,isDownlink) ;
    write(m_sockId,(void*)message,sizeof(TlvBuffer));
    free(message);

    while(1)
    {
        // init fd_set
        FD_ZERO(&readfds); // to initialize socket descriptors
        
        FD_SET(m_sockId, &readfds); 
        max_sd = (max_sd>m_sockId)?max_sd:m_sockId;
        
        ret = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0)
        {
            printf("select failed\n ");
            return false;
        }
        // warning: you don't know the max_sd value
        sd = m_sockId ;
        if (FD_ISSET(sd, &readfds)) 
        {
            ret = recv(sd,(char *)&recvBuf,sizeof(TlvBuffer), 0);
            if(ret > 0 )
            {
                if(ProcessTlvMessage(&recvBuf, isDownlink) == 1) {
                    return 1;
                }
                memset(&recvBuf,0x00,sizeof(recvBuf));
            }
        }
    }
    return false;
}

void
RRMWifiManager::rateControlDataSuccess (RRMWifiRemoteStation *st)
{
  if (m_rateControlSelector == ARF)
    {
      st->m_failed = 0;
      st->m_success++;
      if (st->m_success > st->m_arfSuccessThreshold)
      {
        st->m_success = 0;
        st->m_mcsVal++;
      }
      if (st->m_mcsVal > 11)
        st->m_mcsVal = 11;
    }
  else if (m_rateControlSelector == AARF)
    {
      st->m_timer++;
      st->m_success++;
      st->m_failed = 0;
      st->m_recovery = false;
      st->m_retry = 0;
      NS_LOG_DEBUG ("station=" << st << " data ok success=" << st->m_success << ", timer=" << st->m_timer);
      if ((st->m_success == st->m_successThreshold
           || st->m_timer == st->m_timerTimeout)
          && (st->m_mcsVal < 11))
        {
          NS_LOG_DEBUG ("station=" << st << " inc rate");
          st->m_mcsVal++;
          st->m_timer = 0;
          st->m_success = 0;
          st->m_recovery = true;
        }
      if (st->m_mcsVal > 11)
        st->m_mcsVal = 11;
    }
  NS_LOG_DEBUG("Mcs Value for station " << st->m_aid << " is : " << st->m_mcsVal << " DoReportDataOk");
}

void
RRMWifiManager::rateControlDataFailed (RRMWifiRemoteStation *st)
{
  if (m_rateControlSelector == ARF)
    {
      st->m_failed++;
      st->m_success = 0;
      st->m_mcsVal--;
      if(st->m_mcsVal < 0)
      st->m_mcsVal = 0;
      if (st->m_failed > st->m_arfFailureThreshold)
        {
          st->m_failed = 0;
          st->m_mcsVal = 0;
        }
    }
  else if (m_rateControlSelector == AARF)
    {
      st->m_timer++;
      st->m_failed++;
      st->m_retry++;
      st->m_success = 0;

      if (st->m_recovery)
        {
          NS_ASSERT (st->m_retry >= 1);
          if (st->m_retry == 1)
            {
              //need recovery fallback
              st->m_successThreshold = (st->m_successThreshold * m_successK < m_maxSuccessThreshold) ? (int)(st->m_successThreshold * m_successK) : m_maxSuccessThreshold;
              st->m_timerTimeout = (st->m_timerTimeout * m_timerK > m_minSuccessThreshold) ? (int)(st->m_timerTimeout * m_timerK) : m_minSuccessThreshold;
              if (st->m_mcsVal != 0)
                {
                  st->m_mcsVal--;
                }
            }
          st->m_timer = 0;
        }
      else
        {
          NS_ASSERT (st->m_retry >= 1);
          if (((st->m_retry - 1) % 2) == 1)
            {
              //need normal fallback
              st->m_timerTimeout = m_minTimerThreshold;
              st->m_successThreshold = m_minSuccessThreshold;
              if (st->m_mcsVal != 0)
                {
                  st->m_mcsVal--;
                }
            }
          if (st->m_retry >= 2)
            {
              st->m_timer = 0;
            }
        }
    }
  NS_LOG_DEBUG("Mcs Value for station " << st->m_aid << " is : " << st->m_mcsVal << " DoReportDataFailed");
}

void
RRMWifiManager::AddSnrThreshold (WifiTxVector txVector, double snr)
{
  NS_LOG_FUNCTION (this << txVector.GetMode ().GetUniqueName () << snr);
  m_thresholds.push_back (std::make_pair (snr, txVector));
}

double
RRMWifiManager::GetSnrThreshold (WifiTxVector txVector) const
{
  NS_LOG_FUNCTION (this << txVector.GetMode ().GetUniqueName ());
  for (Thresholds::const_iterator i = m_thresholds.begin (); i != m_thresholds.end (); i++)
    {
      NS_LOG_DEBUG ("Checking " << i->second.GetMode ().GetUniqueName () <<
                    " nss " << (uint16_t) i->second.GetNss () <<
                    " width " << (uint16_t) i->second.GetChannelWidth ());
      NS_LOG_DEBUG ("against TxVector " << txVector.GetMode ().GetUniqueName () <<
                    " nss " << (uint16_t) txVector.GetNss () <<
                    " width " << (uint16_t) txVector.GetChannelWidth ());
      if (txVector.GetMode () == i->second.GetMode ()
          && txVector.GetNss () == i->second.GetNss ()
          && txVector.GetChannelWidth () == i->second.GetChannelWidth ())
        {
          return i->first;
        }
    }
  NS_ASSERT (false);
  return 0.0;
}

uint32_t
RRMWifiManager::rateControlIdeal(RRMWifiRemoteStation *station)
{
  if (m_rateControlSelector == IDEAL)
    {
      WifiMode maxMode = m_wifiPhy->GetHeMcs (0);
      WifiTxVector txVector;
      WifiMode mode;
      uint64_t bestRate = 0;
      uint8_t channelWidth = std::min (GetChannelWidth (station), GetPhy ()->GetChannelWidth ());
      txVector.SetChannelWidth (channelWidth);
      txVector.SetNss (1);

      if (station->m_lastSnrCached != CACHE_INITIAL_VALUE && station->m_lastSnrObserved == station->m_lastSnrCached)
        {
          // SNR has not changed, so skip the search and use the last
          // mode selected

          NS_LOG_DEBUG ("Using cached mode = " << maxMode.GetUniqueName () <<
                        " last snr observed " << station->m_lastSnrObserved <<
                        " cached " << station->m_lastSnrCached);

          return station->m_mcsVal;
        }
      else if (station->m_lastSnrObserved != 0)
        {
          // HE selection
          for (uint32_t i = 0; i < m_wifiPhy->GetNMcs(); i++)
            {
              mode = m_wifiPhy->GetMcs(i);
              if (mode.GetModulationClass () != WIFI_MOD_CLASS_HE)
                {
        	  continue;
                }
              txVector.SetMode (mode);
              double threshold = GetSnrThreshold (txVector);
              uint64_t dataRate = mode.GetDataRate (txVector);
              NS_LOG_DEBUG ("mode = " << mode.GetUniqueName () <<
                            " threshold " << threshold  <<
                            " last snr observed " <<
                            station->m_lastSnrObserved);
              if (dataRate > bestRate && threshold < station->m_lastSnrObserved)
                {
                  NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () <<
                                " data rate " << dataRate <<
                                " threshold " << threshold  <<
                                " last snr observed " <<
                                station->m_lastSnrObserved);
                  bestRate = dataRate;
                  maxMode = mode;
                }
            }
	}
      station->m_lastSnrCached = station->m_lastSnrObserved;
      station->m_mcsVal = maxMode.GetMcsValue();
      NS_LOG_DEBUG("Best mcs chosen is : " << +station->m_mcsVal);
      return station->m_mcsVal;
    }
  else
    {
      NS_LOG_DEBUG("MCS chosen by ARF/AARF  is : " << +station->m_mcsVal);
      return station->m_mcsVal;
    }
}

void
RRMWifiManager::RxDrop (Ptr<const Packet> p)
{
  Ptr<Packet> packet = p->Copy();
  /**
   * RRM Manager runs only on AP. if the packet is dropped here, it means an UL packet.
   */
  AmpduTag ampdu;
  AmpduSubframeHeader hdr;
  WifiMacHeader macHdr;
  RRMWifiRemoteStation *station = NULL;

  if (packet->RemovePacketTag (ampdu))
    {
      packet->RemoveHeader (hdr);
    }
  packet->RemoveHeader (macHdr);
  if (macHdr.IsQosData() && !macHdr.GetAddr1().IsGroup())
    {
      //It is a unicast and data frame that we lost
      station = (RRMWifiRemoteStation *)Lookup (macHdr.GetAddr2(), macHdr.GetQosTid ());
      rateControlDataFailed(station);
    }
  NS_LOG_DEBUG ("RxDrop at RRM " << Simulator::Now ().GetSeconds ());
}

}
