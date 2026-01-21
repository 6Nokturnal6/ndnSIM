/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "ndn-consumer-cromo2Beacon.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

#include "utils/ndn-ns3-packet-tag.hpp"
#include "utils/ndn-rtt-mean-deviation.hpp"

#include <ndn-cxx/lp/tags.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>


#include <ns3/node-list.h>
#include <ns3/node.h>
#include "ns3/mobility-model.h"
#include "ns3/core-module.h"


#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>
#include "ndn-cxx/util/bloom-filter.hpp"

#include "ns3/ndnSIM/model/map-common.hpp"

// extern std::string tmp_dir;
int BloomFilter_size = 50000;
int BloomFilter_NumberOfHashFunctions = 20;
// bool invokeFromClientBeacon = false;

// std::vector<std::tuple<std::string, int>> NodeIdMacMap;
// std::string tmp_dir;

NS_LOG_COMPONENT_DEFINE("ndn.ConsumerCRoMo2Beacon");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ConsumerCRoMo2Beacon);

TypeId
ConsumerCRoMo2Beacon::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ConsumerCRoMo2Beacon")
      .SetGroupName("Ndn")
      .SetParent<Consumer>()
      .AddConstructor<ConsumerCRoMo2Beacon>()

      .AddAttribute("Frequency", "Frequency of interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&ConsumerCRoMo2Beacon::m_frequency), MakeDoubleChecker<double>())

      .AddAttribute("Randomize",
                    "Type of send time randomization: none (default), uniform, exponential",
                    StringValue("none"),
                    MakeStringAccessor(&ConsumerCRoMo2Beacon::SetRandomize, &ConsumerCRoMo2Beacon::GetRandomize),
                    MakeStringChecker())

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerCRoMo2Beacon::m_seqMax), MakeIntegerChecker<uint32_t>())

    ;

  return tid;
}

ConsumerCRoMo2Beacon::ConsumerCRoMo2Beacon()
  : m_frequency(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();
  // std::cout << "Constructor ConsumerCRoMo2Beacon: " << "tmp_dir: " << tmp_dir << " nodeNum:" << nodeNum << std::endl;
}

ConsumerCRoMo2Beacon::~ConsumerCRoMo2Beacon()
{
}


// Application Methods
void
ConsumerCRoMo2Beacon::StartApplication() // Called at time specified by Start
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  // std::cout << " Did it start?\n";
  NS_LOG_INFO(" m_interestLifeTime " << m_interestLifeTime.GetMilliSeconds() << "ms\n");
  ScheduleNextPacket();
}


void
ConsumerCRoMo2Beacon::ScheduleNextPacket()
{
  // double mean = 8.0 * m_payloadSize / m_desiredRate.GetBitRate ();
  // std::cout << "next: " << Simulator::Now().ToDouble(Time::S) + mean << "s\n";
  // std::cout << "Chamada?\n";

  if (m_firstTime) {
    // m_sendEvent = Simulator::Schedule(Seconds(0.0), &ConsumerCRoMo2Beacon::SendPacket2, this);
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &ConsumerCRoMo2Beacon::SendBeacon, this);
    m_firstTime = true;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(1.0 / m_frequency)
                                                      : Seconds(m_random->GetValue()),
                                      &ConsumerCRoMo2Beacon::SendBeacon, this);
}

void
ConsumerCRoMo2Beacon::SetRandomize(const std::string& value)
{
  if (value == "uniform") {
    m_random = CreateObject<UniformRandomVariable>();
    m_random->SetAttribute("Min", DoubleValue(0.0));
    m_random->SetAttribute("Max", DoubleValue(2 * 1.0 / m_frequency));
  }
  else if (value == "exponential") {
    m_random = CreateObject<ExponentialRandomVariable>();
    m_random->SetAttribute("Mean", DoubleValue(1.0 / m_frequency));
    m_random->SetAttribute("Bound", DoubleValue(50 * 1.0 / m_frequency));
  }
  else
    m_random = 0;

  m_randomType = value;
}

