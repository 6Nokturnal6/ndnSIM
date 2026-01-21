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

#include "forwarder.hpp"

#include "algorithm.hpp"
#include "best-route-strategy2.hpp"
#include "strategy.hpp"
#include "common/global.hpp"
#include "common/logger.hpp"
#include "table/cleanup.hpp"

#include <ndn-cxx/lp/tags.hpp>

#include "face/null-face.hpp"

// Elidio: For node position
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"

/**** For getting Node ID ****/
#include <ns3/simulator.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
// #include <tuple>
#include <algorithm> // std::find_if


#include <iostream>
#include <fstream>
#include <cstdio>


//

#include "ndn-cxx/lp/speed-tag.hpp"
#include "ndn-cxx/lp/geo-tag.hpp"
#include "ndn-cxx/util/tools.hpp"
#include "ndn-cxx/util/time.hpp"
//
#include "mobility-aware-strategy.hpp"
#include "ns3/ndnSIM/model/map-common.hpp"
#include "ns3/random-variable-stream.h"


#include "model/ndn-l3-protocol.hpp"
#include "ns3/net-device.h"
#include "ns3/ndnSIM/model/ndn-net-device-transport.hpp"
// #include <filesystem>

// extern std::string tmp_dir;
::ndn::util::Tools tools;
// uint64_t nodeNum = tools.GetNodeNum (tmp_dir);
// uint64_t numberOfNodes = nodeNum;

std::vector<std::tuple<std::string, int>> NodeIdMacMap;
uint64_t nodeNum = 0;
std::string tmp_dir;
float FIBManagementTimer = 500; // ms
// bool invokeConsumerClient = false;
// bool invokeConsumerCromo = false; // for cromo and beacon
// bool invokeFromClient = false;

