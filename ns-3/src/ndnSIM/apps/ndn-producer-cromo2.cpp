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

#include "ndn-producer-cromo2.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>

#include "utils/ndn-ns3-packet-tag.hpp"
#include "utils/ndn-rtt-mean-deviation.hpp"

#include <ndn-cxx/lp/tags.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/ref.hpp>

#include "ndn-app.hpp"


#include <ns3/node-list.h>
#include <ns3/node.h>
#include "ns3/mobility-model.h"
#include "ns3/core-module.h"
// For redirecting std::cout
#include <bits/stdc++.h>
#include <list>
// // For including LCCT to the special Data packet
#include "ndn-cxx/util/nmsi.hpp"
#include "ndn-cxx/util/tools.hpp"

// For CS-CCT stuff
#include "table/cs.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/forwarder.hpp"
#include "ns3/ndnSIM/model/map-common.hpp"

// std::vector<std::tuple<std::string, int>> NodeIdMacMap;
// extern std::string tmp_dir;

// ::ndn::util::Tools toolsX;
// uint64_t nodeNum = toolsX.GetNodeNum (tmp_dir);

uint64_t destinationID;
bool printToFileProducerCromo = true;
std::string DataOrigId = "0f:ff:ff:ff:ff:ff";
ndn::util::Nmsi DataOrigNmsi;

NS_LOG_COMPONENT_DEFINE("ndn.ProducerCRoMo2");

// using namespace nfd::cs::cromo2;
// class Nmsi;

using namespace ::ndn::util;

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ProducerCRoMo2);

TypeId
ProducerCRoMo2::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ProducerCRoMo2")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<ProducerCRoMo2>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&ProducerCRoMo2::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&ProducerCRoMo2::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&ProducerCRoMo2::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&ProducerCRoMo2::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&ProducerCRoMo2::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&ProducerCRoMo2::m_keyLocator), MakeNameChecker());
  return tid;
}

ProducerCRoMo2::ProducerCRoMo2()
{
  NS_LOG_FUNCTION_NOARGS();
  // std::cout << "Constructor ProducerCRoMo2: " << "tmp_dir: " << tmp_dir << " nodeNum:" << nodeNum << std::endl;
}

ProducerCRoMo2::~ProducerCRoMo2()
{
}

// inherited from Application base class.
void
ProducerCRoMo2::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  auto node = GetNode();
  uint64_t nodeId = node->GetId();
  destinationID = nodeId;
  // FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
  FibHelper::AddRoute(GetNode(), 0/*nodeNum*/, 0, 0, 0.0, 0.0, 0.0, 0.0, m_prefix, m_face, 0);

      // setting origId to Data packet
  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&nodeId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<1>(NodeIdMacMap) == (int) nodeId;});
  if (it != NodeIdMacMap.end()) {
    // data->setOrigID(std::get<0>(*it));//std::get<0>(*it)
    DataOrigId = std::get<0>(*it);
    // std::cout << "DataOrigId: " << DataOrigId << std::endl;
  }
  else{
    // data->setOrigID("88:88:88:FF:FF:FF");//std::get<0>(*it)
    DataOrigId = "88:88:88:FF:FF:FF";
    // std::cout << "Else DataOrigId: " << DataOrigId << std::endl;
  }
}

void
ProducerCRoMo2::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
ProducerCRoMo2::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;

  Name dataName(interest->getName());

  // dataName.append(m_postfix);
  // dataName.appendVersion();

  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  //   data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));

  /************************* Elidio ***************************************/
  ns3::Ptr<ns3::Node> node;
  uint64_t x = ns3::Simulator::GetContext();
  if(x < nodeNum)
    node = ns3::NodeList::GetNode(x);
  else
    return;

  uint64_t nodeId = node->GetId();


  if (dataName.getSubName(1,1).toUri() == "/beacon"){
    // std::cout << "Beacon... Bailing out..\n";
    return;
  }

  ns3::Ptr<ns3::MobilityModel> mobModel = node->GetObject<ns3::MobilityModel> ();
  auto speed = mobModel->GetVelocity ();
  data->setTag<lp::SpeedTag>(make_shared<lp::SpeedTag>(std::make_tuple(speed.x, speed.y, speed.z)));

  auto position = mobModel->GetPosition ();
  data->setTag<lp::GeoTag>(make_shared<lp::GeoTag>(std::make_tuple(position.x, position.y, position.z)));
  // data->setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(nodeId));

  // redirecting std::cout to file
//   std::string tmp_dir = "/home/dasilva/PDEEC2021/testingENV/ns-3/testingVM_logs/";
  // std::string tmp_dir = "/var/tmp/ns-3/testingVM_logs/";
  std::string file = tmp_dir + "ndn-producer-cromo2_Node" + std::to_string(nodeId) + ".txt";

