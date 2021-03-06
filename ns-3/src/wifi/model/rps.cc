/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "rps.h"
#include "ns3/assert.h"
#include "ns3/log.h" //for test

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE("RPS");

    RPS::RawAssignment::RawAssignment() {
    }

    RPS::RawAssignment::~RawAssignment() {
    }

    void
    RPS::RawAssignment::SetRawControl(uint8_t control) {
        m_rawcontrol = control;
    }

    void
    RPS::RawAssignment::SetSlotFormat(uint8_t format) {
        NS_ASSERT(format <= 1);
        m_SlotFormat = format;
    }

    void
    RPS::RawAssignment::SetSlotCrossBoundary(uint8_t cross) {
        NS_ASSERT(cross <= 1);
        m_slotCrossBoundary = cross;
    }

    void
    RPS::RawAssignment::SetSlotDurationCount(uint16_t count) {
        NS_ASSERT((!m_SlotFormat & (count < 256)) || (m_SlotFormat & (count < 2048)));
        m_slotDurationCount = count;
    }

    void
    RPS::RawAssignment::SetSlotNum(uint16_t count) {
        NS_ASSERT((!m_SlotFormat & (count < 64)) || (m_SlotFormat & (count < 8)));
        m_slotNum = count;
    }

    void
    RPS::RawAssignment::SetRawStart(uint8_t start) {
        m_rawstart = start;
    }

    void
    RPS::RawAssignment::SetRawGroup(uint32_t group) {
        m_rawgroup = group;
    }

    //     uint32_t rawinfo = (aid_end << 13) | (aid_start << 2) | page;

    uint8_t
	RPS::RawAssignment::GetRawGroupPage() const {
    	return m_rawgroup & 0x03;
    }

    uint16_t
	RPS::RawAssignment::GetRawGroupAIDStart()  const {
    	return (m_rawgroup >> 2) & 0x07FF;
    }

    uint16_t
	RPS::RawAssignment::GetRawGroupAIDEnd()  const {
    	return (m_rawgroup >> 13) & 0x07FF;
    }


    void
    RPS::RawAssignment::SetChannelInd(uint16_t channel) {
        m_channelind = channel;
    }

    void
    RPS::RawAssignment::SetPRAW(uint32_t praw) {
        m_prawparam = praw;
    }

    uint8_t
    RPS::RawAssignment::GetRawControl(void) const {
        return m_rawcontrol;
    }

    uint8_t
    RPS::RawAssignment::GetSlotFormat(void) const {
        return m_SlotFormat;
    }

    uint8_t
    RPS::RawAssignment::GetSlotCrossBoundary(void) const {
        return m_slotCrossBoundary;
    }

    uint16_t
    RPS::RawAssignment::GetSlotDurationCount(void) const {
        return m_slotDurationCount;
    }

    uint16_t
    RPS::RawAssignment::GetSlotNum(void) const {
        return m_slotNum;
    }

    uint16_t
    RPS::RawAssignment::GetRawSlot(void) {
        NS_ASSERT(m_SlotFormat <= 1);
        if (m_SlotFormat == 0) {
            m_rawslot = (uint16_t(m_slotCrossBoundary) << 14) | (m_slotDurationCount << 6) | m_slotNum;
        } else {
            m_rawslot = 0x8000 | (uint16_t(m_slotCrossBoundary) << 14) | (m_slotDurationCount << 3) | m_slotNum;
        }
        return m_rawslot;
    }

    void
    RPS::RawAssignment::SetSlot(uint16_t value) {
        uint8_t slotFormat = (value & 0x8000) == 0x8000;

        uint8_t slotCrossBoundary = ((value >> 14) & 0x01);

        this->SetSlotFormat(slotFormat);
        this->SetSlotCrossBoundary(slotCrossBoundary);

        uint16_t slotDuration;
        uint8_t slotNum;
        if (slotFormat == 0) {
            // slot duration is 
            slotDuration = (value >> 6) & 0xFF;
            slotNum = (value) & 0x003F;
        } else {
            slotDuration = (value >> 3) & 0x07FF;
            slotNum = (value) & 0x0007;
        }
        this->SetSlotDurationCount(slotDuration);
        this->SetSlotNum(slotNum);
    }

    uint8_t
    RPS::RawAssignment::GetRawStart(void) const {
        return m_rawstart;
    }

    uint32_t
    RPS::RawAssignment::GetRawGroup(void) const {
        return m_rawgroup;
    }

    uint16_t
    RPS::RawAssignment::GetChannelInd(void) const {
        return m_channelind;
    }

    uint32_t
    RPS::RawAssignment::GetPRAW(void) const {
        return m_prawparam;
    }

    uint8_t
    RPS::RawAssignment::GetSize(void) const {
        //return raw_length;
        return 12;
    }

    RPS::RPS() {
        m_length = 0;
    }

    RPS::~RPS() {
    }

    void
    RPS::Print(std::ostream& os) const {
        auto m_raw = GetRawAssignment();
        os << "RAW Control: " << std::to_string(m_raw[0]) << std::endl;
        os << "RAW Slot: " << std::to_string(m_raw[2] << 8 + m_raw[1]) << std::endl;
        os << "RAW Start: " << std::to_string(m_raw[3]) << std::endl;
        os << "RAW Group: " << std::to_string(((int) m_raw[6] << 16) + ((int) m_raw[5] << 8) + m_raw[4]) << std::endl;
        os << "Channel index: " << std::to_string(((int) m_raw[8] << 16) + m_raw[7]) << std::endl;
        os << "PRAW: " << std::to_string(((int) m_raw[11] << 16) + ((int) m_raw[10] << 8) + m_raw[9]) << std::endl;
    }


    //suppose all subfield of RAW Assignment are presented, 12 octets
    // change in future

    void
    RPS::SetRawAssignment(RPS::RawAssignment raw) {
        m_length = 0;
        assignment = raw;
        static uint8_t m_rpsarry[12]; //! Support up to 10 RAW Assignment subfield
        //uint8_t len = assignment.GetSize ();
        m_rpsarry[m_length] = assignment.GetRawControl();
        m_length++;
        m_rpsarry[m_length] = (uint8_t) assignment.GetRawSlot();
        m_length++;
        m_rpsarry[m_length] = (uint8_t) (assignment.GetRawSlot() >> 8);
        m_length++;
        m_rpsarry[m_length] = assignment.GetRawStart();
        m_length++;
        m_rpsarry[m_length] = (uint8_t) (assignment.GetRawGroup()); //(7-0)
        m_length++;
        m_rpsarry[m_length] = (uint8_t) (assignment.GetRawGroup() >> 8); //(15-8)
        m_length++;
        m_rpsarry[m_length] = (uint8_t) (assignment.GetRawGroup() >> 16); //(23-16)
        m_length++;
        m_rpsarry[m_length] = (uint8_t) assignment.GetChannelInd();
        m_length++;
        m_rpsarry[m_length] = (uint8_t) (assignment.GetChannelInd() >> 16);
        m_length++;
        m_rpsarry[m_length] = (uint8_t) assignment.GetPRAW();
        m_length++;
        m_rpsarry[m_length] = (uint8_t) (assignment.GetPRAW() >> 8);
        m_length++;
        m_rpsarry[m_length] = (uint8_t) (assignment.GetPRAW() >> 16);
        m_length++;
        m_rps = m_rpsarry;

    }

    uint8_t *
    RPS::GetRawAssignment(void) const {
        return m_rps;
    }

    RPS::RawAssignment
    RPS::GetRawAssigmentObj() const {
        auto m_raw = this->GetRawAssignment();

        RPS::RawAssignment ass;
        ass.SetRawControl(m_raw[0]);

        uint16_t slot =(uint16_t(m_raw[2]) << 8) | (uint16_t(m_raw[1]));
        ass.SetSlot(slot);

        ass.SetRawStart(m_raw[3]);
        ass.SetRawGroup((uint32_t(m_raw[6]) << 16) + (uint32_t(m_raw[5]) << 8) + uint32_t(m_raw[4]));
        ass.SetChannelInd((uint16_t(m_raw[8]) << 16) + uint16_t(m_raw[7]));
        ass.SetPRAW((uint32_t(m_raw[11]) << 16) + (uint32_t(m_raw[10]) << 8) + uint32_t(m_raw[9]));
        
        return ass;
    }

    WifiInformationElementId
    RPS::ElementId() const {
        return IE_RPS;
    }

    uint8_t
    RPS::GetInformationFieldSize() const {
        return m_length;
    }

    void
    RPS::SerializeInformationField(Buffer::Iterator start) const {
        start.Write(m_rps, m_length);
    }

    uint8_t
    RPS::DeserializeInformationField(Buffer::Iterator start, uint8_t length) {
        static uint8_t m_rpsarry[12];
        start.Read(m_rpsarry, length);
        m_rps = m_rpsarry;
        m_length = length;
        return length;
    }


    //ATTRIBUTE_HELPER_CPP (RPS);

} //namespace ns3