// uint64_t nodeNum = tools.GetNodeNum (tmp_dir);
namespace nfd {

NFD_LOG_INIT(Forwarder);

static Name
getDefaultStrategyName()
{
  return fw::BestRouteStrategy2::getStrategyName();
}

Forwarder::Forwarder(FaceTable& faceTable)
  : m_faceTable(faceTable)
  , m_unsolicitedDataPolicy(make_unique<fw::DefaultUnsolicitedDataPolicy>())// redifined to admitall > Elidio
  , m_fib(m_nameTree)
  , m_pit(m_nameTree)
  , m_measurements(m_nameTree)
  , m_strategyChoice(*this)
  , m_csFace(face::makeNullFace(FaceUri("contentstore://")))
  /* Elidio */
  , m_Fibtimer (ns3::Timer::CANCEL_ON_DESTROY)
//   , m_lcctFace(face::makeNullFace(FaceUri("lcctcontentstore://")))
{
  m_faceTable.addReserved(m_csFace, face::FACEID_CONTENT_STORE);
  /* Elidio */
//   m_faceTable.addReserved(m_lcctFace, face::FACEID_LCCT_CONTENT_STORE);

  m_faceTable.afterAdd.connect([this] (const Face& face) {
    face.afterReceiveInterest.connect(
      [this, &face] (const Interest& interest, const EndpointId& endpointId) {
          uint64_t endpoint;
          auto fromId = interest.getTag<lp::fromIdTag>();
          if(fromId != nullptr){
            endpoint = fromId->get();
            //
            // if (endpoint == nodeId){
            //   // std::cout << endpoint << " = " << nodeId << "Elidio INTEREST OUT " << std::endl;
            //   return;
            // }
          }
          else{
            // std::cout << "[Forwarder::onIncomingInterest]. fromIdTag NOT set! Default 99, set." << std::endl;
            NFD_LOG_DEBUG("[Forwarder::onIncomingInterest]. fromIdTag NOT set! Default 99, set.");
            endpoint = 99;// TODO
          }

        this->startProcessInterest(FaceEndpoint(face, endpoint), interest);
      });
    face.afterReceiveData.connect(
      [this, &face] (const Data& data, const EndpointId& endpointId) {

        uint64_t endpoint;
        auto fromId = data.getTag<lp::fromIdTag>();
        if(fromId != nullptr){
          endpoint = fromId->get();
          //
          // if (endpoint == nodeId){
          //   // std::cout << endpoint << " = " << nodeId << "Elidio INTEREST OUT " << std::endl;
          //   return;
          // }
        }
        else{
          // std::cout << "[Forwarder::onIncomingInterest]. fromIdTag NOT set! Default 99, set." << std::endl;
          NFD_LOG_DEBUG("[Forwarder::onIncomingData]. fromIdTag NOT set! Default 99, set.");
          endpoint = 99;// TODO
        }

        this->startProcessData(FaceEndpoint(face, endpoint), data);//endpoint
      });
    face.afterReceiveNack.connect(
      [this, &face] (const lp::Nack& nack, const EndpointId& endpointId) {
        this->startProcessNack(FaceEndpoint(face, endpointId), nack);
      });
    face.onDroppedInterest.connect(
      [this, &face] (const Interest& interest) {
          uint64_t endpoint;
          auto fromId = interest.getTag<lp::fromIdTag>();
          if(fromId != nullptr){
            endpoint = fromId->get();
            //
            // if (endpoint == nodeId){
            //   // std::cout << endpoint << " = " << nodeId << "Elidio INTEREST OUT " << std::endl;
            //   return;
            // }
          }
          else{
            // std::cout << "[Forwarder::onIncomingInterest]. fromIdTag NOT set! Default 99, set." << std::endl;
            NFD_LOG_DEBUG("[Forwarder::onIncomingInterest]. fromIdTag NOT set! Default 99, set.");
            endpoint = 99;// TODO
          }

        this->onDroppedInterest(FaceEndpoint(face, endpoint), interest);// from 0 to endpoint
      });
  });

  m_faceTable.beforeRemove.connect([this] (const Face& face) {
    cleanupOnFaceRemoval(m_nameTree, m_fib, m_pit, face);
  });

  m_fib.afterNewNextHop.connect([&] (const Name& prefix, const fib::NextHop& nextHop) {
    this->startProcessNewNextHop(prefix, nextHop);
  });

  m_strategyChoice.setDefaultStrategy(getDefaultStrategyName());
  // std::cout << "[Forwarder] tmp_dir: " << tmp_dir << " nodeNum:" << nodeNum << std::endl;

  // Extract the parent folder path
  // std::size_t lastSlashPos = tmp_dir.find_last_of('/');
  // std::string parentTmp_dir = tmp_dir.substr(0, lastSlashPos - 3);

  std::size_t lastSlashPos = tmp_dir.find_last_of('/');
  std::size_t secondToLastSlashPos = tmp_dir.find_last_of('/', lastSlashPos - 1);
  std::string parentTmp_dir = tmp_dir.substr(0, secondToLastSlashPos + 1);

  std::string filePath = parentTmp_dir + "NodeIdMacMapping.txt";
  NFD_LOG_DEBUG("[Forwarder]. NodeIdMacMapping filePath: " << filePath);
  // std::cout << "[Forwarder] filePath: " << filePath << std::endl;
  tools.getNodeIdMacMapContent(filePath, NodeIdMacMap);

  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  m_Fibtimer.SetDelay (ns3::MilliSeconds(FIBManagementTimer)); // 15s
  m_Fibtimer.SetFunction (&Forwarder::FIBManagement, this);
  // m_Fibtimer.SetArguments (currentNode, tmp_dir + "DeleteFIB_Node_" + std::to_string(currentNodeId1) + ".txt", false);
  m_Fibtimer.SetArguments (currentNode, false);
  m_Fibtimer.Schedule();
  // std::cout << "Evoked from Constructor..." << std::endl;
}

Forwarder::~Forwarder() = default;


void
Forwarder::onIncomingInterest(const FaceEndpoint& ingress, const Interest& interest)
{
  auto node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto nodeId = node->GetId();

  uint64_t hopCount = 0;
  auto hopCountTag = interest.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  // std::cout << "hopCount:  " << hopCount << std::endl;

  uint64_t fromId = 1000; // Invalid node ID
  auto FromId = interest.getTag<lp::fromIdTag>();
  if(FromId != nullptr)
    fromId = *FromId;//FromId->get();
  // else
  //   std::cout << "forwarder.OnIncomingInterest. Invalid fromId... Node:  " << nodeId << std::endl;


  // if(fromId == nodeId){
  //   std::cout << "Exiting from Forwarder::onIncomingInterest... fromId: " << fromId << " - nodeId: " << nodeId << std::endl;
  //   return;
  // }

  // static uint64_t control = 0;
  // if((interest.getName().getSubName(2,1) == "/sharingBF") && (fromId != nodeId)){
  //   // This CN is only issued for retrieving the BF for updating CCT. Only the node who sends the interest care about it
  //   // std::cout << "nterest.getName().getSubName(2,1): " << interest.getName().getSubName(2,1) << " ...getSubName(3,1).toUri(): " << interest.getName().getSubName(3,1).toUri() << " - std::to_string(nodeId): " << std::to_string(nodeId) << std::endl;
  //   // std::cout << "hopCount:  " << hopCount << "-" << interest.getName().getSubName(3,1) << std::endl;
  //   return;
  // }




  // if(fromId == nodeId){//(hopCount == 1) && (fromId == nodeId)
  //   // Elidio. Discards received interest from this same node
  //   NFD_LOG_DEBUG("onIncomingInterest from the same node, descarded... Prefix: " << interest.getName());
  //   // >Does not display! std::cout << "Discarded Interest: " << interest.getName() << " with hopCount: " << hopCount << " fromId: "<< fromId << " nodeId" << nodeId << std::endl;
  //   return;
  // }

  // receive Interest
  NFD_LOG_DEBUG("onIncomingInterest in=" << ingress << " interest=" << interest.getName() << " node: " << nodeId);// Elidio: ADDED nodeId
  // std::cout << "Forwarder:onIncomingInterest in = " << ingress << " interest = " << interest.getName() << " node: " << nodeId << " - ingress.face.getId(): " << ingress.face.getId() << std::endl;
  interest.setTag(make_shared<lp::IncomingFaceIdTag>(ingress.face.getId()));
  ++m_counters.nInInterests;

  // if (interest.getName().getSubName(1,1) == "/sharingBF")
    // std::cout << "onIncomingInterest in=" << ingress << " interest=" << interest.getName() << " node: " << nodeId << " - hopCount: " << hopCount << std::endl;

  // /localhost scope control
  bool isViolatingLocalhost = ingress.face.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL &&
                              scope_prefix::LOCALHOST.isPrefixOf(interest.getName());
  if (isViolatingLocalhost) {
    NFD_LOG_DEBUG("onIncomingInterest in=" << ingress
                  << " interest=" << interest.getName() << " violates /localhost");
    // (drop)
    return;
  }

  // detect duplicate Nonce with Dead Nonce List
  bool hasDuplicateNonceInDnl = m_deadNonceList.has(interest.getName(), interest.getNonce());
  if (hasDuplicateNonceInDnl) {
    // goto Interest loop pipeline
    this->onInterestLoop(FaceEndpoint(ingress.face, ingress.endpoint), interest);//ingress.endpoint
    return;
  }

  // strip forwarding hint if Interest has reached producer region
  if (!interest.getForwardingHint().empty() &&
      m_networkRegionTable.isInProducerRegion(interest.getForwardingHint())) {
    NFD_LOG_DEBUG("onIncomingInterest in=" << ingress
                  << " interest=" << interest.getName() << " reaching-producer-region");
    const_cast<Interest&>(interest).setForwardingHint({});
  }


  // PIT insert
  /*Elidio: *first-> the method returns only the pitEntry. The insert returns the  [std::pair<shared_ptr<Entry>, bool>]*/
  shared_ptr<pit::Entry> pitEntry = m_pit.insert(interest).first;


  // detect duplicate Nonce in PIT entry
  int dnw = fw::findDuplicateNonce(*pitEntry, interest.getNonce(), ingress.face);// Elidio. 20221106. instead of face, it should be endpointId!!...
  bool hasDuplicateNonceInPit = dnw != fw::DUPLICATE_NONCE_NONE;
//   std::cout <<" transporte " << ingress.face.getTransport() << std::endl;
  if (ingress.face.getLinkType() == ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    // for p2p face: duplicate Nonce from same incoming face is not loop
    hasDuplicateNonceInPit = hasDuplicateNonceInPit && !(dnw & fw::DUPLICATE_NONCE_IN_SAME);
  }
  if (hasDuplicateNonceInPit) {
    // goto Interest loop pipeline
    this->onInterestLoop(FaceEndpoint(ingress.face, ingress.endpoint), interest);// changed from ingress to FaceEndpoint(ingress.face, endpoint)
    this->dispatchToStrategy(*pitEntry,
      [&] (fw::Strategy& strategy) { strategy.afterReceiveLoopedInterest(FaceEndpoint(ingress.face, ingress.endpoint), interest, *pitEntry); }); // Elidio. 20221106. Instead of face, it should be endpointId!!...
    return;
  }


  // is pending?
  if (!pitEntry->hasInRecords()) {
    m_cs.find(interest,
              bind(&Forwarder::onContentStoreHit, this, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry, _1, _2), /*TODO - changed from ingress to endpointId*/
              bind(&Forwarder::onContentStoreMiss, this, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry, _1)); /*TODO - changed from ingress to FaceEndpoint(ingress.face, endpoint)*/
  }
  else {
    // std::cout << "Delete... ::: " << ingress.endpoint << std::endl;
    this->onContentStoreMiss(FaceEndpoint(ingress.face, ingress.endpoint), pitEntry, interest); /*TODO - changed from ingress to FaceEndpoint(ingress.face, endpoint)*/
  }
}

void
Forwarder::onInterestLoop(const FaceEndpoint& ingress, const Interest& interest)
{
  // if multi-access or ad hoc face, drop
  if (ingress.face.getLinkType() != ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    NFD_LOG_DEBUG("onInterestLoop in=" << ingress
                  << " interest=" << interest.getName() << " drop");
    return;
  }

  NFD_LOG_DEBUG("onInterestLoop in=" << ingress << " interest=" << interest.getName()
                << " send-Nack-duplicate");

  // send Nack with reason=DUPLICATE
  // note: Don't enter outgoing Nack pipeline because it needs an in-record.
  lp::Nack nack(interest);
  nack.setReason(lp::NackReason::DUPLICATE);
  ingress.face.sendNack(nack, ingress.endpoint);

}

void
Forwarder::onContentStoreMiss(const FaceEndpoint& ingress,/* //TODO ATTENTION here*/
                              const shared_ptr<pit::Entry>& pitEntry, const Interest& interest)
{
  NFD_LOG_DEBUG("onContentStoreMiss interest=" << interest.getName());
  ++m_counters.nCsMisses;
  afterCsMiss(interest);

  auto NewFromId = interest.getTag<lp::fromIdTag>();
  // // auto FromIdInterest = interest.getTag<lp::fromIdTag>();
  // data.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(*interest.getTag<lp::fromIdTag>()));
  // auto NewDestId = data.getTag<lp::destIdTag>();

  // std::cout << "ingress.endpoint:" << ingress.endpoint << " *NewFromId: " << *NewFromId << std::endl;

  // insert in-record

  // int hopCount = 0;
  // auto hopCountTag = interest.getTag<lp::HopCountTag>();
  // if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
  //   hopCount = *hopCountTag;
  // }
  //
  // if (hopCount > 0)// Elidio... 20240313
  pitEntry->insertOrUpdateInRecord(ingress.face, *NewFromId, interest); // Elidio: ingress.endpoint to endpoit (FromId) -- *NewFromId

  // set PIT expiry timer to the time that the last PIT in-record expires
  auto lastExpiring = std::max_element(pitEntry->in_begin(), pitEntry->in_end(),
                                       [] (const auto& a, const auto& b) {
                                         return a.getExpiry() < b.getExpiry();
                                       });
  auto lastExpiryFromNow = lastExpiring->getExpiry() - time::steady_clock::now();
  this->setExpiryTimer(pitEntry, time::duration_cast<time::milliseconds>(lastExpiryFromNow));


  auto destId = interest.getTag<lp::destIdTag>(); // Will be extracted from LSTM --- SET from the interest fromId Tag, in forwarder.cpp
  uint64_t dest;
  if(destId != nullptr){
    dest = destId->get();
  }
  else{
    dest = 666;
    // std::cout << "\tDestId NOT set!. Setting it to 666 from the forwarder afterCsMiss" << std::endl;
  }


  // has NextHopFaceId?
  auto nextHopTag = interest.getTag<lp::NextHopFaceIdTag>();
  if (nextHopTag != nullptr) {
    // chosen NextHop face exists?
    Face* nextHopFace = m_faceTable.get(*nextHopTag);
    if (nextHopFace != nullptr) {
      NFD_LOG_DEBUG("onContentStoreMiss interest=" << interest.getName()
                    << " nexthop-faceid=" << nextHopFace->getId());
      // go to outgoing Interest pipeline
      // scope control is unnecessary, because privileged app explicitly wants to forward
      this->onOutgoingInterest(pitEntry, FaceEndpoint(*nextHopFace, dest), interest);// Elidio: ingress.endpoint to endpoint to *NewFromId - 20240330: 666
    }
    return;
  }

  // dispatch to strategy: after incoming Interest
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) {
      // std::cout << "Dispatching to afterreceive from afterCsMiss: " << interest.getName() << std::endl;
      strategy.afterReceiveInterest(FaceEndpoint(ingress.face, dest), interest, pitEntry);// Elidio: ingress.endpoint to endpoint
    });
}

