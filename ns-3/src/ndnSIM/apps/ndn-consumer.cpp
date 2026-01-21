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

#include "ndn-consumer.hpp"
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

/**** For getting Node ID ****/
// #include <ns3/simulator.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include "ns3/mobility-model.h"
#include "ns3/core-module.h"
// Elidio
#include "ns3/ndnSIM/model/map-common.hpp"

// extern std::vector<std::tuple<std::string, int>> NodeIdMacMap;

NS_LOG_COMPONENT_DEFINE("ndn.Consumer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(Consumer);

TypeId
Consumer::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::Consumer")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddAttribute("StartSeq", "Initial sequence number", IntegerValue(0),
                    MakeIntegerAccessor(&Consumer::m_seq), MakeIntegerChecker<int32_t>())

      .AddAttribute("Prefix", "Name of the Interest", StringValue("/"),
                    MakeNameAccessor(&Consumer::m_interestName), MakeNameChecker())
      .AddAttribute("LifeTime", "LifeTime for interest packet", StringValue("8s"),/* Elidio. Changed in SendPacket()*/
                    MakeTimeAccessor(&Consumer::m_interestLifeTime), MakeTimeChecker())

      .AddAttribute("RetxTimer",
                    "Timeout defining how frequent retransmission timeouts should be checked",
                    StringValue("50ms"),/*50ms*/
                    MakeTimeAccessor(&Consumer::GetRetxTimer, &Consumer::SetRetxTimer),
                    MakeTimeChecker())

      .AddTraceSource("LastRetransmittedInterestDataDelay",
                      "Delay between last retransmitted Interest and received Data",
                      MakeTraceSourceAccessor(&Consumer::m_lastRetransmittedInterestDataDelay),
                      "ns3::ndn::Consumer::LastRetransmittedInterestDataDelayCallback")

      .AddTraceSource("FirstInterestDataDelay",
                      "Delay between first transmitted Interest and received Data",
                      MakeTraceSourceAccessor(&Consumer::m_firstInterestDataDelay),
                      "ns3::ndn::Consumer::FirstInterestDataDelayCallback");

  return tid;
}

Consumer::Consumer()
  : m_rand(CreateObject<UniformRandomVariable>())
  , m_seq(0)
  , m_seqMax(0) // don't request anything
{
  NS_LOG_FUNCTION_NOARGS();

  m_rtt = CreateObject<RttMeanDeviation>();
}

void
Consumer::SetRetxTimer(Time retxTimer)
{
  m_retxTimer = retxTimer;
  if (m_retxEvent.IsRunning()) {
    // m_retxEvent.Cancel (); // cancel any scheduled cleanup events
    Simulator::Remove(m_retxEvent); // slower, but better for memory
  }

  // schedule even with new timeout
  m_retxEvent = Simulator::Schedule(m_retxTimer, &Consumer::CheckRetxTimeout, this);
}

Time
Consumer::GetRetxTimer() const
{
  return m_retxTimer;
}

void
Consumer::CheckRetxTimeout()
{
  Time now = Simulator::Now();

  Time rto = m_rtt->RetransmitTimeout();
  // Commented for now
  // NS_LOG_DEBUG ("Current RTO: " << rto.ToDouble (Time::S) << "s");

  while (!m_seqTimeouts.empty()) {
    SeqTimeoutsContainer::index<i_timestamp>::type::iterator entry =
      m_seqTimeouts.get<i_timestamp>().begin();
    if (entry->time + rto <= now) // timeout expired?
    {
      uint32_t seqNo = entry->seq;
      m_seqTimeouts.get<i_timestamp>().erase(entry);
      OnTimeout(seqNo);
    }
    else
      break; // nothing else to do. All later packets need not be retransmitted
  }

  m_retxEvent = Simulator::Schedule(m_retxTimer, &Consumer::CheckRetxTimeout, this);
}

// Application Methods
void
Consumer::StartApplication() // Called at time specified by Start
{
  NS_LOG_FUNCTION_NOARGS();

  // do base stuff
  App::StartApplication();

  NS_LOG_INFO(" m_interestLifeTime " << m_interestLifeTime.GetMilliSeconds() << "ms\n");
  ScheduleNextPacket();
}

