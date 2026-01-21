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

#include "ndn-consumer-cromo2.hpp"
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
#include <ndn-cxx/util/nmsi.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>


#include <ns3/node-list.h>
#include <ns3/node.h>
#include "ns3/mobility-model.h"
#include "ns3/core-module.h"
#include "ns3/ndnSIM/model/map-common.hpp"

#include "ns3/ndnSIM/model/ndn-l3-protocol.hpp"
// #include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

// extern std::vector<std::tuple<std::string, int>> NodeIdMacMap;
// std::vector<std::tuple<std::string, int>> NodeIdMacMap;
// uint64_t nodeNum = 0;
// std::string tmp_dir;
// uint64_t destinationID;
ndn::util::Nmsi interestFinalDestinationNmsi; //Instead of destinationID...
// std::string DataOrigId = "ff:ff:ff:ff:ff:77";

NS_LOG_COMPONENT_DEFINE("ndn.ConsumerCRoMo2");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ConsumerCRoMo2);

TypeId
ConsumerCRoMo2::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ConsumerCRoMo2")
      .SetGroupName("Ndn")
      .SetParent<Consumer>()
      .AddConstructor<ConsumerCRoMo2>()

      .AddAttribute("Frequency", "Frequency of interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&ConsumerCRoMo2::m_frequency), MakeDoubleChecker<double>())

      .AddAttribute("Randomize",
                    "Type of send time randomization: none (default), uniform, exponential",
                    StringValue("none"),
                    MakeStringAccessor(&ConsumerCRoMo2::SetRandomize, &ConsumerCRoMo2::GetRandomize),
                    MakeStringChecker())

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerCRoMo2::m_seqMax), MakeIntegerChecker<uint32_t>())

    ;

  return tid;
}

ConsumerCRoMo2::ConsumerCRoMo2()
  : m_frequency(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();
  // std::cout << "Constructor ConsumerCRoMo2: " << "tmp_dir: " << tmp_dir << " nodeNum:" << nodeNum << std::endl;
}

ConsumerCRoMo2::~ConsumerCRoMo2()
{
}

void
ConsumerCRoMo2::ScheduleNextPacket()
{
  // double mean = 8.0 * m_payloadSize / m_desiredRate.GetBitRate ();
  // std::cout << "next: " << Simulator::Now().ToDouble(Time::S) + mean << "s\n";

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &ConsumerCRoMo2::SendPacket, this);
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule((m_random == 0) ? Seconds(1.0 / m_frequency)
                                                      : Seconds(m_random->GetValue()),
                                      &ConsumerCRoMo2::SendPacket, this);
}

void
ConsumerCRoMo2::SetRandomize(const std::string& value)
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

void
ConsumerCRoMo2::OnData(shared_ptr<const Data> data)
{
  if (!m_active)
    return;

  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);

  // NS_LOG_INFO ("Received content object: " << boost::cref(*data));

  // This could be a problem......
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_INFO("< DATA for " << seq);

    /* Elidio */ // SET on Base class
  /***************************************************************************************/
  auto node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
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
 /***************************************************************************************/

// Working stuff
//   ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   uint32_t nodeId = node->GetId();
//
//   /*GeoTag*/
//   std::shared_ptr<lp::GeoTag> Geotag = data->getTag<lp::GeoTag>();
//
//   if(Geotag == nullptr){
//     std::cout<<"My Custom Tag value is: null" << std::endl;
//   }
//   else{
//     std::tuple<double, double, double> location = Geotag->getPos();
//
//     std::cout<<"ndn.consumer onData()  Node-Id: "<< nodeId << std::endl;
//     std::cout<<"ndn.consumer onData()  InterestName: " << data->getName().toUri() << std::endl;
//     std::cout<<"ndn.consumer onData()  Geo tag x location: " << std::get<0>(location) << std::endl;
//     std::cout<<"ndn.consumer onData()  Geo tag y location: " << std::get<1>(location) << std::endl;
//     std::cout<<"ndn.consumer onData()  Geo tag z location: " << std::get<2>(location) << std::endl;
//   }


// Working stuff
  /*SpeedTag*/
