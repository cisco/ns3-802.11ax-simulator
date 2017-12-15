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
 * Authors: Achuthananda M P
 *          Balamurugan Ramachandran
 *          Ramachandra Murthy
 *          Bibek Sahu
 *          Mukesh Taneja
 */
 
/* 
 * This file is for OFDMA/802.11ax type of systems. It is not
 * fully compliant to IEEE 802.11ax standards.
 */

#ifndef TLV_H
#define TLV_H


#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include <iostream>
#include <sstream>
#include <vector>

#define TLV_TYPE_SIZE 2
#define TLV_LEN_SIZE 2


#define TYPE_11AX_RRM_QOS_START 0
#define TYPE_11AX_ACTIVATE_RRM 1
#define TYPE_11AX_DL_STATION_MAC_REQ 2
#define TYPE_11AX_DL_STATION_MAC_RESP 3
#define TYPE_11AX_UL_STATION_MAC_REQ 4
#define TYPE_11AX_UL_STATION_MAC_RESP 5
#define TYPE_11AX_DL_MCS_INFO_REQ 6
#define TYPE_11AX_DL_MCS_INFO_RESP 7
#define TYPE_11AX_UL_MCS_INFO_REQ 8
#define TYPE_11AX_UL_MCS_INFO_RESP 9
#define TYPE_11AX_DL_BUFFER_DEPTH_REQ 10
#define TYPE_11AX_DL_BUFFER_DEPTH_RESP 11
#define TYPE_11AX_UL_BUFFER_DEPTH_REQ 12
#define TYPE_11AX_UL_BUFFER_DEPTH_RESP 13
#define TYPE_11AX_DL_WAITING_TIME_REQ 14
#define TYPE_11AX_DL_WAITING_TIME_RESP 15
#define TYPE_11AX_UL_WAITING_TIME_REQ 16
#define TYPE_11AX_UL_WAITING_TIME_RESP 17
#define TYPE_11AX_RRM_RESULTS_REQ 18
#define TYPE_11AX_RRM_RESULTS_RESP 19
#define TYPE_11AX_RRM_RESULTS_STATIONS_REQ 20
#define TYPE_11AX_RRM_RESULTS_STATIONS_RESP 21
#define TYPE_11AX_RRM_RESULTS_RUVECTOR_REQ 22
#define TYPE_11AX_RRM_RESULTS_RUVECTOR_RESP 23
#define TYPE_11AX_RRM_RESULTS_MCS_REQ 24
#define TYPE_11AX_RRM_RESULTS_MCS_RESP 25
#define TYPE_11AX_RRM_RESULTS_TXPOWER_REQ 25
#define TYPE_11AX_RRM_RESULTS_TXPOWER_RESP 26

#define TYPE_11AX_ALL_STATS_REQ 27
#define TYPE_11AX_ALL_STATS_RESP 28

#define TYPE_11AX_ALGO_END 254

#define TYPE_11AX_RRM_QOS_END 255

#define BUFFER_DATA_MAX_SIZE 15000

typedef struct{
char data[BUFFER_DATA_MAX_SIZE];
int write_offset,read_offset;
int len;
}__attribute__((packed))TlvBuffer;

#define MAC_ADDR_LEN 6

typedef struct
{
   uint8_t macStr[MAC_ADDR_LEN];
   int bufferDepthDL[4];
   int bufferDepthUL[4];
   double WaitingTimeDL[4];
   double WaitingTimeUL[4]; //Time spent since last BS update
   double ThroughputDL[4];
   uint32_t mcsVal;
}__attribute__((packed))AllStats_t;

typedef struct
{
        uint8_t macStr[MAC_ADDR_LEN];
        short trafficType;
        uint8_t ruBitMap;
        uint8_t mcsValue;
        uint8_t chanW;
}__attribute__((packed))RRMClientResponse_t;


#if 0
int tlvWriteTypeLen(TlvBuffer* buf,short type,short len);
int tlvEncode1Byte(TlvBuffer* buf,short type,char val);
int tlvEncode2Bytes(TlvBuffer* buf,short type,short val);
int tlvEncode4Bytes(TlvBuffer* buf,short type,int val);
int tlvEncodeString(TlvBuffer* buf,short type,char* val);
int tlvEncode2ByteIntArray(TlvBuffer* buf,short type,short* array,short count);

int tlvReadType(TlvBuffer* message,short* type);
int tlvReadLen(TlvBuffer* message,short* len);
int tlvReadLen1(TlvBuffer* message,short* len);

int tlvDecode1Byte(TlvBuffer* message,char* val);
int tlvDecode2Bytes(TlvBuffer* message,short* val);
int tlvDecode4Bytes(TlvBuffer* message,int* val);
int tlvDecodeString(TlvBuffer* message,char* val);
int tlvDecode2ByteIntArray(TlvBuffer* message,short** array,short *count);

int tlvAppend2Bytes(TlvBuffer* buf,short val);
int tlvAppend1Byte(TlvBuffer* buf,char val);
int tlvDecodeAppended2Bytes(TlvBuffer* message,short* val);
void print_bytes(const void *object, size_t size);

int tlvEncodeAllStats(TlvBuffer* message,short type,AllStats_t* clientArray,short count);
int tlvEncodeResults(TlvBuffer* message,short type,RRMClientResponse_t* respArray,short count);

int tlvDecodeAllStats(TlvBuffer* message,AllStats_t** clientArray,short* count);
int tlvDecodeResults(TlvBuffer* message,RRMClientResponse_t** respArray,short* count);
#endif

#endif //TLV_H
