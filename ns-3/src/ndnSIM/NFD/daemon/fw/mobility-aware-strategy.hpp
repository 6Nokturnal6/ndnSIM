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

#ifndef NFD_DAEMON_FW_MOBILITY_AWARE_STRATEGY_HPP
#define NFD_DAEMON_FW_MOBILITY_AWARE_STRATEGY_HPP

#include "fw/strategy.hpp"

#include <ndn-cxx/lp/prefix-announcement-header.hpp>

#include <fw/neighborhood-list.hpp>
#include "neighbor.hpp"

#include "ns3/node-list.h"
#include <ndn-cxx/util/nmsi.hpp>
#include <ndn-cxx/util/stmp-kf.hpp>
#include <ndn-cxx/util/ltmp-hmm.hpp>
#include "ns3/timer.h"
#include "ns3/simulator.h"
#include "ns3/callback.h"
#include "ns3/nstime.h"
#include <vector>

#include "ndn-cxx/util/tools.hpp"

#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>
#include "ndn-cxx/util/bloom-filter.hpp"


#include "ns3/ndnSIM/model/map-common.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

// using namespace ns3;
// using namespace std;

namespace nfd {
namespace fw {

// using BloomFilter = ::ndn::util::BloomFilter;
using CCTEntry = std::tuple< uint64_t, std::vector<std::tuple<uint64_t, ::ndn::util::BloomFilter> > >;

/******************* Begin Tracer Record Class *******************/

class NodeId {
public:

  NodeId(){

  };

  ~NodeId(){

  };

  void setNodeId(uint64_t nodeId){
    m_nodeId3 = nodeId;
  }

  uint64_t getNodeId() const {
    return m_nodeId3;
  }

// private:
  std::vector<Neighbor> m_tracingDataNeighborOriginal;
  std::vector<Neighbor> m_tracingDataNeighborModified;
  std::vector<uint64_t> m_tracingDataTimestamp;
  uint64_t m_nodeId3;
  uint64_t m_control;
  // bool control = false;
  uint64_t m_count;
};

class NeighborTracer : public ns3::Object
{
public:
  static ns3::TypeId GetTypeId (){
    static ns3::TypeId tid = ns3::TypeId ("NeighborTracer")
    .SetParent<Object> ()
    .AddConstructor<NeighborTracer> ()
    ;
    return tid;
  };

  NeighborTracer (){
    // for insertOrUpdate method
    m_tracingDataNmsi.clear();
  };

  ~NeighborTracer (){
  };


  NeighborTracer (NeighborTracer &) = delete;
  NeighborTracer &operator=(NeighborTracer) = delete;
  NeighborTracer (NeighborTracer &&) = default;
  NeighborTracer &operator=(NeighborTracer &&) = default;

  void
  Record (const std::string context, const uint64_t timestamp, const uint64_t nodeId, const Neighbor neighborO, const Neighbor neighborM){
    m_context = context;
    // static int x = 0;

    // do {
      if (nodeId < NeighborTracer::m_numberOfNodes){
          nodeInfo[nodeId].m_tracingDataNeighborOriginal.push_back(neighborO);
          nodeInfo[nodeId].m_tracingDataNeighborModified.push_back(neighborM);
          nodeInfo[nodeId].m_tracingDataTimestamp.push_back(timestamp);
          nodeInfo[nodeId].setNodeId(nodeId);
          nodeInfo[nodeId].m_control++;
      }
  };


  // For insertOrUpdate method
  void
  Record (const std::string context, const uint64_t nodeId, const ndn::util::Nmsi nmsi){
    m_context = context;
    m_tracingDataNmsi.push_back(nmsi);
    m_nodeId2 = nodeId;
  };

  std::vector<NodeId> getNodeInfo(){
    return nodeInfo;
  };

private:
  std::string m_context;
  // for updateNeighborInfo

  static std::vector<NodeId> nodeInfo;


