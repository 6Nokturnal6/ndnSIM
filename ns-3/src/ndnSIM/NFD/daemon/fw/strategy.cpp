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

#include "strategy.hpp"
#include "forwarder.hpp"
#include "common/logger.hpp"

#include <ndn-cxx/lp/pit-token.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>


#include <ndn-cxx/lp/tags.hpp>


// Elidio
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/ndnSIM/model/map-common.hpp"
#include "model/ndn-l3-protocol.hpp"
#include "ns3/net-device.h"
#include "ns3/ndnSIM/model/ndn-net-device-transport.hpp"
#include "helper/ndn-fib-helper.hpp"
#include "ns3/names.h"

// #include "mobility-aware-strategy.hpp"
namespace nfd {
namespace fw {

NFD_LOG_INIT(Strategy);

Strategy::Registry&
Strategy::getRegistry()
{
  static Registry registry;
  return registry;
}

Strategy::Registry::const_iterator
Strategy::find(const Name& instanceName)
{
  const Registry& registry = getRegistry();
  ParsedInstanceName parsed = parseInstanceName(instanceName);

  if (parsed.version) {
    // specified version: find exact or next higher version

    auto found = registry.lower_bound(parsed.strategyName);
    if (found != registry.end()) {
      if (parsed.strategyName.getPrefix(-1).isPrefixOf(found->first)) {
        NFD_LOG_TRACE("find " << instanceName << " versioned found=" << found->first);
        return found;
      }
    }

    NFD_LOG_TRACE("find " << instanceName << " versioned not-found");
    return registry.end();
  }

  // no version specified: find highest version

  if (!parsed.strategyName.empty()) { // Name().getSuccessor() would be invalid
    auto found = registry.lower_bound(parsed.strategyName.getSuccessor());
    if (found != registry.begin()) {
      --found;
      if (parsed.strategyName.isPrefixOf(found->first)) {
        NFD_LOG_TRACE("find " << instanceName << " unversioned found=" << found->first);
        return found;
      }
    }
  }

  NFD_LOG_TRACE("find " << instanceName << " unversioned not-found");
  return registry.end();
}

bool
Strategy::canCreate(const Name& instanceName)
{
  return Strategy::find(instanceName) != getRegistry().end();
}

unique_ptr<Strategy>
Strategy::create(const Name& instanceName, Forwarder& forwarder)
{
  auto found = Strategy::find(instanceName);
  if (found == getRegistry().end()) {
    NFD_LOG_DEBUG("create " << instanceName << " not-found");
    return nullptr;
  }

  unique_ptr<Strategy> instance = found->second(forwarder, instanceName);
  NFD_LOG_DEBUG("create " << instanceName << " found=" << found->first
                << " created=" << instance->getInstanceName());
  BOOST_ASSERT(!instance->getInstanceName().empty());
  return instance;
}

bool
Strategy::areSameType(const Name& instanceNameA, const Name& instanceNameB)
{
  return Strategy::find(instanceNameA) == Strategy::find(instanceNameB);
}

std::set<Name>
Strategy::listRegistered()
{
  std::set<Name> strategyNames;
  boost::copy(getRegistry() | boost::adaptors::map_keys,
              std::inserter(strategyNames, strategyNames.end()));
  return strategyNames;
}

Strategy::ParsedInstanceName
Strategy::parseInstanceName(const Name& input)
{
  for (ssize_t i = input.size() - 1; i > 0; --i) {
    if (input[i].isVersion()) {
      return {input.getPrefix(i + 1), input[i].toVersion(), input.getSubName(i + 1)};
    }
  }
  return {input, nullopt, PartialName()};
}

Name
Strategy::makeInstanceName(const Name& input, const Name& strategyName)
{
  BOOST_ASSERT(strategyName.at(-1).isVersion());

  bool hasVersion = std::any_of(input.rbegin(), input.rend(),
                                [] (const name::Component& comp) { return comp.isVersion(); });
  return hasVersion ? input : Name(input).append(strategyName.at(-1));
}

Strategy::Strategy(Forwarder& forwarder)
  : afterAddFace(forwarder.m_faceTable.afterAdd)
  , beforeRemoveFace(forwarder.m_faceTable.beforeRemove)
  , m_forwarder(forwarder)
  , m_measurements(m_forwarder.getMeasurements(), m_forwarder.getStrategyChoice(), *this)
{
}

Strategy::~Strategy() = default;


void
Strategy::afterReceiveLoopedInterest(const FaceEndpoint& ingress, const Interest& interest,
                                     pit::Entry& pitEntry)
{
  NFD_LOG_DEBUG("afterReceiveLoopedInterest pitEntry=" << pitEntry.getName()
                << " in=" << ingress);
  // std::cout << "afterReceiveLoopedInterest pitEntry=" << pitEntry.getName()
                // << " in=" << ingress << std::endl;
}

void
Strategy::beforeSatisfyInterest(const shared_ptr<pit::Entry>& pitEntry,
                                const FaceEndpoint& ingress, const Data& data)
{
  NFD_LOG_DEBUG("beforeSatisfyInterest pitEntry=" << pitEntry->getName()
                << " in=" << ingress << " data=" << data.getName());
}

void
Strategy::afterContentStoreHit(const shared_ptr<pit::Entry>& pitEntry,
                               const FaceEndpoint& ingress, const Data& data)
{
  NFD_LOG_DEBUG("afterContentStoreHit pitEntry=" << pitEntry->getName()
                << " in=" << ingress << " data=" << data.getName());


  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // auto currentNodeId1 = currentNode->GetId ();

  // uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();

  // data.setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(currentNodeId1));
  // data.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(ingress.endpoint));

  // auto destId = data.getTag<lp::destIdTag>();
  // auto dest = broadcastId;
  // if(destId != nullptr){
  //   dest = destId->get();
  //   std::cout << "\t[Strategy::afterContentStoreHit]. Extracted DestId: " << dest << std::endl;
  // }

  // uint32_t fromId;
  // auto FromId = data.getTag<lp::fromIdTag>();
  // if(FromId != nullptr){
  //   fromId = FromId->get();
  //   std::cout << "\t[Strategy::afterContentStoreHit]. Extracted FromId: " << fromId << std::endl;
  // }
  // else{
  //   fromId = 666;// TODO
  //   std::cout << "[Strategy::afterContentStoreHit]. mobility-aware strategy... Interest. FromId nullptr. SET to 666" << std::endl;
  // }




      // redirecting std::cout to file
  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto currentNodeId1 = currentNode->GetId ();



  auto destId = data.getTag<lp::destIdTag>(); // Will be extracted from LSTM --- SET from the interest fromId Tag, in forwarder.cpp
  uint64_t dest;
  if(destId != nullptr){
    dest = destId->get();
    // if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted DestId: " << dest << std::endl;
  }
  // else
    // if(printToFileMobilityAwareStrategy) std::cout << "\tDestId NOT set!" << std::endl;


  uint64_t fromId;
  auto FromId = data.getTag<lp::fromIdTag>();
  if(FromId != nullptr){
    fromId = FromId->get();
    // if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted FromId: " << fromId << std::endl;
  }
  // else{
    // fromId = 99;// TODO
  // }


  uint64_t hopCount = 0;
  auto hopCountTag = data.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  else{
    NFD_LOG_DEBUG("Not possible to extract the hopCount from Data... ");
  }


  // if(printToFileMobilityAwareStrategy) std::cout << "Data found in CS - Data: " << data.getName().getSubName(0,5).toUri() << " - currentNodeId1: " << currentNodeId1 << " - destId: " << dest << " - fromId: " << fromId << " - hopCount: " << hopCount << std::endl;
  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // auto currentNodeId1 = currentNode->GetId ();

  Data data2 = data;
     // setting origId to Data packet
  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&currentNodeId1](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<1>(NodeIdMacMap) == (int) currentNodeId1;});
  if (it != NodeIdMacMap.end()) {
    data2.setOrigID(std::get<0>(*it));//std::get<0>(*it)
    DataOrigId = std::get<0>(*it);
  }
  else
    data2.setOrigID("00:FF:FF:FF:FF:FF");//std::get<0>(*it)

