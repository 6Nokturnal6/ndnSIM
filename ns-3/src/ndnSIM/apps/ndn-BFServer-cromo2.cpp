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

#include "ndn-BFServer-cromo2.hpp"
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


// Prefix announcement
#include "ndn-cxx/prefix-announcement.hpp"
#include "ndn-cxx/security/signing-helpers.hpp"
#include "ndn-cxx/security/key-chain.hpp"
#include "ndn-cxx/util/tools.hpp"
#include "ndn-cxx/security/pib/identity.hpp" //ndn::security::pib::
#include "ndn-cxx/security/v2/certificate-cache.hpp"
#include "ndn-cxx/security/signing-info.hpp"
#include "ndn-cxx/security/pib/impl/pib-memory.hpp"
#include "ndn-cxx/security/pib/pib.hpp"

// For CS-CCT stuff
#include "table/cs.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/forwarder.hpp"

#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>
#include "ndn-cxx/util/bloom-filter.hpp"
#include "ns3/ndnSIM/model/map-common.hpp"

// static int BloomFilter_size = 50000; // 40000 -> 5kb equivalent
// static int BloomFilter_NumberOfHashFunctions = 20;

bool printToFileBFServerCRoMo2 = true;

// extern std::vector<std::tuple<std::string, int>> NodeIdMacMap;
// extern std::string tmp_dir;

NS_LOG_COMPONENT_DEFINE("ndn.BFServerCRoMo2");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(BFServerCRoMo2);

TypeId
BFServerCRoMo2::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::BFServerCRoMo2")
      .SetGroupName("Ndn")
      .SetParent<App>()/*App*/
      .AddConstructor<BFServerCRoMo2>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&BFServerCRoMo2::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&BFServerCRoMo2::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&BFServerCRoMo2::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&BFServerCRoMo2::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&BFServerCRoMo2::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&BFServerCRoMo2::m_keyLocator), MakeNameChecker());
  return tid;
}

BFServerCRoMo2::BFServerCRoMo2()
{
  NS_LOG_FUNCTION_NOARGS();
}

BFServerCRoMo2::~BFServerCRoMo2()
{
}

// inherited from Application base class.
void
BFServerCRoMo2::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  // std::cout << "[BFServerCRoMo2::StartApplication]... " << std::endl;

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
BFServerCRoMo2::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
BFServerCRoMo2::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;

  static int invokationsServer = 0;
  // std::cout << "NodeId: " << GetNode()->GetId() << " [BFServer]: #" << invokationsServer++ << std::endl;

  // ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto node = GetNode();
  uint64_t nodeId = node->GetId();

  Name dataName(interest->getName());// .getPrefix(-1) to exclude the last component, e.g., %FE%04
  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  ns3::Ptr<ns3::MobilityModel> mobModel = node->GetObject<ns3::MobilityModel> ();
  // std::cout << "From producerBeacon. Data CN set: " << data->getName().getPrefix(-1) << std::endl;
  auto speed = mobModel->GetVelocity ();
  data->setTag<lp::SpeedTag>(make_shared<lp::SpeedTag>(std::make_tuple(speed.x, speed.y, speed.z)));

  auto position = mobModel->GetPosition ();
  data->setTag<lp::GeoTag>(make_shared<lp::GeoTag>(std::make_tuple(position.x, position.y, position.z)));

  uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();
  data->setTag<lp::timestampTag>(make_shared<lp::timestampTag>(timestamp));
  data->setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(nodeId));


  uint64_t fromId;
  auto FromId = interest->getTag<lp::fromIdTag>();
  if(FromId != nullptr){
    fromId = FromId->get();
    data->setTag<lp::destIdTag>(make_shared<lp::destIdTag>(fromId));
  }

  auto destId = interest->getTag<lp::destIdTag>();
  uint64_t dest;
  if(destId != nullptr){
    dest = destId->get();
  }

  // setting origId to Data packet
  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&nodeId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<1>(NodeIdMacMap) == (int) nodeId;});
  if (it != NodeIdMacMap.end()) {
    data->setOrigID(std::get<0>(*it));//std::get<0>(*it)
  }
  else{
    // std::cout << "OrigId NOT set?... " << std::endl;
    data->setOrigID("88:88:88:FF:FF:FF");//std::get<0>(*it)
  }

  const nfd::Cs& cs = node->GetObject<ndn::L3Protocol>()->getForwarder()->getCs();
  auto csSize = cs.size();

