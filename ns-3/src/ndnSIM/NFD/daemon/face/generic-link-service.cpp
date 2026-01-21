/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "generic-link-service.hpp"

#include <ndn-cxx/lp/pit-token.hpp>
#include <ndn-cxx/lp/tags.hpp>

#include <cmath>


// Elidio
#include "ns3/wifi-net-device.h"

#include "ns3/node-list.h"
#include "ns3/node.h"


#include "ns3/mobility-model.h"
#include "ns3/core-module.h"

#include "ns3/ndnSIM/model/map-common.hpp"

bool printToFileGenericLink = false;

namespace nfd {
namespace face {

NFD_LOG_INIT(GenericLinkService);

constexpr size_t CONGESTION_MARK_SIZE = tlv::sizeOfVarNumber(lp::tlv::CongestionMark) + // type
                                        tlv::sizeOfVarNumber(sizeof(uint64_t)) +        // length
                                        tlv::sizeOfNonNegativeInteger(UINT64_MAX);      // value

GenericLinkService::GenericLinkService(const GenericLinkService::Options& options)
  : m_options(options)
  , m_fragmenter(m_options.fragmenterOptions, this)
  , m_reassembler(m_options.reassemblerOptions, this)
  , m_reliability(m_options.reliabilityOptions, this)
  , m_lastSeqNo(-2)
  , m_nextMarkTime(time::steady_clock::TimePoint::max())
  , m_nMarkedSinceInMarkingState(0)
{
  m_reassembler.beforeTimeout.connect([this] (auto...) { ++this->nReassemblyTimeouts; });
  m_reliability.onDroppedInterest.connect([this] (const auto& i) { this->notifyDroppedInterest(i); });
  nReassembling.observe(&m_reassembler);
}

void
GenericLinkService::setOptions(const GenericLinkService::Options& options)
{
  m_options = options;
  m_fragmenter.setOptions(m_options.fragmenterOptions);
  m_reassembler.setOptions(m_options.reassemblerOptions);
  m_reliability.setOptions(m_options.reliabilityOptions);
}

void
GenericLinkService::requestIdlePacket(const EndpointId& endpointId)
{
  // No need to request Acks to attach to this packet from LpReliability, as they are already
  // attached in sendLpPacket
  this->sendLpPacket({}, endpointId);
}

void
GenericLinkService::sendLpPacket(lp::Packet&& pkt, const EndpointId& endpointId)
{
  const ssize_t mtu = this->getTransport()->getMtu();

  if (m_options.reliabilityOptions.isEnabled) {
    m_reliability.piggyback(pkt, mtu);
  }

  if (m_options.allowCongestionMarking) {
    checkCongestionLevel(pkt);
  }

  auto block = pkt.wireEncode();
  if (mtu != MTU_UNLIMITED && block.size() > static_cast<size_t>(mtu)) {
    ++this->nOutOverMtu;
    NFD_LOG_FACE_WARN("attempted to send packet over MTU limit");
    return;
  }
  this->sendPacket(block, endpointId);
}

void
GenericLinkService::doSendInterest(const Interest& interest, const EndpointId& endpointId)
{
  // if(endpointId == 666){
  //   std::cout << "[GenericLinkService::doSendInterest] endpointId: " << endpointId << std::endl;
  //   interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(666));
  // }

//   auto destId = interest.getTag<lp::destIdTag>(); // Will be extracted from LSTM
//   auto dest = 666;
//   if(destId != nullptr){
//     // dest = destId->get();
//     std::cout << "\t[GenericLinkService::doSendInterest]. Extracted DestId: " << dest << " - endpoint: " << endpointId << std::endl;
//   }
//
//   uint32_t fromId;
//   auto FromId = interest.getTag<lp::fromIdTag>();
//   if(FromId != nullptr){
//     // fromId = FromId->get();
//     std::cout << "\t[GenericLinkService::doSendInterest]. Extracted FromId: " << fromId << std::endl;
//   }
//   else{
//     fromId = 666;// TODO
//     std::cout << "[GenericLinkService::doSendInterest]... Interest. FromId nullptr. SET to 666" << std::endl;
//   }


  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // auto nodeId = currentNode->GetId ();

  // std::string Ffile = tmp_dir + "GenericLinkService_doSendInterest_" + std::to_string(nodeId) + ".txt";
  // std::ofstream out;
  // out.open(Ffile, std::fstream::app);
  //
  // auto *coutbuf = std::cout.rdbuf();
  // std::cout.rdbuf(out.rdbuf());
  //
  // std::cout << ns3::Simulator::Now().GetMilliSeconds();// << std::endl;
  // // std::cout << "endpointId : " << endpointId << std::endl;
  // std::cout << "interest: " << interest.getName().getSubName(0,4).toUri() << std::endl;



  // auto destId = interest.getTag<lp::destIdTag>(); // Will be extracted from LSTM
  // auto dest = 6662;
  // if(destId != nullptr){
  //   dest = destId->get();
  //   std::cout << "\t[GenericLinkService::doSendInterest]. Extracted DestId: " << dest << " - endpoint: " << endpointId << std::endl;
  // }

  // uint32_t fromId;
  // auto FromId = interest.getTag<lp::fromIdTag>();
  // if(FromId != nullptr){
  //   fromId = FromId->get();
  //   std::cout << "\t[GenericLinkService::doSendInterest]. Extracted FromId: " << fromId << std::endl;
  // }
  // else{
  //   fromId = 666;// TODO
  //   std::cout << "[GenericLinkService::doSendInterest]... Interest. FromId nullptr. SET to 666" << std::endl;
  // }
  //
  //
  // /** reset cout buffer **/
  // std::cout.rdbuf(coutbuf);

  lp::Packet lpPacket(interest.wireEncode());

  encodeLpFields(interest, lpPacket);

  this->sendNetPacket(std::move(lpPacket), endpointId, true);
}

void
GenericLinkService::doSendData(const Data& data, const EndpointId& endpointId)
{
  // if(endpointId == 666){
  //   std::cout << "[GenericLinkService::doSendData] endpointId: " << endpointId << std::endl;
  //   data.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(666));
  // }


  // auto destId = data.getTag<lp::destIdTag>(); // Will be extracted from LSTM
  // auto dest = 666;
  // if(destId != nullptr){
  //   dest = destId->get();
  //   std::cout << "\t[GenericLinkService::doSendDatat]. Extracted DestId: " << dest << " - endpoint: " << endpointId << std::endl;
  // }
  //
  // uint32_t fromId;
  // auto FromId = data.getTag<lp::fromIdTag>();
  // if(FromId != nullptr){
  //   fromId = FromId->get();
  //   std::cout << "\t[GenericLinkService::doSendData]. Extracted FromId: " << fromId << std::endl;
  // }
  // else{
  //   fromId = 666;// TODO
  //   std::cout << "[GenericLinkService::doSendData]... Interest. FromId nullptr. SET to 666" << std::endl;
  // }

  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // auto nodeId = currentNode->GetId ();
  //
  // std::string Ffile = tmp_dir + "GenericLinkService_doSendData_" + std::to_string(nodeId) + ".txt";
  // std::ofstream out;
  // out.open(Ffile, std::fstream::app);
  //
  // auto *coutbuf = std::cout.rdbuf();
  // std::cout.rdbuf(out.rdbuf());
  //
  // std::cout << ns3::Simulator::Now().GetMilliSeconds();// << std::endl;
  // // std::cout << "endpointId : " << endpointId << std::endl;
  // std::cout << "CN: " << data.getName().getSubName(0,4).toUri() << std::endl;




  // auto destId = data.getTag<lp::destIdTag>(); // Will be extracted from LSTM
  // auto dest = 6662;
  // if(destId != nullptr){
  //   dest = destId->get();
  //   std::cout << "\t[GenericLinkService::doSendDatat]. Extracted DestId: " << dest << " - endpoint: " << endpointId << std::endl;
  // }
  //
  // uint32_t fromId;
  // auto FromId = data.getTag<lp::fromIdTag>();
  // if(FromId != nullptr){
  //   fromId = FromId->get();
  //   std::cout << "\t[GenericLinkService::doSendData]. Extracted FromId: " << fromId << std::endl;
  // }
  // else{
  //   // fromId = 666;// TODO
  //   std::cout << "[GenericLinkService::doSendData]... Interest. FromId nullptr. SET to 666" << std::endl;
  // }
  // /** reset cout buffer **/
  // std::cout.rdbuf(coutbuf);



  lp::Packet lpPacket(data.wireEncode());

  encodeLpFields(data, lpPacket);

  this->sendNetPacket(std::move(lpPacket), endpointId, false);
}

void
GenericLinkService::doSendNack(const lp::Nack& nack, const EndpointId& endpointId)
{
  lp::Packet lpPacket(nack.getInterest().wireEncode());
  lpPacket.add<lp::NackField>(nack.getHeader());

  encodeLpFields(nack, lpPacket);

  this->sendNetPacket(std::move(lpPacket), endpointId, false);
}

void
GenericLinkService::encodeLpFields(const ndn::PacketBase& netPkt, lp::Packet& lpPacket)
{
  if (m_options.allowLocalFields) {
    auto incomingFaceIdTag = netPkt.getTag<lp::IncomingFaceIdTag>();
    if (incomingFaceIdTag != nullptr) {
      lpPacket.add<lp::IncomingFaceIdField>(*incomingFaceIdTag);
    }
  }

  auto congestionMarkTag = netPkt.getTag<lp::CongestionMarkTag>();
  if (congestionMarkTag != nullptr) {
    lpPacket.add<lp::CongestionMarkField>(*congestionMarkTag);
  }

  if (m_options.allowSelfLearning) {
    auto nonDiscoveryTag = netPkt.getTag<lp::NonDiscoveryTag>();
    if (nonDiscoveryTag != nullptr) {
      lpPacket.add<lp::NonDiscoveryField>(*nonDiscoveryTag);
  }

    auto prefixAnnouncementTag = netPkt.getTag<lp::PrefixAnnouncementTag>();
    if (prefixAnnouncementTag != nullptr) {
      lpPacket.add<lp::PrefixAnnouncementField>(*prefixAnnouncementTag);
    }
  }

  auto pitToken = netPkt.getTag<lp::PitToken>();
  if (pitToken != nullptr) {
    lpPacket.add<lp::PitTokenField>(*pitToken);
  }

  shared_ptr<lp::HopCountTag> hopCountTag = netPkt.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) {
    lpPacket.add<lp::HopCountTagField>(*hopCountTag);
  }
  else {
    lpPacket.add<lp::HopCountTagField>(0);
  }