  // for insertOrUpdate
  std::vector<ndn::util::Nmsi> m_tracingDataNmsi;
  uint64_t m_nodeId2;

public:
  static uint64_t m_numberOfNodes;
  // nodeNum = tools.GetNodeNum ();
};
/******************* End Tracer Record Class *******************/

/** \brief Mobility-aware strategy
 *
 *  This strategy first broadcasts Interest to learn a single path towards data,
 *  then unicasts subsequent Interests along the learned path
 *
 *  \see https://redmine.named-data.net/attachments/864/Mobility-aware-strategy-v1.pdf
 *
 *  \note This strategy is EndpointId-aware
 */
class MobilityAwareStrategy : public Strategy
{
public:
  explicit
  MobilityAwareStrategy(Forwarder& forwarder, const Name& name = getStrategyName());

  ~MobilityAwareStrategy();

  static const Name&
  getStrategyName();

  /// StrategyInfo on pit::InRecord
  class InRecordInfo : public StrategyInfo
  {
  public:
    static constexpr int
    getTypeId()
    {
      return 1040;// 9000Elidio. See: https://redmine.named-data.net/projects/nfd/wiki/StrategyInfoType
    }

  public:
    bool isNonDiscoveryInterest = false;
    uint64_t endpoint; // Elidio. To test. Repeated in the (InRecord) PIT entry
  };

  /// StrategyInfo on pit::OutRecord
  class OutRecordInfo : public StrategyInfo
  {
  public:
    static constexpr int
    getTypeId()
    {
      return 9001;// Elidio. See: https://redmine.named-data.net/projects/nfd/wiki/StrategyInfoType
    }

  public:
    bool isNonDiscoveryInterest = false;
  };

public: // triggers
  void
  afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
                       const shared_ptr<pit::Entry>& pitEntry) override;

  void
  afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
                   const FaceEndpoint& ingress, const Data& data) override;


  /*! \brief Inserts an adjacency into the list.
  \param neighbor The adjacency that we want to add to this list.

  \retval -1 Indicates discard.
  \retval 0 Indicates new insertion.
  \retval 1 Indicates update.

  This function attempts to insert the supplied adjacency into this
  object, which is an adjacency list.
  */
  uint64_t
  insertOrUpdate(const uint64_t nodeId, const bool rsu, const ndn::util::Nmsi& nmsi, const bool fromForwarder); // true -> to inform the method that has been invoked from forwarder

   /* */
  void
  updateNeighborInfo(uint64_t currentNodeId1, std::list<Neighbor>::iterator neighbor);


  uint64_t
  deleteInActiveNeighbor(std::shared_ptr<NeighborhoodList>& neighborhoodTable);


  void
  afterReceiveNack(const FaceEndpoint& ingress, const lp::Nack& nack,
                   const shared_ptr<pit::Entry>& pitEntry) override;

  // void
  // afterContentStoreHit(const shared_ptr<pit::Entry>& pitEntry,
  //                       const FaceEndpoint& ingress, const Data& data) override;



  // static void EraseLambda(const ns3::EventId&, std::unordered_set<std::string>& cont, const std::string& input) {
  //     cont.erase(input);
  // }
  //
  // static void ScheduleErase(std::unordered_set<std::string>& container, const std::string& input_string, int timerDuration) {
  //     ns3::Simulator::Schedule(ns3::MilliSeconds(timerDuration),
  //         &MobilityAwareStrategy::EraseLambda, std::ref(container), input_string);
  // }
  //
  // bool addCN(std::unordered_set<std::string>& container, const std::string& input_string, int timerDuration) {
  //     auto result = container.insert(input_string);
  //
  //     if (result.second) {
  //         ScheduleErase(container, input_string, timerDuration);
  //     }
  //
  //     return result.second;
  // }



private: // operations

  /** \brief Send an Interest to all possible faces
   *
   *  This function is invoked when the forwarder has no matching FIB entries for
   *  an incoming discovery Interest, which will be forwarded to faces that
   *    - do not violate the Interest scope
   *    - are non-local
   *    - are not the face from which the Interest arrived, unless the face is ad-hoc
   */
  void
  broadcastInterest(const Interest& interest, const FaceEndpoint& ingress,
                    const shared_ptr<pit::Entry>& pitEntry);

  /** \brief Send an Interest to \p nexthops
   */
  void
  multicastInterest(const Interest& interest, const FaceEndpoint& ingress,
                    const shared_ptr<pit::Entry>& pitEntry,
                    const fib::NextHopList& nexthops);

// Elidio
  /** \brief Send an Interest to \p nexthops
   */
  void
  unicastInterest(const Interest& interest, const FaceEndpoint& ingress,
                    const shared_ptr<pit::Entry>& pitEntry);