/*  std::string to_compare = "/" + std::to_string(nodeId);
  if(to_compare != interest->getName().getSubName(2,1).toUri()){
    std::cout << "getSubName(2,1): " << interest->getName().getSubName(2,1).toUri() << " - to_compare: " << to_compare << std::endl;
    return; // Nodes do not respond to the requests from other nodes
  }
  else
    std::cout << "getSubName(2,1): " << interest->getName().getSubName(2,1).toUri() << " - to_compare: " << to_compare << std::endl;*/
  if (dest != nodeId) {
    // std::cout << "Interest destId: " <<  dest << " - currentNodeId: " << nodeId << ". Exiting..." << std::endl;
    return;
  }

  if (csSize == 0){
    // std::cout << "CS is empty, returning - CN: " << interest->getName() << " with \"!BF\"" << std::endl;
    const std::string string = "!BF";
    data->setContent(make_shared< ::ndn::Buffer>(&string[0], string.size()));
    // return;
  }
  else {
    std::string file = tmp_dir + "ndn-BFServer-cromo2_Node" + std::to_string(nodeId) + ".txt";
    std::ofstream out;
    out.open(file, std::fstream::app);
    auto *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());

    ::ndn::util::BloomFilter bf(BloomFilter_NumberOfHashFunctions, BloomFilter_size);
    //Reads directly the CS

    std::cout << timestamp << std::endl;
    // if (interest->getName().getSubName(1,2).toUri() != "/sharingBF/"+std::to_string(nodeId)){
    //   std::cout << "Return here. interest->getName().getSubName(1,2).toUri()-> " << interest->getName().getSubName(1,2).toUri() << " and -> \"/sharingBF/\"" << nodeId << std::endl;
    //   std::cout.rdbuf(coutbuf);
    //   return;
    // }
    // else{// "/sharingBF/"+std::to_string(currentNodeId1)
      std::cout << "Number of stored contents: " << cs.size() << std::endl;// getFullName

    // if(csSize > 0){
      for (nfd::Cs::const_iterator entry = cs.begin(); entry != cs.end(); entry++){
        if (entry->getName().getSubName(0,1).toUri() != "/localhost")
          bf.add(entry->getName().getSubName(0,6).toUri());// Do add Data to BF
      }

      auto encodedBf = bf.encodeAsBinaryBlock();
      data->setContent(encodedBf);
      // std::cout << "sending BF to Node_" << fromId << std::endl;
      // NS_LOG_INFO("> sending BF to Node_" << fromId);
      std::cout << "BF(not printed anymore) destination: Node_" << fromId << std::endl;
      // bf.printHex();

      /** reset cout buffer **/
    std::cout.rdbuf(coutbuf);
  }
    // /** reset cout buffer **/
    // std::cout.rdbuf(coutbuf);
  // }

  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  // std::cout << timestamp << "node(" << GetNode()->GetId() << ") responding with Data: " << data->getName() << " to Node_" << fromId << std::endl;
  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName() << " to Node_" << fromId);

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);


}












