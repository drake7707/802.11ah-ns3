./waf --run "scratch/ahsimulation/ahsimulation"\
" --NGroup=1"\
" --SlotFormat=0"\
" --NRawSlotCount=162"\
" --NRawSlotNum=5"\
" --DataMode=\"OfdmRate650KbpsBW2MHz\""\
" --Datarate=0.65"\
" --BandWidth=2"\
" --Rho=\"500\""\
" --SimulationTime=100"\
" --TrafficPacketSize=100"\
" --TrafficInterval=10000"\
" --TrafficIntervalDeviation=1000"\
" --BeaconInterval=102400"\
" --MinRTO=819200000"\
" --TCPConnectionTimeout=60000000"\
" --APAlwaysSchedulesForNextSlot=true"\
" --APScheduleTransmissionForNextSlotIfLessThan=5000"\
" --NRawSta=96"\
" --Nsta=1"\
" --VisualizerIP=\"localhost\""\
" --VisualizerPort=7707"\
" --VisualizerSamplingInterval=1"\
" --APPcapFile=\"appcap\""\
" --Name=\"test\""
