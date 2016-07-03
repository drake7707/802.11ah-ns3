#pragma once

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/extension-headers.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#include <string>

using namespace ns3;
using namespace std;

struct Configuration {
	double simulationTime = 10;
	uint32_t seed = 1;
	uint32_t Nsta = 1;
	uint32_t NRawSta = 1;
	uint32_t SlotFormat = 1;
	uint32_t NRawSlotCount = 829;
	uint32_t NRawSlotNum = 1;
	uint32_t NGroup = 1;
	uint32_t BeaconInterval = 102400;

	string DataMode = "OfdmRate2_4MbpsBW1MHz";
	double datarate = 2.4;
	double bandWidth = 1;

	string visualizerIP = "";
	int visualizerPort = 7707;

	string rho = "250.0";

	string name = "";

	uint32_t trafficInterval = 10000;
	uint16_t trafficPacketSize = 256;
	Configuration();
	Configuration(int argc, char** argv);

};