//   std::shared_ptr<lp::SpeedTag> Speedtag = data->getTag<lp::SpeedTag>();
//
//   if(Speedtag == nullptr){
//     std::cout<<"My Custom Tag value is: null" << std::endl;
//   }
//   else{
//     std::tuple<double, double, double> speed = Speedtag->getSpeed();
//
//     std::cout<<"ndn.consumer onData()  Node-Id: " << nodeId << std::endl;
//     std::cout<<"ndn.consumer onData()  InterestName: " << data->getName().toUri() << std::endl;
//     std::cout<<"ndn.consumer onData()  Speed x: " << std::get<0>(speed) << std::endl;
//     std::cout<<"ndn.consumer onData()  Speed y: " << std::get<1>(speed) << std::endl;
//     std::cout<<"ndn.consumer onData()  Speed z: " << std::get<2>(speed) << std::endl;
//   }
  /****************/



  int hopCount = 0;
  auto hopCountTag = data->getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  NS_LOG_DEBUG("Hop count: " << hopCount);


  uint64_t fromId = 666;
  auto FromId = data->getTag<lp::fromIdTag>();
  if(FromId != nullptr){
    fromId = FromId->get();
  }
  destinationID = fromId; // used to set Interest destId in the base Class


  // double px = 0.0;
  // double py = 0.0;
  // double pz = 0.0;
  // std::tie(px, py, pz) = toolsStrategy.GetNodeLocationFromInterestPacket(data);
  // ndn::util::GeoPosition geoposition(px, py, pz);
  //
  // double vx_gt = 0.0;
  // double vy_gt = 0.0;
  // double vz_gt = 0.0;
  // std::tie(vx_gt, vy_gt, vz_gt) = toolsStrategy.GetNodeSpeedFromInterestPacket(data);
  // ndn::util::NodeSpeed nodespeed(vx_gt, vy_gt);
  //
  // uint64_t timestamp1 = ns3::Simulator::Now().GetMilliSeconds();
  // auto timestamp = data->getTag<lp::timestampTag>();
  // if(timestamp != nullptr){
  //   timestamp1 = timestamp->get();
  // }
  //
  //
  // uint64_t dataorigin;
  // auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
  //                   [&from](const std::tuple<std::string, int>& NodeIdMacMap)
  //                   {
  //                     std::string endereco = std::get<0>(NodeIdMacMap);
  //                     endereco.pop_back(); //do not include the last \"
  //                     const char * endereco2 = &endereco[1];
  //                     return Mac48Address(endereco2) == Mac48Address::ConvertFrom (DataOrigId);// NOT to but from
  //                   });
  // if (it != NodeIdMacMap.end()) {
  //   auto endP = std::get<1>(*it);
  //   dataorigin = endP;//607;//
  // }
  //
  // std::cout << "Data origins: Node_" << dataorigin << std::endl;
  // ndn::util::Nmsi Nmsi(dataorigin, data->getName().getSubName(0, 5), geoposition, nodespeed, hopCount, timestamp1);
  // interestFinalDestinationNmsi = Nmsi;
  // std::cout << "destinationID: " << destinationID << std::endl;


  // DataOrigId = data->getOrigID();
  // interest->setDestFinalID(DataOrigId);

  // Add to FIB?
  // Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
  // shared_ptr<Face> face = ndn->getFaceById(257);
  // FibHelper::AddRoute(GetNode(), 0, 0, 60, 0.0, 0.0, 0.0, 0.0, data->getName().toUri(), face, 0);

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

  auto payload = data->getContent();
//   Block::element_const_iterator ptr;

  // std::cout << "Received payload content: " << std::endl;// REACTIVATE
  NS_LOG_INFO("Received payload content: "); // Commented 20230510
  // for(auto ptr = payload.begin(); ptr != payload.end(); ptr++){
  //  NS_LOG_INFO(*ptr);
  // }
  // NS_LOG_INFO("\n");



//   int payloadSize = data->GetPayload ()->GetSize ();
//   std::string content = "";
//   for (int i=0; i<payloadSize; i++){     // allocate the same number of characters as the number of characters in the payload
//       content = content + ".";
//   }
//   data->GetPayload()->CopyData(reinterpret_cast<uint8_t*>(&content[0]), content.size());
//   std::cout << "Received payload content: " << content << std::endl;
  /***************/
}

std::string
ConsumerCRoMo2::GetRandomize() const
{
  return m_randomType;
}

} // namespace ndn
} // namespace ns3
