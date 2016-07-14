abstract class SimulationNode {

    id: number = -1;

    x: number = 0;
    y: number = 0;
    aId: number = 0;
    groupNumber: number = 0;
    rawSlotIndex: number = 0;

    type: string = "";

    values:NodeValue[] = [];
}

class NodeValue {

    timestamp:number;

    totalTransmitTime: number = 0;
    totalReceiveTime: number = 0;
    totalDozeTime: number = 0;
    totalActiveTime: number = 0;

    nrOfTransmissions: number = 0;
    nrOfTransmissionsDropped: number = 0;
    nrOfReceives: number = 0;
    nrOfReceivesDropped: number = 0;

    nrOfSentPackets: number = 0;
    nrOfSuccessfulPackets: number = 0;
    nrOfDroppedPackets: number = 0;

    avgSentReceiveTime: number = 0;
    goodputKbit: number = 0;

    edcaQueueLength:number = 0;
    nrOfSuccessfulRoundtripPackets: number = 0;
    avgRoundtripTime: number = 0;

    tcpCongestionWindow: number = 0;
    numberOfTCPRetransmissions: number = 0;

    nrOfReceivesDroppedByDestination: number = 0;

    numberOfTCPRetransmissionsFromAP : number = 0;
    numberOfMACTxRTSFailed : number = 0;
    numberofMACTxMissedACKAndDroppedPacket : number = 0;
    numberOfMACTxMissedACK : number = 0;

    numberOfDropsByReasonUnknown:number = 0;
    numberOfDropsByReasonPhyInSleepMode:number = 0;
    numberOfDropsByReasonPhyNotEnoughSignalPower:number = 0;
    numberOfDropsByReasonPhyUnsupportedMode:number = 0;
    numberOfDropsByReasonPhyPreambleHeaderReceptionFailed:number = 0;
    numberOfDropsByReasonPhyRxDuringChannelSwitching:number = 0;
    numberOfDropsByReasonPhyAlreadyReceiving:number = 0;
    numberOfDropsByReasonPhyAlreadyTransmitting:number = 0;
    numberOfDropsByReasonPhyAlreadyPlcpReceptionFailed:number = 0;
    numberOfDropsByReasonMacNotForAP:number = 0;
    numberOfDropsByReasonMacAPToAPFrame:number = 0;
    numberOfDropsByReasonMacQueueDelayExceeded:number = 0;
    numberOfDropsByReasonMacQeueuSizeExceeded:number = 0;

    numberOfDropsFromAPByReasonUnknown:number = 0;
    numberOfDropsFromAPByReasonPhyInSleepMode:number = 0;
    numberOfDropsFromAPByReasonPhyNotEnoughSignalPower:number = 0;
    numberOfDropsFromAPByReasonPhyUnsupportedMode:number = 0;
    numberOfDropsFromAPByReasonPhyPreambleHeaderReceptionFailed:number = 0;
    numberOfDropsFromAPByReasonPhyRxDuringChannelSwitching:number = 0;
    numberOfDropsFromAPByReasonPhyAlreadyReceiving:number = 0;
    numberOfDropsFromAPByReasonPhyAlreadyTransmitting:number = 0;
    numberOfDropsFromAPByReasonPhyAlreadyPlcpReceptionFailed:number = 0;
    numberOfDropsFromAPByReasonMacNotForAP:number = 0;
    numberOfDropsFromAPByReasonMacAPToAPFrame:number = 0;

    //numberOfDropsFromAPByReasonMacQueueDelayExceeded:number = 0;
    //numberOfDropsFromAPByReasonMacQeueuSizeExceeded:number = 0;

    tcpRTO: number = 0;

    numberOfAPScheduledPacketForNodeInNextSlot:number = 0;
    numberOfAPSentPacketForNodeImmediately:number = 0;
    avgRemainingSlotTimeWhenAPSendingInSameSlot:number = 0;

    numberOfCollisions:number = 0;
}

class APNode extends SimulationNode {
    type: string = "AP";
}

class STANode extends SimulationNode {
    type: string = "STA";

    isAssociated: boolean = false;
}


class SimulationConfiguration {

    AIDRAWRange: number;
    numberOfRAWGroups: number;

    RAWSlotFormat: string;
    numberOfRAWSlots: number;
    RAWSlotDuration: number;

    dataMode: string;
    dataRate: number;
    bandwidth: number;

    trafficInterval: number;
    trafficPacketsize: number;

    beaconInterval: number;

    name:string = "";


    propagationLossExponent:number;
    propagationLossReferenceLoss:number;
    apAlwaysSchedulesForNextSlot:string;
    minRTO:number;
    simulationTime:number;
}
 
 
class Simulation {

    nodes: SimulationNode[] = [];

    slotUsage:number[][] = [];

    totalSlotUsage:number[] = [];

    config: SimulationConfiguration = new SimulationConfiguration();
}