void
Forwarder::onContentStoreHit(const FaceEndpoint& ingress, const shared_ptr<pit::Entry>& pitEntry,
                             const Interest& interest, const Data& data)
{
  NFD_LOG_DEBUG("onContentStoreHit interest=" << interest.getName());
  ++m_counters.nCsHits;
  afterCsHit(interest, data);

  data.setTag(make_shared<lp::IncomingFaceIdTag>(face::FACEID_CONTENT_STORE)); // Elidio See this //TODO Elidio. new NMSI parameters


  // FIXME Should we lookup PIT for other Interests that also match the data?

  pitEntry->isSatisfied = true;
  pitEntry->dataFreshnessPeriod = data.getFreshnessPeriod();
  //
  // // set PIT expiry timer to now
  this->setExpiryTimer(pitEntry, 0_ms);


  beforeSatisfyInterest(*pitEntry, *m_csFace, data);
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.beforeSatisfyInterest(pitEntry, FaceEndpoint(*m_csFace, ingress.endpoint), data); }); // Elidio  0 to ingress.endpoint

  // dispatch to strategy: after Content Store hit


  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto currentNodeId1 = currentNode->GetId ();


  // auto FromId = data.getTag<lp::fromIdTag>();
  // auto DestId = data.getTag<lp::destIdTag>();
  // std::cout << "\tBefore Setting -> Data: " << data.getName().getSubName(0,5).toUri() << " from CS. fromId: " << *FromId << " destId: " << *DestId << " Node_" << currentNodeId1 << std::endl;





  auto NewFromId = data.getTag<lp::fromIdTag>();
  // auto FromIdInterest = interest.getTag<lp::fromIdTag>();
  auto dest = *interest.getTag<lp::fromIdTag>();
  data.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(dest));
  // data.setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(currentNodeId1));
  auto NewDestId = data.getTag<lp::destIdTag>();

  // auto hopCountTag = data.getTag<lp::HopCountTag>();


  // std::cout << "\tAfter Setting -> Data: " << data.getName().getSubName(0,5).toUri() << " from CS. fromId: " << *NewFromId << " destId: " << *NewDestId << " Node_" << currentNodeId1 << " ingress.endpoint: " << ingress.endpoint << std::endl;

  data.setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(currentNodeId1));
  // this->dispatchToStrategy(*pitEntry,
  //   [&] (fw::Strategy& strategy) { strategy.afterContentStoreHit(pitEntry, FaceEndpoint(ingress.face, dest), data); });
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.afterContentStoreHit(pitEntry, FaceEndpoint(ingress.face, dest), data); });
}

void
Forwarder::onOutgoingInterest(const shared_ptr<pit::Entry>& pitEntry,
                              const FaceEndpoint& egress, const Interest& interest)
{
  NFD_LOG_DEBUG("onOutgoingInterest out=" << egress << " interest=" << pitEntry->getName());


  auto NewFromId = interest.getTag<lp::fromIdTag>();

  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto nodeId = currentNode->GetId ();

  // insert out-record
  pitEntry->insertOrUpdateOutRecord(egress.face, nodeId, interest);// Elidio TODO -- egress.endpoint -- 0 to currentNodeId1 --- nodeId to *NewFromId

  // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(egress.endpoint));
  // std::cout << "currentNodeId1: " << nodeId << " - egress.endpoint: " << egress.endpoint << std::endl;
  // send Interest
  egress.face.sendInterest(interest, egress.endpoint);
  ++m_counters.nOutInterests;
}

void
Forwarder::onInterestFinalize(const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG("onInterestFinalize interest=" << pitEntry->getName()
                << (pitEntry->isSatisfied ? " satisfied" : " unsatisfied"));

  if (!pitEntry->isSatisfied) {
    beforeExpirePendingInterest(*pitEntry);
  }

  // Dead Nonce List insert if necessary
  this->insertDeadNonceList(*pitEntry, nullptr);

  // Increment satisfied/unsatisfied Interests counter
  if (pitEntry->isSatisfied) {
    ++m_counters.nSatisfiedInterests;
  }
  else {
    ++m_counters.nUnsatisfiedInterests;
  }

  // PIT delete
  pitEntry->expiryTimer.cancel();
  m_pit.erase(pitEntry.get());
}

