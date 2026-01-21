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

#include "ndn-producer-cromo2Emergency.hpp"
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
// #include <list>
// // For including LCCT to the special Data packet
// #include "ndn-cxx/util/nmsi.hpp"

#include "ns3/ndnSIM/model/map-common.hpp"

// extern std::vector<std::tuple<std::string, int>> NodeIdMacMap;
// extern std::string tmp_dir;

NS_LOG_COMPONENT_DEFINE("ndn.ProducerCRoMo2Emergency");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ProducerCRoMo2Emergency);

TypeId
ProducerCRoMo2Emergency::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ProducerCRoMo2Emergency")
      .SetGroupName("Ndn")
      .SetParent<App>()/*App*/
      .AddConstructor<ProducerCRoMo2Emergency>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&ProducerCRoMo2Emergency::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&ProducerCRoMo2Emergency::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&ProducerCRoMo2Emergency::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&ProducerCRoMo2Emergency::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&ProducerCRoMo2Emergency::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&ProducerCRoMo2Emergency::m_keyLocator), MakeNameChecker());
  return tid;
}

ProducerCRoMo2Emergency::ProducerCRoMo2Emergency()
{
  NS_LOG_FUNCTION_NOARGS();
}

ProducerCRoMo2Emergency::~ProducerCRoMo2Emergency()
{
}

// inherited from Application base class.
void
ProducerCRoMo2Emergency::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

//     // Create a name components object for name
//   ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   uint32_t nodeId = node->GetId();
//
//   ns3::Ptr<ns3::MobilityModel> mobModel = node->GetObject<ns3::MobilityModel> ();
//   auto position = mobModel->GetPosition ();
//   auto speed = mobModel->GetVelocity ();
//   auto timestamp = ns3::Simulator::Now().GetMilliSeconds();
//
//   std::string nome = std::to_string(nodeId) + "/" + std::to_string(position.x) + "-" + std::to_string(position.y) + "-" + std::to_string(position.z) + "/";
//
//   m_postfix = nome;

  // FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
  FibHelper::AddRoute(GetNode(), 0, 0, 0, 0.0, 0.0, 0.0, 0.0, m_prefix, m_face, 0);
}

void
ProducerCRoMo2Emergency::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
ProducerCRoMo2Emergency::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;
   // Create a name components object for name
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  uint64_t nodeId = node->GetId();

  ns3::Ptr<ns3::MobilityModel> mobModel = node->GetObject<ns3::MobilityModel> ();
  auto position = mobModel->GetPosition ();
  auto speed = mobModel->GetVelocity ();
  uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();

  std::string nome = std::to_string(nodeId) + "/" + std::to_string(position.x) + "-" + std::to_string(position.y) + "-" + std::to_string(position.z) + "/";

  m_postfix = nome;



  Name dataName(interest->getName().getPrefix(-1));// .getPrefix(-1) to exclude the last component, e.g., %FE%04
  //std::cout << "Nomessss... " << interest->getName() << std::endl;


//   std::string nome = "/push-message/emergency" + std::to_string(nodeId) + "/" + std::to_string(position.x) + "-" + std::to_string(position.y) + "-" + std::to_string(position.z) + "/";

//   m_prefix = nome;
//   dataName.appendVersion();
//   dataName.append("emergency");
//   dataName.append(std::to_string(nodeId));
//   dataName.append(std::to_string(position.x));
//   dataName.append(std::to_string(position.y));
//   dataName.append(std::to_string(position.z));


  dataName.append(m_postfix);
  dataName.appendVersion(); //appends the version removed by getPrefix(-1). Appends at the end of the name prefix

//   Name dataName(nome);


  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));


    //auto speed = mobModel->GetVelocity ();
  // data->setTag(make_shared<lp::SpeedTag>(std::make_tuple(speed.x, speed.y, speed.z)));

  //auto positionn = mobModel->GetPosition ();
  // data->setTag(make_shared<lp::GeoTag>(std::make_tuple(position.x, position.y, position.z)));
  //   data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));

  /************************* Elidio ***************************************/
  /*uint64_t */nodeId = GetNode()->GetId();

  // redirecting std::cout to file
//   std::string tmp_dir = "/home/dasilva/PDEEC2021/testingENV/ns-3/testingVM_logs/";
  // std::string tmp_dir = "/var/tmp/ns-3/testingVM_logs/";

  std::string file = tmp_dir + "ndn-producer-cromo2Emergency_Node" + std::to_string(nodeId) + ".txt";

//   std::ios_base::openmode mode;
  static int forAppend = 0;

  std::ofstream out;
  if (forAppend == 0){
    out.open(file, std::fstream::trunc);
    forAppend++;
  }
  else
    out.open(file, std::fstream::app);

  auto *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(out.rdbuf());

  // std::cout << timestamp << std::endl;
  /**
   * Extract the interest origId, which will be the data destId
   */
  // std::string InterestOrigId = interest->getOrigID();
  // // std::cout << "Extracted origId from received Interest: " << InterestOrigId << std::endl;
  //
  // auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
  //                          [&InterestOrigId](const std::tuple<std::string, int>& NodeIdMacMap)
  //                          {return std::get<0>(NodeIdMacMap) == InterestOrigId;});
  // if (it != NodeIdMacMap.end()) {
  //   data->setTag(make_shared<lp::destIdTag>(std::get<1>(*it)));
  // }
  // else{
  //   std::cout << "[ndn-producer-cromo2Emergency] Problems setting OrigId... " << std::endl;
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


//   const std::string string = "payloadString";
//   data->setContent(make_shared< ::ndn::Buffer>(&string[0], string.size()));

  // setting origId to Data packet
  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&nodeId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<1>(NodeIdMacMap) == (int) nodeId;});
  if (it != NodeIdMacMap.end()) {
    data->setOrigID(std::get<0>(*it));
  }
  else {
    std::cout << "[ndn-producer-cromo2Emergency] Setting OrigI 88:88:88:FF:FF:FF" << std::endl;
    data->setOrigID("88:88:88:FF:FF:FF");//std::get<0>(*it)
  }


//   std::list<::ndn::util::Nmsi> vnmsi;
//
//   getFileContent("/home/dasilva/PDEEC2021/testingENV/ns-3/Node0.txt", vnmsi);
//
//   std::list<::ndn::util::Nmsi>::iterator lccqueueIt;
//   for (lccqueueIt = vnmsi.begin(); lccqueueIt != vnmsi.end(); lccqueueIt++)// comment out for now
//     std::cout << *lccqueueIt << std::endl;


  const std::string string = "payloadString - cromo2Emergency";

//   if (dataName.getSubName(1,1) == "/active-safety"){
//     data->setContent(reinterpret_cast<const uint8_t*>(&vnmsi), sizeof(vnmsi));
//   }
//   else {
    data->setContent(make_shared< ::ndn::Buffer>(&string[0], string.size()));
//   }

  data->setTag<lp::timestampTag>(make_shared<lp::timestampTag>(timestamp));
  data->setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(nodeId));

  /** reset cout buffer **/
  std::cout.rdbuf(coutbuf);
 /***************************************************************************************/

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