  // Every Packet get the following fields updated
  /* Elidio GeoTag*/
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());

  ns3::Ptr<ns3::MobilityModel> mobModel = node->GetObject<ns3::MobilityModel> ();
  auto position = mobModel->GetPosition ();
  lpPacket.add<lp::GeoTagField>(lp::GeoTag(std::make_tuple(position.x, position.y, position.z)));


  /*
   *SpeedTag
   */
  auto speed = mobModel->GetVelocity ();
  lpPacket.add<lp::SpeedTagField>(lp::SpeedTag(std::make_tuple(speed.x, speed.y, speed.z)));



  /*
   * destIdTag
   */
  shared_ptr<lp::destIdTag> DestIdTag = netPkt.getTag<lp::destIdTag>();
  if (DestIdTag != nullptr) {
	lpPacket.add<lp::destIdField>(*DestIdTag);
  }
  else {
    // std::cout << "[GenericLinkService::encodeLpFields]. destIdTag NOT set. Default to RSU, set for testing." << std::endl;
    // NFD_LOG_FACE_DEBUG("[GenericLinkService::encodeLpFields]. destIdTag NOT set. Default 66, set for testing.");
    lpPacket.add<lp::destIdField>(nodeNum);//
  }


  /*
   * fromIdTag
   */
  auto nodeId = node->GetId();
  // lpPacket.add<lp::fromIdField>(fromIdField);