void
Forwarder::onIncomingData(const FaceEndpoint& ingress, const Data& data)
{
  auto node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  uint64_t nodeId = node->GetId();

  uint64_t hopCount = 0;
  auto hopCountTag = data.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }

  uint64_t fromId = 1000; // Invalid node ID
  auto FromId = data.getTag<lp::fromIdTag>();
  if(FromId != nullptr){
    fromId = *FromId;
  }
  // else{
  //   std::cout << "Data without fromId!" << std::endl;
  // }

  uint64_t dest;
  auto destId = data.getTag<lp::destIdTag>(); // Will be extracted from LSTM
  if(destId != nullptr){
    dest = destId->get();
  }
  else{
    destId = 0; // Just for precaution!
  }

  uint64_t timestamp1 = 0;
  auto timestamp = data.getTag<lp::timestampTag>();
  if(timestamp != nullptr){
    timestamp1 = timestamp->get();
  }
  else
    timestamp1 = 0; // Just for precaution!

    //
  // ndn::util::Tools tools;

  double vx_gt = 0.0;
  double vy_gt = 0.0;
  double vz_gt = 0.0;
  std::tie(vx_gt, vy_gt, vz_gt) = tools.GetNodeSpeedFromDataPacket(data);
  ndn::util::NodeSpeed nodespeed(vx_gt, vy_gt);

  double px = 0.0;
  double py = 0.0;
  double pz = 0.0;
  std::tie(px, py, pz) = tools.GetNodeLocationFromDataPacket(data);
  ndn::util::GeoPosition geoposition(px, py, pz);

  std::string prefix = data.getName().getSubName(0,5).toUri();//

  // receive Data
  NFD_LOG_DEBUG("onIncomingData in=" << ingress << " data=" << data.getName().toUri() << " node: " << nodeId);// Elidio: ADDED nodeId
  // std::cout << "onIncomingData in=" << ingress << " data=" << data.getName().toUri() << " node: " << nodeId << std::endl;// Elidio: ADDED nodeId
  // if(data.getName().getSubName(0,1) != "/localhost") std::cout << "onIncomingData in=" << ingress << " data=" << data.getName().toUri() << " node: " << nodeId << std::endl;
  data.setTag(make_shared<lp::IncomingFaceIdTag>(ingress.face.getId()));
  ++m_counters.nInData;

  // /localhost scope control
  bool isViolatingLocalhost = ingress.face.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL &&
                              scope_prefix::LOCALHOST.isPrefixOf(data.getName());
  if (isViolatingLocalhost) {
    NFD_LOG_DEBUG("onIncomingData in=" << ingress << " data=" << data.getName().toUri() << " violates /localhost");
    // std::cout << "onIncomingData in=" << ingress << " data=" << data.getName().toUri() << " violates /localhost" << std::endl;
    // (drop)
    return;
  }

    // Add route to FIB 20240320
  // auto NodeToAdd = ns3::NodeList::GetNode(fromId);
  ns3::Ptr<ns3::ndn::L3Protocol> ndn = node->GetObject<ns3::ndn::L3Protocol>();
  shared_ptr<Face> face = ndn->getFaceById(ingress.face.getId());
  // ns3::Ptr<Fib> fib_ = &ndn->getForwarder()->getFib();

  // std::cout << "ingress.face.getId()::: " << ingress.face.getId() << std::endl; -------------  !!!!!!!!!!! 20240404
  if ((ingress.face.getId() == 257) || (ingress.face.getId() == 261) || (ingress.face.getId() == 256) /*&& (data.getName().getSubName(3,1).toUri() == "/consumer")*/){
    auto ndn = node->GetObject<::ns3::ndn::L3Protocol>();
    // auto&& entry = ndn->getForwarder()->getFib().findLongestPrefixMatch(data.getName().getSubName(0,6).toUri());
    // std::cout << "[FibHelper::AddRoute]. entry.getPrefix(): " << entry.getPrefix() << " - data.getName(): " << data.getName() << " - Fib.size(): " << ndn->getForwarder()->getFib().size() << " -- Node_" << node->GetId() << std::endl; //--> This is ok with/without RSU as consumer...
    if(1/*(entry.getPrefix().getSubName(0,6).toUri() != "/") && (entry.getPrefix().getSubName(0,6).toUri() != "/cromo/test")*/){
      // ndn->getForwarder()->getFib().addOrUpdateNextHop(entry, ingress.face, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 60);
      // entry.addOrUpdateNextHop(ingress.face, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 60);
      ::ns3::ndn::FibHelper::AddRoute(node, fromId, timestamp1, hopCount, px, py, vx_gt, vy_gt, data.getName().getSubName(0, 5), face, hopCount); // <---------------------------------------------- Uncomment this
      // std::cout << "... Tree 1... entry.getPrefix().getSubName(0,6).toUri(): " << entry.getPrefix().getSubName(0,6).toUri() << std::endl;


      // ::ns3::ndn::StackHelper::Update(node, true);
      ::ns3::ndn::FibHelper::RemoveRoute(node, "/", face, nodeNum); // Route to RSU with prefix "/", inserted from stackHelper
      // std::cout << "A apagar no com id: " << nodeNum << std::endl;
      shared_ptr<Face> face2 = ndn->getFaceById(259); // appFace://
      Name namesharingBF("/cromo/sharingBF");
      ::ns3::ndn::FibHelper::RemoveRoute(node, namesharingBF, face2, 0);
      // ::ns3::ndn::FibHelper::RemoveRoute(node, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, namesharingBF, face2, 0); //20241111

      // // From: Producer-cromo2.cpp
      // shared_ptr<Face> face3 = ndn->getFaceById(261); // appFace://
      // ::ns3::ndn::FibHelper::RemoveRoute(node, "/cromo/test/", face3, 999);
    }
  }

  // PIT match
  pit::DataMatchResult pitMatches = m_pit.findAllDataMatches(data);
  if (pitMatches.size() == 0) {
    // std::cout << "Sending to the unsolicited pipeline CN: " << data.getName().getSubName(0,5).toUri() << std::endl;
    NFD_LOG_DEBUG("Sending to the unsolicited pipeline CN: " << data.getName().getSubName(0,5).toUri());
    this->onDataUnsolicited(ingress, data);
    return;
  }

  // Unsolicited beacon (such as /cromo/beacon/sharingBF/...) does not reach here!!! --->> (data.getName().getSubName(0,1).toUri() != "/localhost") && --- (data.getName().getSubName(1,1).toUri() != "/beacon") &&
  if ((data.getName().getSubName(1,1).toUri() != "/active-safety")) // Eidio. Active-safety Data are not cached! ----  && (data.getName().getSubName(1,1).toUri() != "/sharingBF")
  {
    // std::cout << "Storing in CS CN: " << data.getName().getSubName(0,5).toUri() << std::endl;
    ndn::util::Nmsi nmsi(fromId, prefix, geoposition, nodespeed, hopCount, timestamp1);
    //   CS insert
    m_cs.insert(data, nmsi); // Elidio. Included NMSI
  }

  // // // Add route to FIB 20240320
  // // auto NodeToAdd = ns3::NodeList::GetNode(fromId);
  // // ns3::Ptr<ns3::ndn::L3Protocol> ndn = node->GetObject<ns3::ndn::L3Protocol>();
  // // shared_ptr<Face> face = ndn->getFaceById(ingress.face.getId());
  // // // ns3::Ptr<Fib> fib_ = &ndn->getForwarder()->getFib();
  // //
  // // // std::cout << "ingress.face.getId()::: " << ingress.face.getId() << std::endl; -------------  !!!!!!!!!!! 20240404
  // // if ((ingress.face.getId() == 257) /*&& (data.getName().getSubName(3,1).toUri() == "/consumer")*/){
  // //   auto ndn = node->GetObject<::ns3::ndn::L3Protocol>();
  // //   auto&& entry = ndn->getForwarder()->getFib().findLongestPrefixMatch(data.getName().getSubName(0,6).toUri());
  // //   std::cout << "[FibHelper::AddRoute]. entry.getPrefix(): " << entry.getPrefix() << " - data.getName(): " << data.getName() << " - Fib.size(): " << ndn->getForwarder()->getFib().size() << " -- Node_" << node->GetId() << std::endl;
  // //   if(1/*(entry.getPrefix().getSubName(0,6).toUri() != "/") && (entry.getPrefix().getSubName(0,6).toUri() != "/cromo/test")*/){
  // //     // ndn->getForwarder()->getFib().addOrUpdateNextHop(entry, ingress.face, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 60);
  // //     // entry.addOrUpdateNextHop(ingress.face, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 60);
  // //     ::ns3::ndn::FibHelper::AddRoute(node, fromId, timestamp1, hopCount, px, py, vx_gt, vy_gt, data.getName(), face, hopCount);
  // //     std::cout << "... Tree 1... entry.getPrefix().getSubName(0,6).toUri(): " << entry.getPrefix().getSubName(0,6).toUri() << std::endl;
  // //   }
  // // }

    // for (auto& entry : ndn->getForwarder()->getFib()) {
    //   if(entry.getPrefix().getSubName(0,6).toUri() == data.getName().getSubName(0,6).toUri()){
    //     std::cout << "Adding Rout now.. entry.getPrefix().getSubName(0,6).toUri(): " << entry.getPrefix().getSubName(0,6).toUri() << " -- data.getName().getSubName(0,6).toUri(): " << data.getName().getSubName(0,6).toUri() << std::endl;
    //     break;
    //   }
    //   // std::cout << "Adding.. Node_" << node->GetId() << " - getEndpointId(): " << entry.getNextHops().front().getEndpointId() <<  " getPrefix(): " << entry.getPrefix().getSubName(0,6).toUri() << " -- getName(): " << data.getName().getSubName(0,6).toUri() << std::endl;
    // }




      // for (const auto entry : ndn->getForwarder()->getFib()) {
      //   if(entry.getPrefix().getSubName(0,6).toUri() == data.getName().getSubName(0,6).toUri()){
      //     std::cout << "Adding Rout now.. entry.getPrefix().getSubName(0,6).toUri(): " << entry.getPrefix().getSubName(0,6).toUri() << " -- data.getName().getSubName(0,6).toUri(): " << data.getName().getSubName(0,6).toUri() << std::endl;
      //     ::ns3::ndn::FibHelper::AddRoute(node, fromId, timestamp1, hopCount, px, py, vx_gt, vy_gt, data.getName(), face, hopCount);
      //   }
      //   else{
      //     std::cout << "Adding it now!.. entry.getPrefix().getSubName(0,6).toUri(): " << entry.getPrefix().getSubName(0,6).toUri() << " -- data.getName().getSubName(0,6).toUri(): " << data.getName().getSubName(0,6).toUri() << std::endl;
      //     ndn->getForwarder()->getFib().addOrUpdateNextHop(entry, ingress.face, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 60);
      //     // entry.addOrUpdateNextHop(ingress.face, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 60);
      //   }
      // }

  // }


  // when only one PIT entry is matched, trigger strategy: after receive Data
  if (pitMatches.size() == 1) {
    auto& pitEntry = pitMatches.front();

    NFD_LOG_DEBUG("onIncomingData matching=" << pitEntry->getName());
    // std::cout << "pitMatches.size() == 1 CN xxx: " << data.getName().getSubName(0,6).toUri() << std::endl; // Good

    // set PIT expiry timer to now
    this->setExpiryTimer(pitEntry, 0_ms);// TODO!!From 0 to 10ms, soo the process of adding neighbor will finish //FIXME

    beforeSatisfyInterest(*pitEntry, ingress.face, data);
    // trigger strategy: after receive Data
    // std::cout << "trigger strategy: after receive Data. CN: " << data.getName().getSubName(0,5).toUri() << " -- pitEntry->getName(): " << pitEntry->getName() << std::endl;
    this->dispatchToStrategy(*pitEntry,
      [&] (fw::Strategy& strategy) { strategy.afterReceiveData(pitEntry, ingress, data); });

    // mark PIT satisfied
    pitEntry->isSatisfied = true;
    pitEntry->dataFreshnessPeriod = data.getFreshnessPeriod();

    // Dead Nonce List insert if necessary (for out-record of inFace)
    this->insertDeadNonceList(*pitEntry, &ingress.face);

    // // Add route to FIB 20240320
    // // auto NodeToAdd = ns3::NodeList::GetNode(fromId);
    // // std::cout << "ingress.face.getId()::: " << ingress.face.getId() << std::endl;
    // if ((ingress.face.getId() >= 0) && (data.getName().getSubName(3,1).toUri() == "/consumer") /*&& (fromId != nodeId)*/){
    //   ::ns3::ndn::FibHelper::AddRoute(node, fromId, timestamp->get(), hopCount, px, py, vx_gt, vy_gt, data.getName(), ingress.face.getId(), hopCount);//
    //   // std::cout << "extracting data. nodeId: " << nodeId << " - fromId: " << fromId << " - prefix: " << prefix << std::endl;
    // }



    // auto printFib = [](ns3::Ptr<ns3::Node> node) {
    //   auto ndn = node->GetObject<::ns3::ndn::L3Protocol>();
    //   // std::cout << "[onIncomingDataxx] node_ " << node->GetId() << std::endl;
    //   for (const auto& entry : ndn->getForwarder()->getFib()) {
    //     cout << "onIncomingDataxxx: - : node_" << node->GetId() << " - " << entry.getPrefix() << " (";
    //
    //     bool isFirst = true;
    //     for (auto& nextHop : entry.getNextHops()) {
    //       cout << nextHop.getFace();
    //       auto& face = nextHop.getFace();
    //       auto transport = dynamic_cast<ns3::ndn::NetDeviceTransport*>(face.getTransport());
    //       if (transport == nullptr) {
    //         continue;
    //       }
    //
    //       cout << " - FaceId: " << nextHop.getFace().getId() << " endpointId: " << nextHop.getEndpointId() << " - Timestamp: " << nextHop.getTimestamp() << " - Hops: " << nextHop.getHops() << " - Px: " << nextHop.getPx() << " - Py: " << nextHop.getPy() << " - Vx: " << nextHop.getVx() << " - Vy: " << nextHop.getVy();
    //
    //       if (!isFirst)
    //         cout << ", ";
    //       cout << ns3::Names::FindName(transport->GetNetDevice()->GetChannel()->GetDevice(1)->GetNode());
    //       isFirst = false;
    //     }
    //     cout << ")" << endl;
    //   }
    // };
    //
    // cout << "FIB content on node node_" << nodeId << endl;
    // // auto NodeToAdd = ns3::NodeList::GetNode(nodeId);
    // printFib(node);


    // delete PIT entry's out-record
    // pitEntry->deleteOutRecord(ingress.face); // 20240325
    pitEntry->deleteOutRecord(ingress.face, nodeId);// Elidio 20230109 --- Which one?
    // this->setExpiryTimer(pitEntry, 10_ms); //--> added to extend the pitEntry lifetime
  }
  else { // when more than one PIT entry is matched, trigger strategy: before satisfy Interest,  and send Data to all matched out faces
    // std::cout << "Else... pitMatches.size() == 1 CN: " << data.getName().getSubName(0,5).toUri() << std::endl; // No 20240329
    std::set<std::pair<Face*, EndpointId>> pendingDownstreams;
    auto now = time::steady_clock::now();
    // static uint16_t controlNeighborInsertion = 0;
    for (const auto& pitEntry : pitMatches) {
      // if(controlNeighborInsertion == 0){// We want only one insertion here
      //   std::string filename = "mobility-aware-strategy_AfterReceiveData";
      //   this->dispatchToStrategy(*pitEntry,
      //       [&] (fw::Strategy& strategy) { strategy.insertNeighborData(ingress, nmsi, nodeId, filename); }); // Elidio. 20221106. Instead of face, it should be endpointId!!...
      //   ++controlNeighborInsertion;
      // }
      NFD_LOG_DEBUG("onIncomingData matching=" << pitEntry->getName());
      // std::cout << "onIncomingData matching = " << pitEntry->getName() << std::endl;

      // remember pending downstreams
      for (const pit::InRecord& inRecord : pitEntry->getInRecords()) {
        if (inRecord.getExpiry() > now) {
          pendingDownstreams.emplace(&inRecord.getFace(), inRecord.getEndpointId()); // Elidio: from 0 to inRecord.getEndpointId()
          auto xx = inRecord.getEndpointId();
          // std::cout << "onIncomingData*EndpointId: " << xx << " - FaceId(): " << inRecord.getFace().getId() << std::endl; // No! 20240329
        }
      }

      // set PIT expiry timer to now
      this->setExpiryTimer(pitEntry, 0_ms);// 0 to 100

      // invoke PIT satisfy callback
      beforeSatisfyInterest(*pitEntry, ingress.face, data);
      this->dispatchToStrategy(*pitEntry,
        [&] (fw::Strategy& strategy) { strategy.beforeSatisfyInterest(pitEntry, ingress, data); });

      // mark PIT satisfied
      pitEntry->isSatisfied = true;
      pitEntry->dataFreshnessPeriod = data.getFreshnessPeriod();

      // Dead Nonce List insert if necessary (for out-record of inFace)
      this->insertDeadNonceList(*pitEntry, &ingress.face);

      // clear PIT entry's in and out records
      pitEntry->clearInRecords();
	  pitEntry->deleteOutRecord(ingress.face, nodeId); // Elidio 20230109
      // pitEntry->deleteOutRecord(ingress.face); // Elidio 20230325
    }

    // foreach pending downstream
    for (const auto& pendingDownstream : pendingDownstreams) {
      if (pendingDownstream.first->getId() == ingress.face.getId() &&
          // pendingDownstream.second == ingress.endpoint &&
          pendingDownstream.first->getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) {
        continue;
      }
      // goto outgoing Data pipeline
      // data.setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(nodeId));
      // std::cout << "foreach pending downstream CN = " << data.getName().getSubName(0,5).toUri() << std::endl; // No 20240329
      data.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(pendingDownstream.second)); //pontoDeControlo
      this->onOutgoingData(data, FaceEndpoint(*pendingDownstream.first, pendingDownstream.second));
    }
  }
}

