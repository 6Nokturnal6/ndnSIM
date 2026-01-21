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

#include "ndn-producer-cromo2Beacon.hpp"
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

bool printToFileProducerCromoBeacon = false;

// extern std::vector<std::tuple<std::string, int>> NodeIdMacMap;
// extern std::string tmp_dir;

NS_LOG_COMPONENT_DEFINE("ndn.ProducerCRoMo2Beacon");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ProducerCRoMo2Beacon);

TypeId
ProducerCRoMo2Beacon::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ProducerCRoMo2Beacon")
      .SetGroupName("Ndn")
      .SetParent<App>()/*App*/
      .AddConstructor<ProducerCRoMo2Beacon>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&ProducerCRoMo2Beacon::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&ProducerCRoMo2Beacon::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&ProducerCRoMo2Beacon::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&ProducerCRoMo2Beacon::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&ProducerCRoMo2Beacon::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&ProducerCRoMo2Beacon::m_keyLocator), MakeNameChecker());
  return tid;
}

ProducerCRoMo2Beacon::ProducerCRoMo2Beacon()
{
  NS_LOG_FUNCTION_NOARGS();
  // std::cout << "Constructor ProducerCromoBeacon: " << "tmp_dir: " << tmp_dir << " nodeNum:" << nodeNum << std::endl;
}

ProducerCRoMo2Beacon::~ProducerCRoMo2Beacon()
{
}

// inherited from Application base class.
void
ProducerCRoMo2Beacon::StartApplication()
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
  FibHelper::AddRoute(GetNode(), 998, 0, 0, 0.0, 0.0, 0.0, 0.0, m_prefix, m_face, 0);
}

void
ProducerCRoMo2Beacon::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
ProducerCRoMo2Beacon::OnInterest(shared_ptr<const Interest> interest)
{
  App::OnInterest(interest); // tracing inside
  // std::cout << "[ProducerCRoMo2Beacon::OnInterest] Invoked anywhere????... \n";

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;

  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
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

  // setting origId to Data packet
  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&nodeId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<1>(NodeIdMacMap) == (int) nodeId;});
  if (it != NodeIdMacMap.end()) {
    data->setOrigID(std::get<0>(*it));//std::get<0>(*it)
  }
  else{
    // if (printToFileProducerCromoBeacon) std::cout << "OrigId NOT set?... " << std::endl;
    data->setOrigID("88:88:88:FF:FF:FF");//std::get<0>(*it)
  }


  const nfd::Cs& cs = node->GetObject<ndn::L3Protocol>()->getForwarder()->getCs();
  auto csSize = cs.size();

  // if (interest->getName().getSubName(1,1).toUri() == "/sharingBF")
  // return; // its not the task of this producer (ProducerCRoMo2Beacon) to provide sharingBF-like BFs --- Not necessary anymore but leaving it here

  if (csSize == 0){
    // std::cout << "CS is empty, returning - CN: " << interest->getName() << " with \"!BF\"" << std::endl;
    const std::string string = "!BF";
    data->setContent(make_shared< ::ndn::Buffer>(&string[0], string.size()));
  }
  else {
    std::string file = tmp_dir + "ndn-producer-cromo2Beacon_Node" + std::to_string(nodeId) + ".txt";
    std::ofstream out;
    out.open(file, std::fstream::app);
    auto *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());

    ::ndn::util::BloomFilter bf(BloomFilter_NumberOfHashFunctions, BloomFilter_size);
    //Reads directly the CS

    // if (dataName.getSubName(1,1).toUri() == "/beacon"){
      // const nfd::Cs& cs = node->GetObject<ndn::L3Protocol>()->getForwarder()->getCs();
      // auto timestamp = ns3::Simulator::Now().GetMilliSeconds();
      // if(printToFileProducerCromoBeacon)
        // std::cout << ns3::Simulator::Now().GetMilliSeconds() << std::endl;;

      if(printToFileProducerCromoBeacon)
        std::cout << "\nNumber of stored contents: " << cs.size() << std::endl;// getFullName

    // if(csSize > 0){
      for (nfd::Cs::const_iterator entry = cs.begin(); entry != cs.end(); entry++){
        if (entry->getName().getSubName(0,1).toUri() != "/localhost")
          bf.add(entry->getName().getSubName(0,6).toUri());// Do add Data to BF
      }

      auto encodedBf = bf.encodeAsBinaryBlock();
      data->setContent(encodedBf);
      // if(printToFileProducerCromoBeacon)
        // std::cout << "BF(not printed anymore) destination: Node_" << fromId << std::endl;
      // bf.printHex(); // Does not print it anymore
    // }
    // /** reset cout buffer **/
    std::cout.rdbuf(coutbuf);
  }


  ::ndn::Name identity("dummy");
  // ::ndn::Name identity("/cromo/prefix");

  // In the context of ndnSIM, KeyChain is a class that manages a key chain, which stores keys and certificates for use in signing and verifying packets.

  // KeyChain constructor has two optional arguments: pibLocator and tpmLocator. These arguments specify the location of the PIB (Public Information Base) and TPM (Trusted Platform Module) storage, respectively.

  // In the code snippet you provided, the KeyChain object is created with "pib-memory" and "tpm-memory" locators, which indicate that the key chain should be stored in memory. This means that the keys and certificates will only exist in the memory while the program is running and will not persist after the program terminates.

  // KeyChain keyChain("pib-memory", "tpm-memory");
  KeyChain keyChain("pib-dummy", "tpm-dummy");
  ::ndn::security::pib::Identity identityObj = keyChain.createIdentity(identity);

  keyChain.setDefaultIdentity(identityObj);

  // set prefix announcement
  // ::ndn::Name announcedName(identity);//dataName
  ::ndn::PrefixAnnouncement pa;
  pa.setAnnouncedName(identity);//announcedName
  pa.setExpiration(::ndn::time::milliseconds(10000));//::ndn::time::milliseconds(10000)


  // sign prefix announcement
  pa.toData(keyChain, ::ndn::security::signingWithSha256());
  lp::PrefixAnnouncementHeader pah(pa);

  data->setTag(make_shared<lp::PrefixAnnouncementTag>(pah));

  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());
  // std::cout << "node(" << GetNode()->GetId() << ") responding with Data: " << data->getName() << std::endl;

  // to create real wire encoding
  data->wireEncode();

  /** reset cout buffer **/
  // std::cout.rdbuf(coutbuf);

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
}


} // namespace ndn
} // namespace ns3