  shared_ptr<lp::fromIdTag> FromIdTag = netPkt.getTag<lp::fromIdTag>();
  if (FromIdTag != nullptr) {
	lpPacket.add<lp::fromIdField>(*FromIdTag);
    // std::cout << "Setting fromIdTag... " << std::endl;
  }
  else {
    // std::cout << "[GenericLinkService::encodeLpFields]. fromIdField NOT set. Default 66, set for testing." << std::endl;
    // NFD_LOG_FACE_DEBUG("[GenericLinkService::encodeLpFields]. fromIdField NOT set. Default 66, set for testing.");
    lpPacket.add<lp::fromIdField>(nodeId);//
  }



  /*
   * timestampTag
   */
  uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();
  lpPacket.add<lp::timestampField>(timestamp);//
}




void
GenericLinkService::sendNetPacket(lp::Packet&& pkt, const EndpointId& endpointId, bool isInterest)
{
  std::vector<lp::Packet> frags;
  ssize_t mtu = this->getTransport()->getMtu();

  // Make space for feature fields in fragments
  if (m_options.reliabilityOptions.isEnabled && mtu != MTU_UNLIMITED) {
    mtu -= LpReliability::RESERVED_HEADER_SPACE;
  }

  if (m_options.allowCongestionMarking && mtu != MTU_UNLIMITED) {
    mtu -= CONGESTION_MARK_SIZE;
  }

  BOOST_ASSERT(mtu == MTU_UNLIMITED || mtu > 0);

  if (m_options.allowFragmentation && mtu != MTU_UNLIMITED) {
    bool isOk = false;
    std::tie(isOk, frags) = m_fragmenter.fragmentPacket(pkt, mtu);
    if (!isOk) {
      // fragmentation failed (warning is logged by LpFragmenter)
      ++this->nFragmentationErrors;
      return;
    }
  }
  else {
    if (m_options.reliabilityOptions.isEnabled) {
      frags.push_back(pkt);
    }
    else {
      frags.push_back(std::move(pkt));
    }
  }

  if (frags.size() == 1) {
    // even if indexed fragmentation is enabled, the fragmenter should not
    // fragment the packet if it can fit in MTU
    BOOST_ASSERT(!frags.front().has<lp::FragIndexField>());
    BOOST_ASSERT(!frags.front().has<lp::FragCountField>());
  }

  // Only assign sequences to fragments if packet contains more than 1 fragment
  if (frags.size() > 1) {
    // Assign sequences to all fragments
    this->assignSequences(frags);
  }

  if (m_options.reliabilityOptions.isEnabled && frags.front().has<lp::FragmentField>()) {
    m_reliability.handleOutgoing(frags, std::move(pkt), isInterest);
  }

  for (lp::Packet& frag : frags) {
    this->sendLpPacket(std::move(frag), endpointId);
  }
}