/******************** Elidio - For Beacon ***********************/
void
ConsumerCRoMo2Beacon::SendBeacon()
{
  if (!m_active)
    return;

  static int invokationsBeacon = 0;
  // std::cout << "NodeId: " << GetNode()->GetId() << " [SendBeacon]: #" << invokationsBeacon++ << std::endl;

  // std::cout << "invokeConsumerBeacon = " << invokeConsumerBeacon << std::endl;
  if (1){//!invokeConsumerBeacon                                                        <<<======================= Important!
    invokeConsumerBeacon = true;
    // ScheduleNextPacket();
    return; // first time the method is called from the scenario, it does not send an Interest
  }

  // invokeConsumerCromon = false;//

  // NS_LOG_INFO("> Sending....");
  NS_LOG_FUNCTION_NOARGS();

  uint64_t retxTimeout = 5;
  SetRetxTimer(Seconds(retxTimeout));
  // std::cout << "getRetxTimer = " << GetRetxTimer() << std::endl;

  // auto node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto node = GetNode();
  uint64_t nodeId = node->GetId();

  uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

  while (m_retxSeqs.size()) {
    seq = *m_retxSeqs.begin();
    m_retxSeqs.erase(m_retxSeqs.begin());
    break;
  }

  if (seq == std::numeric_limits<uint32_t>::max()) {
    if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
      if (m_seq >= m_seqMax) {
        return; // we are totally done
      }
    }

    seq = m_seq++;
  }

  // NS_LOG_INFO("GetRetxTimer: " << GetRetxTimer());
  //
  // ndn::Name contentName(m_interestName.toUri() + std::to_string(nodeId));

  shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
  // nameWithSequence->append(std::to_string(nodeId));
  nameWithSequence->appendSequenceNumber(seq);

  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  interest->setInterestLifetime(ndn::time::seconds(8));


  // interest->setTag(make_shared<lp::NonDiscoveryTag>(0));
  // auto NonD = interest->getTag<lp::NonDiscoveryTag>();

//   std::cout << "ndn.ndn-consumer (nodeId): " << nodeId << std::endl;

  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&nodeId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<1>(NodeIdMacMap) == (int) nodeId;});
  if (it != NodeIdMacMap.end()) {
    interest->setOrigID(std::get<0>(*it));
  }
  else
    interest->setOrigID("00:00:00:FF:FF:FF");

  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO("> Interest for " << seq);

  WillSendOutInterest(seq);

  ns3::Ptr<ns3::MobilityModel> mobModel = node->GetObject<ns3::MobilityModel> ();

  auto speed = mobModel->GetVelocity ();
  interest->setTag<lp::SpeedTag>(make_shared<lp::SpeedTag>(std::make_tuple(speed.x, speed.y, speed.z)));

  auto position = mobModel->GetPosition ();
  interest->setTag<lp::GeoTag>(make_shared<lp::GeoTag>(std::make_tuple(position.x, position.y, position.z)));

  uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();
  interest->setTag<lp::timestampTag>(make_shared<lp::timestampTag>(timestamp));
  interest->setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(nodeId));
  interest->setTag<lp::destIdTag>(make_shared<lp::destIdTag>(666));


  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  // static uint64_t NRetransmissions = 1;
  // if(NRetransmissions % 2 != 0){
  //   invokeConsumerBeacon = true;
  //   ScheduleNextPacket();
  //   std::cout << "NRetransmissions++: " << NRetransmissions++ << std::endl;
  // }

  // static uint64_t NRetransmissions = 0;
  // if (NRetransmissions <= 5) {
  //   ScheduleNextPacket();
  //   NRetransmissions++;
  // }
  // std::cout << "Does it reach here?\n";
  // ScheduleNextPacket();

}









void
ConsumerCRoMo2Beacon::OnData(shared_ptr<const Data> data)
{
  if (!m_active)
    return;

  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  uint64_t nodeId = node->GetId();

  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);

  // NS_LOG_INFO ("Received content object: " << boost::cref(*data));

  // This could be a problem......
  uint64_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_INFO("< DATA for " << seq);

    /* Elidio */ // SET on Base class
  /***************************************************************************************/
//   auto node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   uint32_t nodeId = GetNode()->GetId();
//
//   std::cout << "ndn.ndn-consumer (nodeId): " << nodeId << std::endl;
//
//   auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
//                            [&nodeId](const std::tuple<std::string, int>& NodeIdMacMap)
//                            {return std::get<1>(NodeIdMacMap) == (int) nodeId;});
//   if (it != NodeIdMacMap.end()) {
//         std::cout << "Found at address: " << std::get<0>(*it) << std::endl;
//   }
//
//   data->setOrigID(std::get<0>(*it));




  int hopCount = 0;
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  NS_LOG_DEBUG("Hop count: " << hopCount);

  SeqTimeoutsContainer::iterator entry = m_seqLastDelay.find(seq);
  if (entry != m_seqLastDelay.end()) {
    m_lastRetransmittedInterestDataDelay(this, seq, Simulator::Now() - entry->time, hopCount);
  }

  entry = m_seqFullDelay.find(seq);
  if (entry != m_seqFullDelay.end()) {
    m_firstInterestDataDelay(this, seq, Simulator::Now() - entry->time, m_seqRetxCounts[seq], hopCount);
  }

  m_seqRetxCounts.erase(seq);
  m_seqFullDelay.erase(seq);
  m_seqLastDelay.erase(seq);

  m_seqTimeouts.erase(seq);
  m_retxSeqs.erase(seq);

  m_rtt->AckSeq(SequenceNumber32(seq));

  /*Elidio*/

  std::string file = tmp_dir + "ndn-consumer-cromo2Beacon_Node" + std::to_string(nodeId) + ".txt";

