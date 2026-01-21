/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
 *
 * This file is part of NLSR (Named-data Link State Routing).
 * See AUTHORS.md for complete list of NLSR authors and contributors.
 *
 * NLSR is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef NFD_DAEMON_FW_NEIGHBORHOOD_LIST_HPP
#define NFD_DAEMON_FW_NEIGHBORHOOD_LIST_HPP

// #pragma once // Tells the compiler to include only once

#include "neighbor.hpp"
#include "common.hpp"

#include <list>
#include <tuple>
#include <vector>
#include <set>

#include <boost/cstdint.hpp>

#include <functional>

#include <ndn-cxx/face.hpp>
#include "ndn-cxx/util/scheduler.hpp"
#include <boost/asio/io_service.hpp>

#include "ns3/timer.h"
#include "ns3/simulator.h"
#include "ns3/callback.h"
#include "ns3/node-list.h"
#include "ns3/node.h"

#include "ndn-cxx/util/stmp-kf.hpp"
#include "ndn-cxx/util/tools.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "ns3/ndnSIM/model/map-common.hpp"

// uint16_t delayT = 7; // The delay for the neghbor timer
extern std::string tmp_dir;// = "/var/tmp/ns-3/testingVM_logs/";
// ndn::util::Tools tools3;
// static uint32_t nodeNums = tools3.GetNodeNum ();

namespace nfd {
namespace fw {

/******************* Begin Tracer Record Class *******************/

// class NodeId {
// public:

//   NodeId(){

//   };

//   ~NodeId(){

//   };

//   void setNodeId(uint64_t nodeId){
//     m_nodeId3 = nodeId;
//   }

//   uint64_t getNodeId() const {
//     return m_nodeId3;
//   }

// // private:
//   std::vector<Neighbor> m_tracingDataNeighborOriginal;
//   std::vector<Neighbor> m_tracingDataNeighborModified;
//   std::vector<uint64_t> m_tracingDataTimestamp;
//   uint64_t m_nodeId3;
//   uint64_t m_control;
//   // bool control = false;
//   uint64_t m_count;
// };

// class NeighborTracer : public ns3::Object
// {
// public:
//   static ns3::TypeId GetTypeId (){
//     static ns3::TypeId tid = ns3::TypeId ("NeighborTracer")
//     .SetParent<Object> ()
//     .AddConstructor<NeighborTracer> ()
//     ;
//     return tid;
//   };

//   NeighborTracer (){
//     // for insertOrUpdate method
//     m_tracingDataNmsi.clear();
//   };

//   ~NeighborTracer (){
//     // std::cout << "[~NeighborTracer ()]. m_numberOfNodes = " << NeighborTracer::m_numberOfNodes << std::endl;
//     std::ofstream out_file_;
//     std::string out_file_name_;

//     for(uint64_t k = 0; k < NeighborTracer::m_numberOfNodes; ++k){//m_context == "updateNeighborInfo"
//       if(nodeInfo[k].m_tracingDataNeighborOriginal.empty() == false){

//         if (1){//nodeInfo[k].m_control != 0
//           out_file_name_ = tmp_dir + "updateNeighborInfo_Node" + std::to_string(nodeInfo[k].getNodeId()) + ".txt";
//           // out_file_.open(out_file_name_.c_str(), std::ofstream::out | std::ios::app);//app

//           static uint8_t control = 1;
//           if(control == 0){
//             out_file_.open(out_file_name_.c_str(), std::ofstream::out | std::ios::trunc);//app
//             control++;
//           }
//           else{
//             out_file_.open(out_file_name_.c_str(), std::ofstream::out | std::ios::app);//app
//           }

//           for (int8_t t = 0; t < nodeInfo[k].m_tracingDataNeighborOriginal.size(); ++t){
//             out_file_ << nodeInfo[k].m_tracingDataTimestamp[t] << "\n";
//             out_file_ << "Original Neighbor Info:\n" << nodeInfo[k].m_tracingDataNeighborOriginal[t] << "\n";
//             out_file_ << "Modified Neighbor Info:\n"  << nodeInfo[k].m_tracingDataNeighborModified[t] << "\n";
//           }
//         }
//       }

//       if (out_file_.is_open()) {
//         out_file_.close();
//       }
//     }
//   };


//   NeighborTracer (NeighborTracer &) = delete;
//   NeighborTracer &operator=(NeighborTracer) = delete;
//   NeighborTracer (NeighborTracer &&) = default;
//   NeighborTracer &operator=(NeighborTracer &&) = default;

//   void
//   Record (const std::string context, const uint64_t timestamp, const uint64_t nodeId, const Neighbor neighborO, const Neighbor neighborM){
//     m_context = context;
//     // static int x = 0;

//     // do {
//       if (nodeId < NeighborTracer::m_numberOfNodes){
//            nodeInfo[nodeId].m_tracingDataNeighborOriginal.push_back(neighborO);
//           nodeInfo[nodeId].m_tracingDataNeighborModified.push_back(neighborM);
//           nodeInfo[nodeId].m_tracingDataTimestamp.push_back(timestamp);
//           nodeInfo[nodeId].setNodeId(nodeId);
//           nodeInfo[nodeId].m_control++;
//       }
//   };


//   // For insertOrUpdate method
//   void
//   Record (const std::string context, const uint64_t nodeId, const ndn::util::Nmsi nmsi){
//     m_context = context;
//     m_tracingDataNmsi.push_back(nmsi);
//     m_nodeId2 = nodeId;
//   };

//   std::vector<NodeId> getNodeInfo(){
//     return nodeInfo;
//   };

// private:
//   std::string m_context;
//   // for updateNeighborInfo

//   static std::vector<NodeId> nodeInfo;


//   // for insertOrUpdate
//   std::vector<ndn::util::Nmsi> m_tracingDataNmsi;
//   uint64_t m_nodeId2;

// public:
//   static uint64_t m_numberOfNodes;
//   // nodeNum = tools.GetNodeNum ();
// };
/******************* End Tracer Record Class *******************/

class NeighborhoodList
{
public:

//   class Neighbor;