void
GenericLinkService::assignSequence(lp::Packet& pkt)
{
  pkt.set<lp::SequenceField>(++m_lastSeqNo);
}

void
GenericLinkService::assignSequences(std::vector<lp::Packet>& pkts)
{
  std::for_each(pkts.begin(), pkts.end(), [this] (auto& pkt) { this->assignSequence(pkt); });
}

void
GenericLinkService::checkCongestionLevel(lp::Packet& pkt)
{
  ssize_t sendQueueLength = getTransport()->getSendQueueLength();
  // The transport must support retrieving the current send queue length
  if (sendQueueLength < 0) {
    return;
  }

  if (sendQueueLength > 0) {
    NFD_LOG_FACE_TRACE("txqlen=" << sendQueueLength << " threshold=" <<
                       m_options.defaultCongestionThreshold << " capacity=" <<
                       getTransport()->getSendQueueCapacity());
  }

  // sendQueue is above target
  if (static_cast<size_t>(sendQueueLength) > m_options.defaultCongestionThreshold) {
    const auto now = time::steady_clock::now();

    if (m_nextMarkTime == time::steady_clock::TimePoint::max()) {
      m_nextMarkTime = now + m_options.baseCongestionMarkingInterval;
    }
    // Mark packet if sendQueue stays above target for one interval
    else if (now >= m_nextMarkTime) {
      pkt.set<lp::CongestionMarkField>(1);
      ++nCongestionMarked;
      NFD_LOG_FACE_DEBUG("LpPacket was marked as congested");

      ++m_nMarkedSinceInMarkingState;
      // Decrease the marking interval by the inverse of the square root of the number of packets
      // marked in this incident of congestion
      time::nanoseconds interval(static_cast<time::nanoseconds::rep>(
                                   m_options.baseCongestionMarkingInterval.count() /
                                   std::sqrt(m_nMarkedSinceInMarkingState + 1)));
      m_nextMarkTime += interval;
    }
  }
  else if (m_nextMarkTime != time::steady_clock::TimePoint::max()) {
    // Congestion incident has ended, so reset
    NFD_LOG_FACE_DEBUG("Send queue length dropped below congestion threshold");
    m_nextMarkTime = time::steady_clock::TimePoint::max();
    m_nMarkedSinceInMarkingState = 0;
  }
}

void
GenericLinkService::doReceivePacket(const Block& packet, const EndpointId& endpoint)
{
  try {
    lp::Packet pkt(packet);

    if (m_options.reliabilityOptions.isEnabled) {
      m_reliability.processIncomingPacket(pkt);
    }

    if (!pkt.has<lp::FragmentField>()) {
      NFD_LOG_FACE_TRACE("received IDLE packet: DROP");
      return;
    }

    if ((pkt.has<lp::FragIndexField>() || pkt.has<lp::FragCountField>()) &&
        !m_options.allowReassembly) {
      NFD_LOG_FACE_WARN("received fragment, but reassembly disabled: DROP");
      return;
    }

    bool isReassembled = false;
    Block netPkt;
    lp::Packet firstPkt;
    std::tie(isReassembled, netPkt, firstPkt) = m_reassembler.receiveFragment(endpoint, pkt);
    if (isReassembled) {
      this->decodeNetPacket(netPkt, firstPkt, endpoint);
    }
  }
  catch (const tlv::Error& e) {
    ++this->nInLpInvalid;
    NFD_LOG_FACE_WARN("packet parse error (" << e.what() << "): DROP");
  }
}