  destinationID = currentNodeId1;

  // std::cout << "Is this shit being executed?" << std::endl;
  data2.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(ingress.endpoint));
  data2.setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(currentNodeId1));
  this->sendData(pitEntry, data2, FaceEndpoint(ingress.face, dest)); //TODO Elidio... From ingress to FaceEndpoint(ingress.face, dest)
}



void
Strategy::afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
                           const FaceEndpoint& ingress, const Data& data)
{
  NFD_LOG_DEBUG("afterReceiveData pitEntry=" << pitEntry->getName()
                << " in=" << ingress << " data=" << data.getName());

  this->beforeSatisfyInterest(pitEntry, ingress, data);

  this->sendDataToAll(pitEntry, ingress, data);
}

void
Strategy::insertNeighborData(const FaceEndpoint& ingress, const ndn::util::Nmsi& nmsi, const uint32_t currentNodeId, const std::string& filename)//const pit::Entry& pitEntry,
{
  NFD_LOG_DEBUG(" in=" << ingress << " data=" << nmsi.getPrefix());

  // this->beforeSatisfyInterest(pitEntry, ingress, data);

  // this->sendDataToAll(pitEntry, ingress, data);
}

void
Strategy::insertNeighborInterest(const FaceEndpoint& ingress, const Interest& interest, const std::string& filename)//const pit::Entry& pitEntry,
{
  NFD_LOG_DEBUG(" in=" << ingress << " interest=" << interest.getName());

  // this->beforeSatisfyInterest(pitEntry, ingress, data);

  // this->sendDataToAll(pitEntry, ingress, data);
}