/*

  // App::OnData(data); // tracing inside

  // NS_LOG_FUNCTION(this << data);

  if (!m_active)
    return;

  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  uint64_t nodeId = node->GetId();

  uint64_t fromId = 1000; // Invalid node ID
  auto FromId = interest->getTag<lp::fromIdTag>();
  if(FromId != nullptr)
    fromId = *FromId;//FromId->get();
  else
    std::cout << "fromId NOT SET!! " << std::endl;

  // std::cout << "[BFServer]. fromId: " << fromId << " nodeId: " << nodeId << std::endl;
  const nfd::Cs& cs = node->GetObject<ndn::L3Protocol>()->getForwarder()->getCs();
  auto csSize = cs.size();
  std::string to_compare = "/" + std::to_string(nodeId);
  if((csSize == 0) && (to_compare != interest->getName().getSubName(3,1).toUri())){
    // std::cout << "[BFServer] Exiting... fromId != nodeId: " << fromId << " = " << nodeId << " or csSize = " << csSize << std::endl;
    return; // Only responds to own consumer
  }

  Name dataName(interest->getName());//.getPrefix(-1));
  // Name dataName(data_Name.getPrefix(-1));// .getPrefix(-1) to exclude the last component, e.g., %FE%04
  // dataName.append("sharingBF");
  // dataName.append(std::to_string(nodeId));
  // dataName.appendVersion();


  // dataName.append(m_postfix);


  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  ns3::Ptr<ns3::MobilityModel> mobModel = node->GetObject<ns3::MobilityModel> ();
  auto speed = mobModel->GetVelocity ();
  data->setTag<lp::SpeedTag>(make_shared<lp::SpeedTag>(std::make_tuple(speed.x, speed.y, speed.z)));

  auto position = mobModel->GetPosition ();
  data->setTag<lp::GeoTag>(make_shared<lp::GeoTag>(std::make_tuple(position.x, position.y, position.z)));



  // const nfd::Cs& cs = node->GetObject<ndn::L3Protocol>()->getForwarder()->getCs();
  // auto csSize = cs.size();

  std::ofstream out;
  std::string file = tmp_dir + "ndn-BFServer-cromo2_Node" + std::to_string(nodeId) + ".txt";

  // if(csSize > 0)  {
    out.open(file, std::fstream::app);
  // }
  auto *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(out.rdbuf());

  data->setTag<lp::destIdTag>(make_shared<lp::destIdTag>(fromId));
  // std::cout << "fromId: " << nodeId << std::endl;
  std::cout << "Data destination (extracted form Interest fromId): " << fromId << std::endl;

  // setting origId to Data packet
  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&nodeId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<1>(NodeIdMacMap) == (int) nodeId;});
  if (it != NodeIdMacMap.end()) {
    // if (csSize > 0) std::cout << "OrigId set to Data packet: " << std::get<0>(*it) << std::endl;
    data->setOrigID(std::get<0>(*it));//std::get<0>(*it)
    // std::cout << "SET data->getOrigID(): " << data->getOrigID() << std::endl;
  }
  else{
    std::cout << "OrigId NOT set?... " << std::endl;
    data->setOrigID("88:88:88:FF:FF:FF");//std::get<0>(*it)
  }


  // const nfd::Cs& cs = node->GetObject<ndn::L3Protocol>()->getForwarder()->getCs();
  // auto csSize = cs.size();
  ::ndn::util::BloomFilter bf(BloomFilter_NumberOfHashFunctions, BloomFilter_size);
  //Reads directly the CS

  // std::cout << "BloomFilter_NumberOfHashFunctions: " << BloomFilter_NumberOfHashFunctions << " BloomFilter_size: " << BloomFilter_size << std::endl;
  if ((dataName.getSubName(2,1) == "/sharingBF") && (csSize > 0)){
    // const nfd::Cs& cs = node->GetObject<ndn::L3Protocol>()->getForwarder()->getCs();
    std::cout << "Number of stored contents: " << cs.size() << std::endl;// getFullName

    // if(csSize > 0){
      for (nfd::Cs::const_iterator entry = cs.begin(); entry != cs.end(); entry++){
        if(printToFileBFServerCRoMo2)
          std::cout << ns3::Simulator::Now().GetMilliSeconds() << "\n" << "NMSI:\n" << entry->getNmsi() << std::endl;// getFullName

          bf.add(entry->getName().getSubName(0,5).toUri());// Do add Data to BF
        }

      // Respond to beacon
      // ::ndn::Buffer buf(string.begin(), string.end());// Bloom Filter
      auto encodedBf = bf.encodeAsBinaryBlock();
      data->setContent(encodedBf);

      std::cout << "node(" << GetNode()->GetId() << ") responding with Data: " << data->getName() << std::endl;
      bf.printHex();
  }
  else {
    std::cout << "Requests from other nodes than -this- a note answered... CN: " << data->getName() << std::endl;
    // const std::string string = "payloadString";
    // data->setContent(make_shared< ::ndn::Buffer>(&string[0], string.size()));
    return;
  }

  uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();
  data->setTag<lp::timestampTag>(make_shared<lp::timestampTag>(timestamp));
  data->setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(nodeId));


   //reset cout buffer
  std::cout.rdbuf(coutbuf);

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

}*/


} // namespace ndn
} // namespace ns3