void
GenericLinkService::decodeNetPacket(const Block& netPkt, const lp::Packet& firstPkt,
									const EndpointId& endpointId)
{
  try {
    switch (netPkt.type()) {
      case tlv::Interest:
        if (firstPkt.has<lp::NackField>()) {
          this->decodeNack(netPkt, firstPkt, endpointId);
        }
        else {
          this->decodeInterest(netPkt, firstPkt, endpointId);
// 		  std::cout << "Decoding Interest from endpointId = " << endpointId << std::endl;
        }
        break;
      case tlv::Data:
        this->decodeData(netPkt, firstPkt, endpointId);
// 		std::cout << "Decoding Data from endpointId = " << endpointId << std::endl;
        break;
      default:
        ++this->nInNetInvalid;
        NFD_LOG_FACE_WARN("unrecognized network-layer packet TLV-TYPE " << netPkt.type() << ": DROP");
        return;
    }
  }
  catch (const tlv::Error& e) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("packet parse error (" << e.what() << "): DROP");
  }
}

void
GenericLinkService::decodeInterest(const Block& netPkt, const lp::Packet& firstPkt,
								   const EndpointId& endpointId)
{
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());

  BOOST_ASSERT(netPkt.type() == tlv::Interest);
  BOOST_ASSERT(!firstPkt.has<lp::NackField>());