//   std::ios_base::openmode mode;
  // static int forAppend = 1;

  std::ofstream out;
  // if (forAppend == 0){
  //   out.open(file, std::fstream::trunc);
  //   forAppend++;
  // }
  // else
  // out.open(file, std::fstream::app);


  out.open(file, std::fstream::app);
  auto *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(out.rdbuf());


  auto payload = data->getContent();
//   Block::element_const_iterator ptr;

  if (payload.size() < 3){
    std::cout << "Received Non beacon payload content with size: " << payload.size() << std::endl;// REACTIVATE
    // No need to show it
    // for(auto ptr = payload.begin(); ptr < payload.end(); ptr++){
    //   std::cout << *ptr;// << std::endl;
    //   NS_LOG_INFO(*ptr);
    // }
    //   NS_LOG_INFO("\n");
  }
  else{

    uint64_t destId;
    auto DestId = data->getTag<lp::destIdTag>(); // Will be extracted from LSTM
    if(DestId != nullptr){
      destId = DestId->get();
      // if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted DestId: " << dest << std::endl;
    }
    else
      std::cout << "\tDestId NOT set!" << std::endl;

    uint64_t fromId;
    auto FromId = data->getTag<lp::fromIdTag>();
    if(FromId != nullptr){
      fromId = FromId->get();
      // if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted FromId: " << fromId << std::endl;
    }
    else{
      std::cout << "\tDestId NOT set!" << std::endl;
    }

    uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();
    std::cout << timestamp << std::endl;

    // int BloomFilter_size = 3000;
    // int BloomFilter_NumberOfHashFunctions = 50;
    ::ndn::util::BloomFilter bf(BloomFilter_NumberOfHashFunctions, BloomFilter_size);
    ::ndn::util::BloomFilter bf2 = bf.decodeFromBinaryBlock(data->getContent(), BloomFilter_NumberOfHashFunctions, BloomFilter_size);
    NS_LOG_INFO("Beacon received!!\n");
    NS_LOG_INFO("Received payload content with size: " << payload.size());
    NS_LOG_INFO("fromId: " << fromId << " destId: " << destId);
    std::cout << "Received payload content with size: " << payload.size() << std::endl;
    std::cout << "fromId: " << fromId << " destId: " << destId << std::endl;
    // bf2.print();
    bf2.printHex();
  }

  /** reset cout buffer **/
  std::cout.rdbuf(coutbuf);
}

void
ConsumerCRoMo2Beacon::OnTimeout(uint32_t sequenceNumber)
{
  NS_LOG_FUNCTION(sequenceNumber);
  // std::cout << Simulator::Now () << ", TO: " << sequenceNumber << ", current RTO: " <<
  // m_rtt->RetransmitTimeout ().ToDouble (Time::S) << "s\n";

  // To comment
  // NS_LOG_INFO(Simulator::Now () << ", TO: " << sequenceNumber << ", current RTO: " << m_rtt->RetransmitTimeout ().ToDouble (Time::S) << "s\n");

  // m_rtt->IncreaseMultiplier(); // Double the next RTO //From ndnSIM/util/ndn-rtt-estimator.cpp - ndn-cxx/ndn-cxx/util/rtt-estimator.cpp
  m_rtt->SentSeq(SequenceNumber32(sequenceNumber),
                 1); // make sure to disable RTT calculation for this sample
  m_retxSeqs.insert(sequenceNumber);
  ScheduleNextPacket();
}


void
ConsumerCRoMo2Beacon::WillSendOutInterest(uint32_t sequenceNumber)
{
  NS_LOG_DEBUG("Trying to add " << sequenceNumber << " with " << Simulator::Now() << ". already "
                                << m_seqTimeouts.size() << " items");

  m_seqTimeouts.insert(SeqTimeout(sequenceNumber, Simulator::Now()));
  m_seqFullDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqLastDelay.erase(sequenceNumber);
  m_seqLastDelay.insert(SeqTimeout(sequenceNumber, Simulator::Now()));

  m_seqRetxCounts[sequenceNumber]++;

  m_rtt->SentSeq(SequenceNumber32(sequenceNumber), 1);
}



std::string
ConsumerCRoMo2Beacon::GetRandomize() const
{
  return m_randomType;
}

} // namespace ndn
} // namespace ns3