void
Strategy::afterReceiveNack(const FaceEndpoint& ingress, const lp::Nack& nack,
                           const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG("afterReceiveNack in=" << ingress << " pitEntry=" << pitEntry->getName());
}

void
Strategy::onDroppedInterest(const FaceEndpoint& egress, const Interest& interest)
{
  NFD_LOG_DEBUG("onDroppedInterest out=" << egress << " name=" << interest.getName());
}

void
Strategy::sendInterest(const shared_ptr<pit::Entry>& pitEntry,
                       const FaceEndpoint& egress, const Interest& interest)
{
  if (interest.getTag<lp::PitToken>() != nullptr) {
    Interest interest2 = interest; // make a copy to preserve tag on original packet
    interest2.removeTag<lp::PitToken>();
    // std::cout << "Chega aqui??? 1" << std::endl;
    m_forwarder.onOutgoingInterest(pitEntry, egress, interest2);
    return;
  }

  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // auto nodeId = currentNode->GetId ();

  // std::string Ffile = tmp_dir + "Strategy_sendInterest_" + std::to_string(nodeId) + ".txt";
  // std::ofstream out;
  // out.open(Ffile, std::fstream::app);

  // auto *coutbuf = std::cout.rdbuf();
  // std::cout.rdbuf(out.rdbuf());

  // std::cout << ns3::Simulator::Now().GetMilliSeconds();// << std::endl;
  // // std::cout << "egress.endpoint : " << egress.endpoint << std::endl;
  // std::cout << "CN: " << interest.getName().getSubName(0,4).toUri() << std::endl;




  // auto destId = interest.getTag<lp::destIdTag>(); // Will be extracted from LSTM
  // auto dest = 6662;
  // if(destId != nullptr){
  //   dest = destId->get();
  //   std::cout << "\t[Strategy::sendInterest]. Extracted DestId: " << dest << " - egress.endpoint: " << egress.endpoint << std::endl;
  // }

  // uint32_t fromId;
  // auto FromId = interest.getTag<lp::fromIdTag>();
  // if(FromId != nullptr){
  //   fromId = FromId->get();
  //   std::cout << "\t[Strategy::sendInterest]. Extracted FromId: " << fromId << std::endl;
  // }
  // else{
  //   fromId = 666;// TODO
  //   std::cout << "[Strategy::sendInterest]... Interest. FromId nullptr. SET to 666" << std::endl;
  // }


  // /** reset cout buffer **/
  // std::cout.rdbuf(coutbuf);

  // std::cout << "Chega aqui??? 2" << std::endl;
  m_forwarder.onOutgoingInterest(pitEntry, egress, interest);
}

void
Strategy::afterNewNextHop(const fib::NextHop& nextHop, const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG("afterNewNextHop pitEntry=" << pitEntry->getName()
                << " nexthop=" << nextHop.getFace().getId());

  // std::cout << "afterNewNextHop pitEntry=" << pitEntry->getName()
                // << " nexthop=" << nextHop.getFace().getId() << std::endl; // Never reaches here??
}

