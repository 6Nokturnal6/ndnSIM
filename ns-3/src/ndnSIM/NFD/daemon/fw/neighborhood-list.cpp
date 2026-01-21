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

#include "neighborhood-list.hpp"

#include "neighbor.hpp"
#include "common.hpp"
// #include "common/logger.hpp"

#include <algorithm>

#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ndn-cxx/util/time.hpp"


#include "ndn-cxx/util/stmp-kf.hpp"
#include "ns3/callback.h"
#include "ns3/type-id.h"
#include "ns3/object.h"
#include <tuple>
#include <vector>

#include "ns3/log.h"
// #include "ns3/ndnSIM/model/map-common.hpp"

// #include "ndn-cxx/util/scheduler.hpp"
// #include <boost/scope_exit.hpp>

// using namespace ns3;
extern std::string tmp_dir;// = "/var/tmp/ns-3/testingVM_logs/";
// extern uint64_t nodeNum;
bool printToFileNeighborList = true;//false;

// extern int neighborTimer;// = 5; // Timer for updating neigbor info in NT
// extern int neighborhoodTimer;// = 16; // Timer for cleaning neigbor info up in N

static uint8_t controlxx = 0;//0

NS_LOG_COMPONENT_DEFINE("ndn.Neighborhood");
// std::vector< ndn::util::NodeIdSTMP > ndn::util::NeighborTracerSTMP::m_nodeInfoSTMP(nodeNum, ndn::util::NodeIdSTMP());//(31) +1

// std::shared_ptr<std::vector< ndn::util::NodeIdSTMP>> NodeInfoSTMP = std::make_shared<std::vector<ndn::util::NodeIdSTMP>>();//(31) +1

// uint32_t ndn::util::NeighborTracerSTMP::m_numberOfNodes;

// ndn::util::Tools tools3;
// uint64_t numberOfNodes = tools3.GetNodeNum ();

namespace nfd {
namespace fw {

  // uint64_t NeighborTracer::m_numberOfNodes = numberOfNodes;
  // uint64_t NeighborhoodList::m_numberOfNodes = numberOfNodes;

  // std::vector<NodeId> NeighborTracer::nodeInfo (numberOfNodes);// +1 NeighborhoodList::m_numberOfNodes

