Overview
========
The motivation of this project is to simulate OFDMA/802.11ax type of systems. 
As part of this project we have developed few new classes and modified many 
existing ones to support OFDMA/802.11ax type of WLAN systems. This simulator 
includes MAC and abstracted PHY layers.
Note : Our code is not fully compliant with IEEE802.11ax standard

Downloading and Installation - http://www.nsnam.org/wiki/Installation
============================
Follow the https://www.nsnam.org/wiki/Installation#Installation to install and 
build an ns3 repo. Make sure you install the ns-3.26 version as the patch is 
compatible with ns-3.26 only

	
Patch and build 
=============
Clone the cisco github repository 

    $ git clone https://github.com/cisco/ns3-802.11ax-simulator <cloneDir>
	
Apply the patch and add files from ciso cloned repo to the ns3 installated directory

    $ cd <ns3_installed_dir>
    
    $ find <cloneDir>/ns-3.26 -type f  -exec cp {} {} \;
    
    $ git apply <cloneDir>/tools/ofdma.patch
    
    
Build

    $ ./waf build


Usage with example
================

Run Program

    $ ./waf --run scratch/wifi-ofdma-multi-traffic	 - builds and runs
    
To debug if something goes wrong in run time

    $ ./waf --run scratch/wifi-ofdma-multi-traffic "--command-template=gdb/lldb %s"
    
Passing parameters to the program

    Some variables such as packet size, number of packet etc can be modified by command 
    line argument passing as well
    
    $ ./waf --run scratch/wifi-ofdma-multi-traffic "--numPackets=10"


Models
======
Newly added Models:

    RRMWifiManager, HEWifiChannelHelper, HEWifiPhyHelper, HEWifiPhy, HEWifiChannel, 
    Enterprise11axPropagationLossModel, HEBitMap, PerTag
    
    RRMWifiManager            : 	Once resources are allocated, it triggers MAC layer for packet transmission.
    				 
    HEWifiChannelHelper       : 	This class helps to configure HEWifiChannel parameters from
    					program such as channel number etc.
    
    HEWifiPhyHelper           : 	This class helps to configure HEWifiPhy parameters from
    					program such as error rate models etc.
    				 
    HEWifiPhy                 :		This class supports some of the OFDMA aspects
    				 
    HEWifiChannel  	      :		This class helps for OFDMA transmissions and calculates 
    					propagation loss using Enterprise11axPropagationLossModel.
    						       
    HEBitMap		      : 	This class is used to for RU (Resource Unit) bitmap

    
    PerTag 		      :		This class is used to log PER (Packet Error Rate) for each packet.
    				 
Some other classes such as MacLow, WifiTxVector, WifiMacHeader, EdcaTxopN etc. have been modified to 
support RU concept.
    
    