  /** \brief Send an Interest (routing here) to \p nexthops
   */
  void
  multicastInterestFIB(const Interest& interest, const FaceEndpoint& ingress,
                    const shared_ptr<pit::Entry>& pitEntry,
                    const fib::NextHopList& nexthops);
  void
  multicastInterestFIB(const Interest& interest, const FaceEndpoint& ingress,
                    const shared_ptr<pit::Entry>& pitEntry,
                    const fib::NextHopList& nexthops, const fib::Entry& fibEntry);


  void
  sendDataToAllCafs(const shared_ptr<pit::Entry>& pitEntry,
                        const FaceEndpoint& ingress, const Data& data);


  /** \brief Find a Prefix Announcement for the Data on the RIB thread, and forward
   *         the Data with the Prefix Announcement on the main thread
   */
  void
  asyncProcessData(const shared_ptr<pit::Entry>& pitEntry, const FaceEndpoint& ingress, const Data& data);

  /** \brief Check whether a PrefixAnnouncement needs to be attached to an incoming Data
   *
   *  The conditions that a Data packet requires a PrefixAnnouncement are
   *    - the incoming Interest was discovery and
   *    - the outgoing Interest was non-discovery and
   *    - this forwarder does not directly connect to the consumer
   */
  static bool
  needPrefixAnn(const shared_ptr<pit::Entry>& pitEntry);

  /** \brief Add a route using RibManager::slAnnounce on the RIB thread
   */
  void
  addRoute(const shared_ptr<pit::Entry>& pitEntry, const FaceEndpoint& ingress,
           const Data& data, const ndn::PrefixAnnouncement& pa);

  /** \brief renew a route using RibManager::slRenew on the RIB thread
   */
  void
  renewRoute(const Name& name, const FaceEndpoint& ingress, time::milliseconds maxLifetime);


  void
  insertTable(Neighbor neighbor) {
    m_neighborhoodTable->m_neighList.push_back(neighbor);
  }

  void
  insertRSUTable(Neighbor neighbor) {
    m_neighborhoodRSUTable->m_neighList.push_back(neighbor);
  }

  // std::list<Neighbor>::iterator
  // findInternediateNode(const uint64_t nodeId)
  // {
  //   std::list<Neighbor>::iterator it = std::find_if(m_neighborhoodTable->m_neighList.begin(),
  //                                         m_neighborhoodTable->m_neighList.end(),
  //                                         std::bind(&Neighbor::compare,
  //                                                   _1, std::ref(nodeId)));
  //   return it;
  // }


  // bool
  // findRSU(const uint64_t nodeId, Neighbor& rsu)
  // {
  //   std::list<Neighbor>::iterator it;
  //   if(m_neighborhoodRSUTable->m_neighList.empty() == false){
  //     it = m_neighborhoodRSUTable->m_neighList.begin();
  //     for(; it != m_neighborhoodRSUTable->m_neighList.end(); it++){
  //       if (it->getNodeId() == nodeNum){
  //         rsu = std::move(*it);
  //         return true;
  //       }
  //       else{
  //         return false;
  //       }
  //     }
  //   }
  //   else {
  //     return false;
  //   }
  // }

  // std::tuple<bool, uint64_t>
  // getRsu()
  // {
  //   if(m_neighborhoodRSUTable->m_neighList.empty() == false){
  //     std::list<Neighbor>::iterator iterator = m_neighborhoodRSUTable->m_neighList.begin();
  //     return std::make_tuple(true, iterator->getNodeId());// Only one RSU?
  //   }
  //   else{
  //     // std::cout << "No RSU entry found! " << std::endl;
  //     return std::make_tuple(false, 1000); // invalid value
  //   }
  // }

  // std::tuple<bool, Neighbor>
  // getNeighborRsu()
  // {
  //   if(m_neighborhoodRSUTable->m_neighList.empty() == false){
  //     std::list<Neighbor>::iterator iterator = m_neighborhoodRSUTable->m_neighList.begin();
  //     return std::make_tuple(true, *iterator);// Only one RSU?
  //   }
  //   else{
  //     // std::cout << "No RSU entry found! " << std::endl;
  //     return std::make_tuple(false, Neighbor()); // invalid value --- dumb neighbor
  //   }
  // }