void
Forwarder::onDataUnsolicited(const FaceEndpoint& ingress, const Data& data)
{
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto nodeId = node->GetId();
  ns3::Ptr<ns3::MobilityModel> mobModel = node->GetObject<ns3::MobilityModel> ();

  auto hopCountTag = data.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    m_hopCount = *hopCountTag;
  }


  auto FromId = data.getTag<lp::fromIdTag>();
  if (FromId != nullptr){
    m_fromId = FromId->get();
  }
  else {
    NFD_LOG_DEBUG("[Forwarder-OnDataUnsolicited]. Data fromIdTag NOT set! Setting default 55 for testing");
    m_fromId = 55;
  }


  auto destID = data.getTag<lp::destIdTag>(); // Will be extracted from LSTM
  if(destID != nullptr){
    m_destId = destID->get();
  }
  else{
    m_destId = 0; // Just for precaution!
  }

  // if((data.getName().getSubName(1,1).toUri() == "/beacon") || (data.getName().getSubName(1,1).toUri() == "/sharingBF"))
  //   std::cout << "onDataUnsolicited CN: " << data.getName().getSubName(0,5).toUri() << " nodeId: " << nodeId << " - fromId: " << m_fromId << " - destId: " << m_destId << " -  hopCount: " <<  m_hopCount << std::endl;
  // accept to cache?
  fw::UnsolicitedDataDecision decision = m_unsolicitedDataPolicy->decide(ingress.face, data);
  if (decision == fw::UnsolicitedDataDecision::CACHE) {
    /********************************** Elidio ****************************************/


    // uint64_t fromId;
    // auto FromId = data.getTag<lp::fromIdTag>();
    // if (FromId != nullptr){
    //   m_fromId = FromId->get();
    // }
    // else {
    //   NFD_LOG_DEBUG("[Forwarder-OnDataUnsolicited]. Data fromIdTag NOT set! Setting default 55 for testing");
    //   m_fromId = 55;
    // }

    // std::string prefix;
    // uint64_t hopCount = 0;
    // auto hopCountTag = data.getTag<lp::HopCountTag>();
    // if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    //   m_hopCount = *hopCountTag;
    // }

    // ndn::util::Tools tools;

    double vx_gt = 0.0;
    double vy_gt = 0.0;
    double vz_gt = 0.0;
    std::tie(vx_gt, vy_gt, vz_gt) = tools.GetNodeSpeedFromDataPacket(data);
    ndn::util::NodeSpeed nodespeed(vx_gt, vy_gt);

    double px = 0.0;
    double py = 0.0;
    double pz = 0.0;
    std::tie(px, py, pz) = tools.GetNodeLocationFromDataPacket(data);
    ndn::util::GeoPosition geoposition(px, py, pz);

    // auto destID = data.getTag<lp::destIdTag>(); // Will be extracted from LSTM
    // if(destID != nullptr){
    //   m_destId = destID->get();
    // }
    // else{
    //   m_destId = 0; // Just for precaution!
    // }

    // prefix = data.getName().getSubName(0,5).toUri();// Only first four components

    uint64_t timestamp1 = 0;
    auto timestamp = data.getTag<lp::timestampTag>();
    if(timestamp != nullptr){
      timestamp1 = timestamp->get();
    }
    else
      timestamp1 = 0; // Just for precaution!

    // std::cout << "From Unsolicited... data.getName().getSubName(1,1)" << data.getName().getSubName(1,1) << " CN: " << data.getName() << std::endl;



    nfd::fw::Strategy& baseStrategy = m_strategyChoice.findEffectiveStrategy(data.getName());
    nfd::fw::MobilityAwareStrategy* mobilityStrategy = static_cast<nfd::fw::MobilityAwareStrategy*>(&baseStrategy); // --->> (data.getName().getSubName(0,1).toUri() != "/localhost") &&

    if ((m_fromId != nodeId) && (data.getName().getSubName(1,1).toUri() != "/active-safety") && (data.getName().getSubName(0,1).toUri() != "/localhost"))
    {//Active-safety Data are not cached! -- (fromId != nodeId) && (data.getName().getSubName(1,1).toUri() != "/active-safety") && (data.getName().getSubName(0,1).toUri() != "/localhost") && (data.getName().getSubName(1,1).toUri() != "/beacon") && (data.getName().getSubName(1,1).toUri() != "/sharingBF")
      // m_prefix = data.getName().getSubName(0,5).toUri();// Only first four components
      // auto prefixo = data.getName().toUri();// Only first four components
      shared_ptr<ndn::util::Nmsi> nmsi = make_shared<ndn::util::Nmsi>();
      // nmsi->setNmsi(m_fromId, m_prefix, std::move(geoposition), std::move(nodespeed), m_hopCount, std::move(timestamp1));
      nmsi->setNmsi(m_fromId, data.getName().toUri(), std::move(geoposition), std::move(nodespeed), m_hopCount, std::move(timestamp1));

      m_cs.insert(data, *nmsi, true); // it was: *verifiedPosition // Elidio. Included geoposition ----  && (data.getName().getSubName(1,1).toUri() != "/sharingBF")
      // if (m_fromId > 120)
      //   std::cout << "[onDataUnsolicited] fromId > nodeNum: " << fromId << std::endl;

      // nfd::fw::Strategy& baseStrategy = m_strategyChoice.findEffectiveStrategy(data.getName());
      // nfd::fw::MobilityAwareStrategy* mobilityStrategy = static_cast<nfd::fw::MobilityAwareStrategy*>(&baseStrategy);

      // Check if the dynamic cast succeeded before using the mobilityStrategy pointer
      if (mobilityStrategy != nullptr) {
        mobilityStrategy->insertOrUpdate(nodeId, false, nmsi->getNmsi(), true);// true -> to inform the method that has been invoked from forwarder
        // std::cout << "Insert from Unsolicited... data.getName().getSubName(1,1)" << data.getName().getSubName(1,1) << std::endl;
      }
      // else {
      //   std::cout << "Cannot insert from Unsolicited..." << std::endl;
      // }

    }

  // sharingBF to dispatch to the strategy layer -- remove "(0){//" 20240330 --- add: (0){//
    if((data.getName().getSubName(1,1).toUri() == "/sharingBF") || (data.getName().getSubName(1,1).toUri() == "/beacon")){// Send all unsolicited Beacons to the strategy layer
    // trigger strategy: after receive Data
    // std::cout << "Received a sharingBF with CN: " << data.getName().toUri() << " .. will try tto trigger the strategy.afterReceiveData." << std::endl;
    NS_LOG_INFO("Received a sharingBF with CN: " << data.getName().toUri() << " .. will try to trigger the strategy.afterReceiveData.");
    // static uint32_t seq = 0;
    // ns3::Ptr<ns3::UniformRandomVariable> randomNum;// = CreateObject<ns3::UniformRandomVariable> ();
    //

      if ((mobilityStrategy != nullptr)) {// && (nodeId != ingress.endpoint)

        shared_ptr<Name> nameWithSequence = make_shared<Name>();
        shared_ptr<Interest> interest = make_shared<Interest>();
        interest->setName(*nameWithSequence);
        interest->setCanBePrefix(false);
        interest->setInterestLifetime(ndn::time::seconds(8));

        interest->setTag(make_shared<lp::IncomingFaceIdTag>(ingress.endpoint));// <----- commented

        // shared_ptr<lp::IncomingFaceIdTag> incomingFaceIdTag = data.getTag<lp::IncomingFaceIdTag>();
        // interest->setTag(make_shared<lp::IncomingFaceIdTag>(*incomingFaceIdTag));

        // std::cout << "Entering the afterReceiveData, from the OnUnsolicitedData!" << std::endl;
        shared_ptr<pit::Entry> pitEntry2 = m_pit.insert(*interest).first;
        mobilityStrategy->afterReceiveData(pitEntry2, ingress, data);

        pit::DataMatchResult pitMatches2 = m_pit.findAllDataMatches(data);
        // std::cout << "Unsolicited. pitMatches.size(): " << pitMatches2.size() << std::endl;

          // auto NewFromId = interest.getTag<lp::fromIdTag>();
        // insert out-record
        pitEntry2->insertOrUpdateOutRecord(ingress.face, nodeId, *interest);// Elidio TODO -- egress.endpoint 0 to nodeId
        // std::cout << "PIT being inserted from Unsolicited..." << std::endl;
        // m_pit.erase(pitEntry2);
        // set PIT expiry timer to now
        this->setExpiryTimer(pitEntry2, 0_ms);
      }
      // else {
      //   std::cout << "Cannot insert from Unsolicited." << std::endl;
      //   std::cout << "nodeId: " << nodeId << " ingress.endpoint: " << ingress.endpoint << std::endl;
      // }
    }
  }

  NFD_LOG_DEBUG("onDataUnsolicited in=" << ingress << " data=" << data.getName() << " decision=" << decision);