  typedef std::list<Neighbor>::const_iterator const_iterator;
  typedef std::list<Neighbor>::iterator iterator;

  NeighborhoodList();//neighborhoodTimer
  ~NeighborhoodList();

  /*! \brief Inserts an adjacency into the list.

    \param neighbor The adjacency that we want to add to this list.

    \retval -1 Indicates discard.
    \retval 0 Indicates new insertion.
    \retval 1 Indicates update.

    This function attempts to insert the supplied adjacency into this
    object, which is an adjacency list.
   */
  // int64_t
  // insertOrUpdate(const uint64_t nodeId, const bool rsu, const ndn::util::Nmsi nmsi);

//   int32_t
//   insertOrUpdate(Neighbor& neighbor);

  std::list<Neighbor>&
  getNeighList();

  const std::list<Neighbor>&
  getNeighList() const;

  bool
  isNeighbor(const uint64_t nodeId) const;


  uint64_t
  deleteInActiveNeighbor(std::shared_ptr<NeighborhoodList> neighborList);


  int64_t
  suspendResumeAllNeighborTimers(const int16_t s);


  //   void
//   setTimedOutInterestCount(const uint32_t nodeId, uint32_t count);

  /*! \brief Copies the adjacencies in a list to this one.

    \param adl The adjacency list, the entries of which we want to
    copy into this object.

    Copies the entries contained in one list into this object.
   */
  void
  addNeighbors(NeighborhoodList& adl);

  int64_t
  getNumOfActiveNeighbor() const;

  int64_t
  getNumOfInActiveNeighbor() const;

  Neighbor
  getNeighbor(const uint64_t nodeId);

  bool
  operator==(const NeighborhoodList& adl) const;

  size_t
  size() const
  {
    return m_neighList.size();
  }

  void
  reset()
  {
    if (m_neighList.size() > 0) {
      m_neighList.clear();
    }
  }

  void updateNeighborInfo(uint64_t nodeId, std::list<Neighbor>::iterator neighbor);

//   void
//   updateNeighborInfo(shared_ptr<ndn::util::StmpKF> kf);


  // std::shared_ptr<std::vector< ndn::util::NodeIdSTMP>> getList();

  // NeighborhoodList::iterator
  // findNeighbor(const uint64_t nodeId);