void
Strategy::sendData(const shared_ptr<pit::Entry>& pitEntry, const Data& data,
                   const FaceEndpoint& egress)
{
  BOOST_ASSERT(pitEntry->getInterest().matchesData(data));

  shared_ptr<lp::PitToken> pitToken;
  // auto inRecord = pitEntry->getInRecord(egress.face); // 20240325
  auto inRecord = pitEntry->getInRecord(egress.face, egress.endpoint); // Elidio: egress.endpoint

  if (inRecord != pitEntry->in_end()) {
    // std::cout << "sendData: inRecord.getFace()->getId(): " << inRecord->getFace().getId() << " - inRecord->getEndpointId(): " << inRecord->getEndpointId() << " - inRecord->getInterest().getName(): " << inRecord->getInterest().getName() << std::endl;
    pitToken = inRecord->getInterest().getTag<lp::PitToken>();
  }

  // delete the PIT entry's in-record based on egress,
  // since Data is sent to face and endpoint from which the Interest was received
  // pitEntry->deleteInRecord(egress.face); // 20240325
  pitEntry->deleteInRecord(egress.face, egress.endpoint); // Elidio: egress.endpoint


  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // auto nodeId = currentNode->GetId ();;

  // std::string Ffile = tmp_dir + "Strategy_sendData_" + std::to_string(nodeId) + ".txt";
  // std::ofstream out;
  // out.open(Ffile, std::fstream::app);

  // auto *coutbuf = std::cout.rdbuf();
  // std::cout.rdbuf(out.rdbuf());

  // std::cout << ns3::Simulator::Now().GetMilliSeconds();// << std::endl;
  // // std::cout << "egress.endpoint : " << egress.endpoint << std::endl;
  // std::cout << "CN: " << data.getName().getSubName(0,4).toUri() << std::endl;



  // auto destId = data.getTag<lp::destIdTag>(); // Will be extracted from LSTM
  // auto dest = 6662;
  // if(destId != nullptr){
  //   dest = destId->get();
  //   std::cout << "\t[Strategy::sendData]. Extracted DestId: " << dest << " - egress.endpoint: " << egress.endpoint << std::endl;
  // }

  // uint32_t fromId;
  // auto FromId = data.getTag<lp::fromIdTag>();
  // if(FromId != nullptr){
  //   fromId = FromId->get();
  //   std::cout << "\t[Strategy::sendData]. Extracted FromId: " << fromId << std::endl;
  // }
  // else{
  //   fromId = 666;// TODO
  //   std::cout << "[Strategy::sendData]... . FromId nullptr. SET to 666" << std::endl;
  // }

  // /** reset cout buffer **/
  // std::cout.rdbuf(coutbuf);




  // auto endpoint = egress.endpoint;
  if (pitToken != nullptr) {
    Data data2 = data; // make a copy so each downstream can get a different PIT token
    data2.setTag(pitToken);
    // Elidio. Setting the destIdTag
    // data2.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(endpoint));

    m_forwarder.onOutgoingData(data2, egress);
    return;
  }
  // Elidio. Setting the destIdTag
  // data.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(endpoint));
  m_forwarder.onOutgoingData(data, egress);

  if (pitEntry->getInRecords().empty()) { // if nothing left, "closing down" the entry
    // set PIT expiry timer to now
    m_forwarder.setExpiryTimer(pitEntry, 0_ms);

    // mark PIT satisfied
    pitEntry->isSatisfied = true;
  }
}

void
Strategy::sendDataToAll(const shared_ptr<pit::Entry>& pitEntry,
                        const FaceEndpoint& ingress, const Data& data)
{
  std::set<std::pair<Face*, EndpointId>> pendingDownstreams; // Elidio: std::set<Face*> to ...
  auto now = time::steady_clock::now();

  // remember pending downstreams
  for (const pit::InRecord& inRecord : pitEntry->getInRecords()) {
    if (inRecord.getExpiry() > now) {
      if (inRecord.getFace().getId() == ingress.face.getId() &&
          inRecord.getFace().getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) {
        continue;
      }
// 	  uint64_t& endpointId = inRecord.getEndpointId(); (To be corrected)
      pendingDownstreams.emplace(&inRecord.getFace(), inRecord.getEndpointId());// Elidio: inRecord.getEndpointId()
    }
  }

  for (const auto& pendingDownstream : pendingDownstreams) {
    data.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(pendingDownstream.second)); // Elidio For each data<->endpoint
    this->sendData(pitEntry, data, FaceEndpoint(*pendingDownstream.first, pendingDownstream.second)); // Elidio: from *pendingDownstream to ...
  }
}

void
Strategy::sendNacks(const shared_ptr<pit::Entry>& pitEntry, const lp::NackHeader& header,
                    std::initializer_list<FaceEndpoint> exceptFaceEndpoints)
{
  // populate downstreams with all downstreams faces
  std::set<Face*> downstreams;
  std::transform(pitEntry->in_begin(), pitEntry->in_end(), std::inserter(downstreams, downstreams.end()),
                 [] (const pit::InRecord& inR) {
                  return &inR.getFace();
                 });

  // delete excluded faces
  for (const auto& exceptFaceEndpoint : exceptFaceEndpoints) {
    downstreams.erase(&exceptFaceEndpoint.face);
  }

  // send Nacks
  for (const auto& downstream : downstreams) {
    this->sendNack(pitEntry, FaceEndpoint(*downstream, 0), header);
  }
  // warning: don't loop on pitEntry->getInRecords(), because in-record is deleted when sending Nack
}