  // Not developed...
  // std::tuple<bool, uint64_t>
  // findRsu(uint64_t nodeRsu)
  // {
  //   if(m_neighborhoodRSUTable->m_neighList.empty() == false){
  //     std::list<Neighbor>::iterator iterator = m_neighborhoodRSUTable->m_neighList.begin();


  //     return std::make_tuple(true, iterator->getNodeId());// Only one RSU?
  //   }
  //   else{
  //     // std::cout << "No RSU entry found! " << std::endl;
  //     return std::make_tuple(false, 1000); // invalid value
  //   }
  // }

  std::shared_ptr<NeighborhoodList>
  getTable()
  {
    return m_neighborhoodTable;;
  }

  void printCCTable(){
    // uint64_t currentNodeId1 = 666;

    for (const auto& entry: m_ccTable) {
      uint64_t currentNodeId1 = 666;
      currentNodeId1 = std::get<0>(entry);

      if (currentNodeId1 != 666){
        std::string file = tmp_dir + "ccTable_Node_" + std::to_string(currentNodeId1) + ".txt";

        std::ofstream out(file, std::fstream::trunc);
        auto *coutbuf = std::cout.rdbuf();
        std::cout.rdbuf(out.rdbuf());

        // std::cout << "Node: " << currentNodeId1 << std::endl;
        for (const auto& receivedFromTuple: std::get<1>(entry)) {
          uint64_t fromId = std::get<0>(receivedFromTuple);
          ::ndn::util::BloomFilter bf = std::get<1>(receivedFromTuple);
          std::cout << "Bloom Filter from Node_" << fromId << " - we surpress printing the BF for now... \"printCCTable()\"" << std::endl;//- we surpress printing the BF for now... \"printCCTable()\"
          // bf.printHex();
          // std::cout << std::endl;
        }
        /** reset cout buffer **/
        std::cout.rdbuf(coutbuf);
      }
    }
  }



// For delaying broadcasting an Interest

void
BroadcastDelayedPacket (const Interest& interest, const FaceEndpoint& ingress, const shared_ptr<pit::Entry>& pitEntry, std::shared_ptr<Face> sharedFace);

// std::tuple<bool, uint64_t>
// existsInCCTable(const uint64_t actualnode, const std::string cn);
bool
existsInCCTable(const uint64_t actualnode, const std::string cn, std::list<uint64_t>& fromIdList);

void
SendBeacon(uint64_t nodeId, bool control);

void
holdSendingBeacon(uint64_t nodeId, bool control);

void
BroadcastBeaconRSU(uint64_t nodeId, bool control);

// void
// FIBManagement(ns3::Ptr<ns3::Node> node, const std::string& Ffile, bool control);

// void
// FIBManagement(ns3::Ptr<ns3::Node> node, bool control);


private:
  static const time::milliseconds ROUTE_RENEW_LIFETIME;
//   ndn::Scheduler m_scheduler;

public:
  std::shared_ptr<NeighborhoodList> m_neighborhoodTable;// = std::make_shared<NeighborhoodList>();
  std::shared_ptr<NeighborhoodList> m_neighborhoodRSUTable;// = std::make_shared<NeighborhoodList>();
  ns3::Timer m_ntimer;
  // ns3::Timer m_Fibtimer;

  shared_ptr<ns3::Timer> m_nodeBeaconTimer;
  shared_ptr<ns3::Timer> m_waitToBroadcastTimer; // If there are no neighbors, we wait this timer time for sending the packet... This way we avoid sending packets that will not be received and processed, which could increase packet loss

  // For CCT
  static std::list<CCTEntry> m_ccTable;// = std::make_shared<std::list<CCTEntry>>();
  static bool initialized;

  Neighbor m_neighbor;
  ns3::Ptr<NeighborTracer> m_neighborTracer;
};

} // namespace fw
} // namespace nfd

#endif // NFD_DAEMON_FW_MOBILITY_AWARE_STRATEGY_HPP
