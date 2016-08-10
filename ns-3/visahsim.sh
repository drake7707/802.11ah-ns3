./waf --run "scratch/ahsimulation/ahsimulation"\
" --NGroup=4"\
" --SlotFormat=0"\
" --NRawSlotCount=162"\
" --NRawSlotNum=5"\
" --DataMode=\"OfdmRate650KbpsBW2MHz\""\
" --Datarate=0.65"\
" --BandWidth=2"\
" --Rho=\"500\""\
" --MaxTimeOfPacketsInQueue=1000"\
" --SimulationTime=100"\
" --TrafficPacketSize=100"\
" --TrafficInterval=1000"\
" --TrafficIntervalDeviation=1000"\
" --TrafficType=\"tcpipcamera\""\
" --IpCameraMotionPercentage=0.1"\
" --IpCameraMotionDuration=60"\
" --IpCameraDataRate=4"\
" --BeaconInterval=102400"\
" --MinRTO=819200000"\
" --TCPConnectionTimeout=60000000"\
" --TCPSegmentSize=100"\
" --APAlwaysSchedulesForNextSlot=true"\
" --APScheduleTransmissionForNextSlotIfLessThan=5000"\
" --NRawSta=96"\
" --Nsta=1"\
" --VisualizerIP=\"192.168.0.247\""\
" --VisualizerPort=7707"\
" --VisualizerSamplingInterval=1"\
" --APPcapFile=\"appcap\""\
" --Name=\"test\""