void
Consumer::StopApplication() // Called at time specified by Stop
{
  NS_LOG_FUNCTION_NOARGS();

  // cancel periodic packet generation
  Simulator::Cancel(m_sendEvent);

  // cleanup base stuff
  App::StopApplication();
}

void
Consumer::SendPacket()
{
  if (!m_active)
    return;

  NS_LOG_FUNCTION_NOARGS();

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
  shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
  nameWithSequence->appendSequenceNumber(seq);
  //

  // shared_ptr<Interest> interest = make_shared<Interest> ();
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);




    // set the interest lifetime
  // double timeout = 1.0 * EstimatedRTT + 4.0 * DeviationRTT;
  //
  // // if (timeout < MINIMUM_TIMEOUT)
  // //   timeout = MINIMUM_TIMEOUT;
  //
  //
  // m_interestLifeTime = m_rtt->RetransmitTimeout ();//.ToDouble (Time::S); //ns3::Time::FromDouble(timeout, ns3::Time::MS);

  // m_interestLifeTime = ns3::Time::FromDouble(m_rtt->RetransmitTimeout ().ToDouble (Time::MS), ns3::Time::MS);
  // NS_LOG_INFO(" m_interestLifeTime " << m_rtt->RetransmitTimeout ().ToDouble (Time::MS) << "ms\n");

  // time::seconds interestLifeTime(m_interestLifeTime.GetSeconds());

  time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds()); // retransmission will/SHOULD be started after the interestLifeTime expires
  interest->setInterestLifetime(interestLifeTime);

  // auto x = m_rtt->RetransmitTimeout ().ToDouble (Time::S);
  // interest->setInterestLifetime(ndn::time::seconds(8));

  auto node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  uint64_t nodeId = node->GetId();

  /* Elidio */
  /***************************************************************************************/
  // interest->setTag(make_shared<lp::NonDiscoveryTag>()); // NonDiscovery
  // interest->setTag(make_shared<lp::fromIdTag>(nodeId));

  // auto NonD = interest->getTag<lp::NonDiscoveryTag>();
  // if (NonD != nullptr)
    // std::cout << "[ndn-consumer.cpp - Consumer::SendPacket()] NonDiscovery SET = " << *NonD << std::endl;




//   std::cout << "ndn.ndn-consumer (nodeId): " << nodeId << std::endl;

  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&nodeId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<1>(NodeIdMacMap) == (int) nodeId;});
  if (it != NodeIdMacMap.end()) {
    interest->setOrigID(std::get<0>(*it));
  }
  else
    interest->setOrigID("00:00:00:FF:FF:FF");

  // Set final destination
  interest->setDestFinalID(DataOrigId);

  ns3::Ptr<ns3::MobilityModel> mobModel = node->GetObject<ns3::MobilityModel> ();
  auto speed = mobModel->GetVelocity ();
  auto position = mobModel->GetPosition ();

  interest->setTag<lp::SpeedTag>(make_shared<lp::SpeedTag>(std::make_tuple(speed.x, speed.y, speed.z)));
  interest->setTag<lp::GeoTag>(make_shared<lp::GeoTag>(std::make_tuple(position.x, position.y, position.z)));

  uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();
  // uint64_t idd = 666;
  interest->setTag<lp::timestampTag>(make_shared<lp::timestampTag>(timestamp));
  interest->setTag<lp::destIdTag>(make_shared<lp::destIdTag>(destinationID)); // Will be set from LSTM algorithm (*destId)
  interest->setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(nodeId));

  //   interest->setOrigID(std::get<0>(*it));
 /***************************************************************************************/

  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO("> Interest for " << seq);

  WillSendOutInterest(seq);

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  ScheduleNextPacket();
}

