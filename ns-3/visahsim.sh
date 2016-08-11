./waf --run "scratch/ahsimulation/ahsimulation"\
" --NGroup=4"\
" --SlotFormat=0"\
" --NRawSlotCount=162"\
" --NRawSlotNum=5"\
" --DataMode=\"MCS2_0\""\
" --Rho=\"500\""\
" --MaxTimeOfPacketsInQueue=1000"\
" --SimulationTime=200"\
" --TrafficPacketSize=100"\
" --TrafficInterval=1000"\
" --TrafficIntervalDeviation=1000"\
" --TrafficType=\"tcpecho\""\
" --IpCameraMotionPercentage=1"\
" --IpCameraMotionDuration=10"\
" --IpCameraDataRate=8"\
" --BeaconInterval=102400"\
" --MinRTO=819200000"\
" --TCPConnectionTimeout=60000000"\
" --TCPSegmentSize=536"\
" --APAlwaysSchedulesForNextSlot=false"\
" --APScheduleTransmissionForNextSlotIfLessThan=5000"\
" --NRawSta=96"\
" --Nsta=5"\
" --VisualizerIP=\"localhost\""\
" --VisualizerPort=7000"\
" --VisualizerSamplingInterval=1"\
" --APPcapFile=\"appcap\""\
" --NSSFile=\"/home/dwight/ns3ah/802.11ah-ns3/ns-3/test.nss\""\
" --Name=\"test\""
