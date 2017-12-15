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
 * Authors: Achutananda M P
 *          Balamurugan Ramachandran
 *          Ramachandra Murthy
 *          Bibek Sahu
 *          Mukesh Taneja
 */
 
/* 
 * This file is for OFDMA/802.11ax type of systems. It is not
 * fully compliant to IEEE 802.11ax standards.
 */

#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>
#include <arpa/inet.h>
#include "tlv.h"

int tlvWriteTypeLen(TlvBuffer* buf,short type,short len){
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

int tlvEncode1Byte(TlvBuffer* buf,short type,char val){
    tlvWriteTypeLen(buf,type,1);

    memcpy((buf->data+buf->write_offset),&val,sizeof(char));
    buf->write_offset += sizeof(char);

    buf->len += 1;
   return 1;

}
int tlvAppend1Byte(TlvBuffer* buf,char val)
{
    memcpy((buf->data+buf->write_offset),&val,sizeof(char));
    buf->write_offset += sizeof(char);

    buf->len += 1;
   return 1;

}

int tlvEncode2Bytes(TlvBuffer* buf,short type,short val){

    short tmp_val = htons(val);
    tlvWriteTypeLen(buf,type,2);

    memcpy((buf->data+buf->write_offset),&tmp_val,sizeof(short));
    buf->write_offset += sizeof(short);

    buf->len +=  2;
   return 1;

}
int tlvAppend2Bytes(TlvBuffer* buf,short val){

    short tmp_val = htons(val);

    memcpy((buf->data+buf->write_offset),&tmp_val,sizeof(short));
    buf->write_offset += sizeof(short);

    buf->len +=  2;
   return 1;

}

int tlvEncode4Bytes(TlvBuffer* buf,short type,int val){

    int tmp_val = htonl(val);
    tlvWriteTypeLen(buf,type,4);

    memcpy((buf->data+buf->write_offset),&tmp_val,sizeof(int));
    buf->write_offset += sizeof(int);

    buf->len +=  4;
   return 1;

}
int tlvEncodeString(TlvBuffer* buf,short type,char* val){
    tlvWriteTypeLen(buf,type,strlen(val));

    memcpy((buf->data+buf->write_offset),val,strlen(val));
    buf->write_offset += strlen(val);

    buf->len +=  strlen(val);
   return 1;

}
int tlvEncode2ByteIntArray(TlvBuffer* buf,short type,short* array,short count){
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
int tlvEncodeAllStats(TlvBuffer* buf,short type,AllStats_t* clientArray,short count)
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

int tlvEncodeResults(TlvBuffer* buf,short type,RRMClientResponse_t* respArray,short count)
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


int tlvReadType(TlvBuffer* message,short* type){
    
    *type = ntohs(*(short*)(message->data+message->read_offset));
    message->read_offset += sizeof(short);
    return 1;

}
int tlvReadLen(TlvBuffer* message,short* len){

    *len = ntohs(*(short*)(message->data+message->read_offset));
    return 1;
}
int tlvReadLen1(TlvBuffer* message,short* len){

    *len = ntohs(*(short*)(message->data+message->read_offset));
    message->read_offset += sizeof(short);
    return 1;
}

int tlvDecode1Byte(TlvBuffer* message,char* val)
{
    short len ;
    len = ntohs(*(short*)(message->data+message->read_offset));
	len = len*1;//To escape compiler warning
    message->read_offset += sizeof(short);
    *val = *(short*)(message->data+message->read_offset);
    message->read_offset += 1;
   return 1;

}
int tlvDecode2Bytes(TlvBuffer* message,short* val)
{
    short len ;
    len = ntohs(*(short*)(message->data+message->read_offset));
    len = len*1;//To escape warnings
    message->read_offset += sizeof(short);
    *val = ntohs(*(short*)(message->data+message->read_offset));
    message->read_offset += 2;
   return 1;

}
int tlvDecodeAppended2Bytes(TlvBuffer* message,short* val)
{
    *val = ntohs(*(short*)(message->data+message->read_offset));
    message->read_offset += 2;
    return 1;

}

int tlvDecode4Bytes(TlvBuffer* message,int* val)
{
    short len ;
    len = ntohs(*(short*)(message->data+message->read_offset));
    len = len*1; //To escape compiler warning
    message->read_offset += sizeof(short);
    *val = ntohl(*(int*)(message->data+message->read_offset));
     message->read_offset += 4;
   return 1;

}
int tlvDecodeString(TlvBuffer* message,char* val)
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
int tlvDecode2ByteIntArray(TlvBuffer* message,short** array,short *count){
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
int tlvDecodeAllStats(TlvBuffer* message,AllStats_t** clientArray,short* count)
{
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
int tlvDecodeResults(TlvBuffer* message,RRMClientResponse_t** respArray,short* count)
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

void print_bytes(const void *object, size_t size)
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