// std::cout << "fromIdField: " << from << std::endl;
  // forwarding expects Interest to be created with make_shared
  auto interest = make_shared<Interest>(netPkt);

  // Increment HopCount
  if (firstPkt.has<lp::HopCountTagField>()) {
    interest->setTag(make_shared<lp::HopCountTag>(firstPkt.get<lp::HopCountTagField>() + 1));
// 	std::cout << "HopCount " << firstPkt.get<lp::HopCountTagField>() << std::endl;
  }

  if (firstPkt.has<lp::GeoTagField>()) {// Elidio: m_options.enableGeoTags && firstPkt...
    interest->setTag(make_shared<lp::GeoTag>(firstPkt.get<lp::GeoTagField>()));
  }
  else{
    if (interest->getName().getSubName(0,1) != "/localhost"){// if(printToFileGenericLink)
      // std::cout << "[GenericLinkService::decodeInterest]. GeoTag NOT set. Node: " << node->GetId() << " - Interest name: " << interest->getName() << std::endl;
      // NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeInterest]. GeoTag NOT set");
    }
  }

  if (firstPkt.has<lp::NextHopFaceIdField>()) {
    if (m_options.allowLocalFields) {
      interest->setTag(make_shared<lp::NextHopFaceIdTag>(firstPkt.get<lp::NextHopFaceIdField>()));
    }
    else {
      NFD_LOG_FACE_WARN("received NextHopFaceId, but local fields disabled: DROP");
      return;
    }
  }

  if (firstPkt.has<lp::CachePolicyField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received CachePolicy with Interest: DROP");
    return;
  }

  if (firstPkt.has<lp::IncomingFaceIdField>()) {
    NFD_LOG_FACE_WARN("received IncomingFaceId: IGNORE");
  }

  if (firstPkt.has<lp::CongestionMarkField>()) {
    interest->setTag(make_shared<lp::CongestionMarkTag>(firstPkt.get<lp::CongestionMarkField>()));
  }

  if (firstPkt.has<lp::NonDiscoveryField>()) {
    if (m_options.allowSelfLearning) {
      interest->setTag(make_shared<lp::NonDiscoveryTag>(firstPkt.get<lp::NonDiscoveryField>()));
       // std::cout << "[GenericLinkService::decodeInterest]. received NonDiscovery, but self-learning enabled" << std::endl;
      NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeInterest]. received NonDiscovery, but self-learning enabled");
    }
    else {
      NFD_LOG_FACE_WARN("received NonDiscovery, but self-learning disabled: IGNORE");
      // std::cout << "[GenericLinkService::decodeInterest]. received NonDiscovery, but self-learning disabled" << std::endl;
    }
  }

  if (firstPkt.has<lp::PrefixAnnouncementField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received PrefixAnnouncement with Interest: DROP");
    // std::cout << "[GenericLinkService::decodeInterest]. received PrefixAnnouncement with Interest: DROP" << std::endl;
    // return;
  }

  if (firstPkt.has<lp::PitTokenField>()) {
    interest->setTag(make_shared<lp::PitToken>(firstPkt.get<lp::PitTokenField>()));
  }

  /***************** Elidio ************/

  if (firstPkt.has<lp::destIdField>()) {
    interest->setTag(make_shared<lp::destIdTag>(firstPkt.get<lp::destIdField>()));//?
    // interest->setTag(make_shared<lp::destIdTag>(std::move(endpointId)));//?
  }
  else{
    if (interest->getName().getSubName(0,1) != "/localhost"){// if(printToFileGenericLink)
      // std::cout << "[GenericLinkService::decodeInterest]. destIdTag NOT set for: " << interest->getName().getSubName(0,5).toUri() << std::endl;
      // NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeInterest]. destIdTag NOT set");
    }
  }

  if (firstPkt.has<lp::SpeedTagField>()) {
    interest->setTag(make_shared<lp::SpeedTag>(firstPkt.get<lp::SpeedTagField>())); // added from encodeLpFields
  }
  else{
    if (interest->getName().getSubName(0,1) != "/localhost"){// if(printToFileGenericLink)
      // std::cout << "[GenericLinkService::decodeInterest]. SpeedTag NOT set. Node: " << node->GetId() << " - Interest name: " << interest->getName().getSubName(0,5).toUri() << std::endl;
      // NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeInterest]. SpeedTag NOT set");
    }
  }

  if (firstPkt.has<lp::timestampField>()) {
    interest->setTag(make_shared<lp::timestampTag>(firstPkt.get<lp::timestampField>())); // set from encondeLpFields
  }
  else{
   if (interest->getName().getSubName(0,1) != "/localhost"){ // if(printToFileGenericLink)
      // std::cout << "[GenericLinkService::decodeInterest]. timestampTag NOT set for: " << interest->getName().getSubName(0,5).toUri() << std::endl;
      // NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeInterest]. timestampTag NOT set");
    }
  }

  if (firstPkt.has<lp::fromIdField>()) {
    interest->setTag(make_shared<lp::fromIdTag>(firstPkt.get<lp::fromIdField>())); // firstPkt.get<lp::fromIdField>() --- set from producer/consumer
    // std::cout << "Interest set??: " << *interest->getTag<lp::fromIdTag>() << " - endpointId: " << endpointId << std::endl;
  }
  else{
    // std::cout << "[decodeInterest]. using endpointId..." << endpointId << std::endl;
    interest->setTag(make_shared<lp::fromIdTag>(endpointId));
  }

  // else{
  //     std::cout << "[GenericLinkService::decodeInterest]. fromIdTag NOT set for: " << interest->getName().getSubName(0,5).toUri() << std::endl;
  //     // NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeInterest]. fromIdTag NOT set");
  //   // }
  // }

  // std::cout << "[decodeInterest]. " << endpointId << " - firstPkt.get<lp::fromIdField>(): " << *interest->getTag<lp::fromIdTag>() << std::endl;
  /********************************************************************/

/*
  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto nodeId = currentNode->GetId ();

  std::string Ffile = tmp_dir + "GenericLinkService_decodeInterest_" + std::to_string(nodeId) + ".txt";
  std::ofstream out;
  out.open(Ffile, std::fstream::app);

  auto *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(out.rdbuf());

  std::cout << ns3::Simulator::Now().GetMilliSeconds();// << std::endl;
  // std::cout << "endpointId (to fromIdTag): " << endpointId << std::endl;
  std::cout << "CN: " << interest->getName().getSubName(0,4).toUri() << std::endl;


  auto destId = interest->getTag<lp::destIdTag>(); // Will be extracted from LSTM
  auto dest = 6662;
  if(destId != nullptr){
    dest = destId->get();
    std::cout << "\t[GenericLinkService::decodeInterest]. Extracted DestId: " << dest << " - endpoint: " << endpointId << std::endl;
  }

  uint32_t fromId;
  auto FromId = interest->getTag<lp::fromIdTag>();
  if(FromId != nullptr){
    fromId = FromId->get();
    std::cout << "\t[GenericLinkService::decodeInterest]. Extracted FromId: " << fromId << std::endl;
  }
  else{
    fromId = 666;// TODO
    std::cout << "[GenericLinkService::decodeInterest]... Interest. FromId nullptr. SET to 666" << std::endl;
  }

  // reset cout buffer
  std::cout.rdbuf(coutbuf);*/


  this->receiveInterest(*interest, 0/*FromNodeId*/); //endpointId... The endpointId is not being "forwarded" from ndn-net-device-transport... endpointId is used in mobility-aware-strategy
}