//   std::ios_base::openmode mode;
  static int forAppend = 0;

  std::ofstream out;
  // if (forAppend == 0){
  //   out.open(file, std::fstream::trunc);
  //   forAppend++;
  // }
  // else
    out.open(file, std::fstream::app);//app//trunc

  auto *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(out.rdbuf());

  uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();

  if(printToFileProducerCromo) std::cout << timestamp << std::endl;


    // setting origId to Data packet
  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&nodeId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<1>(NodeIdMacMap) == (int) nodeId;});
  if (it != NodeIdMacMap.end()) {
    data->setOrigID(std::get<0>(*it));//std::get<0>(*it)
    // DataOrigId = std::get<0>(*it);
    // std::cout << "DataOrigId: " << DataOrigId << std::endl;
  }
  else{
    data->setOrigID("88:88:88:FF:FF:FF");//std::get<0>(*it)
    // DataOrigId = "88:88:88:FF:FF:FF";
    // std::cout << "Else DataOrigId: " << DataOrigId << std::endl;
  }

  uint64_t dataorigin = nodeId;
  // std::cout << "Data origins: Node_" << dataorigin << std::endl;
  ::ndn::util::GeoPosition position2(position.x, position.y, position.z);
  ::ndn::util::NodeSpeed speed2(speed.x, speed.y);
  ::ndn::util::Nmsi Nmsi(dataorigin, interest->getName().toUri(), position2, speed2, 0, timestamp);
  DataOrigNmsi = Nmsi;
  DataOrigId = data->getOrigID();
// TODO -> To mobility...cpp


  /**
   * Extract the interest origId, which will be the data destId
   */
  std::string InterestOrigId = interest->getOrigID();
  data->setDestFinalID(InterestOrigId);
  // // std::cout << "Extracted origId from received Interest: " << InterestOrigId << std::endl;
  //
  // auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
  //                          [&InterestOrigId](const std::tuple<std::string, int>& NodeIdMacMap)
  //                          {return std::get<0>(NodeIdMacMap) == InterestOrigId;});
  // if (it != NodeIdMacMap.end()) {
  //   // data->setTag<lp::destIdTag>(make_shared<lp::destIdTag>(std::get<1>(*it)));
  // }
  // else{
  //   std::cout << "[ProducerCRoMo2Beacon]. destId NOT found/Set" << std::endl;
  // // data->setTag<lp::destIdTag>(make_shared<lp::destIdTag>(std::get<1>(*it)));
  // }



  uint64_t fromId;
  auto FromId = interest->getTag<lp::fromIdTag>();
  if(FromId != nullptr){
    fromId = FromId->get();
    data->setTag<lp::destIdTag>(make_shared<lp::destIdTag>(fromId));
  }
  else{
    std::cout << "\tDestId NOT set!" << std::endl;
  }



//   LcctQueue::iterator lccqueueIt;
//
//   for (nfd::cs::cromo2::CRoMo2Policy::lccqueueIt = nfd::cs::cromo2::CRoMo2Policy::m_lcctqueue.begin(); nfd::cs::cromo2::CRoMo2Policy::lccqueueIt != nfd::cs::cromo2::CRoMo2Policy::m_lcctqueue.end(); nfd::cs::cromo2::CRoMo2Policy::lccqueueIt++)// comment out for now
//     std::cout << *nfd::cs::cromo2::CRoMo2Policy::lccqueueIt << std::endl;

//   nfd::cs::cromo2::CRoMo2Policy::PrintLCCT ();

//   auto x = nfd::cs::cromo2::CRoMo2Policy::ObjectCRoMo2 ();
  /** reset cout buffer **/

  // std::list<::ndn::util::Nmsi> vnmsi;
  // vnmsi.clear();
  //
  // ::ndn::util::Tools tools;
  // tools.getFileContent("/var/tmp/ns-3/testingVM_logs/Node" + std::to_string(nodeId) + ".txt", vnmsi);
  //
  // std::list<::ndn::util::Nmsi>::iterator lccqueueIt;
  // for (lccqueueIt = vnmsi.begin(); lccqueueIt != vnmsi.end(); lccqueueIt++)// comment out for now
  //   if(printToFileProducerCromo) std::cout << *lccqueueIt;


  //Reads directly the CS

  // if (dataName.getSubName(2,1) == "/beacon"){//FIXME -- Does not need  to be here!
  //   const nfd::Cs& cs = node->GetObject<ndn::L3Protocol>()->getForwarder()->getCs();
  //   if(printToFileProducerCromo) std::cout << "Number of stored contents: " << cs.size() << std::endl;// getFullName
  //   for (nfd::Cs::const_iterator entry = cs.begin(); entry != cs.end(); entry++)
  //       if(printToFileProducerCromo) std::cout << Simulator::Now ().ToDouble (Time::S) << "\n" << "Content name: " << entry->getName().toUri() << "\t" << "Node Speed: " << entry->getNodeSpeed() << "\t" << "Geo-Position: " << entry->getNodePosition() /*<< "\t" << "Prefix: " << entry->getPrefix()*/ << "\t" << "Hops: " << entry->getNodeHops() << "\t" << "Timestamp: " << entry->getNodeTimestamp() << std::endl;// getFullName
  // }

  const std::string string = "payloadString";

  if (dataName.getSubName(2,1) == "/active-safety"){//FIXME
    // if(printToFileProducerCromo) std::cout << "ISTO" << std::endl;
    // data->setContent(reinterpret_cast<const uint8_t*>(&vnmsi), sizeof(vnmsi));// Commented
  }
  else {
    // if(printToFileProducerCromo) std::cout << "OU ISTO" << std::endl;
    data->setContent(make_shared< ::ndn::Buffer>(&string[0], string.size()));
    // data->setContent(reinterpret_cast<const uint8_t*>(&vnmsi), sizeof(vnmsi));
  }




  std::cout.rdbuf(coutbuf);
 /***************************************************************************************/

  // uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();
  data->setTag<lp::timestampTag>(make_shared<lp::timestampTag>(timestamp));
  data->setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(nodeId));

  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
}

} // namespace ndn
} // namespace ns3
