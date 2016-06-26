#include "Configuration.h"

Configuration::Configuration() {
    
}

Configuration::Configuration(int argc, char** argv) {
    CommandLine cmd;
    cmd.AddValue("seed", "random seed", seed);
    cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
    cmd.AddValue("payloadSize", "Size of payload", payloadSize);
    cmd.AddValue("Nsta", "number of total stations", Nsta);
    cmd.AddValue("NRawSta", "number of stations supporting RAW", NRawSta);
    cmd.AddValue("SlotFormat", "format of NRawSlotCount", SlotFormat);
    cmd.AddValue("NRawSlotCount", "number of stations supporting RAW", NRawSlotCount);
    cmd.AddValue("NRawSlotNum", "number of stations supporting RAW", NRawSlotNum);
    cmd.AddValue("NGroup", "number of RAW group", NGroup);
    cmd.AddValue("BeaconInterval", "Beacon interval time in us", BeaconInterval);
    cmd.AddValue("DataMode", "Date mode", DataMode);
    cmd.AddValue("datarate", "data rate in Mbps", datarate);
    cmd.AddValue("bandWidth", "bandwidth in MHz", bandWidth);
    cmd.AddValue("UdpInterval", "traffic mode", UdpInterval);
    cmd.AddValue("rho", "maximal distance between AP and stations", rho);
    cmd.AddValue("folder", "folder where result files are placed", folder);
    cmd.AddValue("file", "files containing reslut information", file);
    cmd.AddValue("pcapfile", "files containing reslut information", pcapfile);
    cmd.Parse(argc, argv);
}