void
GenericLinkService::decodeData(const Block& netPkt, const lp::Packet& firstPkt,
							   const EndpointId& endpointId)
{
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());// Elidio
  BOOST_ASSERT(netPkt.type() == tlv::Data);

  // forwarding expects Data to be created with make_shared
  auto data = make_shared<Data>(netPkt);

  if (firstPkt.has<lp::HopCountTagField>()) {
    data->setTag(make_shared<lp::HopCountTag>(firstPkt.get<lp::HopCountTagField>() + 1));
  }

  if (firstPkt.has<lp::GeoTagField>()) { // Elidio. m_options.enableGeoTags && firstPkt...
    data->setTag(make_shared<lp::GeoTag>(firstPkt.get<lp::GeoTagField>()));

    // shared_ptr<lp::GeoTag> geoTag= data->getTag<lp::GeoTag>();
    // std::cout << "From Generic link service: " << std::get<0>(geoTag->getPos()) << "," <<  std::get<1>(geoTag->getPos()) << "," << std::get<2>(geoTag->getPos()) << std::endl;

  }
  else{
    if (data->getName().getSubName(0,1) != "/localhost"){// if(printToFileGenericLink)
      // std::cout << "[GenericLinkService::decodeData]. GeoTag NOT set. Node: " << node->GetId() << " - Data name: " << data->getName() << std::endl;
      NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeData]. NO geoTAG!");
    }
  }

  if (firstPkt.has<lp::NackField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received Nack with Data: DROP");
    return;
  }

  if (firstPkt.has<lp::NextHopFaceIdField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received NextHopFaceId with Data: DROP");
    return;
  }

  if (firstPkt.has<lp::CachePolicyField>()) {
    // CachePolicy is unprivileged and does not require allowLocalFields option.
    // In case of an invalid CachePolicyType, get<lp::CachePolicyField> will throw,
    // so it's unnecessary to check here.
    data->setTag(make_shared<lp::CachePolicyTag>(firstPkt.get<lp::CachePolicyField>()));
  }

  if (firstPkt.has<lp::IncomingFaceIdField>()) {
    NFD_LOG_FACE_WARN("received IncomingFaceId: IGNORE");
  }

  if (firstPkt.has<lp::CongestionMarkField>()) {
    data->setTag(make_shared<lp::CongestionMarkTag>(firstPkt.get<lp::CongestionMarkField>()));
  }

  if (firstPkt.has<lp::NonDiscoveryField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received NonDiscovery with Data: DROP");
    return;
  }

  if (firstPkt.has<lp::PrefixAnnouncementField>()) {
    if (m_options.allowSelfLearning) {
      data->setTag(make_shared<lp::PrefixAnnouncementTag>(firstPkt.get<lp::PrefixAnnouncementField>()));// Elidio: TODO .. see this
    }
    else {
      NFD_LOG_FACE_WARN("received PrefixAnnouncement, but self-learning disabled: IGNORE");
    }
  }
  /***************** Elidio ************/

  if (firstPkt.has<lp::timestampField>()) {
    data->setTag(make_shared<lp::timestampTag>(firstPkt.get<lp::timestampField>()));
  }
  else{
    if (data->getName().getSubName(0,1) != "/localhost"){
      // std::cout << "[GenericLinkService::decodeData]. timestampTag set. Node: " << node->GetId() << " - Data name: " << data->getName().getSubName(0.5).toUri() << std::endl;
      // NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeData]. timestampTag NOT set");
    }
  }

  if (firstPkt.has<lp::fromIdField>()) {
    data->setTag(make_shared<lp::fromIdTag>(firstPkt.get<lp::fromIdField>()));//firstPkt.get<lp::fromIdField>()

    // std::cout << "Data set??: " << *data->getTag<lp::fromIdTag>() << " - endpointId: " << endpointId << std::endl;
  }
  else{
     // std::cout << "[decodeData]. using endpointId..." << endpointId << std::endl;
     data->setTag(make_shared<lp::fromIdTag>(endpointId));
  }
  // else{
  //   std::cout << "[GenericLinkService::decodeData. fromIdTag NOT set for: " << data->getName().getSubName(0,5).toUri() << std::endl;
  //   // NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeData]. fromIdTag NOT set. Default from endpoint " << endpointId<< " set");
  // }
  // std::cout << "[decodeData]. " << endpointId << " - firstPkt.get<lp::fromIdField>(): " << *data->getTag<lp::fromIdTag>() << std::endl;
  if (firstPkt.has<lp::destIdField>()) {
    data->setTag(make_shared<lp::destIdTag>(firstPkt.get<lp::destIdField>()));//endpointId
  }
  else{
    if (data->getName().getSubName(0,1) != "/localhost"){
      // std::cout << "[GenericLinkService::decodeData]. destIdTag NOT set for: " << data->getName().getSubName(0,5).toUri() << std::endl;
      // NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeData]. destIdTag NOT set.");
    }
  }


  if (firstPkt.has<lp::SpeedTagField>()) {
    data->setTag(make_shared<lp::SpeedTag>(firstPkt.get<lp::SpeedTagField>()));
  }
  else{
    if (data->getName().getSubName(0,1) != "/localhost"){
      // std::cout << "[GenericLinkService::decodeData]. SpeedTag NOT set. Node: " << node->GetId() << " - Data name: " << data->getName().getSubName(0,5).toUri() << std::endl;
      NFD_LOG_FACE_DEBUG("[GenericLinkService::decodeData]. SpeedTag NOT set.");
    }
  }

  //--------------------
  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // auto nodeId = currentNode->GetId ();
  //
  // std::string Ffile = tmp_dir + "GenericLinkService_decodeData_" + std::to_string(nodeId) + ".txt";
  // std::ofstream out;
  // out.open(Ffile, std::fstream::app);
  //
  // auto *coutbuf = std::cout.rdbuf();
  // std::cout.rdbuf(out.rdbuf());
  //
  // std::cout << ns3::Simulator::Now().GetMilliSeconds();// << std::endl;
  // // std::cout << "endpointId (to fromIdTag): " << endpointId << std::endl;
  // std::cout << "CN: " << data->getName().getSubName(0,4).toUri() << std::endl;
  //
  //
  // auto destId = data->getTag<lp::destIdTag>(); // Will be extracted from LSTM
  // auto dest = 6662;
  // if(destId != nullptr){
  //   dest = destId->get();
  //   std::cout << "\t[GenericLinkService::decodeData]. Extracted DestId: " << dest << " - endpoint: " << endpointId << std::endl;
  // }
  //
  // uint32_t fromId;
  // auto FromId = data->getTag<lp::fromIdTag>();
  // if(FromId != nullptr){
  //   fromId = FromId->get();
  //   std::cout << "\t[GenericLinkService::decodeData]. Extracted FromId: " << fromId << std::endl;
  // }
  // else{
  //   fromId = 666;// TODO
  //   std::cout << "[GenericLinkService::decodeData]... Interest. FromId nullptr. SET to 666" << std::endl;
  // }
  //
  //
  // // reset cout buffer
  // std::cout.rdbuf(coutbuf);

  this->receiveData(*data, 0/*FromNodeId*/);//
}