//   std::cout << "onDataUnsolicited in=" << ingress << " data=" << data.getName() << " decision=" << decision << std::endl; /*Elidio*/
}

void
Forwarder::onOutgoingData(const Data& data, const FaceEndpoint& egress)
{
  if (egress.face.getId() == face::INVALID_FACEID) {
    NFD_LOG_WARN("onOutgoingData out=(invalid) data=" << data.getName().toUri());
    return;
  }
  NFD_LOG_DEBUG("onOutgoingData out=" << egress << " data=" << data.getName().toUri());

  // /localhost scope control
  bool isViolatingLocalhost = egress.face.getScope() == ndn::nfd::FACE_SCOPE_NON_LOCAL &&
                              scope_prefix::LOCALHOST.isPrefixOf(data.getName());
  if (isViolatingLocalhost) {
    NFD_LOG_DEBUG("onOutgoingData out=" << egress << " data=" << data.getName() << " violates /localhost");
    // (drop)
    return;
  }

  // Tags set before. Now just send
  // send Data
  // std::cout << "egress.face: " << egress << std::endl;
  egress.face.sendData(data, egress.endpoint);
  ++m_counters.nOutData;
}

void
Forwarder::onIncomingNack(const FaceEndpoint& ingress, const lp::Nack& nack)
{
  // receive Nack
  nack.setTag(make_shared<lp::IncomingFaceIdTag>(ingress.face.getId()));
  ++m_counters.nInNacks;

  // if multi-access or ad hoc face, drop
  if (ingress.face.getLinkType() != ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    NFD_LOG_DEBUG("onIncomingNack in=" << ingress
                  << " nack=" << nack.getInterest().getName() << "~" << nack.getReason()
                  << " link-type=" << ingress.face.getLinkType());
    return;
  }

  // PIT match
  shared_ptr<pit::Entry> pitEntry = m_pit.find(nack.getInterest());
  // if no PIT entry found, drop
  if (pitEntry == nullptr) {
    NFD_LOG_DEBUG("onIncomingNack in=" << ingress << " nack=" << nack.getInterest().getName()
                  << "~" << nack.getReason() << " no-PIT-entry");
    return;
  }

  // has out-record?
  auto outRecord = pitEntry->getOutRecord(ingress.face, 0); // Elidio 20230109 -- ingress.endpoint
  // if no out-record found, drop
  if (outRecord == pitEntry->out_end()) {
    NFD_LOG_DEBUG("onIncomingNack in=" << ingress << " nack=" << nack.getInterest().getName()
                  << "~" << nack.getReason() << " no-out-record");
    return;
  }

  // if out-record has different Nonce, drop
  if (nack.getInterest().getNonce() != outRecord->getLastNonce()) {
    NFD_LOG_DEBUG("onIncomingNack in=" << ingress << " nack=" << nack.getInterest().getName()
                  << "~" << nack.getReason() << " wrong-Nonce " << nack.getInterest().getNonce()
                  << "!=" << outRecord->getLastNonce());
    return;
  }

  NFD_LOG_DEBUG("onIncomingNack in=" << ingress << " nack=" << nack.getInterest().getName()
                << "~" << nack.getReason() << " OK");

  // record Nack on out-record
  outRecord->setIncomingNack(nack);

  // set PIT expiry timer to now when all out-record receive Nack
  if (!fw::hasPendingOutRecords(*pitEntry)) {
    this->setExpiryTimer(pitEntry, 0_ms);
  }

  // trigger strategy: after receive NACK
  this->dispatchToStrategy(*pitEntry,
    [&] (fw::Strategy& strategy) { strategy.afterReceiveNack(ingress, nack, pitEntry); });
}

