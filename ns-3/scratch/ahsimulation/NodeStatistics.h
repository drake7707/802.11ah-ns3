#ifndef NODESTATISTICS_H
#define NODESTATISTICS_H

#include "ns3/core-module.h"

using namespace std;
using namespace ns3;

class NodeStatistics {

public:
    Time TotalTransmitTime = Time();
    Time TotalReceiveTime = Time();
    Time TotalReceiveDozeTime = Time();
    Time TotalReceiveActiveTime = Time();
    
    long NumberOfTransmissions = 0;
    long NumberOfTransmissionsDropped = 0;
    long NumberOfReceives = 0;
    long NumberOfReceivesDropped = 0;
    
    long NumberOfSuccessfulPackets = 0;
    long NumberOfSentPackets = 0;
    
    long getNumberOfDroppedPackets();

    Time TotalPacketTimeOfFlight = Time();
    long TotalPacketPayloadSize = 0;
    
    Time getAveragePacketTimeOfFlight();

    double getThroughputKbit();
};

#endif /* NODESTATISTICS_H */
