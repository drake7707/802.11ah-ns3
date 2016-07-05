
#include "s1g-ap-wifi-mac.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"
#include "qos-tag.h"
#include "wifi-phy.h"
#include "dcf-manager.h"
#include "mac-rx-middle.h"
#include "mac-tx-middle.h"
#include "mgt-headers.h"
#include "extension-headers.h"
#include "mac-low.h"
#include "amsdu-subframe-header.h"
#include "msdu-aggregator.h"
#include "ns3/uinteger.h"
#include "wifi-mac-queue.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("S1gApWifiMac");

    NS_OBJECT_ENSURE_REGISTERED(S1gApWifiMac);

    TypeId
	S1gApWifiMac::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::S1gApWifiMac")
                .SetParent<RegularWifiMac> ()
                .SetGroupName("Wifi")
                .AddConstructor<S1gApWifiMac> ()
                .AddAttribute("BeaconInterval", "Delay between two beacons",
                TimeValue(MicroSeconds(102400)),
                MakeTimeAccessor(&S1gApWifiMac::GetBeaconInterval,
                &S1gApWifiMac::SetBeaconInterval),
                MakeTimeChecker())
                .AddAttribute("BeaconJitter", "A uniform random variable to cause the initial beacon starting time (after simulation time 0) "
                "to be distributed between 0 and the BeaconInterval.",
                StringValue("ns3::UniformRandomVariable"),
                MakePointerAccessor(&S1gApWifiMac::m_beaconJitter),
                MakePointerChecker<UniformRandomVariable> ())
                .AddAttribute("EnableBeaconJitter", "If beacons are enabled, whether to jitter the initial send event.",
                BooleanValue(false),
                MakeBooleanAccessor(&S1gApWifiMac::m_enableBeaconJitter),
                MakeBooleanChecker())
                .AddAttribute("BeaconGeneration", "Whether or not beacons are generated.",
                BooleanValue(true),
                MakeBooleanAccessor(&S1gApWifiMac::SetBeaconGeneration,
                &S1gApWifiMac::GetBeaconGeneration),
                MakeBooleanChecker())
                .AddAttribute("NRawGroupStas", "Number of stations in one Raw Group",
                UintegerValue(6000),
                MakeUintegerAccessor(&S1gApWifiMac::GetRawGroupInterval,
                &S1gApWifiMac::SetRawGroupInterval),
                MakeUintegerChecker<uint32_t> ())
                .AddAttribute("NRawStations", "Number of total stations support RAW",
                UintegerValue(100),
                MakeUintegerAccessor(&S1gApWifiMac::GetTotalStaNum,
                &S1gApWifiMac::SetTotalStaNum),
                MakeUintegerChecker<uint32_t> ())
                .AddAttribute("SlotFormat", "Slot format",
                UintegerValue(1),
                MakeUintegerAccessor(&S1gApWifiMac::GetSlotFormat,
                &S1gApWifiMac::SetSlotFormat),
                MakeUintegerChecker<uint32_t> ())
                .AddAttribute("SlotCrossBoundary", "cross slot boundary or not",
                UintegerValue(1),
                MakeUintegerAccessor(&S1gApWifiMac::GetSlotCrossBoundary,
                &S1gApWifiMac::SetSlotCrossBoundary),
                MakeUintegerChecker<uint32_t> ())
                .AddAttribute("SlotDurationCount", "slot duration count",
                UintegerValue(1000),
                MakeUintegerAccessor(&S1gApWifiMac::GetSlotDurationCount,
                &S1gApWifiMac::SetSlotDurationCount),
                MakeUintegerChecker<uint32_t> ())
                .AddAttribute("SlotNum", "Number of slot",
                UintegerValue(2),
                MakeUintegerAccessor(&S1gApWifiMac::GetSlotNum,
                &S1gApWifiMac::SetSlotNum),
                MakeUintegerChecker<uint32_t> ())
                .AddTraceSource("S1gBeaconBroadcasted", "Fired when a beacon is transmitted",
                MakeTraceSourceAccessor(&S1gApWifiMac::m_transmitBeaconTrace),
                "ns3::S1gApWifiMac::S1gBeaconTracedCallback");

        return tid;
    }

    S1gApWifiMac::S1gApWifiMac() {
        NS_LOG_FUNCTION(this);
        m_beaconDca = CreateObject<DcaTxop> ();
        m_beaconDca->SetAifsn(1);
        m_beaconDca->SetMinCw(0);
        m_beaconDca->SetMaxCw(0);
        m_beaconDca->SetLow(m_low);
        m_beaconDca->SetManager(m_dcfManager);
        m_beaconDca->SetTxMiddle(m_txMiddle);

        //Let the lower layers know that we are acting as an AP.
        SetTypeOfStation(AP);

        m_enableBeaconGeneration = false;
        AuthenThreshold = 0;
        //m_SlotFormat = 0;
    }

    S1gApWifiMac::~S1gApWifiMac() {
        NS_LOG_FUNCTION(this);
    }

    void
	S1gApWifiMac::DoDispose() {
        NS_LOG_FUNCTION(this);
        m_beaconDca = 0;
        m_enableBeaconGeneration = false;
        m_beaconEvent.Cancel();
        RegularWifiMac::DoDispose();
    }

    void
	S1gApWifiMac::SetAddress(Mac48Address address) {
        NS_LOG_FUNCTION(this << address);
        //As an AP, our MAC address is also the BSSID. Hence we are
        //overriding this function and setting both in our parent class.
        RegularWifiMac::SetAddress(address);
        RegularWifiMac::SetBssid(address);
    }

    void
	S1gApWifiMac::SetBeaconGeneration(bool enable) {
        NS_LOG_FUNCTION(this << enable);
        if (!enable) {
            m_beaconEvent.Cancel();
        } else if (enable && !m_enableBeaconGeneration) {
            m_beaconEvent = Simulator::ScheduleNow(&S1gApWifiMac::SendOneBeacon, this);
        }
        m_enableBeaconGeneration = enable;
    }

    bool
	S1gApWifiMac::GetBeaconGeneration(void) const {
        NS_LOG_FUNCTION(this);
        return m_enableBeaconGeneration;
    }

    Time
	S1gApWifiMac::GetBeaconInterval(void) const {
        NS_LOG_FUNCTION(this);
        return m_beaconInterval;
    }

    uint32_t
	S1gApWifiMac::GetRawGroupInterval(void) const {
        NS_LOG_FUNCTION(this);
        return m_rawGroupInterval;
    }

    uint32_t
	S1gApWifiMac::GetTotalStaNum(void) const {
        NS_LOG_FUNCTION(this);
        return m_totalStaNum;
    }

    uint32_t
	S1gApWifiMac::GetSlotFormat(void) const {
        return m_SlotFormat;
    }

    uint32_t
	S1gApWifiMac::GetSlotCrossBoundary(void) const {
        return m_slotCrossBoundary;
    }

    uint32_t
	S1gApWifiMac::GetSlotDurationCount(void) const {
        return m_slotDurationCount;
    }

    uint32_t
	S1gApWifiMac::GetSlotNum(void) const {
        return m_slotNum;
    }

    void
	S1gApWifiMac::SetWifiRemoteStationManager(Ptr<WifiRemoteStationManager> stationManager) {
        NS_LOG_FUNCTION(this << stationManager);
        m_beaconDca->SetWifiRemoteStationManager(stationManager);
        RegularWifiMac::SetWifiRemoteStationManager(stationManager);
    }

    void
	S1gApWifiMac::SetLinkUpCallback(Callback<void> linkUp) {
        NS_LOG_FUNCTION(this << &linkUp);
        RegularWifiMac::SetLinkUpCallback(linkUp);

        //The approach taken here is that, from the point of view of an AP,
        //the link is always up, so we immediately invoke the callback if
        //one is set
        linkUp();
    }

    void
	S1gApWifiMac::SetBeaconInterval(Time interval) {
        NS_LOG_FUNCTION(this << interval);
        if ((interval.GetMicroSeconds() % 1024) != 0) {
            NS_LOG_WARN("beacon interval should be multiple of 1024us (802.11 time unit), see IEEE Std. 802.11-2012");
        }
        m_beaconInterval = interval;
    }

    void
	S1gApWifiMac::SetRawGroupInterval(uint32_t interval) {
        NS_LOG_FUNCTION(this << interval);
        m_rawGroupInterval = interval;
    }

    void
	S1gApWifiMac::SetTotalStaNum(uint32_t num) {
        NS_LOG_FUNCTION(this << num);
        m_totalStaNum = num;
    }

    void
	S1gApWifiMac::SetSlotFormat(uint32_t format) {
        NS_ASSERT(format <= 1);
        m_SlotFormat = format;
    }

    void
	S1gApWifiMac::SetSlotCrossBoundary(uint32_t cross) {
        NS_ASSERT(cross <= 1);
        m_slotCrossBoundary = cross;
    }

    void
	S1gApWifiMac::SetSlotDurationCount(uint32_t count) {
        NS_ASSERT((!m_SlotFormat & (count < 256)) || (m_SlotFormat & (count < 2048)));
        m_slotDurationCount = count;
    }

    void
	S1gApWifiMac::SetSlotNum(uint32_t count) {
        NS_ASSERT((!m_SlotFormat & (count < 64)) || (m_SlotFormat & (count < 8)));
        m_slotNum = count;
    }

    void
	S1gApWifiMac::StartBeaconing(void) {
        NS_LOG_FUNCTION(this);
        SendOneBeacon();
    }

    int64_t
	S1gApWifiMac::AssignStreams(int64_t stream) {
        NS_LOG_FUNCTION(this << stream);
        m_beaconJitter->SetStream(stream);
        return 1;
    }

    void
	S1gApWifiMac::ForwardDown(Ptr<const Packet> packet, Mac48Address from,
            Mac48Address to) {
        NS_LOG_FUNCTION(this << packet << from << to);
        //If we are not a QoS AP then we definitely want to use AC_BE to
        //transmit the packet. A TID of zero will map to AC_BE (through \c
        //QosUtilsMapTidToAc()), so we use that as our default here.
        uint8_t tid = 0;

        //If we are a QoS AP then we attempt to get a TID for this packet
        if (m_qosSupported) {
            tid = QosUtilsGetTidForPacket(packet);
            //Any value greater than 7 is invalid and likely indicates that
            //the packet had no QoS tag, so we revert to zero, which'll
            //mean that AC_BE is used.
            if (tid > 7) {
                tid = 0;
            }
        }

        ForwardDown(packet, from, to, tid);
    }

    void
	S1gApWifiMac::ForwardDown(Ptr<const Packet> packet, Mac48Address from,
            Mac48Address to, uint8_t tid) {
        NS_LOG_FUNCTION(this << packet << from << to << static_cast<uint32_t> (tid));
        WifiMacHeader hdr;

        //For now, an AP that supports QoS does not support non-QoS
        //associations, and vice versa. In future the AP model should
        //support simultaneously associated QoS and non-QoS STAs, at which
        //point there will need to be per-association QoS state maintained
        //by the association state machine, and consulted here.
        if (m_qosSupported) {
            hdr.SetType(WIFI_MAC_QOSDATA);
            hdr.SetQosAckPolicy(WifiMacHeader::NORMAL_ACK);
            hdr.SetQosNoEosp();
            hdr.SetQosNoAmsdu();
            //Transmission of multiple frames in the same TXOP is not
            //supported for now
            hdr.SetQosTxopLimit(0);
            //Fill in the QoS control field in the MAC header
            hdr.SetQosTid(tid);
        } else {
            hdr.SetTypeData();
        }

        if (m_htSupported) {
            hdr.SetNoOrder();
        }
        hdr.SetAddr1(to);
        hdr.SetAddr2(GetAddress());
        hdr.SetAddr3(from);
        hdr.SetDsFrom();
        hdr.SetDsNotTo();

        if (m_qosSupported) {
            //Sanity check that the TID is valid
            NS_ASSERT(tid < 8);
            m_edca[QosUtilsMapTidToAc(tid)]->Queue(packet, hdr);
        } else {
            m_dca->Queue(packet, hdr);
        }
    }

    void
	S1gApWifiMac::Enqueue(Ptr<const Packet> packet, Mac48Address to, Mac48Address from) {
        NS_LOG_FUNCTION(this << packet << to << from);
        if (to.IsBroadcast() || m_stationManager->IsAssociated(to)) {


        	// strip tags because they'll derefence
        	// TODO re-add a copy of those tags, similar to the PrepareForQueue method in the PacketQueue
        	ConstCast<Packet> (packet)->RemoveAllByteTags();
        	ConstCast<Packet> (packet)->RemoveAllPacketTags();

        	uint16_t aId = macToAIDMap[to];
        	if(aId >= current_aid_start && aId <= current_aid_end) {
        		// send it through directly
        		// technically it should be scheduled to the correct RAW slot
        		std::cout << "sending data directly" << std::endl;
        		ForwardDown(packet, from, to);
        	}
        	else {
        		std::cout << "enqueuing data for " << aId << std::endl;
        		//this->pendingDataForStations.at(aId-1).push(std::move(PendingData(from,to, &packet)));

        		// determine when the next raw window hits for this packet
        		int remainingBeacons = 0;
        		int curStart = current_aid_start;
        		while(! (aId >= curStart && aId <= curStart+ m_rawGroupInterval)) {

        			remainingBeacons++;
        			curStart += m_rawGroupInterval;
        			if(curStart + m_rawGroupInterval > m_totalStaNum)
        				curStart = 1;
        		}

        		Time packetSendTime = m_beaconInterval * remainingBeacons;

        		void (S1gApWifiMac::*fp)(Ptr<const Packet>, Mac48Address, Mac48Address) = &S1gApWifiMac::ForwardDown;
        		Simulator::Schedule(packetSendTime, fp, this, packet, from ,to);
        	}
        }
    }

    void
	S1gApWifiMac::Enqueue(Ptr<const Packet> packet, Mac48Address to) {
        NS_LOG_FUNCTION(this << packet << to);
        //We're sending this packet with a from address that is our own. We
        //get that address from the lower MAC and make use of the
        //from-spoofing Enqueue() method to avoid duplicated code.
        Enqueue(packet, to, m_low->GetAddress());
    }

    bool
	S1gApWifiMac::SupportsSendFrom(void) const {
        NS_LOG_FUNCTION(this);
        return true;
    }

    SupportedRates
	S1gApWifiMac::GetSupportedRates(void) const {
        NS_LOG_FUNCTION(this);
        SupportedRates rates;
        //If it is an HT-AP then add the BSSMembershipSelectorSet
        //which only includes 127 for HT now. The standard says that the BSSMembershipSelectorSet
        //must have its MSB set to 1 (must be treated as a Basic Rate)
        //Also the standard mentioned that at leat 1 element should be included in the SupportedRates the rest can be in the ExtendedSupportedRates
        if (m_htSupported) {
            for (uint32_t i = 0; i < m_phy->GetNBssMembershipSelectors(); i++) {
                rates.SetBasicRate(m_phy->GetBssMembershipSelector(i));
            }
        }
        //Send the set of supported rates and make sure that we indicate
        //the Basic Rate set in this set of supported rates.
        // NS_LOG_LOGIC ("ApWifiMac::GetSupportedRates  1 " ); //for test
        for (uint32_t i = 0; i < m_phy->GetNModes(); i++) {
            WifiMode mode = m_phy->GetMode(i);
            rates.AddSupportedRate(mode.GetDataRate());
            //Add rates that are part of the BSSBasicRateSet (manufacturer dependent!)
            //here we choose to add the mandatory rates to the BSSBasicRateSet,
            //exept for 802.11b where we assume that only the two lowest mandatory rates are part of the BSSBasicRateSet
            if (mode.IsMandatory()
                    && ((mode.GetModulationClass() != WIFI_MOD_CLASS_DSSS) || mode == WifiPhy::GetDsssRate1Mbps() || mode == WifiPhy::GetDsssRate2Mbps())) {
                m_stationManager->AddBasicMode(mode);
            }
        }
        // NS_LOG_LOGIC ("ApWifiMac::GetSupportedRates  2 " ); //for test
        //set the basic rates
        for (uint32_t j = 0; j < m_stationManager->GetNBasicModes(); j++) {
            WifiMode mode = m_stationManager->GetBasicMode(j);
            rates.SetBasicRate(mode.GetDataRate());
        }
        //NS_LOG_LOGIC ("ApWifiMac::GetSupportedRates   " ); //for test
        return rates;
    }

    HtCapabilities
	S1gApWifiMac::GetHtCapabilities(void) const {
        HtCapabilities capabilities;
        capabilities.SetHtSupported(1);
        capabilities.SetLdpc(m_phy->GetLdpc());
        capabilities.SetShortGuardInterval20(m_phy->GetGuardInterval());
        capabilities.SetGreenfield(m_phy->GetGreenfield());
        for (uint8_t i = 0; i < m_phy->GetNMcs(); i++) {
            capabilities.SetRxMcsBitmask(m_phy->GetMcs(i));
        }
        return capabilities;
    }

    void
	S1gApWifiMac::SendProbeResp(Mac48Address to) {
        NS_LOG_FUNCTION(this << to);
        WifiMacHeader hdr;
        hdr.SetProbeResp();
        hdr.SetAddr1(to);
        hdr.SetAddr2(GetAddress());
        hdr.SetAddr3(GetAddress());
        hdr.SetDsNotFrom();
        hdr.SetDsNotTo();
        Ptr<Packet> packet = Create<Packet> ();
        MgtProbeResponseHeader probe;
        probe.SetSsid(GetSsid());
        probe.SetSupportedRates(GetSupportedRates());
        probe.SetBeaconIntervalUs(m_beaconInterval.GetMicroSeconds());
        if (m_htSupported) {
            probe.SetHtCapabilities(GetHtCapabilities());
            hdr.SetNoOrder();
        }
        packet->AddHeader(probe);

        //The standard is not clear on the correct queue for management
        //frames if we are a QoS AP. The approach taken here is to always
        //use the DCF for these regardless of whether we have a QoS
        //association or not.
        m_dca->Queue(packet, hdr);
    }

    void
	S1gApWifiMac::SendAssocResp(Mac48Address to, bool success) {
        NS_LOG_FUNCTION(this << to << success);
        WifiMacHeader hdr;
        hdr.SetAssocResp();
        hdr.SetAddr1(to);
        hdr.SetAddr2(GetAddress());
        hdr.SetAddr3(GetAddress());
        hdr.SetDsNotFrom();
        hdr.SetDsNotTo();
        Ptr<Packet> packet = Create<Packet> ();
        MgtAssocResponseHeader assoc;

        uint8_t mac[6];
        to.CopyTo(mac);
        uint8_t aid_l = mac[5];
        uint8_t aid_h = mac[4] & 0x1f;
        uint16_t aid = (aid_h << 8) | (aid_l << 0); //assign mac address as AID

        // keep track of a mac -> aid map
        macToAIDMap[to] = aid;

        assoc.SetAID(aid); //
        StatusCode code;
        if (success) {
            code.SetSuccess();
        } else {
            code.SetFailure();
        }
        assoc.SetSupportedRates(GetSupportedRates());
        assoc.SetStatusCode(code);

        if (m_htSupported) {
            assoc.SetHtCapabilities(GetHtCapabilities());
            hdr.SetNoOrder();
        }
        packet->AddHeader(assoc);

        //The standard is not clear on the correct queue for management
        //frames if we are a QoS AP. The approach taken here is to always
        //use the DCF for these regardless of whether we have a QoS
        //association or not.
        m_dca->Queue(packet, hdr);
    }




    void
	S1gApWifiMac::SendOneBeacon(void) {
        NS_LOG_FUNCTION(this);
        WifiMacHeader hdr;
        if (m_s1gSupported) {
            hdr.SetS1gBeacon();
            hdr.SetAddr1(Mac48Address::GetBroadcast());
            hdr.SetAddr2(GetAddress()); // for debug, not accordance with draft, need change
            hdr.SetAddr3(GetAddress()); // for debug
            Ptr<Packet> packet = Create<Packet> ();
            S1gBeaconHeader beacon;
            S1gBeaconCompatibility compatibility;
            compatibility.SetBeaconInterval(m_beaconInterval.GetMicroSeconds());
            beacon.SetBeaconCompatibility(compatibility);
            RPS m_rps;
            RPS::RawAssignment raw;
            uint8_t control = 0;
            raw.SetRawControl(control); //support paged STA or not
            raw.SetSlotFormat(m_SlotFormat);
            raw.SetSlotCrossBoundary(m_slotCrossBoundary);
            raw.SetSlotDurationCount(m_slotDurationCount);
            raw.SetSlotNum(m_slotNum);

            uint32_t page = 0;

            uint32_t rawinfo = (current_aid_end << 13) | (current_aid_start << 2) | page;

            raw.SetRawGroup(rawinfo); // (b0-b1, page index) (b2-b12, raw start AID) (b13-b23, raw end AID)


            // TODO set partial bitmap for TIM beacon for individual station indices

            //std::cout << "Sending S1G Beacon with raw range " << current_aid_start << " - " << current_aid_end << std::endl;

            m_rps.SetRawAssignment(raw);

            beacon.SetRPS(m_rps);

            AuthenticationCtrl AuthenCtrl;
            AuthenCtrl.SetControlType(false); //centralized
            Ptr<WifiMacQueue> MgtQueue = m_dca->GetQueue();
            uint32_t MgtQueueSize = MgtQueue->GetSize();
            if (MgtQueueSize < 10) {
                if (AuthenThreshold <= 950) {
                    AuthenThreshold += 50;
                }
            } else {
                if (AuthenThreshold > 50) {
                    AuthenThreshold -= 50;
                }
            }
            AuthenCtrl.SetThreshold(AuthenThreshold); //centralized
            beacon.SetAuthCtrl(AuthenCtrl);


            if(m_currentBeaconTIMGroup == 0) {
            	// DTIM header
            	//TIM tim;
            	//tim.SetDTIMPeriod(m_nrOfTIMGroups);
            	//tim.SetDTIMCount(0);


            	std::vector<bool> map(8, false);

            	for(int i = 0; i < this->m_totalStaNum; i++) {
                	std::queue<PendingData>& q = pendingDataForStations.at(i);
                	if(q.size() > 0) {
                		int group =  i / m_rawGroupInterval;
                		map[group] = true;
                	}
            	}


            	// TODO build a partial virtual map based on
            	// the pending data
            	//tim.SetPartialVBitmap()
            }
            m_currentBeaconTIMGroup = (m_currentBeaconTIMGroup + 1) % m_nrOfTIMGroups;

            packet->AddHeader(beacon);
            m_beaconDca->Queue(packet, hdr);

            m_transmitBeaconTrace(beacon, raw);

            // send pending data for the current RAW


            auto ubound = (current_aid_end >= pendingDataForStations.size()) ? pendingDataForStations.size() : current_aid_end;
            for(int i = current_aid_start; i <= ubound; i++) {
            	std::queue<PendingData>& q = pendingDataForStations.at(i-1);
            	while(!q.empty()) {


            		std::cout << "Sending packet in queue for aId " << i << std::endl;
            		PendingData data =  q.front();

            		Ptr<const Packet> p = *(data.packet);
            		(ConstCast<Packet>)(p)->RemoveAllByteTags();
					(ConstCast<Packet>)(p)->RemoveAllPacketTags();

            		std::cout << "Packet to send: " << p << std::endl;
            		ForwardDown(p, data.from, data.to);
            		q.pop();
            	}
            }

            current_aid_start += m_rawGroupInterval;
		    current_aid_end += m_rawGroupInterval;
		    if (current_aid_end > m_totalStaNum) {
		    	current_aid_start = 1;
		    	current_aid_end = m_rawGroupInterval;
		    }

        } else {
            hdr.SetBeacon();
            hdr.SetAddr1(Mac48Address::GetBroadcast());
            hdr.SetAddr2(GetAddress());
            hdr.SetAddr3(GetAddress());
            hdr.SetDsNotFrom();
            hdr.SetDsNotTo();
            Ptr<Packet> packet = Create<Packet> ();
            MgtBeaconHeader beacon;
            beacon.SetSsid(GetSsid());
            beacon.SetSupportedRates(GetSupportedRates());
            beacon.SetBeaconIntervalUs(m_beaconInterval.GetMicroSeconds());
            if (m_htSupported) {
                beacon.SetHtCapabilities(GetHtCapabilities());
                hdr.SetNoOrder();
            }
            packet->AddHeader(beacon);
            //The beacon has it's own special queue, so we load it in there
            m_beaconDca->Queue(packet, hdr);

        }
        m_beaconEvent = Simulator::Schedule(m_beaconInterval, &S1gApWifiMac::SendOneBeacon, this);
    }

    void
	S1gApWifiMac::TxOk(const WifiMacHeader &hdr) {
        NS_LOG_FUNCTION(this);
        RegularWifiMac::TxOk(hdr);

        if (hdr.IsAssocResp()
                && m_stationManager->IsWaitAssocTxOk(hdr.GetAddr1())) {
            NS_LOG_DEBUG("associated with sta=" << hdr.GetAddr1());
            m_stationManager->RecordGotAssocTxOk(hdr.GetAddr1());
        }
    }

    void
	S1gApWifiMac::TxFailed(const WifiMacHeader &hdr) {
        NS_LOG_FUNCTION(this);
        RegularWifiMac::TxFailed(hdr);

        if (hdr.IsAssocResp()
                && m_stationManager->IsWaitAssocTxOk(hdr.GetAddr1())) {
            NS_LOG_DEBUG("assoc failed with sta=" << hdr.GetAddr1());
            m_stationManager->RecordGotAssocTxFailed(hdr.GetAddr1());
        }
    }

    void
	S1gApWifiMac::Receive(Ptr<Packet> packet, const WifiMacHeader *hdr) {
        NS_LOG_FUNCTION(this << packet << hdr);
        //uint16_t segg =  hdr->GetFrameControl (); // for test
        //NS_LOG_LOGIC ("AP waiting   " << segg); //for test
        Mac48Address from = hdr->GetAddr2();

        if (hdr->IsData()) {
            Mac48Address bssid = hdr->GetAddr1();
            if (!hdr->IsFromDs()
                    && hdr->IsToDs()
                    && bssid == GetAddress()
                    && m_stationManager->IsAssociated(from)) {
                Mac48Address to = hdr->GetAddr3();
                if (to == GetAddress()) {
                    NS_LOG_DEBUG("frame for me from=" << from);
                    if (hdr->IsQosData()) {
                        if (hdr->IsQosAmsdu()) {
                            NS_LOG_DEBUG("Received A-MSDU from=" << from << ", size=" << packet->GetSize());
                            DeaggregateAmsduAndForward(packet, hdr);
                            packet = 0;
                        } else {
                            ForwardUp(packet, from, bssid);
                        }
                    } else {
                        ForwardUp(packet, from, bssid);
                    }
                } else if (to.IsGroup()
                        || m_stationManager->IsAssociated(to)) {
                    NS_LOG_DEBUG("forwarding frame from=" << from << ", to=" << to);
                    Ptr<Packet> copy = packet->Copy();

                    //If the frame we are forwarding is of type QoS Data,
                    //then we need to preserve the UP in the QoS control
                    //header...
                    if (hdr->IsQosData()) {
                        ForwardDown(packet, from, to, hdr->GetQosTid());
                    } else {
                        ForwardDown(packet, from, to);
                    }
                    ForwardUp(copy, from, to);
                } else {
                    ForwardUp(packet, from, to);
                }
            } else if (hdr->IsFromDs()
                    && hdr->IsToDs()) {
                //this is an AP-to-AP frame
                //we ignore for now.
                NotifyRxDrop(packet);
            } else {
                //we can ignore these frames since
                //they are not targeted at the AP
                NotifyRxDrop(packet);
            }
            return;
        } else if (hdr->IsMgt()) {
            if (hdr->IsProbeReq()) {
                NS_ASSERT(hdr->GetAddr1().IsBroadcast());
                SendProbeResp(from);
                return;
            } else if (hdr->GetAddr1() == GetAddress()) {
                if (hdr->IsAssocReq()) {
                    if (m_stationManager->IsAssociated(from)) {
                        return; //test, avoid repeate assoc
                    }
                    //NS_LOG_LOGIC ("Received AssocReq "); // for test
                    //first, verify that the the station's supported
                    //rate set is compatible with our Basic Rate set
                    MgtAssocRequestHeader assocReq;
                    packet->RemoveHeader(assocReq);
                    SupportedRates rates = assocReq.GetSupportedRates();
                    bool problem = false;
                    for (uint32_t i = 0; i < m_stationManager->GetNBasicModes(); i++) {
                        WifiMode mode = m_stationManager->GetBasicMode(i);
                        if (!rates.IsSupportedRate(mode.GetDataRate())) {
                            problem = true;
                            break;
                        }
                    }
                    if (m_htSupported) {
                        //check that the STA supports all MCSs in Basic MCS Set
                        HtCapabilities htcapabilities = assocReq.GetHtCapabilities();
                        for (uint32_t i = 0; i < m_stationManager->GetNBasicMcs(); i++) {
                            uint8_t mcs = m_stationManager->GetBasicMcs(i);
                            if (!htcapabilities.IsSupportedMcs(mcs)) {
                                problem = true;
                                break;
                            }
                        }

                    }
                    if (problem) {
                        //One of the Basic Rate set mode is not
                        //supported by the station. So, we return an assoc
                        //response with an error status.
                        SendAssocResp(hdr->GetAddr2(), false);
                    } else {
                        //station supports all rates in Basic Rate Set.
                        //record all its supported modes in its associated WifiRemoteStation
                        for (uint32_t j = 0; j < m_phy->GetNModes(); j++) {
                            WifiMode mode = m_phy->GetMode(j);
                            if (rates.IsSupportedRate(mode.GetDataRate())) {
                                m_stationManager->AddSupportedMode(from, mode);
                            }
                        }
                        if (m_htSupported) {
                            HtCapabilities htcapabilities = assocReq.GetHtCapabilities();
                            m_stationManager->AddStationHtCapabilities(from, htcapabilities);
                            for (uint32_t j = 0; j < m_phy->GetNMcs(); j++) {
                                uint8_t mcs = m_phy->GetMcs(j);
                                if (htcapabilities.IsSupportedMcs(mcs)) {
                                    m_stationManager->AddSupportedMcs(from, mcs);
                                }
                            }
                        }
                        m_stationManager->RecordWaitAssocTxOk(from);
                        //send assoc response with success status.
                        SendAssocResp(hdr->GetAddr2(), true);
                    }
                    return;
                } else if (hdr->IsDisassociation()) {
                    m_stationManager->RecordDisassociated(from);
                    return;
                }
            }
        }

        //Invoke the receive handler of our parent class to deal with any
        //other frames. Specifically, this will handle Block Ack-related
        //Management Action frames.
        RegularWifiMac::Receive(packet, hdr);
    }

    void
	S1gApWifiMac::DeaggregateAmsduAndForward(Ptr<Packet> aggregatedPacket,
            const WifiMacHeader *hdr) {
        NS_LOG_FUNCTION(this << aggregatedPacket << hdr);
        MsduAggregator::DeaggregatedMsdus packets =
                MsduAggregator::Deaggregate(aggregatedPacket);

        for (MsduAggregator::DeaggregatedMsdusCI i = packets.begin();
                i != packets.end(); ++i) {
            if ((*i).second.GetDestinationAddr() == GetAddress()) {
                ForwardUp((*i).first, (*i).second.GetSourceAddr(),
                        (*i).second.GetDestinationAddr());
            } else {
                Mac48Address from = (*i).second.GetSourceAddr();
                Mac48Address to = (*i).second.GetDestinationAddr();
                NS_LOG_DEBUG("forwarding QoS frame from=" << from << ", to=" << to);
                ForwardDown((*i).first, from, to, hdr->GetQosTid());
            }
        }
    }

    void
	S1gApWifiMac::DoInitialize(void) {
        NS_LOG_FUNCTION(this);
        m_beaconDca->Initialize();
        m_beaconEvent.Cancel();
        if (m_enableBeaconGeneration) {
            if (m_enableBeaconJitter) {
                int64_t jitter = m_beaconJitter->GetValue(0, m_beaconInterval.GetMicroSeconds());
                NS_LOG_DEBUG("Scheduling initial beacon for access point " << GetAddress() << " at time " << jitter << " microseconds");
                m_beaconEvent = Simulator::Schedule(MicroSeconds(jitter), &S1gApWifiMac::SendOneBeacon, this);
            } else {
                NS_LOG_DEBUG("Scheduling initial beacon for access point " << GetAddress() << " at time 0");
                m_beaconEvent = Simulator::ScheduleNow(&S1gApWifiMac::SendOneBeacon, this);
            }
        }

        m_nrOfTIMGroups = ceil(m_totalStaNum / (float)m_rawGroupInterval);
        // initialize queue
        pendingDataForStations = std::vector<std::queue< PendingData> >(m_totalStaNum);

        current_aid_start = 1;
        current_aid_end = m_rawGroupInterval;

        RegularWifiMac::DoInitialize();
    }

} //namespace ns3