void
Forwarder::onOutgoingNack(const shared_ptr<pit::Entry>& pitEntry,
                          const FaceEndpoint& egress, const lp::NackHeader& nack)
{
  if (egress.face.getId() == face::INVALID_FACEID) {
    NFD_LOG_WARN("onOutgoingNack out=(invalid)"
                 << " nack=" << pitEntry->getInterest().getName() << "~" << nack.getReason());
    return;
  }

  // has in-record?
  auto inRecord = pitEntry->getInRecord(egress.face, egress.endpoint); // Elidio: egress.endpoint

  // if no in-record found, drop
  if (inRecord == pitEntry->in_end()) {
    NFD_LOG_DEBUG("onOutgoingNack out=" << egress
                  << " nack=" << pitEntry->getInterest().getName()
                  << "~" << nack.getReason() << " no-in-record");
    return;
  }

  // if multi-access or ad hoc face, drop
  if (egress.face.getLinkType() != ndn::nfd::LINK_TYPE_POINT_TO_POINT) {
    NFD_LOG_DEBUG("onOutgoingNack out=" << egress
                  << " nack=" << pitEntry->getInterest().getName() << "~" << nack.getReason()
                  << " link-type=" << egress.face.getLinkType());
    return;
  }

  NFD_LOG_DEBUG("onOutgoingNack out=" << egress
                << " nack=" << pitEntry->getInterest().getName()
                << "~" << nack.getReason() << " OK");

  // create Nack packet with the Interest from in-record
  lp::Nack nackPkt(inRecord->getInterest());
  nackPkt.setHeader(nack);

  // erase in-record
  pitEntry->deleteInRecord(egress.face, egress.endpoint); // Elidio: egress.face

  // send Nack on face
  egress.face.sendNack(nackPkt, egress.endpoint);
  ++m_counters.nOutNacks;
}

void
Forwarder::onDroppedInterest(const FaceEndpoint& egress, const Interest& interest)
{
  m_strategyChoice.findEffectiveStrategy(interest.getName()).onDroppedInterest(egress, interest);
}

void
Forwarder::onNewNextHop(const Name& prefix, const fib::NextHop& nextHop)
{
  static int x = 0;
  // std::cout << "Isto chega aqui??? CN: " << prefix.getSubName(0.5).toUri() << " x++: " << x++ << std::endl;// YES 20240329
  const auto affectedEntries = this->getNameTree().partialEnumerate(prefix,
    [&] (const name_tree::Entry& nte) -> std::pair<bool, bool> {
      const fib::Entry* fibEntry = nte.getFibEntry();
      const fw::Strategy* strategy = nullptr;
      if (nte.getStrategyChoiceEntry() != nullptr) {
        strategy = &nte.getStrategyChoiceEntry()->getStrategy();
      }
      // current nte has buffered Interests but no fibEntry (except for the root nte) and the strategy
      // enables new nexthop behavior, we enumerate the current nte and keep visiting its children.
      if (nte.getName().size() == 0 ||
          (strategy != nullptr && strategy->wantNewNextHopTrigger() &&
          fibEntry == nullptr && nte.hasPitEntries())) {
        return {true, true};
      }
      // we don't need the current nte (no pitEntry or strategy doesn't support new nexthop), but
      // if the current nte has no fibEntry, it's still possible that its children are affected by
      // the new nexthop.
      else if (fibEntry == nullptr) {
        return {false, true};
      }
      // if the current nte has a fibEntry, we ignore the current nte and don't visit its
      // children because they are already covered by the current nte's fibEntry.
      else {
        return {false, false};
      }
    });

  for (const auto& nte : affectedEntries) {
    for (const auto& pitEntry : nte.getPitEntries()) {
      this->dispatchToStrategy(*pitEntry,
        [&] (fw::Strategy& strategy) {
          strategy.afterNewNextHop(nextHop, pitEntry);
        });
    }
  }
}