  // std::tuple<bool, uint64_t>
  // getRsu();

  // bool
  // getNeighborRsu(Neighbor& nd);


  bool
  findRsu(Neighbor& rsu, uint64_t nodeId = nodeNum)
  {
    std::list<Neighbor>::iterator it;
    if(this->m_neighList.empty() == false){
      it = this->m_neighList.begin();
      for(; it != this->m_neighList.end(); it++){
        if (it->getNodeId() == nodeId){
          rsu = it->CopyNeighbor();
          rsu.m_ntimer_1.Cancel();
          return true;
        }
        else{
          return false;
        }
      }
    }
    else {
      return false;
    }
  }

  bool
  findNeighbor(Neighbor& nb, const uint64_t nodeId)
  {
    std::list<Neighbor>::iterator it;
    if(this->m_neighList.empty() == false){
      it = this->m_neighList.begin();
      for(; it != this->m_neighList.end(); it++){
        if (it->getNodeId() == nodeId){
          nb = std::move(it->CopyNeighbor());//nb = it->CopyNeighbor();
          // nb.m_ntimer_1.Cancel();
          return true;
        }
        else{
          return false;
        }
      }
    }
    else {
      return false;
    }
  }

  // std::list<Neighbor>::iterator
  // findIntermediateNode(const uint64_t nodeId, bool& founNode)
  // {
  //   // std::list<Neighbor>::iterator it = std::find_if(m_neighList.begin(),
  //   //                                       m_neighList.end(),
  //   //                                       std::bind(&Neighbor::compare,
  //   //                                                 _1, std::ref(nodeId)));
    
  //   std::list<Neighbor>::iterator it;

  //   if(m_neighList.empty() == false){
  //     std::cout << "[findIntermediateNode] - empty list\n";
  //     it = m_neighList.begin();
  //     for(; it != m_neighList.end(); it++){
  //       if (it->getNodeId() == nodeId){
  //         // it->m_ntimer_1.Cancel();
  //         founNode = true;
  //         return it;
  //       }
  //     }
  //   }

  //   return it;
  // }


  bool
  findIntermediateNode(const uint64_t nodeId, std::list<Neighbor>::iterator& it)
  {
    // std::list<Neighbor>::iterator it = std::find_if(m_neighList.begin(),
    //                                       m_neighList.end(),
    //                                       std::bind(&Neighbor::compare,
    //                                                 _1, std::ref(nodeId)));
    
    // std::list<Neighbor>::iterator it;

    if(m_neighList.empty() == false){
      // std::cout << "[findIntermediateNode] - empty list\n";
      it = m_neighList.begin();
      for(; it != m_neighList.end(); it++){
        if (it->getNodeId() == nodeId){
          // it->m_ntimer_1.Cancel();
          // founNode = true;
          return true;
        }
      }
    }

    return false;
  }








  /// Schedule m_ntimer.
  void
  ScheduleTimer ()
  {
//     m_ntimer.Cancel ();
    m_ntimer.Schedule ();
  }


  void
  writeLog();

public:
  const_iterator
  begin() const
  {
    return m_neighList.begin();
  }

  const_iterator
  end() const
  {
    return m_neighList.end();
  }

  // std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>> getNodeInfoSTMPVectorAddress() {
  //   return m_nodeInfoSTMP;
  // }

private:
  iterator
  find(const uint64_t nodeId);

  const_iterator
  find(const uint64_t nodeId) const;

// private:
public:
  std::list<Neighbor> m_neighList;
  // ns3::Ptr<NeighborTracer> m_neighborTracer;
  // std::shared_ptr<std::vector< ndn::util::NodeIdSTMP>> NodeInfoSTMP;// = std::make_shared<std::vector<ndn::util::NodeIdSTMP>>();//(31) +1
  // static std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>> m_nodeInfoSTMP;// = std::make_shared<std::vector<std::vector<SomeClass>>>();
  // static bool initialized;

public:
   Neighbor m_neighbor;
   ns3::Timer m_ntimer;
   // uint32_t nodeNums;
//   ndn::util::StmpKF m_stmpInstance;
  static uint64_t m_numberOfNodes;

};

}

} // namespace cromo2
#endif // CROMO2_NEIGHBORHOOD_LIST_HPP