  // Initialize the static member variables
  // std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>> NeighborhoodList::m_nodeInfoSTMP = std::make_shared<std::vector<std::vector<ndn::util::NodeIdSTMP>>>();//(numberOfNodes);
  // bool NeighborhoodList::initialized = false;


NeighborhoodList::NeighborhoodList()
{
  NS_LOG_FUNCTION_NOARGS();
  // m_neighborTracer = ns3::CreateObject<NeighborTracer> ();
  // m_neighList = ns3::CreateObject<std::list<Neighbor> ();
  m_neighList = std::list<Neighbor>();


  // if (!initialized) {
  //   m_nodeInfoSTMP = std::make_shared<std::vector<std::vector<ndn::util::NodeIdSTMP>>>();


  //   m_nodeInfoSTMP->reserve(numberOfNodes);
  //   ndn::util::NodeIdSTMP nodeInfo;
  //   int t = 0;
  //   nodeInfo.m_nodeId = t;

  //   for (int i = 0; i < numberOfNodes; ++i){
  //     m_nodeInfoSTMP->push_back(std::vector<ndn::util::NodeIdSTMP>{});
  //     (*m_nodeInfoSTMP)[i].push_back(nodeInfo);
  //     nodeInfo.m_nodeId = ++t;
  //     // (*m_nodeInfoSTMP)[i].push_back(nodeInfo);
  //     // nodeInfo.m_nodeId = ++t;
  //     (*m_nodeInfoSTMP)[i].clear();
  //   }

  //   // m_nodeInfoSTMP->reserve(numberOfNodes, std::vector<ndn::util::NodeIdSTMP>());
  //   initialized = true;
  // }
}


NeighborhoodList::~NeighborhoodList()
{
  NS_LOG_FUNCTION_NOARGS();

  //REACTIVATE

  // std::ofstream out_file_;
  // std::string out_file_name_;
  // // int nodexx;

  // std::vector<std::vector<ndn::util::NodeIdSTMP>>::iterator it;// = m_nodeInfoSTMP->begin();
  // for (it = m_nodeInfoSTMP->begin(); it != m_nodeInfoSTMP->end() ;++it){
  //   if((*it).empty() != true){// TODO: See if it is correct.. (*m_nodeInfoSTMP)[i].empty() != true

  //     out_file_name_ = tmp_dir + "Node" + std::to_string((*it)[0].m_nodeId) + "_STMP_Estimation.txt";//
  //     // if(controlxx == 0){
  //       out_file_.open(out_file_name_.c_str(), std::ofstream::out | std::ios::trunc);//app
  //     //   controlxx++;
  //     // }
  //     // else{
  //     //   out_file_.open(out_file_name_.c_str(), std::ofstream::out | std::ios::app);//app
  //     // }

  //     for (int i = 0; i < it->size(); ++i){
  //       // output the estimation
  //       out_file_ << (*it)[i].m_trackingKF.ekf_.x_(0) << "\t"; //px
  //       out_file_ << (*it)[i].m_trackingKF.ekf_.x_(1) << "\t"; //py
  //       out_file_ << (*it)[i].m_trackingKF.ekf_.x_(2) << "\t"; //vx
  //       out_file_ << (*it)[i].m_trackingKF.ekf_.x_(3) << "\t"; //vy

  //       out_file_ << (*it)[i].m_scalarSpeed << "\t" << "\t";

  //       // // output the ground truth packages
  //       out_file_ << (*it)[i].m_gt_pack_list.gt_values_(0) << "\t";
  //       out_file_ << (*it)[i].m_gt_pack_list.gt_values_(1) << "\t";
  //       out_file_ << (*it)[i].m_gt_pack_list.gt_values_(2) << "\t";
  //       out_file_ << (*it)[i].m_gt_pack_list.gt_values_(3) << "\t\t";

  //       cout.setf(ios::right);
  //       cout.setf(ios::showpoint);
  //       cout << setprecision(4) << std::setw(8);

  //       out_file_ << (*it)[i].m_timestamp << "\t";
  //       // lastTimestamp = i.m_timestamp;
  //       // std::cout << "\n" << ns3::Simulator::Now ().ToDouble (ns3::Time::S) << std::endl;
  //       auto timestamp = ns3::Simulator::Now ().GetSeconds();
  //       out_file_ << timestamp << "\t";
  //       out_file_ << (*it)[i].m_distance << "\t";
  //       out_file_ << (*it)[i].m_nodeId << "\t";// m_nodeId[0]
  //       out_file_ << (*it)[i].m_neighborId << "\n";

  //           // }
  //       // }
  //     }
  //   }

  //   if (out_file_.is_open()) {
  //     out_file_.close();
  //   }
  // }
    m_ntimer.Cancel();
    m_ntimer.Remove();

};

// void
// NeighborhoodList::updateNeighborInfo(uint64_t currentNodeId1, std::list<Neighbor>::iterator neighbor){


//   Neighbor backupNeigiborOriginal;
//   // This is done in order to copy neighbor but not its Timer
//   backupNeigiborOriginal.setNmsi(neighbor->getNmsi());
//   backupNeigiborOriginal.setDistance(neighbor->getDistance());
//   backupNeigiborOriginal.setStatus(neighbor->getStatus());
//   backupNeigiborOriginal.m_ntimer_1.Cancel();
//   backupNeigiborOriginal.m_ntimer_1.Remove();

//   uint64_t timestamp = ns3::Simulator::Now ().GetSeconds();
//   // std::cout << "\n" << ns3::Simulator::Now ().GetSeconds() << std::endl;


//   ns3::Callback <std::tuple<ndn::util::Nmsi, double>, ndn::util::Nmsi&, bool, uint16_t, std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>> > stmpCallback;
//   uint16_t scheduleOnOff = 0;
//   bool control = false;

//   // ndn::util::Nmsi nmsiTemp = neighbor->getNmsi();
//   ndn::util::Nmsi nmsiTemp2;
//   double distance = 0.0;

//   ndn::util::StmpKF m_stmpInstance(*neighbor->getAdressNmsi(), control, scheduleOnOff, getNodeInfoSTMPVectorAddress());

//   stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance);
//   std::tie(nmsiTemp2, distance) = stmpCallback(*neighbor->getAdressNmsi(), control, scheduleOnOff, getNodeInfoSTMPVectorAddress());//neighbor->getNmsi()

//   neighbor->setNmsi(nmsiTemp2);

//   // *neighbor = nmsiTemp2;
//   // std::cout << "\nModified NMSI: \n" << *neighbor <<  "Distance: " << distance << std::endl;
//   // neighbor->setDistante(distance);
//   if (distance <= 250){
//     neighbor->setDistance(distance);
//     neighbor->setStatus(Neighbor::STATUS_ACTIVE);

//     NS_LOG_INFO("Timer elapsed, rescheduling... on Node: " << neighbor->getNmsi().getNodeId() << currentNodeId1);

//     neighbor->m_ntimer_1.SetDelay (ns3::Seconds(neighborTimer));
//     neighbor->m_ntimer_1.SetFunction (&NeighborhoodList::updateNeighborInfo, this);
//     neighbor->m_ntimer_1.SetArguments (currentNodeId1, neighbor);
//     neighbor->m_ntimer_1.Schedule();

//   }
//   else{
//     neighbor->setStatus(Neighbor::STATUS_INACTIVE);
//     neighbor->m_ntimer_1.Cancel();
//   }

//   Neighbor backupNeigiborModified;
//   backupNeigiborModified.setNmsi(nmsiTemp2);
//   backupNeigiborModified.setDistance(distance);
//   // backupNeigiborModified.setStatus(nmsiTemp2.getStatus());
//   backupNeigiborModified.m_ntimer_1.Cancel();
//   backupNeigiborModified.m_ntimer_1.Remove();
//   if(currentNodeId1 < numberOfNodes)//nodeNum
//     m_neighborTracer->Record ("updateNeighborInfo", timestamp, currentNodeId1, backupNeigiborOriginal, backupNeigiborModified);

//   /** reset cout buffer **/
//   // std::cout.rdbuf(coutbuf);

// }

// int64_t
// NeighborhoodList::insertOrUpdate(const uint64_t currentNodeId1, const bool rsu, const ndn::util::Nmsi nmsi)
// {
//   uint64_t neighborId = nmsi.getNodeId();
//   std::ofstream out;
//   // std::string file = tmp_dir + std::to_string(currentNodeId1) + "_Node_stmp_" + std::to_string(neighborId) + ".txt";
//   std::string file;
//   m_neighbor.setNmsi(nmsi);

//   if(rsu == true){//Insert RSU
//     std::list<Neighbor>::iterator it = find(nmsi.getNodeId());
//       if (it == m_neighList.end()){
//         m_neighList.push_back(m_neighbor);
//         if (printToFileNeighborList) std::cout << "RSU with Id: " << nmsi.getNodeId() << " inserted!" << std::endl;
//       }
//   }
//   else {
//     m_neighbor.m_ntimer_1.SetDelay (ns3::Seconds(neighborTimer));

//     std::list<Neighbor>::iterator it = find(nmsi.getNodeId());

//     if (it != m_neighList.end()){
//       if (it->m_ntimer_1.IsRunning()){ // Update
//           auto delayLeft = it->m_ntimer_1.GetDelayLeft();
//           // std::cout << "Node = " << it->getNodeId() << " found in the list... Delay left: " << delayLeft << std::endl;

//           if ((delayLeft <= ns3::Seconds(0.5)) ){ //update   && (delayLeft >= ns3::Seconds(1)) 500ns...

//             // std::cout << "InsertOrUpdate: REMOVING node: " << it->getNodeId() << std::endl;
//             it->m_ntimer_1.Cancel();
//             it->m_ntimer_1.Remove();

//             file = tmp_dir + std::to_string(currentNodeId1) + "_Node_stmp_" + std::to_string(it->getNodeId()) + ".txt";
//             auto *coutbuf = std::cout.rdbuf();
//             // Dump the existing NMSI to a local file
//             out.open(file.c_str(), std::fstream::app);//std::fstream::trunc
//             std::cout.rdbuf(out.rdbuf());
//             std::cout << it->getNmsi() << '$' << std::endl; // The '$' is necessary to end the input!

//             /** reset cout buffer **/
//             std::cout.rdbuf(coutbuf);

//             // Then erase it
//             it = m_neighList.erase(it);

//             m_neighList.insert(it, m_neighbor);

//             --it;
//             it->m_ntimer_1.Cancel();
//             // int16_t a = 0; // For debugging
//             it->m_ntimer_1.SetDelay (ns3::Seconds(neighborTimer));
//             it->m_ntimer_1.SetFunction (&NeighborhoodList::updateNeighborInfo, this);
//             it->m_ntimer_1.SetArguments (currentNodeId1, it);
//             it->m_ntimer_1.Schedule();

//           return 1;
//           }
//           else {// Discard
//             // std::cout << "InsertOrUpdate: m_ntimer_1.GetDelayLeft() = " << m_neighbor.m_ntimer_1.GetDelayLeft() << " - Node: " << m_neighbor.getNodeId() << " discarded... " << std::endl;
//             if (printToFileNeighborList) std::cout << "Remaining timer delay: " << it->m_ntimer_1.GetDelayLeft() << ", for Neighbor-node " << it->getNmsi().getNodeId() << std::endl;
//             NS_LOG_INFO("Remaining timer delay: " << it->m_ntimer_1.GetDelayLeft() << ", for Neighbor-node " << it->getNmsi().getNodeId());
//             return 3;
//           }
//         }
//         else{ // Neighbor exists in NL but its timer is not running. Means that timer expired and was not rescheduledd - > unsucessful rescheduling from NeighborhoodList::updateNeighborInfo(std::list<Neighbor>::iterator neighbor)

//           if (it->getStatus() == Neighbor::STATUS_ACTIVE){
//             if (printToFileNeighborList) std::cout << "Node found but not running... SHOULD reschedule on Node: " << it->getNodeId() << std::endl;
//             NS_LOG_INFO("Node found but not running... Setting timer on Node: " << it->getNodeId());
//             return 4;
//           }
//           else
//             {// Just testing...
//               return 5;
//             }
//         }
//       }
//       else{// Insert

//         m_neighList.push_back(m_neighbor);

//         std::list<Neighbor>::iterator it2 = m_neighList.end();
//         --it2;
//         // int16_t c = 0; // For debugging
//         it2->m_ntimer_1.SetDelay (ns3::Seconds(neighborTimer));
//         it2->m_ntimer_1.SetFunction (&NeighborhoodList::updateNeighborInfo, this);
//         it2->m_ntimer_1.SetArguments (currentNodeId1, it2);
//         it2->m_ntimer_1.Schedule();


//         file = tmp_dir + std::to_string(currentNodeId1) + "_Node_stmp_" + std::to_string(it2->getNodeId()) + ".txt";
//         out.open(file.c_str(), std::fstream::trunc);//std::fstream::trunc
//         auto *coutbuf = std::cout.rdbuf();

//         std::cout.rdbuf(out.rdbuf());
//         std::cout << it2->getNmsi() << '$' << std::endl; // The '$' is necessary to end the input!

//         /** reset cout buffer **/
//         std::cout.rdbuf(coutbuf);
//         return 0;
//       }
//   }
//     /** reset cout buffer **/
//     // std::cout.rdbuf(coutbuf);

//   return 0;
// }



// bool
// NeighborhoodList::operator==(const NeighborhoodList& adl) const
// {
//   auto theirList = adl.getNeighList();
//   if (m_neighList.size() != theirList.size()) {
//     return false;
//   }

//   std::set<Neighbor> ourSet(m_neighList.cbegin(), m_neighList.cend());
//   std::set<Neighbor> theirSet(theirList.cbegin(), theirList.cend());

//   return ourSet == theirSet;
// }

// bool
// NeighborhoodList::isNeighbor(const uint64_t nodeId) const
// {
//   std::list<Neighbor>::const_iterator it = find(nodeId);
//   if (it == m_neighList.end())
//   {
//     return false;
//   }
//   return true;
// }



// std::list<Neighbor>&
// NeighborhoodList::getNeighList()
// {
//   return m_neighList;
// }

// const std::list<Neighbor>&
// NeighborhoodList::getNeighList() const
// {
//   return m_neighList;
// }



// int64_t
// NeighborhoodList::getNumOfActiveNeighbor() const
// {
//   int64_t actNbrCount = 0;
//   for (std::list<Neighbor>::const_iterator it = m_neighList.begin(); it != m_neighList.end(); it++) {

//     if (it->getStatus() == Neighbor::STATUS_ACTIVE) {
//       actNbrCount++;
//     }
//   }
//   return actNbrCount;
// }


// int64_t
// NeighborhoodList::suspendResumeAllNeighborTimers(const int16_t s)// 0 - Suspend. 1 - Resume
// {
//   int64_t srNeighbor = 0;
//   for (std::list<Neighbor>::iterator it = m_neighList.begin(); it != m_neighList.end(); it++) {
//     if(s == 0){
//       // std::cout << "Here (s == 0)? " << std::endl;
//       it->m_ntimer_1.Suspend();
//       srNeighbor++;
//     }
//     else{
//       if (it->m_ntimer_1.IsSuspended()){
//         // std::cout << "Here (it->m_ntimer_1.IsSuspended())? " << std::endl;
//         it->m_ntimer_1.Resume();
//         srNeighbor++;
//       }
//     }
//   }
//   return srNeighbor;
// }


// int64_t
// NeighborhoodList::getNumOfInActiveNeighbor() const
// {
//   int64_t actNbrCount = 0;
//   for (std::list<Neighbor>::const_iterator it = m_neighList.begin(); it != m_neighList.end(); it++) {

//     if (it->getStatus() == Neighbor::STATUS_INACTIVE) {
//       actNbrCount++;
//     }
//   }
//   return actNbrCount;
// }


// uint64_t
// NeighborhoodList::deleteInActiveNeighbor(std::shared_ptr<NeighborhoodList> neighborList){

//   // static int m = 0;

//   // if(m == 0){
//   //   std::cout << "XXX Number of Nodes: " << nodeNum << std::endl;
//   //   m++;
//   // }


//   // std::list<Neighbor>::iterator it2 = neighborList->m_neighList.begin();
//   // std::cout << "Lets try here... " << ns3::Simulator::Now ().GetSeconds() << std::endl;
//   // while (it2 != neighborList->m_neighList.end()){
//   //   std::cout << "Anything... ? " << *it2 << std::endl;
//   //   ++it2;
//   // }

//   ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   auto currentNodeId1 = currentNode->GetId ();

//   // std::string tmp_dir = "/var/tmp/ns-3/testingVM_logs/";
//   // std::string file = tmp_dir + "NeighborhoodList_Node" + std::to_string(currentNode->GetId()) + ".txt";
//   //
//   //
//   // static int forAppend1 = 0; // truncate or append on file
//   //
//   // std::ofstream out;
//   // if (forAppend1 == 0){
//   //   out.open(file, std::fstream::trunc);
//   //   forAppend1++;
//   // }
//   // else
//   //   out.open(file, std::fstream::app);
//   //
//   // auto *coutbuf = std::cout.rdbuf();
//   // std::cout.rdbuf(out.rdbuf());
//   //
//   // std::cout << ns3::Simulator::Now ().GetSeconds() << std::endl;

// //   std::cout << "Does it reaches here?" << std::endl;
//   uint64_t numberOfInactiveNeighborsDeleted = 0;
//   uint64_t numberOfActiveNeighborsDeleted = 0;

//   uint64_t activeNodes = 1000;
//   uint64_t inactiveNodes = 1000;
//   uint64_t unknownStatusNodes = 1000;

//   // uint32_t activeNodes = 0;
//   // uint32_t inactiveNodes = 0;
//   // uint32_t unknownStatusNodes = 0;

//   // activeNodes.clear();
//   // inactiveNodes.clear();
//   // unknownStatusNodes.clear();

//   uint64_t timestamp;

// //   auto neighborsSuspended = suspendResumeAllNeighborTimers(0); // Suspend all
//   std::list<Neighbor>::iterator it = neighborList->m_neighList.begin();
//   while (it != neighborList->m_neighList.end()){
// //     it->m_ntimer_1.Suspend();


//     if (it->getStatus() == Neighbor::STATUS_INACTIVE){

//      it->m_ntimer_1.Cancel();
//      it->m_ntimer_1.Remove();
//      // std::cout << " Erased Inactive: node " << it->getNodeId() << std::endl;
//      if (it->getNodeId() < numberOfNodes)//nodeNum
//        inactiveNodes = it->getNodeId();
//      // inactiveNodes.push_back(it->getNodeId());

//      NS_LOG_INFO("Erased Inactive node: " << it->getNodeId());
//      it = neighborList->m_neighList.erase(it);// The iterator stays updated, pointing to the next just after the deleted entry.

//      numberOfInactiveNeighborsDeleted++;
//     }
//     else if (it->getStatus() == Neighbor::STATUS_ACTIVE){
//       if (it->getNodeId() < numberOfNodes)//nodeNum
//         activeNodes = it->getNodeId();
//       // activeNodes.push_back(it->getNodeId());
//       // std::cout << " Existing active: node " << it->getNodeId() << std::endl;
//       it++;
//     }
//     else {
//       if (it->getNodeId() < numberOfNodes)//nodeNum
//         unknownStatusNodes = it->getNodeId();
//       // unknownStatusNodes.push_back(it->getNodeId());
//       // std::cout << " Existing unknown_status: node " << it->getNodeId() << std::endl;
//       it++; //TODO See if they still exist
//     }

//     ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//     auto currentNodeId1 = currentNode->GetId ();
//     // if (currentNodeId1 > 7)
//     //   currentNodeId1 = 0;// TODO!!!!

//       timestamp = ns3::Simulator::Now ().GetSeconds();

//     //pack them and send to the tracing process
//     // Record (const std::string context, const uint64_t timestamp, const uint64_t nodeId, const std::tuple<std::vector<uint64_t, uint32_t, uint32_t>> existingNeighbors
//     std::tuple<uint64_t, uint64_t, uint64_t> actualNodeStatus = std::make_tuple(activeNodes, inactiveNodes, unknownStatusNodes);

//     // std::vector<uint32_t>::iterator it3 =  activeNodes.begin();
//     // for(; it3 < activeNodes.end(); it3){
//     //   std::cout << "active Nodes: "<< *it3 << std::endl;
//     // }

// //     std::tuple< std::vector<uint32_t>, std::vector<uint32_t>,  std::vector<uint32_t> > actualNodeStatus = std::make_tuple(activeNodes, inactiveNodes, unknownStatusNodes);
// //
//     // m_neighborTracer->Record ("deleteInActiveNeighbor", timestamp, currentNodeId1, actualNodeStatus);

//     // For testing: Reset to default value
//     activeNodes = 1000;
//     inactiveNodes = 1000;
//     unknownStatusNodes = 1000;

//   }

//   // std::tuple< std::vector<uint32_t>, std::vector<uint32_t>,  std::vector<uint32_t> > actualNodeStatus = std::make_tuple(activeNodes, inactiveNodes, unknownStatusNodes);

//   // m_neighborTracer->Record ("deleteInActiveNeighbor", timestamp, currentNodeId1, std::make_tuple(activeNodes, inactiveNodes, unknownStatusNodes));

//   this->m_ntimer.Cancel();
//   this->m_ntimer.SetDelay (ns3::Seconds(neighborhoodTimer));
//   this->m_ntimer.SetFunction (&NeighborhoodList::deleteInActiveNeighbor, this);
//   this->m_ntimer.SetArguments (neighborList);
//   this->m_ntimer.Schedule();


//   // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   // auto currentNodeId1 = currentNode->GetId ();
//   // if (currentNodeId1 > 7)
//   //   currentNodeId1 = 0;// TODO!!!!
//   //
//   // uint32_t timestamp = ns3::Simulator::Now ().GetSeconds();
//   //
//   // //pack them and send to the tracing process
//   // // Record (const std::string context, const uint32_t timestamp, const uint32_t nodeId, const std::tuple<std::vector<uint32_t, uint32_t, uint32_t>> existingNeighbors
//   // // std::tuple< std::vector<uint32_t>, std::vector<uint32_t>,  std::vector<uint32_t> > actualNodeStatus = std::make_tuple(activeNodes, inactiveNodes, unknownStatusNodes);
//   //
//   // // std::vector<uint32_t>::iterator it3 =  activeNodes.begin();
//   // // for(; it3 < activeNodes.end(); it3){
//   // //   std::cout << "active Nodes: "<< *it3 << std::endl;
//   // // }
//   //
//   // m_neighborTracer->Record ("deleteInActiveNeighbor", timestamp, currentNodeId1, std::make_tuple(activeNodes, inactiveNodes, unknownStatusNodes));


//   /** reset cout buffer **/
//   // std::cout.rdbuf(coutbuf);
//   // forAppend1++;
//   return numberOfInactiveNeighborsDeleted;
// }



// std::list<Neighbor>::iterator
// NeighborhoodList::find(const uint64_t nodeId)
// {
//   std::list<Neighbor>::iterator it = std::find_if(m_neighList.begin(),
//                                                   m_neighList.end(),
//                                                   std::bind(&Neighbor::compare,
//                                                             _1, std::cref(nodeId)));
//   return it;
// }

// std::list<Neighbor>::const_iterator
// NeighborhoodList::find(const uint64_t nodeId) const
// {
//   std::list<Neighbor>::const_iterator it = std::find_if(m_neighList.cbegin(),
//                                                         m_neighList.cend(),
//                                                         std::bind(&Neighbor::compare,
//                                                                   _1, std::cref(nodeId)));
//   return it;
// }


// NeighborhoodList::iterator
// NeighborhoodList::findNeighbor(const uint64_t nodeId)
// {
//   return std::find_if(m_neighList.begin(),
//                       m_neighList.end(),
//                       std::bind(&Neighbor::compare,
//                                 _1, std::cref(nodeId)));
// }




// bool
// NeighborhoodList::getNeighborRsu(Neighbor& nd)
// {
//   // Neighbor nd;
//   if(m_neighList.empty() == false){
//     std::list<Neighbor>::iterator iterator = m_neighList.begin();
//     std::cout << "Importante: " << nodeNum << std::endl;
//     if(iterator->getNodeId() == nodeNum){
//       nd = std::move(iterator->CopyNeighbor());
//       nd.m_ntimer_1.Cancel();
//       return true;// Only one RSU?
//     }
//   }
//   else{
//     // std::cout << "No RSU entry found! " << std::endl;
//     nd.m_ntimer_1.Cancel();
//     return false; // invalid value --- dumb neighbor
//   }
// }

// void
// NeighborhoodList::writeLog()
// {
// //   NLSR_LOG_DEBUG("-------Neighborhood List--------");
//   std::cout << "-------Neighborhood List--------" << std::endl;;
//   for (std::list<Neighbor>::iterator it = m_neighList.begin();
//        it != m_neighList.end(); it++) {
//     (*it).writeLog();
//   }
// }

/**
 * \brief CloseNeighbor structure
 */
// struct CloseNeighbor
// {
//   /**
//    * Check if the entry is expired
//    *
//    * \param nb Neighbors::Neighbor entry
//    * \return true if expired, false otherwise
//    */
//   bool operator() (const NeighborhoodList::Neighbor & nb) const
//   {
//     return ((nb.m_expireTime < Simulator::Now ()) || nb.close);
//   }
// };


// void
// Neighbors::ProcessTxError (WifiMacHeader const & hdr)
// {
//   Mac48Address addr = hdr.GetAddr1 ();
//
//   for (std::vector<Neighbor>::iterator i = m_nb.begin (); i != m_nb.end (); ++i)
//   {
//     if (i->m_hardwareAddress == addr)
//     {
//       i->close = true;
//     }
//   }
//   Purge ();
// }

// void
// NeighborhoodList::Purge ()
// {
// //   if (m_nb.empty ())
// //   {
// //     return;
// //   }
//
//   CloseNeighbor pred;
//   if (!m_handleLinkFailure.IsNull ())
//   {
//     for (std::list<Neighbor>::iterator j = m_nb.begin (); j != m_nb.end (); ++j)
//     {
//       if (pred (*j))
//       {
//         NS_LOG_LOGIC ("Close link to " << j->m_neighborAddress);
//         m_handleLinkFailure (j->m_neighborAddress);
//       }
//     }
//   }
//   m_nb.erase (std::remove_if (m_nb.begin (), m_nb.end (), pred), m_nb.end ());
//   m_ntimer.Cancel ();
//   m_ntimer.Schedule ();
// }


// void
// Neighbor::ScheduleTimer (ns3::Timer t)
// {
//   t.Cancel ();
//   t.Schedule ();
// }

// std::tuple<Time, std::list<Neighbor>::iterator>
// NeighborhoodList::GetExpireTime (const uint32_t nodeId)
// {
// //   Purge ();
//   auto it = NeighborhoodList::find(nodeId);
//   {
//     if (it->getNodeId() == nodeId)
//     {
//       return std::make_tuple((it->m_expireTime - Simulator::Now ()), it);
//     }
//   }
//   return std::make_tuple(Seconds (0), it);
// }


}
} // namespace nlsr