void
Forwarder::setExpiryTimer(const shared_ptr<pit::Entry>& pitEntry, time::milliseconds duration)
{
  BOOST_ASSERT(pitEntry);
  BOOST_ASSERT(duration >= 0_ms);

  pitEntry->expiryTimer.cancel();
  pitEntry->expiryTimer = getScheduler().schedule(duration, [=] { onInterestFinalize(pitEntry); });
}

void
Forwarder::insertDeadNonceList(pit::Entry& pitEntry, Face* upstream)
{
  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto nodeId = currentNode->GetId ();

  // need Dead Nonce List insert?
  bool needDnl = true;
  if (pitEntry.isSatisfied) {
    BOOST_ASSERT(pitEntry.dataFreshnessPeriod >= 0_ms);
    needDnl = static_cast<bool>(pitEntry.getInterest().getMustBeFresh()) &&
              pitEntry.dataFreshnessPeriod < m_deadNonceList.getLifetime();
  }

  if (!needDnl) {
    return;
  }

  // Dead Nonce List insert
  if (upstream == nullptr) {
    // insert all outgoing Nonces
    const auto& outRecords = pitEntry.getOutRecords();
    std::for_each(outRecords.begin(), outRecords.end(), [&] (const auto& outRecord) {
      m_deadNonceList.add(pitEntry.getName(), outRecord.getLastNonce());
    });
  }
  else {
    // insert outgoing Nonce of a specific face
    auto outRecord = pitEntry.getOutRecord(*upstream, nodeId); // Elidio 20230109 -- 0 to nodeId
    if (outRecord != pitEntry.getOutRecords().end()) {
      m_deadNonceList.add(pitEntry.getName(), outRecord->getLastNonce());
    }
  }
}


void
Forwarder::FIBManagement(ns3::Ptr<ns3::Node> node, bool control)
{
	ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
	// 	Only in the simulation: Get direct access to Fib instance on the node, during the simulation
	const nfd::Fib& fib = currentNode->GetObject<ns3::ndn::L3Protocol>()->getForwarder()->getFib();
	// 	(*outputStream) << "FIB content on node" << node->GetId() << endl;
	//
    // for (Fib::Entry& entry : fib)
    for (nfd::Fib::const_iterator entry = fib.begin(); entry != fib.end(); entry++){
		// (*outputStream) << ns3::Simulator::Now ().ToDouble (ns3::Time::MS) << "\t[" << fib.size() << "]: "
		// /*(*outputStream)*/ << entry->getPrefix() << " - (";
		// bool isFirst = true;
      // nfd::Fib::Entry&& fibEntry =  *entry;
      auto nome = entry->getPrefix();
      // nfd::fib::Entry& entrada = std::move(*entry);
        // auto fibEntry = fib.findLongestPrefixMatch(nome);
        int numOfValidNextHopEntries = 0;
        int numOfNextHops = entry->getNextHops().size();
        for (auto nextHop = entry->getNextHops().end(); nextHop != entry->getNextHops().begin(); nextHop--){//for (auto &nexthops = entry->getNextHops().begin(); nextHop != entry->getNextHops().end(); nexthops++) --for (auto& nextHop : entry->getNextHops())
          numOfValidNextHopEntries++;
          const auto& face = nextHop->getFace();
          auto endpointID = nextHop->getEndpointId();
          auto Cost = nextHop->getCost();

          auto Timestamp = nextHop->getTimestamp();
          auto Px = nextHop->getPx();
          auto Py = nextHop->getPy();
          auto Vx = nextHop->getVx();
          auto Vy = nextHop->getVy();


          // const auto& face = nextHop.getFace();
          // auto endpointID = nextHop.getEndpointId();
          // auto Cost = nextHop.getCost();
          //
          // auto Timestamp = nextHop.getTimestamp();
          // auto Px = nextHop.getPx();
          // auto Py = nextHop.getPy();
          // auto Vx = nextHop.getVx();
          // auto Vy = nextHop.getVy();


          if ((numOfNextHops > 2) && (numOfNextHops - numOfValidNextHopEntries > 2)){
            // std::cout << "Trying to remove a nextHop... Endpoint: " << endpointID << std::endl;
            // ::ns3::ndn::FibHelper::RemoveRoute(currentNode, endpointID, Timestamp, Cost, Px, Py, Vx, Vy, entry->getPrefix(), currentNode->GetObject<ns3::ndn::L3Protocol>()->getFaceById(257), endpointID); //OK -- node

            // ::ns3::ndn::FibHelper::RemoveRoute(node, entry->getPrefix(), node->GetObject<ns3::ndn::L3Protocol>()->getFaceById(257), endpointID);

            // fib.removeNextHop(fibEntry, face);
            // auto removed =
            // this->getFib().removeNextHop(entrada, face);
            // removeNextHop(face, endpointID, Timestamp, Cost, Px, Py, Vx, Vy, endpointID);
            // fib.addOrUpdateNextHop(fibEntry, face, endpointID, Timestamp, Cost, Px, Py, Vx, Vy, endpointID);

            // std::cout << " - Removed? = " << removed << std::endl;
            // (*outputStream) << "faceId: " << face.getId() << " - endpointId: " << endpointID << " - Cost: " << Cost << " - Timestamp: " << Timestamp << " - Px: " << Px << " - Py: " << Py << " - Vx: " << Vx << " - Vy: " << Vy << "; ";
          }


          // (*outputStream) << "faceId: " << face.getId() << " - endpointId: " << endpointID << " - Cost: " << Cost << " - Timestamp: " << Timestamp << " - Px: " << Px << " - Py: " << Py << " - Vx: " << Vx << " - Vy: " << Vy << "; ";

// 			std::cout << "\n" << face.endpoint << "\n";
          // auto transport = dynamic_cast<ns3::ndn::NetDeviceTransport*>(face.getTransport());
//             Ptr<WifiMac> wifinetdevice2 = dynamic_cast<ns3::WifiNetDevice>(transport->GetNetDevice());
            // if (transport == nullptr)
              // continue;
			// (*outputStream) << " towards ";
// 			if (!isFirst)
// 				(*outputStream) << ", ";
			// (*outputStream) <<  " Node ID " << transport->GetNetDevice()->GetChannel()->GetDevice(node->GetId())->GetNode()->GetId() << " with Address: " << transport->GetNetDevice()->GetChannel()->GetDevice(node->GetId())->GetAddress();  /*<< "\n\t";*/

// 			if (!isFirst)
// 				(*outputStream) << ", ";

			// isFirst = false;
		}
		// (*outputStream) << ")" << std::endl;
	}



  // 		std:: cout << entry->getPrefix() << std::endl;
  // (*outputStream) << std::endl << std::endl;

// ns3::Simulator::Schedule (ns3::Seconds (5.0), MobilityAwareStrategy::FIBManagement, node, Ffile, control);
// 	std::cout << "Does it come back here? " << endl;

  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // static int x = 0;


  // this->m_Fibtimer.Cancel();
  // this->m_Fibtimer.SetDelay (ns3::MilliSeconds(FIBManagementTimer));
  // this->m_Fibtimer.SetFunction (&Forwarder::FIBManagement, this);
  // // this->m_Fibtimer.SetArguments (currentNode, tmp_dir + "DeleteFIB_Node_" + std::to_string(currentNode->GetId()) + ".txt", control);
  // this->m_Fibtimer.SetArguments (currentNode, control);//node
  // this->m_Fibtimer.Schedule();


  // std::cout << "Auto-Evoked... x = " << x++ << std::endl;

}


} // namespace nfd