const fib::Entry&
Strategy::lookupFib(const pit::Entry& pitEntry) const
{
  const Fib& fib = m_forwarder.getFib();

  const Interest& interest = pitEntry.getInterest();


  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto printFib = [](ns3::Ptr<ns3::Node> node) {
    auto ndn = node->GetObject<ns3::ndn::L3Protocol>();
    for (const auto& entry : ndn->getForwarder()->getFib()) {
      // std::cout << entry.getPrefix() << " (";

      bool isFirst = true;
      for (auto& nextHop : entry.getNextHops()) {
        // std::cout << nextHop.getFace();
        auto& face = nextHop.getFace();
        auto transport = dynamic_cast<ns3::ndn::NetDeviceTransport*>(face.getTransport());
        if (transport == nullptr) {
          // std::cout << "Hmmm..." << std::endl;
          continue;
        }

        // std::cout << " Strategy-endpointId: " << nextHop.getEndpointId() << " - Timestamp: " << nextHop.getTimestamp() << " - Hops: " << nextHop.getHops() << " - Px: " << nextHop.getPx() << " - Py: " << nextHop.getPy() << " - Vx: " << nextHop.getVx() << " - Vy: " << nextHop.getVy();

        if (!isFirst)
          // std::cout << ", ";
        // std::cout << ns3::Names::FindName(transport->GetNetDevice()->GetChannel()->GetDevice(1)->GetNode());
        isFirst = false;
      }
      // std::cout << ")" << std::endl;
    }
  };

  // std::cout << "Strategy.. FIB content on node_" << node->GetId() << std::endl;
  // NodeToAdd = ns3::NodeList::GetNode(currentNodeId1);
  // printFib(node); // <-- This seams OK.


  // std::cout << "Hey!!!! interest: " << interest.getName() << std::endl;
  // has forwarding hint?
  if (interest.getForwardingHint().empty()) {
    // FIB lookup with Interest name
    // const fib::Entry& fibEntry = fib.findLongestPrefixMatch(pitEntry);
    const fib::Entry& fibEntry = fib.findLongestPrefixMatch(pitEntry.getName().getSubName(0, 4));
    NFD_LOG_TRACE("lookupFib noForwardingHint found=" << fibEntry.getPrefix());
    // std::cout << "lookupFib noForwardingHint found=" << fibEntry.getPrefix() << std::endl;
    // std::cout << "lookupFib noForwardingHint found Timestamp = " << fibEntry.hasNextHops() << " - prefix: " << interest.getName().getSubName(0, 6) << std::endl; //fibEntry.getNextHops().back().getTimestamp()

    // for(auto& nxh: fibEntry.getNextHops()){
    //   std::cout << "[Strategy::lookupFib]. YEAP .. Face: " << nxh.getFace().getId() << "  -Endpoint: " << nxh.getEndpointId() << " - Timestamp: " << nxh.getTimestamp() << " - Hops: " << nxh.getHops() << " - Px: " << nxh.getPx() << " - Py: " << nxh.getPy() << " - Vx: " << nxh.getVx() << " - Vy: " << nxh.getVy() << " --" << fibEntry.getPrefix() << std::endl;
    // }

    return fibEntry;
  }

  const DelegationList& fh = interest.getForwardingHint();
  // Forwarding hint should have been stripped by incoming Interest pipeline when reaching producer region
  BOOST_ASSERT(!m_forwarder.getNetworkRegionTable().isInProducerRegion(fh));

  const fib::Entry* fibEntry = nullptr;
  for (const Delegation& del : fh) {
    fibEntry = &fib.findLongestPrefixMatch(del.name);
    if (fibEntry->hasNextHops()) {
      if (fibEntry->getPrefix().size() == 0) {
        // in consumer region, return the default route
        NFD_LOG_TRACE("lookupFib inConsumerRegion found=" << fibEntry->getPrefix());
        // std::cout << "lookupFib inConsumerRegion found=" << fibEntry->getPrefix() << std::endl;
      }
      else {
        // in default-free zone, use the first delegation that finds a FIB entry
        NFD_LOG_TRACE("lookupFib delegation=" << del.name << " found=" << fibEntry->getPrefix());
      }
      // std::cout << "lookupFib other found=" << fibEntry->getPrefix() << std::endl;
      return *fibEntry;
    }
    BOOST_ASSERT(fibEntry->getPrefix().size() == 0); // only ndn:/ FIB entry can have zero nexthop
  }
  BOOST_ASSERT(fibEntry != nullptr && fibEntry->getPrefix().size() == 0);
  // std::cout << "lookupFib no delegation found=" << fibEntry->getPrefix() << std::endl;
  return *fibEntry; // only occurs if no delegation finds a FIB nexthop
}

} // namespace fw
} // namespace nfd