// /******************** Elidio - For Beacon ***********************/
// void
// Consumer::SendPacket2()
// {
//   if (!m_active)
//     return;
//
//   NS_LOG_FUNCTION_NOARGS();
//
//   double retxTimeout = 5;
//   SetRetxTimer(Seconds(retxTimeout));
//
//   auto node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   uint64_t nodeId = node->GetId();
//
//   uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid
//
//   while (m_retxSeqs.size()) {
//     seq = *m_retxSeqs.begin();
//     m_retxSeqs.erase(m_retxSeqs.begin());
//     break;
//   }
//
//   if (seq == std::numeric_limits<uint32_t>::max()) {
//     if (m_seqMax != std::numeric_limits<uint32_t>::max()) {
//       if (m_seq >= m_seqMax) {
//         return; // we are totally done
//       }
//     }
//
//     seq = m_seq++;
//   }
//
//   NS_LOG_INFO("[SendPacket2] GetRetxTimer: " << GetRetxTimer());
//   //
//   // ndn::Name contentName(m_interestName.toUri() + std::to_string(nodeId));
//
//   shared_ptr<Name> nameWithSequence = make_shared<Name>(m_interestName);
//   // nameWithSequence->append(std::to_string(nodeId));
//   nameWithSequence->appendSequenceNumber(seq);
//
//   shared_ptr<Interest> interest = make_shared<Interest>();
//   interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
//   interest->setName(*nameWithSequence);
//   interest->setCanBePrefix(false);
//   // interest->SetRetxTimer("2000ms"); // Change RTO
//   // double retxTimeout = 12.6;
//   // SetRetxTimer(Seconds(retxTimeout));
//
//   // std::cout << "[SendPacket2] interest->getName(): " << interest->getName() << std::endl;
//
//   // time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
//   // interest->setInterestLifetime(interestLifeTime);
//   // interest->setInterestLifetime(ndn::time::seconds(2));
//
//
//   interest->setTag(make_shared<lp::NonDiscoveryTag>(0));
//   auto NonD = interest->getTag<lp::NonDiscoveryTag>();
//
// //   std::cout << "ndn.ndn-consumer (nodeId): " << nodeId << std::endl;
//
//   auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
//                            [&nodeId](const std::tuple<std::string, int>& NodeIdMacMap)
//                            {return std::get<1>(NodeIdMacMap) == (int) nodeId;});
//   if (it != NodeIdMacMap.end()) {
//     interest->setOrigID(std::get<0>(*it));
//   }
//   else
//     interest->setOrigID("00:00:00:FF:FF:FF");
//
//   // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
//   NS_LOG_INFO("> Interest for " << seq);
//
//   WillSendOutInterest(seq);
//
//   ns3::Ptr<ns3::MobilityModel> mobModel = node->GetObject<ns3::MobilityModel> ();
//   // std::cout << "From producerBeacon. Data CN set: " << interest->getName().getPrefix(-1) << std::endl;
//   auto speed = mobModel->GetVelocity ();
//   interest->setTag<lp::SpeedTag>(make_shared<lp::SpeedTag>(std::make_tuple(speed.x, speed.y, speed.z)));
//
//   auto position = mobModel->GetPosition ();
//   interest->setTag<lp::GeoTag>(make_shared<lp::GeoTag>(std::make_tuple(position.x, position.y, position.z)));
//
//   interest->setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(nodeId));
//
//
//   m_transmittedInterests(interest, this, m_face);
//   m_appLink->onReceiveInterest(*interest);
//
//   // ScheduleNextPacket(); // No new scheduling.
// }

/****************************************************************/


///////////////////////////////////////////////////
//          Process incoming packets             //
///////////////////////////////////////////////////

void
Consumer::OnData(shared_ptr<const Data> data)
{
  if (!m_active)
    return;

  // std::cout << "Is this the executed function, instead??... \n";
  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);

  // NS_LOG_INFO ("Received content object: " << boost::cref(*data));

  // This could be a problem......
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_INFO("< DATA for " << seq);

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
}

void
Consumer::OnNack(shared_ptr<const lp::Nack> nack)
{
  /// tracing inside
  App::OnNack(nack);

  NS_LOG_INFO("NACK received for: " << nack->getInterest().getName()
              << ", reason: " << nack->getReason());
}

void
Consumer::OnTimeout(uint32_t sequenceNumber)
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
Consumer::WillSendOutInterest(uint32_t sequenceNumber)
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

} // namespace ndn
} // namespace ns3