void
GenericLinkService::decodeNack(const Block& netPkt, const lp::Packet& firstPkt,
							   const EndpointId& endpointId)
{
  BOOST_ASSERT(netPkt.type() == tlv::Interest);
  BOOST_ASSERT(firstPkt.has<lp::NackField>());

  lp::Nack nack((Interest(netPkt)));
  nack.setHeader(firstPkt.get<lp::NackField>());

  if (firstPkt.has<lp::NextHopFaceIdField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received NextHopFaceId with Nack: DROP");
    return;
  }

  if (firstPkt.has<lp::CachePolicyField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received CachePolicy with Nack: DROP");
    return;
  }

  if (firstPkt.has<lp::IncomingFaceIdField>()) {
    NFD_LOG_FACE_WARN("received IncomingFaceId: IGNORE");
  }

  if (firstPkt.has<lp::CongestionMarkField>()) {
    nack.setTag(make_shared<lp::CongestionMarkTag>(firstPkt.get<lp::CongestionMarkField>()));
  }

  if (firstPkt.has<lp::NonDiscoveryField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received NonDiscovery with Nack: DROP");
    return;
  }

  if (firstPkt.has<lp::PrefixAnnouncementField>()) {
    ++this->nInNetInvalid;
    NFD_LOG_FACE_WARN("received PrefixAnnouncement with Nack: DROP");
    return;
  }

  this->receiveNack(nack, 0);
}

} // namespace face
} // namespace nfd
