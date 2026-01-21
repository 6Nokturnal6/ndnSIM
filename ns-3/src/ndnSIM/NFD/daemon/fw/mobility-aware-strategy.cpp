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

#include "mobility-aware-strategy.hpp"
#include "algorithm.hpp"

#include "common/global.hpp"
#include "common/logger.hpp"
#include "rib/service.hpp"

#include <ndn-cxx/lp/empty-value.hpp>
#include <ndn-cxx/lp/prefix-announcement-header.hpp>
#include <ndn-cxx/lp/tags.hpp>

#include <boost/range/adaptor/reversed.hpp>

// Added Elidio
// #include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/wifi-net-device.h"
#include "ns3/sta-wifi-mac.h"
#include "ns3/mac48-address.h"
#include "ns3/address.h"
#include "model/ndn-net-device-transport.hpp"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ndnSIM-module.h"

#include "ndn-cxx/lp/speed-tag.hpp"
#include "ndn-cxx/lp/geo-tag.hpp"
#include "ndn-cxx/util/tools.hpp"

// For redirecting std::cout
#include <bits/stdc++.h>

/**** For getting Node ID ****/
// #include <ns3/simulator.h>
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/timer.h"
// #include <ndn-cxx/util/scheduler.hpp>
// #include <ns3/node-list.h>
// #include <ns3/node.h>

#include "common/global.hpp"
#include "ns3/ndnSIM/model/map-common.hpp"


#include <tuple>
#include <cmath>
#include <unordered_set>


#include "ns3/ndnSIM/apps/ndn-BFServer-cromo2.hpp"
#include "ns3/ndnSIM/apps/ndn-BFClient-cromo2.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cromo2BeaconAll.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cromo2Beacon.hpp"

#include <functional>
#include <ndn-cxx/face.hpp>
#include "ndn-cxx/util/scheduler.hpp"

#include <cstdio>


// #include "ndn-cxx/mgmt/nfd/fib-entry.hpp"

bool printToFileMobilityAwareStrategy = false;//true;//
// uint16_t nodeNum = 10;
// extern int nodeNum;

// std::string tmp_dir;// = "/var/tmp/ns-3/testingVM_logs/";
static int forAppendData = 0;
static int forAppendInterest = 0;

::ndn::util::Tools toolsStrategy;
// uint64_t nodeNum = toolsStrategy.GetNodeNum (tmp_dir);
uint64_t numberOfNodes = nodeNum;
static bool rsuTimerInitialisation = false;
static bool rsuTimerInitialisationGlobal = false;

float neighborTimer = 600;//4; // Timer for updating neigbor info in NT (was 5. in 4s -> done 4*20m = 80m)
float neighborhoodTimer = 1900;//13; // Timer for cleaning neigbor info up in N (was 16. maximum speed 20m/s -> in 12.5s done 250m)
// float FIBManagementTimer = 2400; // ms

static uint64_t vehicleBeaconInterval = 50000 * 500005000; //00000000; -- 12000000000// Time interval for sending vehicle beacon (in Seconds) when no RSU beacon s sensed for 3 X RSU beacon broadcast frequency
uint64_t vehicleBeaconIntervalGlobal = 10000 * 500005000; //00000000; -- 8000000000
uint64_t broadcastDelayInterval = 5000000000;
uint64_t broadcastFrequency = 10000 * 500005000; //5000; //500005000; //10000; // -- 400000000000
int InterestLife_ForContainer = 50; //2000; //8000 - 40 nodes, OK; -- 500 is the adopted timer. Now, 20231215, with the modification of addCN, we change the timer to 100ms

bool invokeFromClient = false;
bool invokeConsumerClient = false;
bool invokeConsumerBeacon = false;
static bool fromForwarder = false; // true -> to inform the method that has been invoked from forwarder; false here!

std::shared_ptr<ns3::Timer> nodeBeaconTimerGlobal;


static uint32_t partial_sizesData = 0;
static uint32_t partial_sizesInterest = 0;
static uint32_t partial_sizesDataBeacon = 0;
static uint32_t partial_sizesInterestBeacon = 0;

static uint32_t packets_sequenceData = 0;
static uint32_t packets_sequenceInterest = 0;
static uint32_t packets_sequenceDataBeacon = 0;
static uint32_t packets_sequenceInterestBeacon = 0;

static double currentTimeData;// = ns3::Simulator::Now().GetMilliSeconds();
static double currentTimeInterest;
static double currentTimeInterestBeacon;
static double currentTimeDataBeacon;

static double elapsedSecondsData;// = currentTimeData - lastUpdateTimeData;
static double elapsedSecondsInterest;
static double elapsedSecondsInterestBeacon;
static double elapsedSecondsDataBeacon;

static double lastUpdateTimeData;// = ns3::Simulator::Now().GetMilliSeconds();
static double lastUpdateTimeInterest;
static double lastUpdateTimeInterestBeacon;
static double lastUpdateTimeDataBeacon;

uint32_t thresholdData = 10;
uint32_t thresholdInterest = 10;
uint32_t thresholdInterestBeacon = 10;
uint32_t thresholdDataBeacon = 10;

static bool controlData = false;
static bool controlInterest = false;
static bool controlDataBeacon = false;
static bool controlInterestBeacon = false;

uint64_t neighborhoodTableSize = 0;

namespace nfd {
namespace fw {

NFD_LOG_INIT(MobilityAwareStrategy);
NFD_REGISTER_STRATEGY(MobilityAwareStrategy);

uint64_t NeighborTracer::m_numberOfNodes = nodeNum;
std::vector<NodeId> NeighborTracer::nodeInfo (nodeNum);// +1 NeighborhoodList::m_numberOfNodes

// std::shared_ptr<std::vector<std::vector<::ndn::util::BloomFilter>>> MobilityAwareStrategy::m_ccTable = std::make_shared<std::vector<std::vector<::ndn::util::BloomFilter>>>();
// std::shared_ptr<std::list<CCTEntry>> MobilityAwareStrategy::m_ccTable = std::make_shared<std::list<CCTEntry>>();
std::list<CCTEntry> MobilityAwareStrategy::m_ccTable = std::list<CCTEntry>();
bool MobilityAwareStrategy::initialized = false;
const time::milliseconds MobilityAwareStrategy::ROUTE_RENEW_LIFETIME(10_min);


// void
// SendBeacon(uint64_t nodeId) {
//   // Get the node based on the nodeIndex
//   // Ptr<Node> node = nodes.Get(nodeIndex);

//   ns3::Time now = timers[nodeId].GetDelayLeft();
//   std::cout << ns3::Simulator::Now ().GetMilliSeconds() << " - Timer of Node_" << nodeId << " reset while remaining " << now.GetMilliSeconds() << std::endl;

//   // Reset the timer for the current node
//   // timers[nodeId].Cancel();
//   // timers[nodeId].Schedule(ns3::Seconds(vehicleBeaconInterval)); // Assuming 'interval' is defined somewhere in your code
//   // return now;
// }

std::unordered_set<std::string> container_0;
std::unordered_set<std::string> container_1;
std::unordered_set<std::string> container_2;
std::unordered_set<std::string> container_3;
std::unordered_set<std::string> container_4;
std::unordered_set<std::string> container_5; // 20230810
std::unordered_set<std::string> container_6; // 20231220
std::unordered_set<std::string> container_7; // 20231220
std::unordered_set<std::string> container_8; // 20231220
std::unordered_set<std::string> container_9; // 20240306 - For Data and FIB

// ndn::Scheduler scheduler;
// ndn::Face face;

//
// bool checkCN(const std::string& input_string) {
//   return container.find(input_string) != container.end();
// }
//
// bool addCN(const std::string& input_string) {
//   auto result = container.insert(input_string);
//   return result.second; // If the string was inserted, returns true; otherwise, returns false.
// }

bool checkCN(const std::unordered_set<std::string>& container, const std::string& input_string) {
    return container.find(input_string) != container.end();
}

bool addCN(std::unordered_set<std::string>& container, const std::string& input_string) {
    auto result = container.insert(input_string);
    return result.second; // If the string was inserted, returns true; otherwise, returns false.
}

// bool addCN(std::unordered_set<std::string>& container, ndn::Scheduler& scheduler, const std::string& input_string, int timerDuration) {
//     auto result = container.insert(input_string);
//
//     if (result.second) {
//         // Schedule an event to remove the string after the timerDuration milliseconds
//         scheduler.schedule([&, input_string]() -> ndn::time::milliseconds {
//             container.erase(input_string);
//             return ndn::time::milliseconds(0); // Return a time duration (0 milliseconds)
//         }, ndn::time::milliseconds(timerDuration));
//     }
//
//     return result.second;
// }

// bool addCN(std::unordered_set<std::string>& container, ndn::Scheduler& scheduler, const std::string& input_string, int timerDuration) {
//     auto result = container.insert(input_string);
//
//     if (result.second) {
//         // Schedule an event to remove the string after the timerDuration milliseconds
//         scheduler.schedule(ndn::time::milliseconds(timerDuration), [&, input_string](const ndn::Scheduler&) {
//             container.erase(input_string);
//         });
//     }
//
//     return result.second;
// }

// bool addCN(std::unordered_set<std::string>& container, ndn::Face& face, const std::string& input_string, int timerDuration) {
//     auto result = container.insert(input_string);
//
//     if (result.second) {
//         // Schedule an event to remove the string after the timerDuration milliseconds
//         face.getIoService().schedule(ndn::time::milliseconds(timerDuration), [&, input_string](const ndn::Scheduler&) {
//             container.erase(input_string);
//         });
//     }
//
//     return result.second;
// }


// bool addCN(std::unordered_set<std::string>& container, const std::string& input_string, int timerDuration) {
//     auto result = container.insert(input_string);
//
//     if (result.second) {
//         // Schedule an event to remove the string after the timerDuration milliseconds
//         ns3::Simulator::Schedule(ns3::MilliSeconds(timerDuration), [&, input_string]() {
//             container.erase(input_string);
//         });
//     }
//
//     return result.second;
// }

// bool addCN(std::unordered_set<std::string>& container, const std::string& input_string, int timerDuration) {
//     auto result = container.insert(input_string);
//
//     if (result.second) {
//         // Define a function that calls the lambda
//         auto eraseLambda = [&](const ns3::EventId&) {
//             container.erase(input_string);
//         };
//
//         // Schedule the function with ns3::Simulator::Schedule()
//         ns3::Simulator::Schedule(ns3::MilliSeconds(timerDuration), &eraseLambda);
//     }
//
//     return result.second;
// }


// void EraseLambda(const ns3::EventId&, std::unordered_set<std::string>& cont, const std::string& input) {
//     cont.erase(input);
// }
//
// bool addCN(std::unordered_set<std::string>& container, const std::string& input_string, int timerDuration) {
//     auto result = container.insert(input_string);
//
//     if (result.second) {
//         // Schedule the static function with ns3::Simulator::Schedule()
//         ns3::Simulator::Schedule(ns3::MilliSeconds(timerDuration), &EraseLambda, std::ref(container), input_string);
//     }
//
//     return result.second;
// }


// void EraseLambda(const ns3::EventId&, std::unordered_set<std::string>& cont, const std::string& input) {
//     cont.erase(input);
// }
//
// void ScheduleEraseLambda(const ns3::EventId& eventId, std::unordered_set<std::string>& cont, const std::string& input) {
//     EraseLambda(eventId, cont, input);
// }
//
// bool addCN(std::unordered_set<std::string>& container, const std::string& input_string, int timerDuration) {
//     auto result = container.insert(input_string);
//
//     if (result.second) {
//         // Schedule the helper function with ns3::Simulator::Schedule()
//         ns3::Simulator::Schedule(ns3::MilliSeconds(timerDuration), &ScheduleEraseLambda, std::ref(container), input_string);
//     }
//
//     return result.second;
// }

static void EraseLambda(std::unordered_set<std::string>& cont, const std::string& input) {
    cont.erase(input);
}

// 20231215 - Jusst to reset the erase timer whenever the node tries to insert CN before the last timer has expired
// static void ScheduleErase(std::unordered_set<std::string>& container, const std::string& input_string, int timerDuration) {
//     ns3::Simulator::Schedule(ns3::MilliSeconds(timerDuration), &EraseLambda, std::ref(container), input_string);
// }

static void CancelEraseEvent(std::unordered_set<std::string>& container, const std::string& input_string) {
    EraseLambda(container, input_string);
}

// static void ScheduleErase(std::unordered_set<std::string>& container, const std::string& input_string, int timerDuration) {
//     ns3::Simulator::Cancel(&CancelEraseEvent, std::ref(container), input_string);
//     ns3::Simulator::Schedule(ns3::MilliSeconds(timerDuration), &EraseLambda, std::ref(container), input_string);
// }

// bool addCN(std::unordered_set<std::string>& container, const std::string& input_string, int timerDuration) {
//     auto result = container.insert(input_string);
//
//     if (result.second) {
//         ScheduleErase(container, input_string, timerDuration);
//     }
//
//     return result.second;
// }

void CancelEraseEvent(const ns3::EventId& eventId) {
    ns3::Simulator::Cancel(eventId);
}

bool addCN(std::unordered_set<std::string>& container, const std::string& input_string, int timerDuration) {
    auto result = container.insert(input_string);

    // Schedule eraser timer regardless of whether the element is newly inserted or already exists
    ns3::EventId eventId = ns3::Simulator::Schedule(ns3::MilliSeconds(timerDuration), &EraseLambda, std::ref(container), input_string);

    if (!result.second){
      CancelEraseEvent(eventId);
      eventId = ns3::Simulator::Schedule(ns3::MilliSeconds(timerDuration), &EraseLambda, std::ref(container), input_string);
    }

    return result.second;
}



MobilityAwareStrategy::MobilityAwareStrategy(Forwarder& forwarder, const Name& name)
  : Strategy(forwarder)
  , m_ntimer (ns3::Timer::CANCEL_ON_DESTROY)
  // , m_Fibtimer (ns3::Timer::CANCEL_ON_DESTROY)
{
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    NDN_THROW(std::invalid_argument("MobilityAwareStrategy does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    NDN_THROW(std::invalid_argument(
      "MobilityAwareStrategy does not support version " + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));

  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  uint64_t currentNodeId1 = currentNode->GetId ();

  m_neighborTracer = ns3::CreateObject<NeighborTracer> ();
  m_neighborhoodRSUTable = std::make_shared<NeighborhoodList>();//ns3::CreateObject<NeighborhoodList> ();
  m_neighborhoodTable = std::make_shared<NeighborhoodList>();//ns3::CreateObject<NeighborhoodList> ();

  m_ntimer.SetDelay (ns3::MilliSeconds(neighborhoodTimer)); // 15s
  m_ntimer.SetFunction (&MobilityAwareStrategy::deleteInActiveNeighbor, this);//m_neighborhoodTable.get()
  m_ntimer.SetArguments (m_neighborhoodTable);
  m_ntimer.Schedule();
  // std::cout << "Constructor Strategy: " << "tmp_dir: " << tmp_dir << " nodeNum:" << nodeNum << std::endl;

  // m_Fibtimer.SetDelay (ns3::MilliSeconds(FIBManagementTimer)); // 15s
  // m_Fibtimer.SetFunction (&MobilityAwareStrategy::FIBManagement, this);
  // // m_Fibtimer.SetArguments (currentNode, tmp_dir + "DeleteFIB_Node_" + std::to_string(currentNodeId1) + ".txt", false);
  // m_Fibtimer.SetArguments (currentNode, false);
  // m_Fibtimer.Schedule();


  // std::cout << "nodeNum: " << nodeNum << std::endl;
  // nodeBeaconTimer[currentNodeId1] = ns3::Timer();

  m_nodeBeaconTimer = make_shared<ns3::Timer>(ns3::Timer::CANCEL_ON_DESTROY);
  if (currentNodeId1 != nodeNum){
    // std::cout << "Timer set on Node_" << currentNodeId1 << " from the constructor." << std::endl; // Commented for now
    m_nodeBeaconTimer = make_shared<ns3::Timer>(ns3::Timer::CANCEL_ON_DESTROY);
    m_nodeBeaconTimer->SetFunction(&MobilityAwareStrategy::SendBeacon, this);
    m_nodeBeaconTimer->SetArguments(currentNodeId1, false); // on creation, it only schedule the timer. Next calling, for sending Beacon. The control flag is set to True inside the method
    m_nodeBeaconTimer->Schedule(ns3::MilliSeconds(vehicleBeaconInterval));
  }


  // nodeBeaconTimerGlobal = make_shared<ns3::Timer>(ns3::Timer::CANCEL_ON_DESTROY);
  if ((currentNodeId1 == nodeNum) && (rsuTimerInitialisation == false)){
    nodeBeaconTimerGlobal = make_shared<ns3::Timer>(ns3::Timer::CANCEL_ON_DESTROY);

    if(printToFileMobilityAwareStrategy)
      // std::cout << "Global Timer set on Node_" << currentNodeId1 << " from the constructor." << std::endl;
    nodeBeaconTimerGlobal = make_shared<ns3::Timer>(ns3::Timer::CANCEL_ON_DESTROY);
    nodeBeaconTimerGlobal->SetFunction(&MobilityAwareStrategy::holdSendingBeacon, this);
    nodeBeaconTimerGlobal->SetArguments(currentNodeId1, false); // on creation, it only schedule the timer. Next calling, for sending Beacon. The control flag is set to True inside the method
    nodeBeaconTimerGlobal->Schedule(ns3::MilliSeconds(vehicleBeaconIntervalGlobal));

    rsuTimerInitialisation = true; // Only set it once
  }


  if ((currentNodeId1 == nodeNum) && (rsuTimerInitialisationGlobal == false)){// For calling the Beacon consumer from the RSU
    // std::cout << "Timer set on Node_" << currentNodeId1 << " from the constructor." << std::endl; // Commented for now

    m_waitToBroadcastTimer = make_shared<ns3::Timer>(ns3::Timer::CANCEL_ON_DESTROY);
    m_waitToBroadcastTimer->SetFunction(&MobilityAwareStrategy::BroadcastBeaconRSU, this);
    m_waitToBroadcastTimer->SetArguments(currentNodeId1, false); // on creation, it only schedule the timer. Next calling, for sending Beacon. The control flag is set to True inside the method
    m_waitToBroadcastTimer->Schedule(ns3::MilliSeconds(broadcastFrequency));
    rsuTimerInitialisationGlobal = true;


    if(printToFileMobilityAwareStrategy)
      std::cout << "vehicleBeaconInterval: " << vehicleBeaconInterval << " - vehicleBeaconIntervalGlobal: " << vehicleBeaconIntervalGlobal << " - broadcastFrequency:" << broadcastFrequency << std::endl;
  }


  // m_waitToBroadcastTimer = make_shared<ns3::Timer>(ns3::Timer::CANCEL_ON_DESTROY);
  // if (1){
  //   // std::cout << "Timer set on Node_" << currentNodeId1 << " from the constructor." << std::endl; // Commented for now
  //   m_waitToBroadcastTimer = make_shared<ns3::Timer>(ns3::Timer::CANCEL_ON_DESTROY);
  //   // m_waitToBroadcastTimer->SetFunction(&MobilityAwareStrategy::BroadcastDelayedPacket, this);
  //   // m_waitToBroadcastTimer->SetArguments(currentNodeId1, false); // on creation, it only schedule the timer. Next calling, for sending Beacon. The control flag is set to True inside the method
  //   // m_waitToBroadcastTimer->Schedule(ns3::MilliSeconds(broadcastDelayInterval));
  // }

  // for (uint64_t i = 0; i < nodeNum; ++i) {
  //   timers[i] = ns3::Timer(ns3::Timer::CANCEL_ON_DESTROY);
  //   // timers[i].SetFunction(&MobilityAwareStrategy::SendBeacon, this);
  //   timers[i].SetFunction(&MobilityAwareStrategy::SendBeacon, this);
  //   timers[i].SetArguments(i); //  -- std::ref(timers)
  //   timers[i].Schedule(ns3::Seconds(vehicleBeaconInterval));
  // }




  // FIBManagement (currentNode, tmp_dir + "Delete_FIBonNode_" + std::to_string(currentNodeId1) + ".txt", false);
}

MobilityAwareStrategy::~MobilityAwareStrategy(){
  printCCTable();// dump the m_ccTable to file before exiting
}


const Name&
MobilityAwareStrategy::getStrategyName()
{
  static Name strategyName("/localhost/nfd/strategy/mobility-aware/%FD%01");
  return strategyName;
}

/***********************************************************************************************************************************/
/********************************************* afterReceiveInterest ****************************************************************/
/***********************************************************************************************************************************/
void
MobilityAwareStrategy::afterReceiveInterest(const FaceEndpoint& ingress, const Interest& interest,
                                           const shared_ptr<pit::Entry>& pitEntry)
{
  uint64_t broadcastId = nodeNum;// 666; // Not valid, updated to ff:ff:ff:ff... In doSend()...

  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();

  /************************ Elidio ****************************/
  /**
  * LSTM is used to propose the next-hop Relay (relayId)
  * NMSI is included into the packet (LpPacket)
  * endpoint = (Content source)origId [if known, otherways it is zero]
  */

  // std::cout << "NodeNUM: " << nodeNum << std::endl;

  // redirecting std::cout to file
  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto currentNodeId1 = currentNode->GetId ();

  int hopCount = 0;
  auto hopCountTag = interest.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }

  if(hopCount == 0){
    interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(ingress.endpoint));//nodeNum 20240612
    this->sendInterest(pitEntry, FaceEndpoint(ingress.face, ingress.endpoint), interest);//nodeNum 20240612
    // return;
  }

  static int forAppend1 = 0; // truncate or append on file
  std::ofstream out;

  std::string file = tmp_dir + "mobility-aware-strategy_AfterReceiveInterest" + std::to_string(currentNodeId1) + ".txt";
  auto *coutbuf = std::cout.rdbuf();

  if(printToFileMobilityAwareStrategy){
    if (forAppend1 == 0){
      out.open(file, std::fstream::trunc);
      forAppend1++;
    }
    else
      out.open(file, std::fstream::app);

    std::cout.rdbuf(out.rdbuf());
  }

  // if(printToFileMobilityAwareStrategy) std::cout << "\n" << ns3::Simulator::Now ().GetMilliSeconds() << std::endl;

  // Get updated mobility information, for the LSTM algorithm
  // uint32_t timestamp1;

  // auto timestamp = interest.getTag<lp::timestampTag>();
  // if(timestamp != nullptr){
  //   timestamp1 = timestamp->get();
    // if(printToFileMobilityAwareStrategy)
      // std::cout << "\tTimestamp: " << timestamp1 << std::endl;
  // }

  if(printToFileMobilityAwareStrategy)
    std::cout << "\n---> Received Interest at timestamp: " << ns3::Simulator::Now().GetMilliSeconds() << std::endl;
  // if(printToFileMobilityAwareStrategy) std::cout << "\nReceived values!" << "\n" << std::endl;
  uint64_t timestamp1;
  auto timestamp = interest.getTag<lp::timestampTag>();
  if(timestamp != nullptr){
    timestamp1 = timestamp->get();
    if(printToFileMobilityAwareStrategy)
      std::cout << "\tExtracted Timestamp: " << timestamp1 << std::endl;
  }

  double vx_gt = 0.0;
  double vy_gt = 0.0;
  double vz_gt = 0.0;

  std::tie(vx_gt, vy_gt, vz_gt) = toolsStrategy.GetNodeSpeedFromInterestPacket(interest);
  // if(printToFileMobilityAwareStrategy)
    // std::cout << "\tExtracted Node Speed: " << vx_gt << "," << vy_gt << "," << vz_gt << std::endl;

  ndn::util::NodeSpeed nodespeed(vx_gt, vy_gt);

  double px = 0.0;
  double py = 0.0;
  double pz = 0.0;
  std::tie(px, py, pz) = toolsStrategy.GetNodeLocationFromInterestPacket(interest);
  // if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted Node Geoposition: " << px << "," << py << "," << pz << std::endl;
  ndn::util::GeoPosition geoposition(px, py, pz);

  auto destId = interest.getTag<lp::destIdTag>(); // Will later be set from LSTM and LSMP
  auto dest = broadcastId;
  if(destId != nullptr){
    dest = destId->get();
    if(printToFileMobilityAwareStrategy)
      std::cout << "\tExtracted DestId: " << dest << std::endl;
  }

  uint32_t fromId;
  auto FromId = interest.getTag<lp::fromIdTag>();
  if(FromId != nullptr){
    fromId = FromId->get();
    if(fromId > nodeNum)
      if(printToFileMobilityAwareStrategy)
        std::cout << "\tExtracted FromId > nodeNum: " << fromId << " > " << nodeNum << std::endl;
    if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted FromId: " << fromId << std::endl;
  }
  else{
    // fromId = 666;// TODO
    if(printToFileMobilityAwareStrategy) std::cout << "mobility-aware strategy... Interest. FromId nullptr. SET to 666" << std::endl;
  }


  // if(printToFileMobilityAwareStrategy)
    // std::cout << "\torigId (From received Interest): " << interest.getOrigID() << "\n" << std::endl << std::endl;

  if(printToFileMobilityAwareStrategy){
    std::cout << "\tReceived CN: " << interest.getName().getSubName(0,6).toUri() << "\n\tfaceId: " << ingress.face.getId() << "\n\thopCount: " << hopCount << std::endl;
  // std::cout << "Received CN (1,1): " << interest.getName().getSubName(1,1).toUri() << std::endl;
  }


  // Insert the neighbors into the Neighborhood Table
  auto prefix = interest.getName().getSubName(0,6).toUri();// Only first five components
  // double dir = 9.9;

  // Get Interest origId
  std::string InterestOrigId = interest.getOrigID();
  uint64_t OrigId;

  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&InterestOrigId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<0>(NodeIdMacMap) == InterestOrigId;});
  if (it != NodeIdMacMap.end()) {
    OrigId = std::get<1>(*it);
  }
  else{
    if(printToFileMobilityAwareStrategy)
      std::cout << "[AfterReceiveInterest]. Error (OrigId)?? " << std::endl;
  }



  // Get Interest DestFinalId
  std::string InterestDestFinalId = interest.getDestFinalID();
  uint64_t DestFinalId = 666;

  it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&InterestDestFinalId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<0>(NodeIdMacMap) == InterestDestFinalId;});
  if (it != NodeIdMacMap.end()) {
    DestFinalId = std::get<1>(*it);
    if(printToFileMobilityAwareStrategy)
      std::cout << "\tDestFinalId = " << DestFinalId << std::endl;
  }
  else{
    if(printToFileMobilityAwareStrategy)
      std::cout << "\tDestFinalId = " << DestFinalId << std::endl;
  }


  if ((fromId == currentNodeId1) /*&& (hopCount != 0)*/){
    interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(666)); // in the unicastInterest method
    broadcastInterest(interest, FaceEndpoint(ingress.face, 666), pitEntry);


    if(printToFileMobilityAwareStrategy){
      std::cout << "\t>>>Interest FROM this Node. This Node_" << fromId << "... Broadcasting..." << std::endl;
      std::cout.rdbuf(coutbuf);}
    return;
  }

  if ((DestFinalId == currentNodeId1) && (hopCount == 0)){
    if(printToFileMobilityAwareStrategy){
      std::cout << "\t>>>Interest final destination. This Node_" << DestFinalId << "... Exiting Interest procedure..." << std::endl;
      std::cout.rdbuf(coutbuf);}
    return;
  }

  if (1){//interest.getName().getSubName(0,1).toUri() != "/localhost"
    ndn::util::Nmsi nmsi(fromId, prefix, geoposition, nodespeed, hopCount, timestamp1);

    if (hopCount != 0){//OrigId != currentNodeId1 -- // std::cout << "\n\tProcessing NT for Interests with \"fromId != currentNodeId1\"" << std::endl;
      // ndn::util::Nmsi nmsi(fromId, prefix, geoposition, nodespeed, hopCount, timestamp1);
      if(interest.getName().getSubName(1,1).toUri() == "/beacon"){
        // A beacon has been received, then, first reset the timer.
        if (currentNodeId1 != nodeNum) {
          SendBeacon(currentNodeId1, false);
        }

        // Insert to the NT list
        if ((hopCount == 1) && (fromId == nodeNum)){
          Neighbor nb;
          bool foundRsuNode = false;
          foundRsuNode = m_neighborhoodRSUTable->findRsu(nb);

          if(!foundRsuNode){ // Insert the RSU to Neighbor list
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tTrying to insert neighbor RSU_" << fromId << "  from a Beacon CN: "  << interest.getName().getSubName(0,5).toUri() << " - hopCount: " << hopCount << std::endl;
            insertOrUpdate(currentNodeId1, true, nmsi, fromForwarder);
            insertOrUpdate(currentNodeId1, false, nmsi, fromForwarder);// Also insert into normal NT
            NFD_LOG_INFO("RSU_" << nmsi.getNodeId() << " inserted in Node_" << currentNodeId1 << " Neighborood Table");
          }
          else{
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tRSU_" << fromId << " already in the NT." << std::endl;
          }

        }

        if(fromId != nodeNum){// hopCount > 0 --- insert in vehicle NT
          bool foundVehicle = false;
          Neighbor nb;
          foundVehicle = m_neighborhoodTable->findNeighbor(nb, fromId);

          if(!foundVehicle){
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tTrying to insert neighbor Node_" << fromId << " from CN: "  << interest.getName().getSubName(0,5).toUri() << " - hopCount: " << hopCount << std::endl;
            insertOrUpdate(currentNodeId1, false, nmsi, fromForwarder);
            NFD_LOG_INFO("Node_" << nmsi.getNodeId() << " inserted in Node_" << currentNodeId1 << " Neighborood Table." << " hopCount: " << hopCount);
          }
          else{
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tNeighbor Node_" << fromId << " already in the NT." << std::endl;
          }
        }
      }
      else if (interest.getName().getSubName(1,1).toUri() == "/sharingBF"){
        // Insert to the NT list
        bool foundVehicle = false;
        Neighbor nb;
        foundVehicle = m_neighborhoodTable->findNeighbor(nb, fromId);

        if(!foundVehicle){
          if(printToFileMobilityAwareStrategy)
            std::cout << "\tTrying to insert neighbor Node_" << fromId << " from CN: "  << interest.getName().getSubName(0,5).toUri() << " - hopCount: " << hopCount << std::endl;
          insertOrUpdate(currentNodeId1, false, nmsi, fromForwarder);
          NFD_LOG_INFO("Node_" << nmsi.getNodeId() << " inserted in Node_" << currentNodeId1 << " Neighborood Table." << " hopCount: " << hopCount);
        }
        else{
          if(printToFileMobilityAwareStrategy)
            std::cout << "\tNeighbor Node_" << fromId << " already in the NT." << std::endl;
        }
      }
      else{ /*if(interest.getName().getSubName(3,1).toUri() == "/consumer")*/
        // Insert to the NT list

        if ((hopCount == 1) && (fromId == nodeNum)){ // It is a RSU
          Neighbor nb;
          bool foundRsuNode = false;
          foundRsuNode = m_neighborhoodRSUTable->findRsu(nb);

          if(!foundRsuNode){ // Insert the RSU to Neighbor list
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tTrying to insert neighbor RSU_" << fromId << "  from a Beacon CN: "  << interest.getName().getSubName(0,5).toUri() << " - hopCount: " << hopCount << std::endl;
            insertOrUpdate(currentNodeId1, true, nmsi, fromForwarder);
            insertOrUpdate(currentNodeId1, false, nmsi, fromForwarder);// Also insert into normal NT
            NFD_LOG_INFO("RSU_" << nmsi.getNodeId() << " inserted in Node_" << currentNodeId1 << " Neighborood Table");
          }
          else{
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tRSU_" << fromId << " already in the NT." << std::endl;
          }

        }

        if(fromId != nodeNum) {
          bool foundVehicle = false;
          Neighbor nb;
          foundVehicle = m_neighborhoodTable->findNeighbor(nb, fromId);

          if(!foundVehicle){
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tTrying to insert neighbor Node_" << fromId << " from CN: "  << interest.getName().getSubName(0,5).toUri() << " - hopCount: " << hopCount << std::endl;
            insertOrUpdate(currentNodeId1, false, nmsi, fromForwarder);
            NFD_LOG_INFO("Node_" << nmsi.getNodeId() << " inserted in Node_" << currentNodeId1 << " Neighborood Table." << " hopCount: " << hopCount);
          }
          else{
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tNeighbor Node_" << fromId << " already in the NT." << std::endl;
          }
        }
      }
    }



    if(interest.getName().getSubName(3,1).toUri() == "/consumer"){
      if(!controlInterest){
        lastUpdateTimeInterest = ns3::Simulator::Now().GetMilliSeconds();
        controlInterest = true;
      }
      // thresholdInterest = 5;

      // std::cout << "Node (" << currentNodeId1 << ") Data prefix: " << data.getName().getSubName(0, 7).toUri()
      //           << "\t>>> NT size: " << m_neighborhoodTable->m_neighList.size() << std::endl;

      partial_sizesInterest += m_neighborhoodTable->m_neighList.size();
      packets_sequenceInterest++;
      // if (hopCount > thresholdInterest) {
      //     if (printToFileMobilityAwareStrategy)
      //         std::cout << "\t>>> Should Not be sent downstream... " << " - hopCount: " << hopCount << " - DestFinalId: "
      //                   << DestFinalId << std::endl;
      //     std::cout.rdbuf(coutbuf);
      //     return;
      // }

      // Check if 5 seconds have elapsed since the last update
      currentTimeInterest = ns3::Simulator::Now().GetMilliSeconds();
      elapsedSecondsInterest = currentTimeInterest - lastUpdateTimeInterest;

      if (elapsedSecondsInterest >= 50.0) { // 5000 milliseconds is equivalent to 5 seconds // 12500ms = 12.5s = 250m/20mps
          // Calculate the mean of m_neighborhoodTable->m_neighList.size()
          int meanNeighListSizeDataInterest = (packets_sequenceInterest > 0) ? partial_sizesInterest / packets_sequenceInterest : thresholdInterest; //calculateMeanNeighListSize();

          // std::cout << "partial_sizesInterest: " << partial_sizesInterest << " - packets_sequenceInterest: " << packets_sequenceInterest << " - meanNeighListSizeDataInterest: " << meanNeighListSizeDataInterest << " - elapsedSecondsInterest: " << elapsedSecondsInterest << std::endl;
          partial_sizesInterest = 0;
          packets_sequenceInterest = 0;

          // Update the thresholdInterest and reset the update time
          thresholdInterest = meanNeighListSizeDataInterest;
          // lastUpdateTimeInterest = currentTimeInterest;
      }
      lastUpdateTimeInterest = currentTimeInterest;
      // lastUpdateTimeInterest = ns3::Simulator::Now().GetMilliSeconds();

      // if (thresholdInterest > 14)
      //   thresholdInterest -= 2; //Just to correct ISR for scenarios with higher traffic.
      //
      // if (hopCount > 4){//thresholdInterest
      //   std::cout << "OUT! "<< " - hopCount: " << hopCount << " - thresholdInterest: " << thresholdInterest << std::endl;
      //   if(printToFileMobilityAwareStrategy)
      //     std::cout << "\t>>> Should Not be sent downstream... " << " - hopCount: " << hopCount << " - DestFinalId: " << DestFinalId << std::endl;
      //   std::cout.rdbuf(coutbuf);
      //   return;
      // }
    }





    // std::cout << "currentNodeId1: " << currentNodeId1 << " - DestFinalId: " << DestFinalId << " - OrigId: " << OrigId << std::endl;


    // const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
    // const fib::NextHopList& nexthops = fibEntry.getNextHops();
    // std::cout << "MobilityStrategy_lookupFib\n";
    // for(const auto& x : nexthops){
    //   std::cout << "Px: " << x.getPx() << " - Py: " << x.getPy() << " - Timestamp:" << x.getTimestamp() << " pitName(): " << (*pitEntry).getName() << " -- fibPrefix: " << fibEntry.getPrefix() << std::endl;
    // }



    // std::cout << fibEntry << std::endl;
    // for(const auto& x : nexthops){
    //   std::cout << "Let see nexthops.size(): " << nexthops.size() << " - ";
    //   for(const auto z : x){
    //     cout << z << " -- ";
    //   }
    //   cout << " -- fibEntry.getPrefix(): " << fibEntry.getPrefix() << std::endl;
    // }


    // std::cout << "Let see nexthops.size(): " << nexthops.size() << " - nexthops.front().getTimestamp():" << nexthops.back().getTimestamp() << " pitEntry->getName(): " << (*pitEntry).getName() << " -- fibEntry.getPrefix(): " << fibEntry.getPrefix() << std::endl;

      // ========= Thesis -> Algorithm 8: Incoming Interest processing =========
      // TODO: 20230309. Include "Destined to RSU" condition...

    if(nexthops.empty()){
      if(dest != 666){//OrigId != currentNodeId1 --- fromId != currentNodeId1
        // if(printToFileMobilityAwareStrategy){
        //   std::cout << "\tKnown Interest destination: Node_" << dest << ". Checking whether it is a neighbor..." << std::endl;
        // }

        // if(currentNodeId1 == dest)
        //   if(printToFileMobilityAwareStrategy)
        //     std::cout << "\tCurrent Node: Node_" << currentNodeId1 << " - destId: " << dest << ", should only be processed by me... " << std::endl;

        // if((currentNodeId1 != dest) && (hopCount > 33)){//? > 1?
        //   // std::cout << "\tCurrent Node: Node_" << currentNodeId1 << " - destId: " << dest << " - hopCount: " << hopCount << ", not for me! Bailing out! " << std::endl;
        //   std::cout.rdbuf(coutbuf);
        //   return;
        // }


        float fD = 0.0;
        float actualDestNodeDistance = 0.0;
        bool foundNeighbor = false;
        uint64_t nxtHop = 0;
        std::vector<std::tuple<uint64_t, float>> nxtHopList;
        // std::cout << "\tNo RSU found..." << std::endl;
        // std::cout << "\tLooking up mobile neighbors... " << std::endl;
        Neighbor actualDestNode;
        uint64_t neigb = 1;
        bool needToCheckSuitableNeighbors = false;
        for (auto & it: m_neighborhoodTable->m_neighList){
          if((it.getNodeId() != currentNodeId1) && (fromId != it.getNodeId())){// 20240405
            if (it.getStatus() == Neighbor::STATUS_ACTIVE){
              //std::cout << it << std::endl;
              float iDistance = it.getDistance();
              nxtHop = it.getNodeId();

              if(nxtHop == DestFinalId){// Is the DestFinalId nearby?
                actualDestNode = std::move(it.CopyNeighbor());
                actualDestNodeDistance = it.getDistance();
                if(printToFileMobilityAwareStrategy)
                  std::cout << "\tDestination Node_" << DestFinalId << " is a neighbor. Predicting its current location..." << std::endl;

                ns3::Callback <std::tuple<Neighbor, double>, Neighbor*, bool, uint16_t > stmpCallback;//, std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>>
                uint16_t scheduleOnOff = 0;
                bool control = true; // If true, the naame of rmse.txt will be rmse_x.txt, the NodeX_STMP_Estimation.txt will be NodeX_x_STMP_Estimation.txt

                Neighbor neighborTmp;
                double distance = 0.0;

                if(printToFileMobilityAwareStrategy)
                  std::cout << "\tDistance before prediction: " << actualDestNode.getDistance() << std::endl;
                ndn::util::StmpKF m_stmpInstance(&actualDestNode, control, scheduleOnOff);//, getNodeInfoSTMPVectorAddress()

                stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance);
                std::tie(neighborTmp, distance) = std::move(stmpCallback(&actualDestNode, control, scheduleOnOff));// , getNodeInfoSTMPVectorAddress()   -- neighbor->getNmsi()

                // std::cout << "RSU Neighbor after prediction - actualDestNode: " << actualDestNode << std::endl;
                if(printToFileMobilityAwareStrategy)
                  std::cout << "\t>>> Distance after prediction: " << neighborTmp.getDistance() << std::endl;

                if(distance <= 250){
                  if(printToFileMobilityAwareStrategy)
                    std::cout << "\t>>> Destination Node_" << DestFinalId << " is a neighbor, within the range (" << distance << " m). \n\t>>> Unicasting to it..." << std::endl;
                  interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(DestFinalId));// In the unicastInterest method...
                  broadcastInterest(interest, FaceEndpoint(ingress.face, DestFinalId), pitEntry);
                  std::cout.rdbuf(coutbuf);
                  return;
                }
                else{
                  if(printToFileMobilityAwareStrategy)
                    std::cout << "\tDestination Node_" << DestFinalId << " is a neighbor, out of range (" << distance << " m). Checking for suitable neighbors for relaying..." << std::endl;
                  needToCheckSuitableNeighbors = true; //
                  foundNeighbor = true;
                }
              }
              else{ // Other neighbors
                nxtHopList.push_back(std::make_tuple(nxtHop, iDistance));
                neigb++;
                foundNeighbor = true;// At least one neighbor has been found
              }
            }
          }
        }

        // << std::endl;

        if(needToCheckSuitableNeighbors == false)
          if(printToFileMobilityAwareStrategy)
            std::cout << "\tDestination Node_" << DestFinalId << " is not a neighbor. Checking for suitable neighbors for relaying..." << std::endl;

        if(foundNeighbor == true){// && (fD > 0)
          if(printToFileMobilityAwareStrategy)
            std::cout << "\tFound " << neigb << " neighbor(s). Showing only valid: " << std::endl;
          float farthestDistance = 0;
          uint64_t farthestNode = 666;

          uint16_t scheduleOnOff = 0;
          bool control = true; // If true, the naame of rmse.txt will be rmse_x.txt, the NodeX_STMP_Estimation.txt will be NodeX_x_STMP_Estimation.txt
          Neighbor neighborTmp;;


          ns3::Callback <std::tuple<Neighbor, double>, Neighbor*, bool, uint16_t > stmpCallback;//, std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>>
          float distance = 0.0;
          float prevDistance = 32000.0; // Invalid value
          uint64_t SelectedRelayNode = DestFinalId;

          bool foundRelayNode = false;
          int x = 0;
          for(auto & it: m_neighborhoodTable->m_neighList){
            if ((it.getStatus() == Neighbor::STATUS_ACTIVE) && (it.getNodeId() != fromId)){//&& (actualDestNode.getNodeId() != it.getNodeId())  && (actualDestNode.getNodeId() != fromId)
              Neighbor tmpN = std::move(it.CopyNeighbor());
              ndn::util::StmpKF m_stmpInstance(&tmpN, control, scheduleOnOff);//, getNodeInfoSTMPVectorAddress()
              stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance);


              std::tie(neighborTmp, distance) = std::move(stmpCallback(&tmpN, control, scheduleOnOff));// , getNodeInfoSTMPVectorAddress()   -- neighbor->getNmsi()
              if(printToFileMobilityAwareStrategy)
                std::cout << "\t" << ++x << "- Neighbor Node_" << it.getNodeId() << " - current Node_" << currentNodeId1 << ": Distance = " << distance;// << std::endl;

              auto distanceToDestination = toolsStrategy.CalculateDistance(neighborTmp.getGeoPosition().getLatPosition(), neighborTmp.getGeoPosition().getLonPosition(),
                                                                          actualDestNode.getGeoPosition().getLatPosition(), actualDestNode.getGeoPosition().getLonPosition());
              if(printToFileMobilityAwareStrategy)
                std::cout << " <---> Neighbor Node_" << it.getNodeId() << " - destination Node_" << DestFinalId << ": Distance = " << distanceToDestination << std::endl;

              if (distanceToDestination < prevDistance){
                prevDistance = distanceToDestination; // Keeps the shortest calculated distance to the destination
                SelectedRelayNode = it.getNodeId();
                foundRelayNode = true;
              }
            }
          }

          if(foundRelayNode){
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> Node_" << SelectedRelayNode << " is the nearest (" << prevDistance << "m) relay to the destination neighbor. \n\t>>> Unicasting to it..." << std::endl;
            interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(SelectedRelayNode)); // in the unicastInterest method
            broadcastInterest(interest, FaceEndpoint(ingress.face, SelectedRelayNode), pitEntry);
            std::cout.rdbuf(coutbuf);
            return;
          }
          else{
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> [0] No suitable Neighbor found... " << std::endl;
            bool inserted = false;

            if(fromId != currentNodeId1){
              inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
            // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri());
            }
            else{
              inserted = true;
            }


             if(inserted){//inserted
              interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
              broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
              if(printToFileMobilityAwareStrategy)
                std::cout << "\t>>> [0] No suitable Neighbor found. \n\t>>> Broadcasting first Interest..." << std::endl;
            }
            else{
              if(printToFileMobilityAwareStrategy)
                std::cout << "\t>>> [0] No suitable Neighbor found. \n\t>>> Discarding Interest..." << std::endl;
            }

            std::cout.rdbuf(coutbuf);
            return;
          }
        }
        else{// No suitable Neighbor found ... Broadcast
          // std::cout << "\t>>> No suitable Neighbor found. \n\t>>> Broadcasting...(or should we wait for a neighbor?)" << std::endl;
          // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
          // broadcastInterest(interest, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry);
          // std::cout.rdbuf(coutbuf);
          // return;
          if(printToFileMobilityAwareStrategy)
            std::cout << "\t>>> [1] No suitable Neighbor found... " << std::endl;
          // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
          // // bool inserted = addCN(container_1, interest.getName().getSubName(0,6).toUri()); ---_0

          bool inserted = false;

          if(fromId != currentNodeId1){
            inserted = addCN(container_1, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
          // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri());
          }
          else{
            inserted = true;
          }


           if(inserted){//inserted
            interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
            broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> [1] No suitable Neighbor found. \n\t>>> Broadcasting first Interest..." << std::endl;
          }
          else{
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> [1] No suitable Neighbor found. \n\t>>> Discarding Interest..." << std::endl;
          }

          std::cout.rdbuf(coutbuf);
          return;
        }

      } // (dest != 666) && (fromId != currentNodeId1)
      else {/*if(dest == 666)*/  /*if(dest == 666)*///Broadcast - it is destined to 666, broadcast address
        if(printToFileMobilityAwareStrategy)
          std::cout << "\tInterest for broadcast." << std::endl;


        if(interest.getName().getSubName(1,1).toUri() == "/sharingBF"){
          if(hopCount <= 2){
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> Unicasting sharingBF: " << interest.getName().getSubName(0,6).toUri() << " hopCount: " << hopCount << std::endl;
            auto dest = interest.getTag<lp::destIdTag>();

            // 20231220 - Added
            bool inserted = false;

            if(fromId != currentNodeId1){
              inserted = addCN(container_6, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
            // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri());
            }
            else{
              inserted = true;
            }



            if(inserted){//inserted
              interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(*dest));
              broadcastInterest(interest, FaceEndpoint(ingress.face, *dest), pitEntry);
              // if(printToFileMobilityAwareStrategy)
                // std::cout << "\t>>> [2] No suitable Neighbor found. \n\t>>> Broadcasting first Interest..." << std::endl;
            }
            else{
              if(printToFileMobilityAwareStrategy)
                std::cout << "\t>>> Discarding Beacon..." << std::endl;
            }




            // unicastInterest(interest, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry); 20230717
             // 20231220 - Commented
            // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(*dest));
            // broadcastInterest(interest, FaceEndpoint(ingress.face, *dest), pitEntry);
          }
          std::cout.rdbuf(coutbuf);
          return;
        }

        if (interest.getName().getSubName(1,1).toUri() == "/beacon"){
          if (hopCount <= nodeNum){
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> Broadcasting beacon: " << interest.getName().getSubName(0,6).toUri() << " hopCount: " << hopCount << std::endl;

            // 20231220 - Added
            bool inserted = false;

            if(fromId != currentNodeId1){
              inserted = addCN(container_7, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
            // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri());
            }
            else{
              inserted = true;
            }

            // Added for routing only. 20240228
            currentTimeInterestBeacon = ns3::Simulator::Now().GetMilliSeconds();
            elapsedSecondsInterestBeacon = currentTimeInterestBeacon - lastUpdateTimeInterestBeacon;

            if (elapsedSecondsInterestBeacon >= 50.0) { // 5000 milliseconds is equivalent to 5 seconds // 12500ms = 12.5s = 250m/20mps
                // Calculate the mean of m_neighborhoodTable->m_neighList.size()
                int meanNeighListSizeInterestBeacon = (packets_sequenceInterestBeacon > 0) ? partial_sizesInterestBeacon / packets_sequenceInterestBeacon : thresholdInterestBeacon; //calculateMeanNeighListSize();

                // std::cout << "partial_sizesInterestBeacon: " << partial_sizesInterestBeacon << " - packets_sequenceInterestBeacon: " << packets_sequenceInterest << " - meanNeighListSizeInterestBeacon: " << meanNeighListSizeInterestBeacon << " - elapsedSecondsInterestBeacon: " << elapsedSecondsInterest << std::endl;
                partial_sizesInterestBeacon = 0;
                packets_sequenceInterestBeacon = 0;

                // Update the thresholdInterest and reset the update time
                thresholdInterestBeacon = meanNeighListSizeInterestBeacon;
                // lastUpdateTimeInterestBeacon = currentTimeInterestBeacon;
            }
            lastUpdateTimeInterestBeacon = currentTimeInterestBeacon;
            // lastUpdateTimeInterest = ns3::Simulator::Now().GetMilliSeconds();

            if (thresholdInterestBeacon > 14)
              thresholdInterestBeacon -= 2; //Just to correct ISR for scenarios with higher traffic.

            if (0/*hopCount > thresholdInterestBeacon*/){//thresholdInterestBeacon
              // std::cout << "OUT!  - hopCount: " << hopCount << " - thresholdInterestBeacon: " << thresholdInterestBeacon << std::endl;
              // if(printToFileMobilityAwareStrategy)
                // std::cout << "\t>>> Should Not be sent downstream... " << " - hopCount: " << hopCount << " - DestFinalId: " << DestFinalId << std::endl;
              std::cout.rdbuf(coutbuf);
              return;
            }


            if(inserted){//inserted
              interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
              // if(hopCount <= 4){// Limiting hopcount
                broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
              // }
              // if(printToFileMobilityAwareStrategy)
                // std::cout << "\t>>> [2] No suitable Neighbor found. \n\t>>> Broadcasting first Interest..." << std::endl;
            }
            else{
              if(printToFileMobilityAwareStrategy)
                std::cout << "\t>>> Discarding Beacon..." << std::endl;
            }

            // 20231220 - Commented
            // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
            // broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
          }
          std::cout.rdbuf(coutbuf);
          return;
        }

        bool control = false;
        std::list<uint64_t> nodeIdFromBFList;// It will hold the nodes found from the ccTable
        uint64_t nodeIdFromBF = 555;

        control = existsInCCTable(currentNodeId1, prefix, nodeIdFromBFList);

        if(nodeIdFromBFList.empty() == false){// ccTable NOT empty

          float fD = 0.0;
          bool foundNeighbor = false;
          uint64_t nxtHop = 0;;
          uint64_t neigb = 0;


          std::vector<Neighbor> neighborCCTList;
          std::vector<Neighbor> neighborCCTListUpdated;
          bool nodeCctIsNeighbor = false;

          std::vector<Neighbor> otherNeighborsList;
          bool otherNeighbors = false;

          // std::cout << "\tccTable is NOT empty..." << std::endl;
          if(printToFileMobilityAwareStrategy)
            std::cout << "\tRequested content for CN: " << interest.getName().getSubName(0,6).toUri() << " found in the following Node(s), from the ccTable:" << std::endl; // from ccTable

          int t = 1;
          for(auto& nodeIdFromBF: nodeIdFromBFList) {
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t" << t++ << "- Node_" << nodeIdFromBF;

            Neighbor nb;
            auto it = m_neighborhoodTable->m_neighList.begin();
            for (; it != m_neighborhoodTable->m_neighList.end(); it++){
              if (nodeIdFromBF == it->getNodeId()){
                nb = std::move(it->CopyNeighbor());
                nb.m_ntimer_1.Cancel();
                neighborCCTList.push_back(nb); // pushes the neighbor
                if(printToFileMobilityAwareStrategy)
                  std::cout << " - **it is a neighbor. With a previous distance of " << it->getDistance() << " m. Predicting its actual position... ";
                nodeCctIsNeighbor = true;
              }
              else{
                nb = std::move(it->CopyNeighbor());
                nb.m_ntimer_1.Cancel();
                otherNeighborsList.push_back(nb);
                otherNeighbors = true;
              }
            }
            if(printToFileMobilityAwareStrategy)
              std::cout << std::endl;
          }

          double far_distance = 0;
          // Neighbor nexthopNeighbor;
          bool interestSent = false;

          if (nodeCctIsNeighbor == true){
            ns3::Callback <std::tuple<Neighbor, double>, Neighbor*, bool, uint16_t > stmpCallback;//, std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>>
            uint16_t scheduleOnOff = 0;
            bool control = true; // If true, the naame of rmse.txt will be rmse_node.txt, the NodeX_STMP_Estimation.txt will be NodeX_x_STMP_Estimation.txt

            Neighbor neighborTmp;
            double distance = 0.0;
            // double far_distance = 0;

            Neighbor neighborCCT;
            ndn::util::StmpKF m_stmpInstance(&neighborCCT, control, scheduleOnOff);
            stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance);

            auto it = neighborCCTList.begin();
            for (; it != neighborCCTList.end(); it++){
              neighborCCT = std::move(it->CopyNeighbor());
              // neighborCCT.m_ntimer_1.Cancel();

              if(printToFileMobilityAwareStrategy)
                std::cout << "\t Distance before prediction: " << neighborCCT.getDistance() << std::endl;
              std::tie(neighborTmp, distance) = std::move(stmpCallback(&neighborCCT, control, scheduleOnOff));

              // std::cout << "Neighbor after neighborCCT - neighborCCT: " << neighborCCT << std::endl;
              if(printToFileMobilityAwareStrategy)
                std::cout << "\t>>> Distance after prediction: " << neighborTmp.getDistance() << std::endl;

              neighborCCTListUpdated.push_back(std::move(neighborTmp)); // Keep the updated list after mobility prediction

              if (neighborTmp.getDistance() <= 250){     // IMPORTANT: Send to all Nodes in range...
                if(printToFileMobilityAwareStrategy)
                  std::cout << "\t>>> Node_" << neighborTmp.getNodeId() << " is in range (" << neighborTmp.getDistance() << "m).\n\t>>> Unicasting to it... " << std::endl;
                interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(neighborTmp.getNodeId()));
                broadcastInterest(interest, FaceEndpoint(ingress.face, neighborTmp.getNodeId()), pitEntry);
                interestSent = true;
                std::cout.rdbuf(coutbuf);
                return;
              }
              else{
                interestSent = false;
                if(printToFileMobilityAwareStrategy)
                  std::cout << "\t>>> Node_" << neighborTmp.getNodeId() << " out of range (" << neighborTmp.getDistance() << "m)." << std::endl;
              }
            }
          }
          // Is there any other situation in between not yet analysed?
          if((nodeCctIsNeighbor == false) || ((nodeCctIsNeighbor == true) && (interestSent == false))){ // nodeCctIsNeighbor == false
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tNo ccTable neighbors in range. Checking other neighbors for relaying... " << std::endl;

            if (otherNeighbors == true){
              if(printToFileMobilityAwareStrategy)
                std::cout << "\tFound " << otherNeighborsList.size() << " neighbor(s). Showing only valid: " << std::endl;
              float farthestDistance = 0;
              uint64_t farthestNode = 666;


              ns3::Callback <std::tuple<Neighbor, double>, Neighbor*, bool, uint16_t > stmpCallback;//, std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>>
              float distance = 0.0;
              float prevDistance = 32000.0; // Invalid value
              uint64_t SelectedRelayNode = dest;
              Neighbor neighborTmp;

              uint16_t scheduleOnOff = 0;
              bool control = true; // If true, the naame of rmse.txt will be rmse_node.txt, the NodeX_STMP_Estimation.txt will be NodeX_x_STMP_Estimation.txt

              int x = 0;
              bool foundRelayNode = false;
              auto iit = (interestSent == false) ? neighborCCTList.begin() : neighborCCTListUpdated.begin();
              for (; iit != ((interestSent == false) ? neighborCCTList.end() : neighborCCTListUpdated.end()); iit++) {
                for (auto& it: otherNeighborsList){
                  if ((it.getStatus() == Neighbor::STATUS_ACTIVE) && (iit->getNodeId() != it.getNodeId()) && (fromId != it.getNodeId())){
                    Neighbor tmpN = std::move(it.CopyNeighbor());
                    ndn::util::StmpKF m_stmpInstance(&tmpN, control, scheduleOnOff);//, getNodeInfoSTMPVectorAddress()
                    stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance);


                    std::tie(neighborTmp, distance) = std::move(stmpCallback(&tmpN, control, scheduleOnOff));// , getNodeInfoSTMPVectorAddress()   -- neighbor->getNmsi()
                    if(printToFileMobilityAwareStrategy)
                      std::cout << "\t" << ++x << "- Neighbor Node_" << it.getNodeId() << " - current Node_" << currentNodeId1 << ": Distance = " << distance;// << std::endl;

                    auto distanceToDestination = toolsStrategy.CalculateDistance(iit->getGeoPosition().getLatPosition(), iit->getGeoPosition().getLonPosition(),
                                                                                neighborTmp.getGeoPosition().getLatPosition(), neighborTmp.getGeoPosition().getLonPosition());

                    if(printToFileMobilityAwareStrategy)
                      std::cout << " <---> Neighbor Node_" << it.getNodeId() << " - destination Node_" << iit->getNodeId() << ": Distance = " << distanceToDestination << std::endl;

                    if (distanceToDestination < prevDistance){
                      prevDistance = distanceToDestination; // Keeps the shortest calculated distance to the destination
                      SelectedRelayNode = it.getNodeId();
                      foundRelayNode = true;
                    }
                  }
                }
              }
              if(foundRelayNode){
                if(printToFileMobilityAwareStrategy)
                  std::cout << "\t>>> Node_" << SelectedRelayNode << " is the nearest (" << prevDistance << "m) destination neighbor. \n\t>>> Unicasting to it..." << std::endl;
                interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(SelectedRelayNode)); // in the unicastInterest method
                broadcastInterest(interest, FaceEndpoint(ingress.face, SelectedRelayNode), pitEntry);
                std::cout.rdbuf(coutbuf);
                return;
              }
              else{// No suitable Neighbor found ... Broadcast
                // std::cout << "\t>>> No suitable Neighbor found. Broadcasting...(or should we wait for a neighbor?)" << std::endl;
                // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
                // broadcastInterest(interest, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry);
                // std::cout.rdbuf(coutbuf);
                // return;
                if(printToFileMobilityAwareStrategy)
                  std::cout << "\t>>> [2] No suitable/valid Neighbor found... " << std::endl;
                // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer); // 20230825 container_2 to container_1 -- _0
                // // bool inserted = addCN(container_2, interest.getName().getSubName(0,6).toUri());

                bool inserted = false;

                if(fromId != currentNodeId1){
                  inserted = addCN(container_2, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
                // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri());
                }
                else{
                  inserted = true;
                }



                 if(inserted){//inserted
                  interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
                  broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
                  if(printToFileMobilityAwareStrategy)
                    std::cout << "\t>>> [2] No suitable Neighbor found. \n\t>>> Broadcasting first Interest..." << std::endl;
                }
                else{
                  if(printToFileMobilityAwareStrategy)
                    std::cout << "\t>>> [2] No suitable Neighbor found. \n\t>>> Discarding Interest..." << std::endl;
                }

                std::cout.rdbuf(coutbuf);
                return;
              }
            }
            else{// No suitable Neighbor found ... Broadcast
              // std::cout << "\t>>> No suitable Neighbor found. Broadcasting...(or should we wait for a neighbor?)" << std::endl;
              // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
              // broadcastInterest(interest, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry);
              // std::cout.rdbuf(coutbuf);
              // return;

              if(printToFileMobilityAwareStrategy)
                std::cout << "\t>>> [3] No suitable/valid Neighbor found..." << std::endl;

              // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
              // // bool inserted = addCN(container_3, interest.getName().getSubName(0,6).toUri()); .._0

              bool inserted = false;

              if(fromId != currentNodeId1){
                inserted = addCN(container_3, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
              // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri());
              }
              else{
                inserted = true;
              }


               if(inserted){//inserted
                interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
                broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
                if(printToFileMobilityAwareStrategy)
                  std::cout << "\t>>> [3] No suitable Neighbor found. \n\t>>> Broadcasting first Interest..." << std::endl;
              }
              else{
                if(printToFileMobilityAwareStrategy)
                  std::cout << "\t>>> [3] No suitable Neighbor found. \n\t>>> Discarding Interest..." << std::endl;
              }

              std::cout.rdbuf(coutbuf);
              return;
            }
          }
        }
        else{ // control == 0 -> No ccTable entry
          if(printToFileMobilityAwareStrategy)
            std::cout << "\t>>> No local information (ccTable) about content provider..." << std::endl;


          // if(!m_neighborhoodTable->m_neighList.empty()){
          //
          //   auto it = m_neighborhoodTable->m_neighList.begin();
          //   ns3::Callback <std::tuple<Neighbor, double>, Neighbor*, bool, uint16_t > stmpCallback;//,
          //
          //   uint16_t scheduleOnOff = 0;
          //   int x = 0;
          //   bool control = true;
          //   float prevDistance = 32000.0; // Invalid value
          //   bool foundRelayNode = false;
          //   uint64_t SelectedRelayNode = dest;
          //
          //   for (; it != m_neighborhoodTable->m_neighList.end(); it++){
          //     if ((it->getStatus() == Neighbor::STATUS_ACTIVE) && (it->getNodeId() != fromId) && (currentNodeId1 != fromId)){
          //       Neighbor neighborTmp;
          //       float distance = 0.0;
          //
          //       Neighbor actualDestNode = std::move(it->CopyNeighbor());
          //       ndn::util::StmpKF m_stmpInstance(&actualDestNode, control, scheduleOnOff);//, getNodeInfoSTMPVectorAddress()
          //       stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance);
          //
          //
          //       std::tie(neighborTmp, distance) = std::move(stmpCallback(&actualDestNode, control, scheduleOnOff));// , getNodeInfoSTMPVectorAddress()   -- neighbor->getNmsi()
          //       if(printToFileMobilityAwareStrategy)
          //         std::cout << "\t" << ++x << "- Neighbor Node_" << it->getNodeId() << " - current Node_" << currentNodeId1 << ": Distance = " << distance;// << std::endl;
          //
          //       auto distanceToDestination = toolsStrategy.CalculateDistance(neighborTmp.getGeoPosition().getLatPosition(), neighborTmp.getGeoPosition().getLonPosition(),
          //                                                                   actualDestNode.getGeoPosition().getLatPosition(), actualDestNode.getGeoPosition().getLonPosition());
          //       if(printToFileMobilityAwareStrategy)
          //         std::cout << " <---> Neighbor Node_" << it->getNodeId() << " - destination Node_" << dest << ": Distance = " << distanceToDestination << std::endl;
          //
          //       if (distanceToDestination < prevDistance){
          //         prevDistance = distanceToDestination; // Keeps the shortest calculated distance to the destination
          //         SelectedRelayNode = it->getNodeId();
          //         foundRelayNode = true;
          //       }
          //     }
          //   }
          //
          //   if(foundRelayNode == true){
          //     if(printToFileMobilityAwareStrategy)
          //       std::cout << "\t>>> Node_" << SelectedRelayNode << " is the nearest (" << prevDistance << "m) relay to the destination. \n\t>>> Unicasting to it..." << std::endl;
          //     interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(SelectedRelayNode)); // in the unicastInterest method
          //     // unicastInterest(interest, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry); // 20230717
          //     unicastInterest(interest, FaceEndpoint(ingress.face, SelectedRelayNode), pitEntry);
          //     std::cout.rdbuf(coutbuf);
          //     return;
          //   }
          //   else{// No suitable Neighbor found ... Broadcast
          //     if(printToFileMobilityAwareStrategy)
          //       std::cout << "\t>>> No suitable Neighbor found ..." << std::endl;
          //     bool inserted = addCN(container_4, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
          //     // bool inserted = addCN(container_4, interest.getName().getSubName(0,6).toUri());
          //
          //      if(inserted){//inserted
          //       interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
          //       broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
          //       if(printToFileMobilityAwareStrategy)
          //         std::cout << "\t>>> [4] No suitable Neighbor found. \n\t>>> Broadcasting first Interest..." << std::endl;
          //     }
          //     else{
          //       if(printToFileMobilityAwareStrategy)
          //         std::cout << "\t>>> [4] No suitable Neighbor found. \n\t>>> Discarding Interest..." << std::endl;
          //     }
          //
          //     std::cout.rdbuf(coutbuf);
          //     return;
          //   }
          // }
          // else{ // NT empty
          //   // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
          //   // broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
          //   // std::cout.rdbuf(coutbuf);
          //   // return;
          //   // 20230810
          //   if(printToFileMobilityAwareStrategy)
          //     std::cout << "\t>>> No suitable Neighbor found ..." << std::endl;
          //   bool inserted = addCN(container_5, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
          //   // bool inserted = addCN(container_5, interest.getName().getSubName(0,6).toUri());
          //
          //    if(inserted){//inserted
          //     interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
          //     broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
          //     if(printToFileMobilityAwareStrategy)
          //       std::cout << "\t>>> [5] No suitable Neighbor found. \n\t>>> Broadcasting first Interest..." << std::endl;
          //     // std::cout.rdbuf(coutbuf);
          //     // return;
          //   }
          //   else{
          //     if(printToFileMobilityAwareStrategy)
          //       std::cout << "\t>>> [5] No suitable Neighbor found. \n\t>>> Discarding Interest..." << std::endl;
          //   }
          //
          //   std::cout.rdbuf(coutbuf);
          //   return;
          // }



        // Version 20230825
          if(printToFileMobilityAwareStrategy)
            std::cout << "\t>>> No suitable Neighbor found ..." << std::endl;
          // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
          // // bool inserted = addCN(container_4, interest.getName().getSubName(0,6).toUri()); .. _0


          bool inserted = false;

          if(fromId != currentNodeId1){
            inserted = addCN(container_4, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
          // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri());
          }
          else{
            inserted = true;
          }


           if(inserted){//inserted
            interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
            broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> [4] No suitable Neighbor found. \n\t>>> Broadcasting first Interest..." << std::endl;
          }
          else{
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> [4] No suitable Neighbor found. \n\t>>> Discarding Interest..." << std::endl;
          }

          std::cout.rdbuf(coutbuf);
          return;
        }
      }
    }
    else{// !nexthops.empty()
      // std::cout << "Only producer??? Node_" << currentNodeId1 << std::endl;
      // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(ingress.endpoint)); // TODO
      if((interest.getName().getSubName(1,1).toUri() == "/sharingBF") && (hopCount <= 1)){
        // std::cout << "\t>>> Broadcasting sharingBF... " << std::endl;
        // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
        // broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);

        if(printToFileMobilityAwareStrategy)
          std::cout << "\t>>> Unicasting sharingBF... hopCount: " << hopCount << std::endl;
        auto dest = interest.getTag<lp::destIdTag>();
        // broadcastInterest(interest, FaceEndpoint(ingress.face, 0), pitEntry);
        broadcastInterest(interest, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry);
        std::cout.rdbuf(coutbuf);
        return;
      }

      /*else*/
      {// Important: Routing here! 20240228 earlier: const fib::Entry& fibEntry = this->lookupFib(*pitEntry); const fib::NextHopList& nexthops = fibEntry.getNextHops();
        // if(printToFileMobilityAwareStrategy)
          // std::cout << "\t>>> Multicasting - !nexthops.empty(), prefix: " << interest.getName().getSubName(0,6).toUri() << " - nexthops.front().getTimestamp():" << nexthops.back().getTimestamp() <<  std::endl;
        // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(DestFinalId)); // Tag set in the multicastInterest method... 20240228
        // std::cout << "\tfibEntry.getName(): " << fibEntry.getName() << std::endl;



        multicastInterestFIB(interest, FaceEndpoint(ingress.face, DestFinalId), pitEntry, nexthops, fibEntry);// TODO! extract destId from FIB entry - Done! 20240417
        /*************************************/

      //   std::cout << "nexthops.front().getPx(): " << nexthops.front().getPx() << std::endl;
      //   // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
      //
      //   // static int toControl = 0;
      //   Face& inFace = ingress.face;
      //   for (const auto& nexthop : nexthops) {
      //       Face& outFace = nexthop.getFace();
      //       // uint64_t endpointId = nexthop.getEndpointId();
      //
      //       if ((outFace.getId() == inFace.getId() && outFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) ||
      //           wouldViolateScope(inFace, interest, outFace)) {
      //           continue;
      //           }
      //       // std::cout << "\tmulticastInterestFIB - currentNode: " << currentNode->GetId() << " - outFace.getId(): " << outFace.getId() << " nexthop.getEndpointId(): " << nexthop.getEndpointId() << " - nexthop.getTimestamp(): " << nexthop.getTimestamp() << " - nexthop.getPx(): " << nexthop.getPx() << " - nexthop.getPy(): " << nexthop.getPy() << " - nexthop.getVx(): " << nexthop.getVx() << " - nexthop.getVy(): " << nexthop.getVy() << " - prefix: " << interest.getName().toUri() << "- nexthops.size(): " << std::endl;//<< nexthops.size() << std::endl;
      //
      //
      //       // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(outFace.getId()));
      //       this->sendInterest(pitEntry, FaceEndpoint(outFace, nexthop.getEndpointId()), interest);// TODO: extract endpointId from FIB - 20230609 --- nexthop.getEndpointId() --- 20240313 - tag is set in Forwarder::onOutgoingInterest(...)
      //
      //       // pitEntry->getOutRecord(outFace, 0)->insertStrategyInfo<OutRecordInfo>().first->isNonDiscoveryInterest = 1;  // Elidio 20230109
      //       // NFD_LOG_DEBUG("send non-discovery Interest=" << interest << " from="
      //       // << inFace.getId() << " to=" << outFace.getId());
      //       // if(printToFileMobilityAwareStrategy) std::cout << "multicastInterest: send non-discovery Interest=" << interest << " from="
      //       // << inFace.getId() << " to=" << outFace.getId() << std::endl;
      //
      // // 		if(printToFileMobilityAwareStrategy) std::cout << "Multicast Interest - ingress.endpoint =" << ingress.endpoint << std::endl;
      //   }


        /*************************************/







      }

      std::cout.rdbuf(coutbuf);
      return;
    }

  }
  else{// For baseline data extraction
    const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
    const fib::NextHopList& nexthops = fibEntry.getNextHops();

    if(!nexthops.empty()){
      multicastInterest(interest, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry, nexthops);
    }
    else{
      broadcastInterest(interest, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry);
    }
  }
  /** reset cout buffer **/
  if(printToFileMobilityAwareStrategy) std::cout.rdbuf(coutbuf);

  // Start the event loop after all events are scheduled
  // face.getIoService().run();
}








/***********************************************************************************************************************************/
/************************************************* afterReceiveData ****************************************************************/
/***********************************************************************************************************************************/

void
MobilityAwareStrategy::afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
										const FaceEndpoint& ingress, const Data& data)
{

    // redirecting std::cout to file
  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto currentNodeId1 = currentNode->GetId ();



  auto destId = data.getTag<lp::destIdTag>(); // Will be extracted from LSTM
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

  // if(fromId == currentNodeId1){//fromId == currentNodeId1
  // std::cout << "Exiting on CN: " << data.getName().getSubName(0,6).toUri() << " - fromId: " << fromId << " - currentNodeId1: " << currentNodeId1 << std::endl;
  //   // std::cout.rdbuf(coutbuf);
  //   return;
  // }

  uint64_t hopCount = 0;
  auto hopCountTag = data.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  else{
    NFD_LOG_DEBUG("Not possible to extract the hopCount from Data... ");
  }


  // std::cout << "For Logging - Data: " << data.getName().getSubName(0,5).toUri() << " - currentNodeId1: " << currentNodeId1 << " - destId: " << dest << " - fromId: " << fromId << " - hopCount: " << hopCount <<  std::endl;


  if(data.getName().getSubName(1,1).toUri() != "/sharingBF"){// Not for .../sharingBF/ BF..
    // std::cout << "\t[afterReceiveData] Data packet " << data.getName().getSubName(0,4).toUri() << " received!" << std::endl;
    // NFD_LOG_INFO("[afterReceiveData] Beacon Data packet " << data.getName().getSubName(0,4).toUri() << " received!");
    auto outRecord = pitEntry->getOutRecord(ingress.face, currentNodeId1); // TODO -- ingress.endpoint --- 0 to currentNodeId1
    if (outRecord == pitEntry->out_end()) {
      NFD_LOG_DEBUG("Data " << data.getName() << " from=" << ingress << " no out-record");
      // if(printToFileMobilityAwareStrategy)
        // std::cout << ">>>> Data " << data.getName() << " from=" << ingress << " no out-record" << " - ingress.endpoint: " << ingress.endpoint << " - currentNodeId1: " << currentNodeId1 << std::endl;
      // return;
    }
  }

  if(data.getName().getSubName(1,1).toUri() == "/sharingBF"){
    // std::cout << "\tSharing Data packet: " << data.getName().getSubName(0,4).toUri() << " received - hopCount: " << hopCount << std::endl;
    NFD_LOG_INFO("Node_" << currentNodeId1 << " received shared BF -> " << data.getName().getSubName(0,4).toUri());
  }



// 	/***************************************************/
//     /**
//      * LSTM is used to propose the next-hop Relay (relayId)
//      * NMSI is included into the packet (LpPacket)
//      * endpoint = (Interest)origId
//      */



  // std::ios_base::openmode mode;
  static int forAppendData = 0;
  std::ofstream out;

  std::string file = tmp_dir + "mobility-aware-strategy_AfterReceiveData" + std::to_string(currentNodeId1) + ".txt";
  auto *coutbuf = std::cout.rdbuf();

  if(printToFileMobilityAwareStrategy) {
    if (forAppendData == 0){
      out.open(file, std::fstream::trunc);
      forAppendData++;
    }
    else
      out.open(file, std::fstream::app);

    std::cout.rdbuf(out.rdbuf());
  }

  // if(printToFileMobilityAwareStrategy) std::cout << "\n" << ns3::Simulator::Now ().GetMilliSeconds() << std::endl;

  // Get updated mobility information, for the LSTM algorithm

  if(printToFileMobilityAwareStrategy)
    std::cout << "\n---> Received Data at timestamp: " << ns3::Simulator::Now ().GetMilliSeconds() << std::endl;

  uint32_t timestamp1;
  auto timestamp = data.getTag<lp::timestampTag>();
  if(timestamp != nullptr){
    timestamp1 = timestamp->get();
    if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted Timestamp: " << timestamp1 << std::endl;
  }

  double vx_gt = 0.0;
  double vy_gt = 0.0;
  double vz_gt = 0.0;
  std::tie(vx_gt, vy_gt, vz_gt) = toolsStrategy.GetNodeSpeedFromDataPacket(data);
  // if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted Node Speed: " << vx_gt << "," << vy_gt << "," << vz_gt << std::endl;
  ndn::util::NodeSpeed nodespeed(vx_gt, vy_gt);


  double px = 0.0;
  double py = 0.0;
  double pz = 0.0;
  std::tie(px, py, pz) = toolsStrategy.GetNodeLocationFromDataPacket(data);
  // if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted Node Geoposition: " << px << "," << py << "," << pz << std::endl;
  ndn::util::GeoPosition geoposition(px, py, pz);

  // auto destId = data.getTag<lp::destIdTag>(); // Will be extracted from LSTM
  // uint64_t dest;
  // if(destId != nullptr){
  //   dest = destId->get();
    if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted DestId: " << dest << std::endl;
  // }
  // else
  //   if(printToFileMobilityAwareStrategy) std::cout << "\tDestId NOT set!" << std::endl;

  // uint64_t fromId;
  // auto FromId = data.getTag<lp::fromIdTag>();
  // if(FromId != nullptr){
  //   fromId = FromId->get();
    if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted FromId: " << fromId << std::endl;
  // }
  // else{
  //   fromId = 99;// TODO
  // }

  // int hopCount = 0;
  // auto hopCountTag = data.getTag<lp::HopCountTag>();
  // if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
  //   hopCount = *hopCountTag;
  // }
  // else{
  //   NFD_LOG_DEBUG("Not possible to extract the hopCount from Data... ");
  // }



  // std::cout << "testing endpoint: " << ingress.endpoint << std::endl;

  // /** reset cout buffer **/
  // std::cout.rdbuf(coutbuf);


  // Insert the neighbors into the Neighborhood Table
  std::string prefix = data.getName().getSubName(0,6).toUri();// Only first four components
  // double dir = 9.9;


  std::string DataFinalDestId = data.getDestFinalID();
  uint64_t DestFinalId =  666;// Changed from OrigId to DestFinalId

  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&DataFinalDestId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<0>(NodeIdMacMap) == DataFinalDestId;});
  if (it != NodeIdMacMap.end()) {
    DestFinalId = std::get<1>(*it);
  }
  else{
    if(printToFileMobilityAwareStrategy)
      std::cout << "[AfterReceiveData]. Broadcast... DestFinalId = " << DestFinalId << std::endl;
  }

  // if (OrigId == currentNodeId1){
  //   std::cout << "Loop?: " << OrigId << " == " << currentNodeId1 << std::endl;

  // }

  // ============================ Thesis -> Algorithm 7: Incoming Data processing ==================================
  //==================================== Processing the Nighborhood Tables =========================================
  if (1){//data.getName().getSubName(0,1).toUri() != "/localhost"
    // Create NMSI
    ndn::util::Nmsi nmsi(fromId, prefix, geoposition, nodespeed, hopCount, timestamp1);

    if(fromId != currentNodeId1){//fromId == currentNodeId1
    //   return;
    // }
    // else{
      if (fromId == nodeNum){ // It is a RSU

        Neighbor nb;
        bool foundRsuNode = false;
        foundRsuNode = m_neighborhoodRSUTable->findRsu(nb);

        if(!foundRsuNode){ // Insert the RSU into the Neighbor list
          if(printToFileMobilityAwareStrategy)
            std::cout << "\tTrying to insert neighbor RSU_" << fromId << "  from a Beacon CN: "  << data.getName().getSubName(0,5).toUri() << " - hopCount: " << hopCount << std::endl;
          insertOrUpdate(currentNodeId1, true, nmsi, fromForwarder);
          insertOrUpdate(currentNodeId1, false, nmsi, fromForwarder);// Also insert into normal NT
          NFD_LOG_INFO("RSU_" << nmsi.getNodeId() << " inserted in Node_" << currentNodeId1 << " Neighborood Table");
        }
        else{
          if(printToFileMobilityAwareStrategy)
            std::cout << "\t>>> RSU_" << fromId << " already in the NT." << std::endl;
        }
      }
      else {// It is a vehicle
        bool foundVehicle = false;
        Neighbor nb;
        foundVehicle = m_neighborhoodTable->findNeighbor(nb, fromId);

        if(!foundVehicle){
          if(printToFileMobilityAwareStrategy)
            std::cout << "\tTrying to insert neighbor Node_" << fromId << " from CN: "  << data.getName().getSubName(0,5).toUri() << " - hopCount: " << hopCount << std::endl;
          insertOrUpdate(currentNodeId1, false, nmsi, fromForwarder);
          NFD_LOG_INFO("Node_" << nmsi.getNodeId() << " inserted in Node_" << currentNodeId1 << " Neighborood Table." << " hopCount: " << hopCount);
        }
        else{
          if(printToFileMobilityAwareStrategy)
            std::cout << "\t>>> Neighbor Node_" << fromId << " already in the NT." << std::endl;
        }
      }
    }
    // std::cout << "\treceived Data packet: " << data.getName().getSubName(0,5).toUri() << std::endl;

    //==================================== Processing the data and BF =========================================
    // std::cout << "Important! -> " << "OrigId: " << OrigId << " - currentNodeId1: " << currentNodeId1 << " fromId: " << fromId << " endpoint: " << ingress.endpoint << std::endl;

    if(0){//fromId == currentNodeId1
      std::cout.rdbuf(coutbuf);
      return;
    }
    else{
    // if(1){

      // uint64_t timestamp = ns3::Simulator::Now().GetMilliSeconds();
      // std::cout << timestamp << std::endl;

      static int BloomFilter_size = 50000;
      static int BloomFilter_NumberOfHashFunctions = 20;
      ::ndn::util::BloomFilter bf(BloomFilter_NumberOfHashFunctions, BloomFilter_size);
      ::ndn::util::BloomFilter bf2 = bf.decodeFromBinaryBlock(data.getContent(), BloomFilter_NumberOfHashFunctions, BloomFilter_size);


      if(data.getName().getSubName(1,1).toUri() == "/sharingBF"){ //==================================== sharingBF
        if(printToFileMobilityAwareStrategy)
          std::cout << "\tsharingBF Data packet " << data.getName().getSubName(0,4).toUri() << " received - hopCount: " << hopCount << ". Inserting only with hopCount equal to 1 or 2." << std::endl;

        if((hopCount > 1) || (hopCount < 3)){ // 1, 2 - hop --->>
          bf2.printHex();
          auto cctEntry = std::find_if(m_ccTable.begin(), m_ccTable.end(),
            [&](const auto& entry) { return std::get<0>(entry) == currentNodeId1; });

          if (cctEntry == m_ccTable.end()) {//New BF
            NS_LOG_INFO("Inserting a new sharingBF. fromId: " << fromId << " ...");
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tInserting a new sharingBF. fromId: " << fromId << " ..." << std::endl;
            std::tuple<uint64_t, ::ndn::util::BloomFilter> receivedFromTuple = std::make_tuple(fromId, bf2);
            std::vector<std::tuple<uint64_t, ::ndn::util::BloomFilter>> receivedFromTuples{ receivedFromTuple };
            CCTEntry newCCTEntry = std::make_tuple(currentNodeId1, receivedFromTuples);
            m_ccTable.push_back(newCCTEntry);
          }
          else { // existing currentNode and BF
            auto& receivedFromTuples = std::get<1>(*cctEntry);
            auto receivedFromTupleIt = std::find_if(receivedFromTuples.begin(), receivedFromTuples.end(),
              [&](const auto& sEntry) { return std::get<0>(sEntry) == fromId; }); // This fromId already exists?

            if (receivedFromTupleIt == receivedFromTuples.end()) { // No! add fromId and received BF
              NS_LOG_INFO("Existing sharingBF. New fromId: " << fromId << " ...");
              if(printToFileMobilityAwareStrategy)
                std::cout << "\tExisting sharingBF. New fromId: " << fromId << std::endl;
              std::tuple<int, ::ndn::util::BloomFilter> receivedFromTuple = std::make_tuple(fromId, bf2);
              receivedFromTuples.push_back(receivedFromTuple);
            }
            else { // Yes! Add BF to existing fromId
              NS_LOG_INFO("Existing sharingBF. existing fromId: " << fromId << " ...");
              if(printToFileMobilityAwareStrategy)
                std::cout << "\tExisting sharingBF. existing fromId: " << fromId << std::endl;
              auto& bfTuple = *receivedFromTupleIt;
              if(!(std::get<1>(bfTuple) == bf2)){// Only new BF is added
                std::get<1>(bfTuple) = std::move(bf2.copyBF());
              }
            }
          }
        }
      }
      // else

      if(data.getName().getSubName(1,1).toUri() == "/beacon"){  //==================================== Beacon
        if(printToFileMobilityAwareStrategy)
          std::cout << "\tBeacon Data packet " << data.getName().getSubName(0,4).toUri() << " received - hopCount: " << hopCount << ". Inserting only with hopCount greater than 1." << std::endl;

        if(hopCount > 0){
          bf2.printHex();
          auto cctEntry = std::find_if(m_ccTable.begin(), m_ccTable.end(),
            [&](const auto& entry) { return std::get<0>(entry) == currentNodeId1; });

          if (cctEntry == m_ccTable.end()) {//New BF
            NS_LOG_INFO("Inserting a new beaconBF. fromId: " << fromId << " ...");
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tInserting a new beaconBF. fromId: " << fromId << std::endl;
            std::tuple<uint64_t, ::ndn::util::BloomFilter> receivedFromTuple = std::make_tuple(fromId, bf2);
            std::vector<std::tuple<uint64_t, ::ndn::util::BloomFilter>> receivedFromTuples{ receivedFromTuple };
            CCTEntry newCCTEntry = std::make_tuple(currentNodeId1, receivedFromTuples);
            m_ccTable.push_back(newCCTEntry);
          }
          else { // existing currentNode and BF
            auto& receivedFromTuples = std::get<1>(*cctEntry);
            auto receivedFromTupleIt = std::find_if(receivedFromTuples.begin(), receivedFromTuples.end(),
              [&](const auto& sEntry) { return std::get<0>(sEntry) == fromId; }); // This fromId already exists?

            if (receivedFromTupleIt == receivedFromTuples.end()) { // No! add fromId and received BF
              NS_LOG_INFO("Existing beaconBF. New fromId: " << fromId << " ...");
              if(printToFileMobilityAwareStrategy)
                std::cout << "\tExisting beaconBF. New fromId: " << fromId << std::endl;
              std::tuple<int, ::ndn::util::BloomFilter> receivedFromTuple = std::make_tuple(fromId, bf2);
              receivedFromTuples.push_back(receivedFromTuple);
            }
            else { // Yes! Add BF to existing fromId
              NS_LOG_INFO("Existing beaconBF. existing fromId: " << fromId << " ...");
              if(printToFileMobilityAwareStrategy)
                std::cout << "\tExisting beaconBF. existing fromId: " << fromId << std::endl;
              auto& bfTuple = *receivedFromTupleIt;
              if(!(std::get<1>(bfTuple) == bf2)){// Only new BF is added
                std::get<1>(bfTuple) = std::move(bf2.copyBF());
              }
            }
          }
        }

        // Creates local sharingBF - if Node is the Data destination, thus only nodes in the forwarding path send sharingBF
        if(hopCount > 1){// Do not request sharing BF from the producer as it has shared the Beacon Data packet which includes its BF.
          // NS_LOG_INFO("Creating and sharing a local BF. fromId: " << currentNodeId1 << " and destId:" << destId->get());
          if(printToFileMobilityAwareStrategy)
            std::cout << "\tRequesting a sharingBF from Node_" << fromId << "..." << std::endl;
          // std::cout << "Creating and sharing a local BF. fromId: " << fromId << " and destId:" << destId->get() << std::endl;

          invokeFromClient = true;
          auto BFClientCRoMo2 = dynamic_cast<ns3::ndn::BFClientCRoMo2*>(&(*ns3::NodeList::GetNode(ns3::Simulator::GetContext())->GetApplication(0)));


          // // Wrap the method call with a lambda function
          // auto ClientBFCall = [&BFClientCRoMo2](int val) {
            // BFClientCRoMo2->ShareBF(val);
          // };
          // ns3::Simulator::Schedule (ns3::Seconds (0), &ClientBFCall, ClientBFCall, fromId);


          // auto ClientBFCall = std::bind(&ns3::ndn::BFClientCRoMo2::ShareBF, &BFClientCRoMo2, fromId);
          ns3::Simulator::Schedule (ns3::Seconds (0), &ns3::ndn::BFClientCRoMo2::ShareBF, BFClientCRoMo2, fromId);//ScheduleNextPacket -- ShareBF

          // ns3::Simulator::Schedule (ns3::Seconds (0), &ns3::ndn::BFClientCRoMo2::ShareBF, ClientBFCall, fromId);

        }



        // Added for routing only. 20240228
        currentTimeDataBeacon = ns3::Simulator::Now().GetMilliSeconds();
        elapsedSecondsDataBeacon = currentTimeDataBeacon - lastUpdateTimeDataBeacon;

        if (elapsedSecondsDataBeacon >= 50.0) { // 5000 milliseconds is equivalent to 5 seconds // 12500ms = 12.5s = 250m/20mps
            // Calculate the mean of m_neighborhoodTable->m_neighList.size()
            int meanNeighListSizeDataBeacon = (packets_sequenceDataBeacon > 0) ? partial_sizesDataBeacon / packets_sequenceDataBeacon : thresholdDataBeacon; //calculateMeanNeighListSize();

            // std::cout << "partial_sizesDataBeacon: " << partial_sizesDataBeacon << " - packets_sequenceDataBeacon: " << packets_sequenceDataBeacon << " - meanNeighListSizeDataBeacon: " << meanNeighListSizeDataBeacon << " - elapsedSecondsDataBeacon: " << elapsedSecondsDataBeacon << std::endl;
            partial_sizesDataBeacon = 0;
            packets_sequenceDataBeacon = 0;

            // Update the thresholdDataBeacon and reset the update time
            thresholdDataBeacon = meanNeighListSizeDataBeacon;
            // lastUpdateTimeDataBeacon = currentTimeDataBeacon;
        }
        lastUpdateTimeDataBeacon = currentTimeDataBeacon;
        // lastUpdateTimeInterest = ns3::Simulator::Now().GetMilliSeconds();

        if (thresholdDataBeacon > 14)
          thresholdDataBeacon -= 2; //Just to correct ISR for scenarios with higher traffic.

        if (hopCount > thresholdDataBeacon){//thresholdInterestBeacon
          // std::cout << "OUT!  - hopCount: " << hopCount << " - thresholdDataBeacon: " << thresholdDataBeacon << std::endl;
          // if(printToFileMobilityAwareStrategy)
            // std::cout << "\t>>> Should Not be sent downstream... " << " - hopCount: " << hopCount << " - DestFinalId: " << DestFinalId << std::endl;
          std::cout.rdbuf(coutbuf);
          return;
        }

        if(currentNodeId1 != nodeNum){// not RSU, forwards it downstream
          auto paTag = data.getTag<lp::PrefixAnnouncementTag>();
          if (paTag != nullptr) {
      // 	  std::cout << "Was discovery???... Adding route..." << std::endl;

            // ndn::FibHelper::AddRoute("id0", "/data", "n1", 1);
            // if(printToFileMobilityAwareStrategy)
              // std::cout << "Route added... prefix: " << *paTag->get().getPrefixAnn() << std::endl;
            // ndn::FibHelper::AddRoute("id0", *paTag->get().getPrefixAnn(), "n1", 1);



            // addRoute(pitEntry, ingress, data, *paTag->get().getPrefixAnn()); xx

            // ns3::Ptr<ns3::Node> NodeToAdd = ns3::NodeList::GetNode(fromId);
            // std::cout << "Route added... prefix: " << *paTag->get().getPrefixAnn() << " - fromId: " << fromId << " - hopCount: " << hopCount << std::endl;

            // if(hopCount > 0)
            // ::ns3::ndn::FibHelper::AddRoute(std::move(NodeToAdd), fromId, paTag->get().getPrefixAnn()->getAnnouncedName(), ingress.face.getId(), hopCount);// hopCount = metric = Cost
            // uint64_t t = 0;
            // uint32_t h = 0;
            // double ppx = 2.2;
            // double ppy = 3.3;
            // double vvx = 4.4;
            // double vvy = 5.5;
            // ::ns3::ndn::FibHelper::AddRoute(std::move(NodeToAdd), fromId, t, h, ppx, ppy, vvx, vvy, paTag->get().getPrefixAnn()->getAnnouncedName(), ingress.face.getId(), hopCount);// hopCount = metric = Cost



            // auto printFib = [](ns3::Ptr<ns3::Node> node) {
            //   auto ndn = node->GetObject<ns3::ndn::L3Protocol>();
            //   for (const auto& entry : ndn->getForwarder()->getFib()) {
            //     cout << entry.getPrefix() << " (";
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
            //       cout << " endpointId: " << nextHop.getEndpointId() << " - Timestamp: " << nextHop.getTimestamp() << " - Hops: " << nextHop.getHops() << " - Px: " << nextHop.getPx() << " - Py: " << nextHop.getPy() << " - Vx: " << nextHop.getVx() << " - Vy: " << nextHop.getVy();
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
            // cout << "FIB content on node currentNodeId1" << endl;
            // NodeToAdd = ns3::NodeList::GetNode(currentNodeId1);
            // printFib(NodeToAdd);
          }


          // if(hopCount <= 4)
            sendDataToAllCafs(pitEntry, ingress, data); // changed "ingress" to "FaceEndpoint(ingress.face, 0)"
        }
      }
      else if (data.getName().getSubName(3,1).toUri() == "/consumer"){ //=============================== normal Data
        auto paTag = data.getTag<lp::PrefixAnnouncementTag>();
        if (paTag != nullptr) {// THIS will be used with the Routing algorithm!!
          NS_LOG_INFO("Adding route from prefix annoucement! ...");
          if(printToFileMobilityAwareStrategy)
            std::cout << "\tAdding route from prefix annoucement! " << std::endl;
          // // // addRoute(pitEntry, ingress, data, *paTag->get().getPrefixAnn());
        }
        else { // Data contains no PrefixAnnouncement, upstreams do not support mobility-aware
          NS_LOG_INFO("NOP. No prefix annoucements! ");
          if(printToFileMobilityAwareStrategy)
            std::cout << "\t>>> Multicasting the Data downstream... " << std::endl;

          /*if ((DestFinalId == currentNodeId1) && (hopCount > 5)){*/// 40: 4; 60: 5 - low ISR. Use 6 or 7; 80 and 100: 9, (7) - Ok. = To run 6 for 80 =. Now, 60: 5. --- Adopted 60: 6




          // std::cout << " Node (" << currentNodeId1 << ") Data prefix: " << data.getName().getSubName(0,7).toUri() << "\t>>> NT size: " << m_neighborhoodTable->m_neighList.size() << std::endl;




          // Assuming currentTimeData is a variable storing the current simulation time
          if(!controlData){
            lastUpdateTimeData = ns3::Simulator::Now().GetMilliSeconds();
            controlData = true;
          }
          // thresholdData = 5;

          // std::cout << "Node (" << currentNodeId1 << ") Data prefix: " << data.getName().getSubName(0, 7).toUri()
          //           << "\t>>> NT size: " << m_neighborhoodTable->m_neighList.size() << std::endl;

          partial_sizesData += m_neighborhoodTable->m_neighList.size();
          packets_sequenceData++;
          neighborhoodTableSize = partial_sizesData;
          // if (hopCount > thresholdData) {
          //     if (printToFileMobilityAwareStrategy)
          //         std::cout << "\t>>> Should Not be sent downstream... " << " - hopCount: " << hopCount << " - DestFinalId: "
          //                   << DestFinalId << std::endl;
          //     std::cout.rdbuf(coutbuf);
          //     return;
          // }

          // Check if 5 seconds have elapsed since the last update
          currentTimeData = ns3::Simulator::Now().GetMilliSeconds();
          elapsedSecondsData = currentTimeData - lastUpdateTimeData;

          if (elapsedSecondsData >= 3000.0) { // 5000 milliseconds is equivalent to 5 seconds // 12500ms = 12.5s = 250m/20mps
              // Calculate the mean of m_neighborhoodTable->m_neighList.size()
              int meanNeighListSizeData = (packets_sequenceData > 0) ? partial_sizesData / packets_sequenceData : thresholdData; //calculateMeanNeighListSize();

              // std::cout << "partial_sizesData: " << partial_sizesData << " - packets_sequenceData: " << packets_sequenceData << " - meanNeighListSizeData: " << meanNeighListSizeData << " - elapsedSecondsData: " << elapsedSecondsData << std::endl;
              partial_sizesData = 0;
              packets_sequenceData = 0;

              // Update the thresholdData and reset the update time
              thresholdData = meanNeighListSizeData;
              // lastUpdateTimeData = currentTimeData;
          }
          lastUpdateTimeData = currentTimeData;
          // lastUpdateTimeData = ns3::Simulator::Now().GetMilliSeconds();

          //Just to correct ISR for scenarios with higher traffic.
          // if (thresholdData > 15){
          //   thresholdData = 10;
          // }
          // else
          if (thresholdData >= 10){
            thresholdData -= 7;
          }

          // else if(thresholdData > 14){
          //   thresholdData -= 2;
          // }

          // 20240131
          // if (hopCount > 4){//thresholdData
          //   std::cout << "OUT! "<< " - hopCount: " << hopCount << " - thresholdData: " << thresholdData << std::endl;
          //   if(printToFileMobilityAwareStrategy)
          //     std::cout << "\t>>> Should Not be sent downstream... " << " - hopCount: " << hopCount << " - DestFinalId: " << DestFinalId << std::endl;
          //   std::cout.rdbuf(coutbuf);
          //   return;
          // }

          // // Commented 20230826 -- See "sendDataToAllCafs"
          // uint64_t validNeighbor = 0;
          // if(!m_neighborhoodTable->m_neighList.empty()){
          //
          //   auto it = m_neighborhoodTable->m_neighList.begin();
          //   for (; it != m_neighborhoodTable->m_neighList.end(); it++){
          //     if ((it->getStatus() == Neighbor::STATUS_ACTIVE) && (it->getNodeId() != fromId)){// && (currentNodeId1 != fromId)
          //       validNeighbor++;
          //       break; // There is at least one neighbor
          //     }
          //   }
          // }
          // else {
          //   if (m_neighborhoodRSUTable->m_neighList.empty()){
          //     std::cout.rdbuf(coutbuf);
          //     return;
          //   }
          //   else{
          //     if(printToFileMobilityAwareStrategy)
          //       std::cout << "\tm_neighborhoodRSUTable not empty... But not broadcasting..." << std::endl;
          //     // validNeighbor++;
          //   }
          // }

          if ((ingress.face.getId() >= 0) && (data.getName().getSubName(3,1).toUri() == "/consumer")){
            auto ndn = currentNode->GetObject<::ns3::ndn::L3Protocol>();
            auto&& entry = ndn->getForwarder()->getFib().findLongestPrefixMatch(data.getName());
            shared_ptr<Face> face = ndn->getFaceById(ingress.face.getId());
            // std::cout << "[FibHelper::AddRoute]. entry.getPrefix(): " << entry.getPrefix() << " - data.getName(): " << data.getName() << " - Fib.size(): " << ndn->getForwarder()->getFib().size() << " -- Node_" << currentNode->GetId() << std::endl;
            // if(1/*(entry.getPrefix().getSubName(0,6).toUri() != "/") && (entry.getPrefix().getSubName(0,6).toUri() != "/cromo/test")*/){
            //   // ndn->getForwarder()->getFib().addOrUpdateNextHop(entry, ingress.face, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 60);
            //   // entry.addOrUpdateNextHop(ingress.face, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 60);
            //   // ::ns3::ndn::FibHelper::AddRoute(currentNode, fromId, timestamp1, hopCount, px, py, vx_gt, vy_gt, data.getName(), face, hopCount);
            //   std::cout << "... Tree... entry.getPrefix().getSubName(0,6).toUri(): " << entry.getPrefix().getSubName(0,6).toUri() << std::endl;
            // }
          }


          if(!m_neighborhoodTable->m_neighList.empty())//validNeighbor
          {
            ns3::Ptr<ns3::Node> NodeToAdd = ns3::NodeList::GetNode(fromId);
            // std::cout << "Route added... prefix: " << *paTag->get().getPrefixAnn() << " - fromId: " << fromId << " - hopCount: " << hopCount << std::endl;

            // uint64_t t = 0;
            // uint32_t h = 0;
            // double ppx = 2.2;
            // double ppy = 3.3;
            // double vvx = 4.4;
            // double vvy = 5.5;

            // XXX 20240320
            // ::ns3::ndn::FibHelper::AddRoute(NodeToAdd, fromId, timestamp->get(), hopCount, px, py, vx_gt, vy_gt, data.getName().getSubName(0, 6).toUri(), ingress.face.getId(), hopCount);//


            // if ((ingress.face.getId() > 250) && (data.getName().getSubName(3,1).toUri() == "/consumer") && (fromId != currentNodeId1)){
            //   ::ns3::ndn::FibHelper::AddRoute(currentNode, fromId, timestamp->get(), hopCount, px, py, vx_gt, vy_gt, data.getName().getSubName(0, 6).toUri(), ingress.face.getId(), hopCount);//
            //   std::cout << "extracting data. nodeId: " << currentNodeId1 << " - fromId: " << fromId << " - prefix: " << prefix << std::endl;
            // }

            // 20240326 -- Inserted in forwaredr.cpp
            // if ((ingress.face.getId() >= 0) && (data.getName().getSubName(1,1).toUri() != "/beacon") && (data.getName().getSubName(1,1).toUri() != "/sharingBF")){
            //   ::ns3::ndn::FibHelper::AddRoute(currentNode, fromId, timestamp->get(), hopCount, px, py, vx_gt, vy_gt, data.getName(), ingress.face.getId(), hopCount);//
            //   // ::ns3::ndn::FibHelper::AddRoute(currentNode, data.getName(), ingress.face.getId(), hopCount);//
            //
            //
            //   // std::cout << "extracting data. nodeId: " << currentNodeId1 << " - fromId: " << fromId << " - prefix: " << data.getName() << std::endl;
            // }

            auto printFib = [](ns3::Ptr<ns3::Node> node) {
              auto ndn = node->GetObject<ns3::ndn::L3Protocol>();
              for (const auto& entry : ndn->getForwarder()->getFib()) {
                cout << entry.getPrefix() << " (";

                bool isFirst = true;
                for (auto& nextHop : entry.getNextHops()) {
                  if (0/*nextHop.getFace().getId() != 257*/)
                    continue;
                  cout << nextHop.getFace();
                  auto& face = nextHop.getFace();
                  auto transport = dynamic_cast<ns3::ndn::NetDeviceTransport*>(face.getTransport());
                  if (transport == nullptr) {
                    continue;
                  }

                  cout << " fromId: " << nextHop.getEndpointId() << " - Timestamp: " << nextHop.getTimestamp() << " - Hops: " << nextHop.getHops() << " - Px: " << nextHop.getPx() << " - Py: " << nextHop.getPy() << " - Vx: " << nextHop.getVx() << " - Vy: " << nextHop.getVy();

                  if (!isFirst)
                    cout << ", ";
                  cout << ns3::Names::FindName(transport->GetNetDevice()->GetChannel()->GetDevice(1)->GetNode());
                  isFirst = false;
                }
                cout << ")" << endl;
              }
            };


            if(currentNodeId1 != fromId){
              // cout << "FIB content on Node_" << currentNodeId1 << " - having received packet from Node_" << fromId << endl;
              // NodeToAdd = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
              // printFib(currentNode);
            }


            sendDataToAllCafs(pitEntry, ingress, data);
          }

          // sendDataToAllCafs(pitEntry, ingress, data);
        }
      }

      // Delete this
      // if (data.getName().getSubName(1,1).toUri() == "/beacon"){ //=============================== normal Data
      //   auto paTag = data.getTag<lp::PrefixAnnouncementTag>();
      //   if (paTag != nullptr) {// THIS will be used with the Routing algorithm!!
      //     NS_LOG_INFO("Adding route from prefix annoucement! ...");
      //     if(printToFileMobilityAwareStrategy)
      //       std::cout << "\tAdding route from prefix annoucement! " << std::endl;
      //     addRoute(pitEntry, ingress, data, *paTag->get().getPrefixAnn());
      //   }
      // }


    }
  }
  else{ // For Baseline
    sendDataToAllCafs(pitEntry, ingress, data);
  }

  /** reset cout buffer **/
  std::cout.rdbuf(coutbuf);
}//auto customApp = dynamic_cast<ns3::ndn::BFServerCRoMo2*>(&(*ns3::NodeList::GetNode(m_forwarder->m_nodeId)->GetApplication(0)));









uint64_t
MobilityAwareStrategy::insertOrUpdate(const uint64_t currentNodeId1, const bool rsu, const ndn::util::Nmsi& nmsi, const bool fromForwarder) // true -> to inform the method that has been invoked from forwarder
{
  uint64_t neighborId = nmsi.getNodeId();
  if (neighborId == currentNodeId1){
    if(printToFileMobilityAwareStrategy)
      std::cout << "Tryed to insert itself as neighbor. Aborted!" << std::endl;
    return 0; // Do not try to insert the current node as its own neighbor...
  }

  std::ofstream out;
  std::string file;
  // Neighbor m_neighbor;
  m_neighbor.setNmsi(nmsi.getNmsi());
  m_neighbor.setStatus(Neighbor::STATUS_ACTIVE);

  auto node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto mobModel = node->GetObject<ns3::MobilityModel> ();
  auto position = mobModel->GetPosition ();
  m_neighbor.setDistance(toolsStrategy.CalculateDistance(position.x, position.y, nmsi.getGeoPosition().getLatPosition(), nmsi.getGeoPosition().getLonPosition()));

  if(rsu == true){//Insert RSU
    bool RSUfound = false;
    Neighbor nb;
    RSUfound = m_neighborhoodRSUTable->findRsu(nb);

    if ((RSUfound == false) || ((RSUfound == true) && (nb.getNodeId() != nmsi.getNodeId()))){
      m_neighborhoodRSUTable->m_neighList.push_back(std::move(m_neighbor));
      auto it = m_neighborhoodRSUTable->m_neighList.end();
      --it;
      if(!fromForwarder)
        if(printToFileMobilityAwareStrategy)
          std::cout << "\t>>> RSU_" << it->getNodeId() << " inserted as neighbor." << std::endl;
      it->m_ntimer_1.Cancel();
    }

  }
  else {
    // m_neighbor.m_ntimer_1.SetDelay (ns3::MilliSeconds(neighborTimer));
    bool foundNode = false;
    std::list<Neighbor>::iterator it;
    foundNode = m_neighborhoodTable->findIntermediateNode(nmsi.getNodeId(), it);

    if (foundNode == true){// Existing neighbor
      if (it->m_ntimer_1.IsRunning()){ // Update
        // std::cout << "Is there some running timer here!... " << std::endl;
        auto delayLeft = it->m_ntimer_1.GetDelayLeft();
        if ((delayLeft <= ns3::MilliSeconds(100)) ){ //update   && (delayLeft >= ns3::Seconds(1)) 500ms...
          if(!fromForwarder)
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> Existing Node_" << nmsi.getNodeId() << ", updating it..." << std::endl;
          it->m_ntimer_1.Cancel();
          // it->m_ntimer_1.Remove();

          // file = tmp_dir + std::to_string(currentNodeId1) + "_Node_stmp_" + std::to_string(it->getNodeId()) + ".txt";
          // auto *coutbuf = std::cout.rdbuf();
          // // Dump the existing NMSI to a local file
          // out.open(file.c_str(), std::fstream::app);//std::fstream::trunc
          // std::cout.rdbuf(out.rdbuf());
          // std::cout << it->getNmsi() << '$' << std::endl; // The '$' is necessary to end the input!

          // /** reset cout buffer **/
          // std::cout.rdbuf(coutbuf);

          // Then erase it
          it = m_neighborhoodTable->m_neighList.erase(it);

          it = m_neighborhoodTable->m_neighList.insert(it, std::move(m_neighbor));
          // --it;

          it->m_ntimer_1.Cancel();
          // int16_t a = 0; // For debugging
          it->m_ntimer_1.SetDelay (ns3::MilliSeconds(neighborTimer));
          it->m_ntimer_1.SetFunction (&MobilityAwareStrategy::updateNeighborInfo, this);
          it->m_ntimer_1.SetArguments (currentNodeId1, it);
          it->m_ntimer_1.Schedule();

        return 1;
        }
        else {// Discard
          // std::cout << "InsertOrUpdate: m_ntimer_1.GetDelayLeft() = " << m_neighbor.m_ntimer_1.GetDelayLeft() << " - Node: " << m_neighbor.getNodeId() << " discarded... " << std::endl;
          // if (printToFileMobilityAwareStrategy) std::cout << "Remaining timer delay: " << it->m_ntimer_1.GetDelayLeft() << ", for Neighbor-node " << it->getNmsi().getNodeId() << std::endl;
          if(!fromForwarder){
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> Existing Node_" << nmsi.getNodeId() << ", no updating needed ..." << std::endl;
            NS_LOG_INFO("Remaining timer delay: " << it->m_ntimer_1.GetDelayLeft() << ", for Neighbor-node " << it->getNmsi().getNodeId());
          }
          return 3;
        }
      }
      else{ // Neighbor exists in NL but its timer is not running. Means that timer expired Or the node status in INACTIVE
        if (it->getStatus() == Neighbor::STATUS_ACTIVE){
          position = mobModel->GetPosition ();
          auto distancex = toolsStrategy.CalculateDistance(position.x, position.y, it->getGeoPosition().getLatPosition(), it->getGeoPosition().getLonPosition());
          if (distancex > 250){
            if(!fromForwarder)
              if(printToFileMobilityAwareStrategy)
                std::cout << "\t>>> Existing Node_" << nmsi.getNodeId() << " but now out of range. Tagging it as inactive" << std::endl;
            it->setStatus(Neighbor::STATUS_INACTIVE);
          }
          else{
            // std::cout << "\tNode_" << it->getNodeId() << ", at distance: " << distancex << ", found with timer stopped. Resetting it..." << std::endl;
            // NS_LOG_INFO("Node_" << it->getNodeId() << ", at distance: " << distancex << ", found with timer stopped. Resetting it...");
            if(!fromForwarder)
              if(printToFileMobilityAwareStrategy)
                std::cout << "\t>>> Existing Node_" << nmsi.getNodeId() << ", but with its timer stopped. Re-scheduling..." << std::endl;
            it->m_ntimer_1.SetDelay (ns3::MilliSeconds(neighborTimer));
            it->m_ntimer_1.SetFunction (&MobilityAwareStrategy::updateNeighborInfo, this);
            it->m_ntimer_1.SetArguments (currentNodeId1, it);
            it->m_ntimer_1.Schedule();
          }
          return 4;
        }
        else{// Just testing...
          // std::cout << "\t>>> Node_" << nmsi.getNodeId() << " not inserted... about to delete? Status: " << it->getStatus() << " - distance: " << it->getDistance() << std::endl;

          if(!fromForwarder)
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t>>> Existing Node_" << nmsi.getNodeId() << " in \"limbo!\", with status \"Inactive\", waiting for deletion. Updating it..." << std::endl;
          it->m_ntimer_1.Cancel();
          // it->m_ntimer_1.Remove();

          // file = tmp_dir + std::to_string(currentNodeId1) + "_Node_stmp_" + std::to_string(it->getNodeId()) + ".txt";
          // auto *coutbuf = std::cout.rdbuf();
          // // Dump the existing NMSI to a local file
          // out.open(file.c_str(), std::fstream::app);//std::fstream::trunc
          // std::cout.rdbuf(out.rdbuf());
          // std::cout << it->getNmsi() << '$' << std::endl; // The '$' is necessary to end the input!

          // /** reset cout buffer **/
          // std::cout.rdbuf(coutbuf);

          // Then erase it
          it = m_neighborhoodTable->m_neighList.erase(it);

          it = m_neighborhoodTable->m_neighList.insert(it, std::move(m_neighbor));
          // --it;

          it->m_ntimer_1.Cancel();
          // int16_t a = 0; // For debugging
          it->m_ntimer_1.SetDelay (ns3::MilliSeconds(neighborTimer));
          it->m_ntimer_1.SetFunction (&MobilityAwareStrategy::updateNeighborInfo, this);
          it->m_ntimer_1.SetArguments (currentNodeId1, it);
          it->m_ntimer_1.Schedule();

          return 5;
        }
      }
    }
    else{// Insert new neighbor

      // insertTable(m_neighbor);
      if(!fromForwarder)
        if(printToFileMobilityAwareStrategy)
          std::cout << "\t>>> Node_" << nmsi.getNodeId() << " inserted." << std::endl;
      m_neighborhoodTable->m_neighList.push_back(std::move(m_neighbor));

      std::list<Neighbor>::iterator it2 = m_neighborhoodTable->m_neighList.end();
      --it2;
      // int16_t c = 0; // For debugging
      it2->m_ntimer_1.SetDelay (ns3::MilliSeconds(neighborTimer));
      it2->m_ntimer_1.SetFunction (&MobilityAwareStrategy::updateNeighborInfo, this);
      it2->m_ntimer_1.SetArguments (currentNodeId1, it2);
      it2->m_ntimer_1.Schedule();


      // file = tmp_dir + std::to_string(currentNodeId1) + "_Node_stmp_" + std::to_string(it2->getNodeId()) + ".txt";
      // out.open(file.c_str(), std::fstream::trunc);//std::fstream::trunc
      // auto *coutbuf = std::cout.rdbuf();

      // std::cout.rdbuf(out.rdbuf());
      // std::cout << it2->getNmsi() << '$' << std::endl; // The '$' is necessary to end the input!

      // /** reset cout buffer **/
      // std::cout.rdbuf(coutbuf);
      return 6;
    }
  }
    /** reset cout buffer **/
    // std::cout.rdbuf(coutbuf);

  return 7;
}




void
MobilityAwareStrategy::updateNeighborInfo(uint64_t currentNodeId1, std::list<Neighbor>::iterator neighbor)
{
  Neighbor backupNeigiborOriginal;
  // This is done in order to copy neighbor but not its Timer
  backupNeigiborOriginal.setNmsi(std::move(neighbor->getNmsi()));
  backupNeigiborOriginal.setDistance(std::move(neighbor->getDistance()));
  backupNeigiborOriginal.setStatus(std::move(neighbor->getStatus()));
  backupNeigiborOriginal.m_ntimer_1.Cancel();
  // backupNeigiborOriginal.m_ntimer_1.Remove();

  uint64_t timestamp = ns3::Simulator::Now ().GetMilliSeconds();
  // std::cout << "\n" << ns3::Simulator::Now ().GetSeconds() << std::endl;


  ns3::Callback <std::tuple<Neighbor, double>, Neighbor*, bool, uint16_t > stmpCallback;//, std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>>
  uint16_t scheduleOnOff = 0;
  bool control = false;

  ndn::util::Nmsi nmsiTemp2;
  Neighbor neighborTmp;
  double distance = 0.0;

  ndn::util::StmpKF m_stmpInstance(&(*neighbor), control, scheduleOnOff);//, getNodeInfoSTMPVectorAddress()

  stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance);
  std::tie(neighborTmp, distance) = stmpCallback(&(*neighbor), control, scheduleOnOff);// , getNodeInfoSTMPVectorAddress()   -- neighbor->getNmsi()

  neighborTmp.m_ntimer_1.Cancel();
  // neighborTmp.m_ntimer_1.Remove();

  // if(currentNodeId1 <= nodeNum)//nodeNum  // REACTIVATE?
  m_neighborTracer->Record ("updateNeighborInfo", timestamp, currentNodeId1, backupNeigiborOriginal, neighborTmp);//backupNeigiborModified

  if (distance <= 250){
    // neighbor->setDistance(distance);
    // neighbor->setStatus(Neighbor::STATUS_ACTIVE);
    // neighbor->setNmsi(std::move(neighborTmp.getNmsi()));
    *neighbor = std::move(neighborTmp.CopyNeighbor());

    NS_LOG_INFO("Timer elapsed, rescheduling... on Node: " << neighbor->getNmsi().getNodeId() << currentNodeId1);
    // std::cout << "Timer elapsed, rescheduling... on Node: " << neighbor->getNodeId() << " From Current Node: " << currentNodeId1 << std::endl;

    neighbor->m_ntimer_1.SetDelay (ns3::MilliSeconds(neighborTimer));
    neighbor->m_ntimer_1.SetFunction (&MobilityAwareStrategy::updateNeighborInfo, this);
    neighbor->m_ntimer_1.SetArguments (currentNodeId1, neighbor);
    neighbor->m_ntimer_1.Schedule();

  }
  else{
    neighbor->setStatus(Neighbor::STATUS_INACTIVE);
    neighbor->m_ntimer_1.Cancel();
    // std::cout << "Node: " << currentNodeId1 << " setting Neighbor: " << neighbor->getNmsi().getNodeId()  << " inactive! " << std::endl;
  }

}



uint64_t
MobilityAwareStrategy::deleteInActiveNeighbor(std::shared_ptr<NeighborhoodList>& neighborhoodTable){

  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto currentNodeId1 = currentNode->GetId ();

  // std::cout << "Node: " << currentNodeId1 << " evoking deleteInActiveNeighbor... " << std::endl;

//   std::cout << "Does it reaches here?" << std::endl;
  uint64_t numberOfInactiveNeighborsDeleted = 0;
  uint64_t numberOfActiveNeighborsDeleted = 0;

  uint64_t activeNodes = 1000;
  uint64_t inactiveNodes = 1000;
  uint64_t unknownStatusNodes = 1000;

   // uint64_t timestamp;// = ns3::Simulator::Now ().GetSeconds();

  std::ofstream out;
  // std::string file = tmp_dir + std::to_string(currentNodeId1) + "_Node_stmp_" + std::to_string(neighborId) + ".txt";
  std::string file;
  file = tmp_dir + "NeighborhoodList_Node" + std::to_string(currentNodeId1) + ".txt";
  auto *coutbuf = std::cout.rdbuf();



//   auto neighborsSuspended = suspendResumeAllNeighborTimers(0); // Suspend all
  if (neighborhoodTable->m_neighList.empty())
    return 0; // Nothing to do here
  std::list<Neighbor>::iterator it = neighborhoodTable->m_neighList.begin();
  // std::cout << "Node: " << currentNodeId1 << " evoking deleteInActiveNeighbor... Neighbor 0: " << it->getNmsi() << std::endl;//it->getNmsi() <<


  std::vector<uint64_t> inactiveNodesV;//20241222
  while (it != neighborhoodTable->m_neighList.end()){
    if (it->getStatus() == Neighbor::STATUS_INACTIVE){

    //  it->m_ntimer_1.Cancel();
    //  it->m_ntimer_1.Remove();
     // std::cout << " Erased Inactive: node " << it->getNodeId() << std::endl;
     if (it->getNodeId() <= nodeNum){//nodeNum
       inactiveNodes = it->getNodeId();
       inactiveNodesV.push_back(it->getNodeId()); //20241222
       }
     // inactiveNodes.push_back(it->getNodeId());

     if(printToFileMobilityAwareStrategy){
      out.open(file.c_str(), std::fstream::app);//std::fstream::trunc
      std::cout.rdbuf(out.rdbuf());}

     if(printToFileMobilityAwareStrategy)
       std::cout << ns3::Simulator::Now ().GetMilliSeconds() << std::endl;

    //  NS_LOG_INFO("Erased Inactive node: " << it->getNodeId());
     if(printToFileMobilityAwareStrategy)
       std::cout << "\tErased Inactive: node " << it->getNodeId() << std::endl;

     it->m_ntimer_1.Cancel();
     it->m_ntimer_1.Remove();
     it = neighborhoodTable->m_neighList.erase(it);// The iterator stays updated, pointing to the next just after the deleted entry.
    //  --it;
     std::cout.rdbuf(coutbuf);

     ++numberOfInactiveNeighborsDeleted;
    //  ++it; //?
    //  std::cout << " # Erased Inactive nodes: " << numberOfInactiveNeighborsDeleted << std::endl;
    }
    else if (it->getStatus() == Neighbor::STATUS_ACTIVE){
      if (it->getNodeId() <= nodeNum)//nodeNum
        activeNodes = it->getNodeId();
      // activeNodes.push_back(it->getNodeId());

      if(printToFileMobilityAwareStrategy){
        out.open(file.c_str(), std::fstream::app);//std::fstream::trunc
        std::cout.rdbuf(out.rdbuf());
      }

      if(printToFileMobilityAwareStrategy)
        std::cout << "\tExisting active: node " << it->getNodeId() << std::endl;
      std::cout.rdbuf(coutbuf);
      // it++;
    }
    else {
      if (it->getNodeId() <= nodeNum)//nodeNum
        unknownStatusNodes = it->getNodeId();
      // unknownStatusNodes.push_back(it->getNodeId());
      if(printToFileMobilityAwareStrategy){
        out.open(file.c_str(), std::fstream::app);//std::fstream::trunc
        std::cout.rdbuf(out.rdbuf());
      }
      if(printToFileMobilityAwareStrategy)
        std::cout << "\tExisting unknown_status: node " << it->getNodeId() << std::endl;
      std::cout.rdbuf(coutbuf);
      // it++; //TODO See if they still exist
    }
    it++;
  }


  /************************* For ROUTING *********************************/

  const nfd::Fib& fib = currentNode->GetObject<ns3::ndn::L3Protocol>()->getForwarder()->getFib();
	// 	(*outputStream) << "FIB content on node" << node->GetId() << endl;
	//
    // for (Fib::Entry& entry : fib)
  // static int xxx = 0;
  for (nfd::Fib::const_iterator entry = fib.begin(); entry != fib.end(); entry++){
    // std::cout << "First for: x = " << xxx++ << std::endl;
      // (*outputStream) << ns3::Simulator::Now ().ToDouble (ns3::Time::MS) << "\t[" << fib.size() << "]: "
      // /*(*outputStream)*/ << entry->getPrefix() << " - (";
      // bool isFirst = true;
    // nfd::Fib::Entry&& fibEntry =  *entry;
    auto nome = entry->getPrefix();
    // nfd::fib::Entry& entrada = std::move(*entry);
      // auto fibEntry = fib.findLongestPrefixMatch(nome);
      int numOfValidNextHopEntries = 0;
      int numOfNextHops = entry->getNextHops().size();
      // static int yyy = 0;
      for (auto nextHop = entry->getNextHops().end(); nextHop != entry->getNextHops().begin(); nextHop--){//for (auto &nexthops = entry->getNextHops().begin(); nextHop != entry->getNextHops().end(); nexthops++) --for (auto& nextHop : entry->getNextHops())
        // std::cout << "First for: y = " << yyy++ << std::endl;
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

        uint64_t timestamp = ns3::Simulator::Now ().GetMilliSeconds();
        uint64_t Timedifference = timestamp - Timestamp;
        Name nome(entry->getPrefix());

        if ((Timedifference > 2000) && (endpointID <= nodeNum)){
          // std::cout << "Strategy["<< currentNode->GetId () <<"]> removing a nextHop... Endpoint: " << nextHop->getEndpointId() << " --Prefix: " << entry->getPrefix() << " -- FIB Timestamp - timestamp = (" << timestamp << " - " << Timestamp << ") = " << Timedifference << " ms" << std::endl;
          ::ns3::ndn::FibHelper::RemoveRoute(currentNode, endpointID, Timestamp, Cost, Px, Py, Vx, Vy, nome, currentNode->GetObject<ns3::ndn::L3Protocol>()->getFaceById(257), Cost); //OK -- node

          // ::ns3::ndn::FibHelper::RemoveRoute(node, entry->getPrefix(), node->GetObject<ns3::ndn::L3Protocol>()->getFaceById(257), endpointID);

          // fib.removeNextHop(fibEntry, face);
          // auto removed =
          // this->getFib().removeNextHop(entrada, face);
          // removeNextHop(face, endpointID, Timestamp, Cost, Px, Py, Vx, Vy, endpointID);
          // fib.addOrUpdateNextHop(fibEntry, face, endpointID, Timestamp, Cost, Px, Py, Vx, Vy, endpointID);

          // std::cout << " - Removed? = " << removed << std::endl;
          // (*outputStream) << "faceId: " << face.getId() << " - endpointId: " << endpointID << " - Cost: " << Cost << " - Timestamp: " << Timestamp << " - Px: " << Px << " - Py: " << Py << " - Vx: " << Vx << " - Vy: " << Vy << "; ";
        }
        for(auto& n: inactiveNodesV){
          if (n == endpointID){
            ::ns3::ndn::FibHelper::RemoveRoute(currentNode, endpointID, Timestamp, Cost, Px, Py, Vx, Vy, nome, currentNode->GetObject<ns3::ndn::L3Protocol>()->getFaceById(257), Cost);
          }
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
  /***********************************************************************/

  /** reset cout buffer **/
  std::cout.rdbuf(coutbuf);

  this->m_ntimer.Cancel();
  this->m_ntimer.SetDelay (ns3::MilliSeconds(neighborhoodTimer));
  this->m_ntimer.SetFunction (&MobilityAwareStrategy::deleteInActiveNeighbor, this);
  this->m_ntimer.SetArguments (neighborhoodTable);
  this->m_ntimer.Schedule();

  return numberOfInactiveNeighborsDeleted;
}



bool
MobilityAwareStrategy::existsInCCTable(const uint64_t actualnode, const std::string cn, std::list<uint64_t>& fromIdList){
  bool FoundBF = false;
  bool foundcctEntry = false;
  if(printToFileMobilityAwareStrategy)
    std::cout << "\tChecking ccTable... " << std::endl;
  for (const auto& entry: m_ccTable) {
    uint64_t fromNodeIdBF = std::get<0>(entry);
    // std::string file = tmp_dir + "cromo_ccTable" + std::to_string(fromNodeIdBF) + ".txt";

    // std::ofstream out(file, std::fstream::trunc);
    // auto *coutbuf = std::cout.rdbuf();
    // std::cout.rdbuf(out.rdbuf());


    // std::list<uint64_t> fromIdList;
    if(actualnode == fromNodeIdBF){
      foundcctEntry = true;
      bool inBF = false;
      for (const auto& receivedFromTuple: std::get<1>(entry)) {
        uint64_t fromId = std::get<0>(receivedFromTuple);
        ::ndn::util::BloomFilter bf = std::get<1>(receivedFromTuple);
        // std::cout << "From: " << fromId << std::endl;
        // std::cout << "Executing inner for from existsInCCTable... " << std::endl;
        inBF = bf.contains(cn);
        if(inBF){
          FoundBF = true;
          inBF = false;
          if(printToFileMobilityAwareStrategy)
            std::cout << "\t>>> Prefix: " << cn << " found in Node_" << fromId << " via received BF." << std::endl;
          // NS_LOG_INFO("Prefix: " << cn << " found in Node: " << fromId << " via BF.");
          fromIdList.push_back(fromId);
          // return std::make_tuple(inBF, fromId);// TODO: Only the first instance found?
        }
        else {
          if(printToFileMobilityAwareStrategy)
            std::cout << "\t>>> Prefix: " << cn << " not found in Node_" << actualnode << "'s ccTable." << std::endl;
        }
      }
    }

    // std::cout << "For Debugging. fromNodeIdBF: " << fromNodeIdBF << std::endl;
  }
  if (!foundcctEntry)
    if(printToFileMobilityAwareStrategy)
      std::cout << "\t>>> Node_" << actualnode << " still does not have a ccTable entry..." << std::endl;

  return FoundBF;
}



void
MobilityAwareStrategy::SendBeacon(uint64_t nodeId, bool control) {
  bool sentBeacon = false;
  uint64_t randomContentionTime = 0;
  uint64_t timestamp = ns3::Simulator::Now ().GetMilliSeconds();
  if(control){
    auto ConsumerCRoMo2BeaconAll = dynamic_cast<ns3::ndn::ConsumerCRoMo2BeaconAll*>(&(*ns3::NodeList::GetNode(ns3::Simulator::GetContext())->GetApplication(2)));

    ns3::Ptr<ns3::UniformRandomVariable> randomNum = ns3::CreateObject<ns3::UniformRandomVariable> ();
    randomContentionTime = (uint64_t) randomNum->GetValue(0, 10); // to give much range for different delays

    // std::cout.rdbuf(coutbuf);
    invokeConsumerClient = true;
    if(nodeBeaconTimerGlobal->IsRunning() == false){
      holdSendingBeacon(nodeId, true);
      if(printToFileMobilityAwareStrategy)
        std::cout << timestamp << " - Node_" << nodeId << " sending a beacon in... " << randomContentionTime << " ms" << std::endl;
      ns3::Simulator::Schedule (ns3::MilliSeconds (randomContentionTime), &ns3::ndn::ConsumerCRoMo2BeaconAll::ScheduleNextPacket, ConsumerCRoMo2BeaconAll);//
      sentBeacon = true;
    }
    else{
      holdSendingBeacon(nodeId, false);
      sentBeacon = false;
    }
  }
  else{
    if(sentBeacon == true)
      if(printToFileMobilityAwareStrategy)
        std::cout << "\tTimer Reset for node_" << nodeId << ", at timestamp: " << ns3::Simulator::Now ().GetMilliSeconds() << " after scheduling Beacon sending... " << std::endl;
    else
      if(printToFileMobilityAwareStrategy)
        std::cout << "\tTimer Reset for node_" << nodeId << ", at timestamp: " << ns3::Simulator::Now ().GetMilliSeconds() << " without scheduling Beacon sending... " << std::endl;
  }
  // Reset the timer for the current node
  m_nodeBeaconTimer->Cancel();
  m_nodeBeaconTimer->SetFunction(&MobilityAwareStrategy::SendBeacon, this);
  m_nodeBeaconTimer->SetArguments(nodeId, true); // on creation, it only schedule the timer. Next calling, for sending Beacon. The control flag is set to True inside the method
  m_nodeBeaconTimer->Schedule(ns3::MilliSeconds(vehicleBeaconInterval + randomContentionTime)); // Assuming 'interval' is defined somewhere in your code
}

void MobilityAwareStrategy::holdSendingBeacon(uint64_t nodeId, bool control){
  if(control == false){
    if(printToFileMobilityAwareStrategy)
      std::cout << "Global Timer still running... " << std::endl;
  }
  else{
    if(printToFileMobilityAwareStrategy)
      std::cout << "Re-scheduling Global Timer... from Node_" << nodeId << std::endl;
    nodeBeaconTimerGlobal = make_shared<ns3::Timer>(ns3::Timer::CANCEL_ON_DESTROY);
    nodeBeaconTimerGlobal->SetFunction(&MobilityAwareStrategy::holdSendingBeacon, this);
    nodeBeaconTimerGlobal->SetArguments(nodeId, false); // on creation, it only schedule the timer. Next calling, for sending Beacon. The control flag is set to True inside the method
    nodeBeaconTimerGlobal->Schedule(ns3::MilliSeconds(vehicleBeaconIntervalGlobal));
  }
}


void
MobilityAwareStrategy::BroadcastBeaconRSU(uint64_t nodeId, bool control){
//ConsumerCRoMo2Beacon::SendBeacon()
  uint64_t timestamp = ns3::Simulator::Now ().GetMilliSeconds();
  auto ConsumerCRoMo2Beacon = dynamic_cast<ns3::ndn::ConsumerCRoMo2Beacon*>(&(*ns3::NodeList::GetNode(ns3::Simulator::GetContext())->GetApplication(2)));

  ns3::Ptr<ns3::UniformRandomVariable> randomNum = ns3::CreateObject<ns3::UniformRandomVariable> ();
  auto randomContentionTime = (uint64_t) randomNum->GetValue(0, 5); // to give much range for different delays

  invokeConsumerBeacon = true;
  // holdSendingBeacon(nodeId, true);
  if(printToFileMobilityAwareStrategy)
    std::cout << timestamp << " - RSU_" << nodeId << " sending a beacon... " << std::endl;
  ns3::Simulator::Schedule (ns3::MilliSeconds (randomContentionTime), &ns3::ndn::ConsumerCRoMo2Beacon::SendBeacon, ConsumerCRoMo2Beacon);//


  m_waitToBroadcastTimer->SetFunction(&MobilityAwareStrategy::BroadcastBeaconRSU, this);
  m_waitToBroadcastTimer->SetArguments(nodeId, false);
  m_waitToBroadcastTimer->Schedule(ns3::MilliSeconds(broadcastFrequency));
}


// void
// MobilityAwareStrategy::BroadcastDelayedPacket (const Interest& interest, const FaceEndpoint& ingress, const shared_ptr<pit::Entry>& pitEntry, std::shared_ptr<Face> sharedFace){
//   interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(666));
//   broadcastInterest(interest, FaceEndpoint(ingress.face, ingress.endpoint), pitEntry);
//   // std::cout.rdbuf(coutbuf);
//   // return;
// }





// 20230109 -- overrided from strategy

// void
// MobilityAwareStrategy::afterContentStoreHit(const shared_ptr<pit::Entry>& pitEntry,
//                                const FaceEndpoint& ingress, const Data& data)
// {
//   NFD_LOG_DEBUG("afterContentStoreHit pitEntry=" << pitEntry->getName()
//   << " in=" << ingress << " data=" << data.getName());
//
//     // redirecting std::cout to file
//   ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   auto currentNodeId1 = currentNode->GetId ();
//
//
//
//   auto destId = data.getTag<lp::destIdTag>(); // Will be extracted from LSTM --- SET from the interest fromId Tag, in forwarder.cpp
//   uint64_t dest;
//   if(destId != nullptr){
//     dest = destId->get();
//     // if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted DestId: " << dest << std::endl;
//   }
//   // else
//     // if(printToFileMobilityAwareStrategy) std::cout << "\tDestId NOT set!" << std::endl;
//
//
//   uint64_t fromId;
//   auto FromId = data.getTag<lp::fromIdTag>();
//   if(FromId != nullptr){
//     fromId = FromId->get();
//     // if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted FromId: " << fromId << std::endl;
//   }
//   // else{
//     // fromId = 99;// TODO
//   // }
//
//
//   uint64_t hopCount = 0;
//   auto hopCountTag = data.getTag<lp::HopCountTag>();
//   if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
//     hopCount = *hopCountTag;
//   }
//   else{
//     NFD_LOG_DEBUG("Not possible to extract the hopCount from Data... ");
//   }
//
//
//   // if(printToFileMobilityAwareStrategy) std::cout << "Data found in CS - Data: " << data.getName().getSubName(0,5).toUri() << " - currentNodeId1: " << currentNodeId1 << " - destId: " << dest << " - fromId: " << fromId << " - hopCount: " << hopCount << std::endl;
//   // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   // auto currentNodeId1 = currentNode->GetId ();
//
//   Data data2 = data;
//      // setting origId to Data packet
//   auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
//                            [&currentNodeId1](const std::tuple<std::string, int>& NodeIdMacMap)
//                            {return std::get<1>(NodeIdMacMap) == (int) currentNodeId1;});
//   if (it != NodeIdMacMap.end()) {
//     data2.setOrigID(std::get<0>(*it));//std::get<0>(*it)
//     DataOrigId = std::get<0>(*it);
//   }
//   else
//     data2.setOrigID("00:FF:FF:FF:FF:FF");//std::get<0>(*it)
//
//   // data2.setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(currentNodeId1));
//   // std::string InterestOrigId = interest.getOrigID();
//   // data2.setDestFinalID(InterestOrigId);
//
//
//   std::cout << "Is this shit being executed Also" << std::endl;
//   // data.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(ingress.endpoint)); -------// 202306_1 -- SET in forwarding.cpp
//   this->sendData(pitEntry, data2, FaceEndpoint(ingress.face, ingress.endpoint));
//   std::cout << "\t>>>Data from Node_" << currentNodeId1 << "'s CS..." << std::endl;
//   // sendData(pitEntry, data, ingress); //TODO Elidio... //sendData(pitEntry, data, FaceEndpoint(ingress.face, currentNodeId1));
//   // sendDataToAllCafs(pitEntry, ingress, data);// 20230610 - data.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(ingress.endpoint)); --- inside
// }



void
MobilityAwareStrategy::afterReceiveNack(const FaceEndpoint& ingress, const lp::Nack& nack,
                                       const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG("Nack for " << nack.getInterest() << " from=" << ingress
                << " reason=" << nack.getReason());

  if(printToFileMobilityAwareStrategy) std::cout << "Nack for " << nack.getInterest() << " from=" << ingress
  << " reason=" << nack.getReason() << std::endl;


  if (nack.getReason() == lp::NackReason::NO_ROUTE) { // remove FIB entries
    BOOST_ASSERT(this->lookupFib(*pitEntry).hasNextHops());
    NFD_LOG_DEBUG("Send NACK to all downstreams");
    if(printToFileMobilityAwareStrategy) std::cout << "Are there some NACK to downstreams, in wireless channel? " << std::endl; // Elidio
    this->sendNacks(pitEntry, nack.getHeader());
    renewRoute(nack.getInterest().getName(), ingress, 0_ms);
  }
}


// void
// MobilityAwareStrategy::broadcastInterest(const Interest& interest, const FaceEndpoint& ingress,
// 										 const shared_ptr<pit::Entry>& pitEntry)
// {
// 	Face& inFace = ingress.face;
// 	for (auto& outFace : this->getFaceTable() | boost::adaptors::reversed) {
// 		if ((outFace.getId() == inFace.getId() && outFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) ||
// 			wouldViolateScope(inFace, interest, outFace) || outFace.getScope() == ndn::nfd::FACE_SCOPE_LOCAL) {
// 			continue;
//     }

//     // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//     // auto currentNodeId1 = currentNode->GetId ();

//     // interest.setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(currentNodeId1));

//     this->sendInterest(pitEntry, FaceEndpoint(outFace, 666), interest);// changed from "0" to "ingress.endpoint"
// 		pitEntry->getOutRecord(outFace, 0)->insertStrategyInfo<OutRecordInfo>().first->isNonDiscoveryInterest = 0;  // Elidio 20230109
// 		NFD_LOG_DEBUG("send discovery Interest=" << interest << " from="
// 		<< inFace.getId() << " to=" << outFace.getId());
// 		// if(printToFileMobilityAwareStrategy) std::cout << "broadcastInterest: send discovery Interest=" << interest << " from="
// 		// << inFace.getId() << " to=" << outFace.getId() << std::endl;

// // 		std::cout << "Broadcast Interest - ingress.endpoint =" << ingress.endpoint << std::endl;
// 	}
// }


void
MobilityAwareStrategy::multicastInterest(const Interest& interest, const FaceEndpoint& ingress,
										 const shared_ptr<pit::Entry>& pitEntry,
										 const fib::NextHopList& nexthops)
{
  uint32_t fromId;
  auto FromId = interest.getTag<lp::fromIdTag>();
  if(FromId != nullptr){
    fromId = FromId->get();
  }
  // else{
    // fromId = 666;// TODO
    // if(printToFileMobilityAwareStrategy) std::cout << "mobility-aware strategy... Interest. FromId nullptr. SET to 666" << std::endl;
  // }

  static int toControl = 0;
	Face& inFace = ingress.face;
	for (const auto& nexthop : nexthops) {
		Face& outFace = nexthop.getFace();
        // uint64_t endpointId = nexthop.getEndpointId();

		if ((outFace.getId() == inFace.getId() && outFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) ||
			wouldViolateScope(inFace, interest, outFace)) {
			continue;
			}
        // std::cout << "\tUsing FIB [multicastInterest(...)]... nexthop.getEndpointId(): " << nexthop.getEndpointId() << " ingress.endpoint: " << ingress.endpoint << " toControl: " << toControl++ << " - fromId: " << fromId << std::endl;
        // std::cout << "outFace.getId(): " << outFace.getId() << std::endl;
        // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(outFace.getId()));
        this->sendInterest(pitEntry, FaceEndpoint(outFace, ingress.endpoint), interest);// TODO: extract endpointId from FIB - 20230609
// 			if(printToFileMobilityAwareStrategy) std::cout << "Multicast ingress.endpoint=" << ingress.endpoint << std::endl;

		// pitEntry->getOutRecord(outFace, 0)->insertStrategyInfo<OutRecordInfo>().first->isNonDiscoveryInterest = 1;  // Elidio 20230109
		// NFD_LOG_DEBUG("send non-discovery Interest=" << interest << " from="
		// << inFace.getId() << " to=" << outFace.getId());
		// if(printToFileMobilityAwareStrategy) std::cout << "multicastInterest: send non-discovery Interest=" << interest << " from="
		// << inFace.getId() << " to=" << outFace.getId() << std::endl;

// 		if(printToFileMobilityAwareStrategy) std::cout << "Multicast Interest - ingress.endpoint =" << ingress.endpoint << std::endl;
	}
}

void
MobilityAwareStrategy::multicastInterestFIB(const Interest& interest, const FaceEndpoint& ingress,
										 const shared_ptr<pit::Entry>& pitEntry,
										 const fib::NextHopList& nexthops, const fib::Entry& fibEntry)
{
    // ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());

  int hopCount = 0;
  auto hopCountTag = interest.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) { // e.g., packet came from local node's cache
    hopCount = *hopCountTag;
  }
  // if(hopCount == 0){
  //   this->sendInterest(pitEntry, FaceEndpoint(*any_outFace, nodeNum), interest);
  //   return;
  // }


  uint32_t fromId;
  auto FromId = interest.getTag<lp::fromIdTag>();
  if(FromId != nullptr){
    fromId = *FromId;
  }
  // else{
    // fromId = 666;// TODO
    // if(printToFileMobilityAwareStrategy) std::cout << "mobility-aware strategy... Interest. FromId nullptr. SET to 666" << std::endl;
  // }
  // std::cout << "nexthops.front().getVx(): " << nexthops.front().getVx() << std::endl;


  uint32_t destId = 888;
  auto DestId = interest.getTag<lp::destIdTag>();
  if(DestId != nullptr){
    destId = DestId->get();
  }
  else {
    destId = 888;
    // std::cout << "destIXd = " << destId << std::endl;
  }

  auto x = ns3::Simulator::GetContext();
  if (x > nodeNum)
    return;
  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(x);
  auto currentNodeId1 = currentNode->GetId();
  // if(currentNode->GetId() == nodeNum){
  //   std::cout << " RSU is multicasting the prefix: " << interest.getName().getSubName(0, 6) << std::endl;
  // }

  // std::cout << "Testing here... Interest: " << interest.getName().toUri() << " - hopCount: " << hopCount << " - Node_" << currentNodeId1 << std::endl;

  // ns3::Callback <std::tuple<double, double>, ndn::util::Nmsi, uint64_t, int > ltmpCallback;
  ns3::Callback <std::tuple<Neighbor, double>, Neighbor*, bool, uint16_t > stmpCallback;//New 20241017
  uint64_t timestampX; // = ns3::Simulator::Now ().GetMilliSeconds();

  uint64_t DestFinalId = destinationID;// nodeNum;
  std::string InterestDestFinalId = interest.getDestFinalID();
  // std::cout << "\tDataOrigNmsi: " << DataOrigNmsi/*.getNodeId()*/ << std::endl;

  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                          [&InterestDestFinalId](const std::tuple<std::string, int>& NodeIdMacMap)
                          {return std::get<0>(NodeIdMacMap) == InterestDestFinalId;});
  if (it != NodeIdMacMap.end()) {
    DestFinalId = std::get<1>(*it);
    // std::cout << "[AfterReceiveInterest]. Broadcast... 1 DestFinalId = " << DestFinalId << std::endl;
  }
  else{
    // if(printToFileMobilityAwareStrategy)
      // std::cout << "[AfterReceiveInterest]. Broadcast... 2 DestFinalId = " << DestFinalId << std::endl;
  }

  int numLineRead = 3;//5; //was 50 // Number of lines to read from dataset. This number is chosen higher for RSU (200)
  if(currentNodeId1 == nodeNum)
    numLineRead = 3;//10; //was 100

  ndn::util::Nmsi contentProviderNodeNmsi;
  ns3::Ptr<ns3::Node> contentProviderNode;// = 666; NMSI stored in map-common.hpp
  // double contentProviderNodePx, contentProviderNodePy;

  // TODO See this!!
  // if(DestFinalId == currentNodeId1)
  //   return;



  // Predict actual geo-position
  timestampX = ns3::Simulator::Now ().GetMilliSeconds();
  // ndn::util::LtmpHMM m_ltmpInstance(contentProviderNodeNmsi, timestampX);//, getNodeInfoSTMPVectorAddress()
  // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance);
  // std::tie(contentProviderNodePx, contentProviderNodePy) = std::move(ltmpCallback(contentProviderNodeNmsi, timestampX, numLineRead));

  if(DestFinalId >= 666){// DestFinalId >= 666
    DestFinalId = nodeNum;
    contentProviderNode = ns3::NodeList::GetNode(nodeNum);
    contentProviderNodeNmsi = ndn::util::Nmsi({}); // From map-common.hpp
    // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(DestFinalId));
    // broadcastInterest(interest, FaceEndpoint(ingress.face, DestFinalId), pitEntry);
    // std::cout << "\t>>>FIB This Interest is broadcast..." << std::endl;
    // return;
    // std::cout << "It will NOT end well!!" << std::endl;
  }
  else{
    contentProviderNode = ns3::NodeList::GetNode(DestFinalId);
    contentProviderNodeNmsi = DataOrigNmsi; // From map-common.hpp
  }

  // contentProviderNodeNmsi = DataOrigNmsi; // From map-common.hpp

  // nfd::fw::Neighbor actualDestNode;
  // actualDestNode.setNmsi(DataOrigNmsi);
  // actualDestNode.setStatus(Neighbor::STATUS_ACTIVE);

  nfd::fw::Neighbor NextHopNeighbor;
  timestampX = ns3::Simulator::Now ().GetMilliSeconds();
  // ndn::util::LtmpHMM m_ltmpInstance(contentProviderNodeNmsi, timestampX);//, getNodeInfoSTMPVectorAddress()
  // ndn::util::StmpKF m_stmpInstance(&actualDestNode, 0, 0);// New 20241017
  // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance);
  // stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance); // New 20241017

  double currentNodePx, currentNodePy, currentNodePz;
  std::unordered_set<uint64_t> neighborIds;
  // std::cout << "nexthops.front().getEndpointId(): " << nexthops.front().getEndpointId() << std::endl;
  if(printToFileMobilityAwareStrategy)
    std::cout << "\tNeighbors: ";
  for (const auto& node : m_neighborhoodTable->m_neighList) {
    if(node.getStatus() == Neighbor::STATUS_ACTIVE){
      neighborIds.insert(node.getNodeId());//
      if(printToFileMobilityAwareStrategy)
        std::cout << node.getNodeId() << " - ";
      if(((nexthops.size() == 1) && (nexthops.front().getEndpointId() == node.getNodeId())) || (DestFinalId == node.getNodeId())){
        // std::cout << "nexthops.front().getEndpointId(): " << nexthops.front().getEndpointId() << std::endl;
        timestampX = ns3::Simulator::Now ().GetMilliSeconds();
        double NextHopNodePx, NextHopNodePy;
        if(printToFileMobilityAwareStrategy)
          std::cout << "\n\tPredicting nexthop position..." << std::endl;
        // std::tie(NextHopNodePx, NextHopNodePy) = std::move(ltmpCallback(node.getNmsi(), timestampX, numLineRead));
        Neighbor neighborTmp;
        Neighbor node2 = std::move(node.CopyNeighbor());
        double distance = 0.0;
        ndn::util::StmpKF m_stmpInstance(&node2, 0, 0);// New 20241017
        // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance);
        stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance); // New 20241017
        std::tie(neighborTmp, distance) = std::move(stmpCallback(&node2, 0, 0));  // New 20241017
        NextHopNodePx = neighborTmp.getNmsi().getGeoPosition().getLatPosition();
        NextHopNodePy = neighborTmp.getNmsi().getGeoPosition().getLonPosition();
        std::tie(currentNodePx, currentNodePy, currentNodePz) = toolsStrategy.getCurrentNodeLocation(currentNode);
        auto ToDestination = toolsStrategy.CalculateDistance(currentNodePx, currentNodePy, NextHopNodePx, NextHopNodePy);


//         TODO: correct ToDestination > 250m
        if(ToDestination <= 255){
          if(printToFileMobilityAwareStrategy)
            std::cout << "\tFIB NextHop (Node_" << node.getNodeId() << ") is a neighbor at " << ToDestination << "m... \n\t>>>Unicasting to it... " << std::endl;
          Face& outFace = nexthops.front().getFace();
          interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(node.getNodeId()));
          this->sendInterest(pitEntry, FaceEndpoint(outFace, node.getNodeId()), interest);
          return;
        }
        else{
          //NOP - will process below!
        }
      }
    }
  }
  std::cout << std::endl;

  static int toControl = 0;
  std::vector<uint64_t> nexHopnodeIds;
  Face& inFace = ingress.face;
  if(printToFileMobilityAwareStrategy)
    std::cout << "\tmulticastInterestFIB -> " << fibEntry.getPrefix() << ": nexthops: " << nexthops.size() << " ";


  // Current node's position
  // double currentNodePx, currentNodePy, currentNodePz;
  // std::tie(currentNodePx, currentNodePy, currentNodePz) = toolsStrategy.getCurrentNodeLocation(currentNode);


  // float distance = 0.0;
  float prevDistance = 32000.0; // Invalid value
  uint64_t SelectedRelayNode = DestFinalId;


  // Extract and choose the nextHop that is nearest to the Interest destination
  std::unordered_set<uint64_t> nextHopIds;
  std::string multicastString("");


  bool IsNextHopNeighbor = false;
  // std::vector<nfd::fw::Neighbor> _nextHopsNotNeighbor; // for nexthops.size() > 1, stacked up neighbors
  // std::vector<Face*> _outFaces;

  Face* _outFaces = &ingress.face;
  std::map<Face*, nfd::fw::Neighbor> _nextHopsNotNeighbor;
  std::map<Face*, nfd::fw::Neighbor> _neighborNextHops;
  bool neighborNextHops = false;
  std::map<Face*, uint64_t> _nextHopsIDGreaterThan666;

  Face* any_outFace;
  uint32_t limitNextHops = 0;
  for (const auto& nexthop : nexthops) {
    limitNextHops++;
    if(limitNextHops == 6)
      break;
    // nextHopIds.insert(nexthop.getEndpointId());
    // std::cout << "Olhem para isto... No_" << nexthop.getEndpointId() << std::endl;
    Face& outFace = nexthop.getFace();
    any_outFace = &nexthop.getFace();
    ns3::Ptr<ns3::Node> NextHopNode;
    // Neighbor nb;
    if(nexthop.getEndpointId() < 666){
      NextHopNode = ns3::NodeList::GetNode(nexthop.getEndpointId());
      for(auto & it: m_neighborhoodTable->m_neighList) {
        if (it.getNodeId() == nexthop.getEndpointId()) {
          IsNextHopNeighbor = true;
          timestampX = ns3::Simulator::Now ().GetMilliSeconds();
          double NextHopNodePx, NextHopNodePy;
          // std::tie(NextHopNodePx, NextHopNodePy) = std::move(ltmpCallback(it.getNmsi(), timestampX, numLineRead));

          Neighbor neighborTmp;
          double distance = 0.0;
          Neighbor node2 = std::move(it.CopyNeighbor());
          ndn::util::StmpKF m_stmpInstance(&node2, 0, 0);// New 20241017
          // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance);
          stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance); // New 20241017
          std::tie(neighborTmp, distance) = std::move(stmpCallback(&node2, 0, 0));  // New 20241017
          NextHopNodePx = neighborTmp.getNmsi().getGeoPosition().getLatPosition();
          NextHopNodePy = neighborTmp.getNmsi().getGeoPosition().getLonPosition();

          std::tie(currentNodePx, currentNodePy, currentNodePz) = toolsStrategy.getCurrentNodeLocation(currentNode);
          auto ToDestination = toolsStrategy.CalculateDistance(currentNodePx, currentNodePy, NextHopNodePx, NextHopNodePy);

          if(ToDestination <= 255){
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tFIB NextHop (Node_" << nexthop.getEndpointId() << ") is a neighbor at " << ToDestination << "m... \n\t>>>Unicasting to it... " << std::endl;
            interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(nexthop.getEndpointId()));
            this->sendInterest(pitEntry, FaceEndpoint(outFace, nexthop.getEndpointId()), interest);
            break;
          }

          _neighborNextHops.insert({&nexthop.getFace(), std::move(it.CopyNeighbor())});
          // neighborNextHops = true;
          // break;

        } //std::cout << "\tFIB NextHop is NOT a neighbor... \n\t>>>Checking for suitable neighbors for relay... " << std::endl;
        else{//nexthops.size() > 1 -> Send to only one neighbor. The farthest one. -- NextHop is NOT a neighbor
          //stack up neighbor-nexthops
          // _nextHopsNotNeighbor.push_back(it);
          // _outFaces.push_back(&nexthop.getFace());
          // std::cout << "Does it enters here??? " << std::endl;
          _nextHopsNotNeighbor.insert({&nexthop.getFace(), std::move(it.CopyNeighbor())});
          neighborNextHops = true;
        }
      }
    }
    else {// nexthop.getEndpointId() >= 666 - These are inherently NOT neighbors!
      if(printToFileMobilityAwareStrategy)
        std::cout << "\tnexthop Id >= 666... These are inherently NOT neighbors! \n\t>>>Forwarding to RSU... " << std::endl;
      // continue; // To be corrected?**
      interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(/*666*/nodeNum));//nodeNum
      this->sendInterest(pitEntry, FaceEndpoint(outFace, /*666*/nodeNum), interest);//nodeNum
      // _nextHopsIDGreaterThan666.insert({&nexthop.getFace(), nexthop.getEndpointId()});
    }
  } //Will close here!



  // NextHop - more than 1 neighbor...
  /********************/
  if(_neighborNextHops.size() != 0){//neighborNextHops && !IsNextHopNeighbor

    if(printToFileMobilityAwareStrategy)
      std::cout << "\tFIB - Found " << _neighborNextHops.size() << " nextHops." << std::endl;

    bool control = true; // If true, the naame of rmse.txt will be rmse_x.txt, the NodeX_STMP_Estimation.txt will be NodeX_x_STMP_Estimation.txt

    // ns3::Callback <std::tuple<double, double>, ndn::util::Nmsi, uint64_t, int > ltmpCallback;
    // uint64_t timestampX = ns3::Simulator::Now ().GetMilliSeconds();

    // float distance = 0.0;
    float prevDistance = 32000.0; // Invalid value
    // uint64_t SelectedRelayNode = DestFinalId;

    bool foundRelayNode = false;
    int x = 0;
    for(auto & it: _neighborNextHops){
      // std::cout << "\tit.getNodeId()" << it.getNodeId() << " -- fromId: " << fromId << std::endl;

      timestampX = ns3::Simulator::Now ().GetMilliSeconds();
      // ndn::util::LtmpHMM m_ltmpInstance(it.second.getNmsi(), timestampX);//, getNodeInfoSTMPVectorAddress()
      // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance);

      double NextHopNodePx, NextHopNodePy;
      // std::tie(nextHopNodePx, nextHopNodePy) = std::move(ltmpCallback(it.second.getNmsi(), timestampX, numLineRead));


      Neighbor neighborTmp;
      double distance = 0.0;
      Neighbor node2 = std::move(it.second.CopyNeighbor());
      ndn::util::StmpKF m_stmpInstance(&node2, 0, 0);// New 20241017
      // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance);
      stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance); // New 20241017
      std::tie(neighborTmp, distance) = std::move(stmpCallback(&node2, 0, 0));  // New 20241017
      NextHopNodePx = neighborTmp.getNmsi().getGeoPosition().getLatPosition();
      NextHopNodePy = neighborTmp.getNmsi().getGeoPosition().getLonPosition();
      std::tie(currentNodePx, currentNodePy, currentNodePz) = toolsStrategy.getCurrentNodeLocation(currentNode);
      auto ToDestination = toolsStrategy.CalculateDistance(currentNodePx, currentNodePy, NextHopNodePx, NextHopNodePy);

      // std::tie(currentNodePx, currentNodePy, currentNodePz) = toolsStrategy.getCurrentNodeLocation(currentNode); // Updated location
      // auto ToDestination = toolsStrategy.CalculateDistance(currentNodePx, currentNodePy, nextHopNodePx, nextHopNodePy);


      if(ToDestination <= 255){
        if(printToFileMobilityAwareStrategy)
          std::cout << "\t" << ++x << "- Nexthop Node_" << it.second.getNodeId() << " - current Node_" << currentNodeId1 << ": Distance = " << ToDestination;// << std::endl;

        // From the intermediate neighbor to the Extracted nextHop
        double contentProviderNodePx, contentProviderNodePy;
        // ndn::util::LtmpHMM m_ltmpInstance2(contentProviderNodeNmsi, timestampX);//, getNodeInfoSTMPVectorAddress()
        // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance2);

        // std::tie(contentProviderNodePx, contentProviderNodePy) = std::move(ltmpCallback(contentProviderNodeNmsi, timestampX, numLineRead));

        Neighbor neighborTmp, neighborTmp2;
        double distance = 0.0;
        neighborTmp2.setNmsi(contentProviderNodeNmsi);
        neighborTmp2.setStatus(Neighbor::STATUS_ACTIVE);

        ndn::util::StmpKF m_stmpInstance(&neighborTmp2, 0, 0);// New 20241017
        // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance);
        stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance); // New 20241017
        std::tie(neighborTmp, distance) = std::move(stmpCallback(&neighborTmp2, 0, 0));  // New 20241017
        contentProviderNodePx = neighborTmp.getNmsi().getGeoPosition().getLatPosition();
        contentProviderNodePy = neighborTmp.getNmsi().getGeoPosition().getLonPosition();
        std::tie(currentNodePx, currentNodePy, currentNodePz) = toolsStrategy.getCurrentNodeLocation(currentNode);

        auto distanceToDestination = toolsStrategy.CalculateDistance(contentProviderNodePx, contentProviderNodePy, NextHopNodePx, NextHopNodePy);

        if(printToFileMobilityAwareStrategy)
          std::cout << " <---> FIB - Nexthop Node_" << it.second.getNodeId() << " - destination Node_" << contentProviderNodeNmsi.getNodeId() << ": Distance = " << distanceToDestination << std::endl;

        if (distanceToDestination < prevDistance){
          if(distanceToDestination != 0)
            prevDistance = distanceToDestination; // Keeps the shortest calculated distance to the destination. 20240626: except zero! It's due to mobility prediction bug
          SelectedRelayNode = it.second.getNodeId();
          _outFaces = it.first;
          foundRelayNode = true;
        }
        x++;
      }
    }

    if(foundRelayNode){
      if(printToFileMobilityAwareStrategy)
        std::cout << "\t>>> FIB - Node_" << SelectedRelayNode << " is the nearest (" << prevDistance << "m) relay to the destination neighbor. \n\t>>> FIB Unicasting to it..." << std::endl;
      interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(SelectedRelayNode)); // in the unicastInterest method
      // broadcastInterest(interest, FaceEndpoint(ingress.face, SelectedRelayNode), pitEntry);
      this->sendInterest(pitEntry, FaceEndpoint(*_outFaces, SelectedRelayNode), interest);
      // std::cout.rdbuf(coutbuf);
      // return;
    }
    else{
      if(printToFileMobilityAwareStrategy)
        std::cout << "\t>>> FIB - No suitable Neighbor found... exiting... " << std::endl;
      // std::cout.rdbuf(coutbuf);
      // return;
    }
  }
  else{
    if(printToFileMobilityAwareStrategy)
      std::cout << "\t_neighborNextHops.size() == 0, nothing to be done here!... exiting..." << std::endl;
    // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(nodeNum)); // 666
    // this->sendInterest(pitEntry, FaceEndpoint(ingress.face, nodeNum), interest); //666
    // return; // 20240626 See this modification, was commented
  }
  /********************/





  // NextHop is not a neighbor...
  /********************/
  if((IsNextHopNeighbor == false) && (_nextHopsNotNeighbor.size() != 0)){// No nexthop neighbor found... Search for a suitable neighbor near the nexthop
    if(printToFileMobilityAwareStrategy)
      std::cout << "\tFIB - NextHop is NOT a neighbor... \n\t>>>Checking for suitable neighbors for relay... " << std::endl;
    if(m_neighborhoodTable->m_neighList.size()){// && (fD > 0)
      if(printToFileMobilityAwareStrategy)
        std::cout << "\tFIB - Found " << m_neighborhoodTable->m_neighList.size() << " neighbor(s). Showing only valid: " << std::endl;

      bool control = true; // If true, the naame of rmse.txt will be rmse_x.txt, the NodeX_STMP_Estimation.txt will be NodeX_x_STMP_Estimation.txt

      ns3::Callback <std::tuple<double, double>, ndn::util::Nmsi, uint64_t, int > ltmpCallback;
      // uint64_t timestampX = ns3::Simulator::Now ().GetMilliSeconds();

      // float distance = 0.0;
      float prevDistance = 32000.0; // Invalid value
      uint64_t SelectedRelayNode = DestFinalId;

      bool foundRelayNode = false;
      int x = 0;
      for(auto & it: m_neighborhoodTable->m_neighList){
        // std::cout << "\tit.getNodeId()" << it.getNodeId() << " -- fromId: " << fromId << std::endl;

        if ((it.getStatus() == Neighbor::STATUS_ACTIVE) /*&& (it.getNodeId() != fromId)*/){//&& (actualNextHopNode.getNodeId() != it.getNodeId())  && (actualNextHopNode.getNodeId() != fromId)
          Neighbor neighborDestNode = std::move(it.CopyNeighbor());

          uint64_t timestampX = ns3::Simulator::Now ().GetMilliSeconds();
          // ndn::util::LtmpHMM m_ltmpInstance(neighborDestNode.getNmsi(), timestampX);//, getNodeInfoSTMPVectorAddress()
          // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance);

          double NextHopNodePx, NextHopNodePy;
          // std::tie(nextHopNodePx, nextHopNodePy) = std::move(ltmpCallback(neighborDestNode.getNmsi(), timestampX, numLineRead));

          Neighbor neighborTmp;
          double distance = 0.0;
          ndn::util::StmpKF m_stmpInstance(&neighborDestNode, 0, 0);// New 20241017
          // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance);
          stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance); // New 20241017
          std::tie(neighborTmp, distance) = std::move(stmpCallback(&neighborDestNode, 0, 0));  // New 20241017
          NextHopNodePx = neighborTmp.getNmsi().getGeoPosition().getLatPosition();
          NextHopNodePy = neighborTmp.getNmsi().getGeoPosition().getLonPosition();
          std::tie(currentNodePx, currentNodePy, currentNodePz) = toolsStrategy.getCurrentNodeLocation(currentNode);
          auto ToDestination = toolsStrategy.CalculateDistance(currentNodePx, currentNodePy, NextHopNodePx, NextHopNodePy);


          if(ToDestination <= 255) {
            if(printToFileMobilityAwareStrategy)
              std::cout << "\t" << ++x << "- Neighbor Node_" << it.getNodeId() << " - current Node_" << currentNodeId1 << ": Distance = " << ToDestination << std::endl;

            // From the intermediate neighbor to the Extracted nextHop
            if(printToFileMobilityAwareStrategy)
              std::cout << "\tPredicting provider (Node_" << DestFinalId << ") position ..." << std::endl;
            double contentProviderNodePx, contentProviderNodePy;
            // std::tie(contentProviderNodePx, contentProviderNodePy) = std::move(ltmpCallback(contentProviderNodeNmsi, timestampX, numLineRead));

            Neighbor neighborTmp;
            double distance = 0.0;
            Neighbor contentProviderNode2;
            contentProviderNode2.setNmsi(contentProviderNodeNmsi);
            contentProviderNode2.setStatus(Neighbor::STATUS_ACTIVE);

            ndn::util::StmpKF m_stmpInstance(&contentProviderNode2, 0, 0);// New 20241017
            // ltmpCallback = ns3::MakeCallback(&ndn::util::LtmpHMM::ltmp, &m_ltmpInstance);
            stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance); // New 20241017
            std::tie(neighborTmp, distance) = std::move(stmpCallback(&contentProviderNode2, 0, 0));  // New 20241017
            contentProviderNodePx = neighborTmp.getNmsi().getGeoPosition().getLatPosition();
            contentProviderNodePy = neighborTmp.getNmsi().getGeoPosition().getLonPosition();
            std::tie(currentNodePx, currentNodePy, currentNodePz) = toolsStrategy.getCurrentNodeLocation(currentNode);
            auto ToDestination = toolsStrategy.CalculateDistance(currentNodePx, currentNodePy, contentProviderNodePx, contentProviderNodePy);



            auto distanceToDestination = toolsStrategy.CalculateDistance(contentProviderNodePx, contentProviderNodePy, NextHopNodePx, NextHopNodePy);

            if(printToFileMobilityAwareStrategy)
              std::cout << " <---> FIB - Neighbor Node_" << it.getNodeId() << " - destination Node_" << DestFinalId << ": Distance = " << distanceToDestination << std::endl;

            if (distanceToDestination < prevDistance){
              prevDistance = distanceToDestination; // Keeps the shortest calculated distance to the destination
              SelectedRelayNode = it.getNodeId();
              foundRelayNode = true;
            }
          }
        }
      }

      if(foundRelayNode){
        if(printToFileMobilityAwareStrategy)
          std::cout << "\t>>> FIB - Node_" << SelectedRelayNode << " is the nearest (" << prevDistance << "m) relay to the destination (Node_" << DestFinalId << "). \n\t>>> FIB Unicasting to it..." << std::endl;
        interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(SelectedRelayNode)); // in the unicastInterest method
        // broadcastInterest(interest, FaceEndpoint(ingress.face, SelectedRelayNode), pitEntry);
        this->sendInterest(pitEntry, FaceEndpoint(*any_outFace, SelectedRelayNode), interest);
        // std::cout.rdbuf(coutbuf);
        // return;
      }
      else{
        if(printToFileMobilityAwareStrategy)
          std::cout << "\t>>> FIB - No suitable Neighbor found... exiting... " << std::endl;
        // std::cout.rdbuf(coutbuf);
        // return;
      }
    }
    else{
      if(hopCount == 0){
        interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(nodeNum));
        this->sendInterest(pitEntry, FaceEndpoint(*any_outFace, nodeNum), interest);
      }
      if(printToFileMobilityAwareStrategy)
        std::cout << "\t>>>No neighbors for relaying... Exiting...xxx " << std::endl;
    }
  }
  else{//(IsNextHopNeighbor != false) || (_nextHopsNotNeighbor.size() == 0)
    // std::cout << "Never here!... " << std::endl;
    // NOP
  }
  /********************/








}

// Changed the original broadcastInterest to this modified in order to broadcast to 666!
void
MobilityAwareStrategy::broadcastInterest(const Interest& interest, const FaceEndpoint& ingress,
                                         const shared_ptr<pit::Entry>& pitEntry)
{
  Face& inFace = ingress.face;

  for (auto& outFace : this->getFaceTable() | boost::adaptors::reversed) {
    if ((outFace.getId() == inFace.getId() && outFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) ||
        wouldViolateScope(inFace, interest, outFace) || outFace.getScope() == ndn::nfd::FACE_SCOPE_LOCAL) {
      continue;
    }// from 0 to 0xF
    // std::cout << "Hey let see...: prefix: " << interest.getName().toUri() << std::endl;
    this->sendInterest(pitEntry, FaceEndpoint(outFace, ingress.endpoint), interest);
  }
}




void
MobilityAwareStrategy::unicastInterest(const Interest& interest, const FaceEndpoint& ingress,
                                         const shared_ptr<pit::Entry>& pitEntry)
{
  Face& inFace = ingress.face;

  for (auto& outFace : this->getFaceTable() | boost::adaptors::reversed) {
    if ((outFace.getId() == inFace.getId() && outFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) ||
        wouldViolateScope(inFace, interest, outFace) || outFace.getScope() == ndn::nfd::FACE_SCOPE_LOCAL) {
      continue;
    }// from 0 to 0xF
    // interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(ingress.endpoint));
    this->sendInterest(pitEntry, FaceEndpoint(outFace, ingress.endpoint), interest);
    // pitEntry->getOutRecord(outFace, 0)->insertStrategyInfo<OutRecordInfo>().first->isNonDiscoveryInterest = false;  // Elidio 20230109
    // NFD_LOG_DEBUG("send discovery Interest=" << interest << " from="
    //               << inFace.getId() << " to=" << outFace.getId());
    // std::cout << "send discovery Interest=" << interest << " from="
    // << inFace.getId() << " to=" << outFace.getId() << std::endl;
  }
}



void
MobilityAwareStrategy::sendDataToAllCafs(const shared_ptr<pit::Entry>& pitEntry,
                        const FaceEndpoint& ingress, const Data& data)
{

  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto currentNodeId1 = currentNode->GetId ();

  std::set<std::pair<Face*, EndpointId>> pendingDownstreams; // Elidio: std::set<Face*> to ...
  auto now = time::steady_clock::now();

  uint32_t fromId;
  auto FromId = data.getTag<lp::fromIdTag>();
  if(FromId != nullptr){
    fromId = FromId->get();
    if(fromId > nodeNum)
      if(printToFileMobilityAwareStrategy)
        std::cout << "\tExtracted FromId > nodeNum: " << fromId << " > " << nodeNum << std::endl;
    if(printToFileMobilityAwareStrategy) std::cout << "\tExtracted FromId: " << fromId << std::endl;
  }
  else{
    // fromId = 666;// TODO
    if(printToFileMobilityAwareStrategy) std::cout << "mobility-aware strategy... Interest. FromId nullptr. SET to 666" << std::endl;
  }


  uint64_t finaldataDestId = 666;
  uint64_t origindataId = 666;
  std::string DataDestId = data.getDestFinalID();
  // std::string DataOrigId = data.getOrigID();
  // data->setOrigID(InterestOrigId);
  // // std::cout << "Extracted origId from received Interest: " << InterestOrigId << std::endl;
  //
  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&DataDestId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<0>(NodeIdMacMap) == DataDestId;});
  if (it != NodeIdMacMap.end()) {
    finaldataDestId = std::get<1>(*it);
    if(printToFileMobilityAwareStrategy)
      std::cout << "[sendDataToAllCafs]. Extracted DesiFinalId from received Data: " << finaldataDestId << std::endl;
  }
  else{
    if(printToFileMobilityAwareStrategy)
      std::cout << "[sendDataToAllCafs]. destId NOT found/Set" << std::endl;
  // data->setTag<lp::destIdTag>(make_shared<lp::destIdTag>(std::get<1>(*it)));
  }


  it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                           [&DataOrigId](const std::tuple<std::string, int>& NodeIdMacMap)
                           {return std::get<0>(NodeIdMacMap) == DataOrigId;});
  if (it != NodeIdMacMap.end()) {
    origindataId = std::get<1>(*it);
    // std::cout << "[sendDataToAllCafs]. Extracted origId from received Data: " << origindataId << std::endl;
  }
  else{
    // std::cout << "[sendDataToAllCafs]. origId NOT found/Set" << std::endl;
  // data->setTag<lp::destIdTag>(make_shared<lp::destIdTag>(std::get<1>(*it)));
  }



  // if (finaldataDestId == currentNodeId1){
  //   std::cout << "Final destination: " << finaldataDestId << " data origin: " << origindataId << std::endl;
  //   // std::cout.rdbuf(coutbuf);
  //   return;
  // }
  bool foundNeighbor = false;
  uint64_t neigb = 1;

  // remember pending downstreams
  for (const pit::InRecord& inRecord : pitEntry->getInRecords()) {
    if (inRecord.getExpiry() > now) {
      if (inRecord.getFace().getId() == ingress.face.getId() &&
          inRecord.getFace().getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) {
        continue;
      }
	  uint64_t endpointId = inRecord.getEndpointId(); //(To be corrected)
      // if(printToFileMobilityAwareStrategy)
        // std::cout << "inRecord.getFace().getId(): " << inRecord.getFace().getId() << " - endpointId: " << endpointId << std::endl;
      // each Data holds its destId from the pending PIT entries

      // auto it = neighborhoodTable->m_neighList.begin();
      // for (; it != neighborhoodTable->m_neighList.end(); it++){
      //   neighborCCT = std::move(it->CopyNeighbor());
      //   // neighborCCT.m_ntimer_1.Cancel();
      //
      //   if(printToFileMobilityAwareStrategy)
      //     std::cout << "\t Distance to Data provider before prediction: " << neighborCCT.getDistance() << std::endl;
      //   std::tie(neighborTmp, distance) = std::move(stmpCallback(&neighborCCT, control, scheduleOnOff));
      //
      //   // std::cout << "Neighbor after neighborCCT - neighborCCT: " << neighborCCT << std::endl;
      //   if(printToFileMobilityAwareStrategy)
      //     std::cout << "\t>>> Distance to Data provider after prediction: " << neighborTmp.getDistance() << std::endl;
      //
      //   neighborCCTListUpdated.push_back(std::move(neighborTmp)); // Keep the updated list after mobility prediction
      //
      //   if (neighborTmp.getDistance() <= 250){     // IMPORTANT: Send to all Nodes in range...
      //     if(printToFileMobilityAwareStrategy)
      //       std::cout << "\t>>> Node_" << neighborTmp.getNodeId() << " is in range (" << neighborTmp.getDistance() << "m).\n\t>>> Unicasting to it... " << std::endl;
      //     interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(neighborTmp.getNodeId()));
      //     unicastInterest(interest, FaceEndpoint(ingress.face, neighborTmp.getNodeId()), pitEntry);
      //     interestSent = true;
      //     std::cout.rdbuf(coutbuf);
      //     return;
      //   }
      //   else{
      //     interestSent = false;
      //     if(printToFileMobilityAwareStrategy)
      //       std::cout << "\t>>> Node_" << neighborTmp.getNodeId() << " out of range (" << neighborTmp.getDistance() << "m)." << std::endl;
      //   }
      // }

      // Comemented 20230826
      // float fD = 0.0;
      // float actualNextHopNodeDistance = 0.0;
      // // bool foundNeighbor = false;
      // uint64_t nxtHop = 0;
      // std::vector<std::tuple<uint64_t, float>> nxtHopList;
      // // std::cout << "\tNo RSU found..." << std::endl;
      // // std::cout << "\tLooking up mobile neighbors... " << std::endl;
      // Neighbor actualNextHopNode;
      // // uint64_t neigb = 1;
      // bool needToCheckSuitableNeighbors = false;
      // if(!m_neighborhoodTable->m_neighList.empty())  {
      //   for (auto & it: m_neighborhoodTable->m_neighList){
      //     if((it.getNodeId() != currentNodeId1) && (fromId != it.getNodeId())){
      //       if (it.getStatus() == Neighbor::STATUS_ACTIVE){
      //         //std::cout << it << std::endl;
      //         float iDistance = it.getDistance();
      //         nxtHop = it.getNodeId();
      //         bool needToCheckSuitableNeighbors = false;
      //
      //         if(nxtHop == endpointId){
      //           actualNextHopNode = std::move(it.CopyNeighbor());
      //           actualNextHopNodeDistance = it.getDistance();
      //           if(printToFileMobilityAwareStrategy)
      //             std::cout << "\tDestination Node_" << endpointId << " is a neighbor. Predicting its current location..." << std::endl;
      //
      //           ns3::Callback <std::tuple<Neighbor, double>, Neighbor*, bool, uint16_t > stmpCallback;//, std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>>
      //           uint16_t scheduleOnOff = 0;
      //           bool control = true; // If true, the naame of rmse.txt will be rmse_x.txt, the NodeX_STMP_Estimation.txt will be NodeX_x_STMP_Estimation.txt
      //
      //           Neighbor neighborTmp;
      //           double distance = 0.0;
      //
      //           if(printToFileMobilityAwareStrategy)
      //             std::cout << "\tDistance before prediction: " << actualNextHopNode.getDistance() << std::endl;
      //           ndn::util::StmpKF m_stmpInstance(&actualNextHopNode, control, scheduleOnOff);//, getNodeInfoSTMPVectorAddress()
      //
      //           stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance);
      //           std::tie(neighborTmp, distance) = std::move(stmpCallback(&actualNextHopNode, control, scheduleOnOff));// , getNodeInfoSTMPVectorAddress()   -- neighbor->getNmsi()
      //
      //           // std::cout << "RSU Neighbor after prediction - actualNextHopNode: " << actualNextHopNode << std::endl;
      //           if(printToFileMobilityAwareStrategy)
      //             std::cout << "\t>>> Distance after prediction: " << neighborTmp.getDistance() << std::endl;
      //
      //           if(distance <= 250){
      //             if(printToFileMobilityAwareStrategy)
      //               std::cout << "\t>>> Destination Node_" << endpointId << " is a neighbor, within the range (" << distance << " m). \n\t>>> Unicasting to it..." << std::endl;
      //             pendingDownstreams.emplace(&inRecord.getFace(), endpointId);
      //             needToCheckSuitableNeighbors = false; //
      //             break;
      //           }
      //           else{ // distance > 250
      //             if(printToFileMobilityAwareStrategy)
      //               std::cout << "\tDestination Node_" << endpointId << " is a neighbor, out of range (" << distance << " m). Checking for suitable neighbors for relaying..." << std::endl;
      //             needToCheckSuitableNeighbors = true; //
      //             foundNeighbor = true;
      //           }
      //         }
      //         else{
      //           needToCheckSuitableNeighbors = true;
      //         }
      //
      //         // std::cout << "needToCheckSuitableNeighbors: " << needToCheckSuitableNeighbors << std::endl;
      //         if(needToCheckSuitableNeighbors == true){ // Other neighbors
      //           // nxtHopList.push_back(std::make_tuple(nxtHop, iDistance));
      //           // neigb++;
      //           // foundNeighbor = true;// At least one neighbor has been found
      //           // if(!printToFileMobilityAwareStrategy)
      //           //   std::cout << "\tFound " << neigb << " neighbor(s). Showing only valid: " << std::endl;
      //           float farthestDistance = 0;
      //           uint64_t farthestNode = 666;
      //
      //           uint16_t scheduleOnOff = 0;
      //           bool control = true; // If true, the naame of rmse.txt will be rmse_x.txt, the NodeX_STMP_Estimation.txt will be NodeX_x_STMP_Estimation.txt
      //           Neighbor neighborTmp;;
      //
      //
      //           ns3::Callback <std::tuple<Neighbor, double>, Neighbor*, bool, uint16_t > stmpCallback;//, std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>>
      //           float distance = 0.0;
      //           float prevDistance = 32000.0; // Invalid value
      //           uint64_t SelectedRelayNode = 0;
      //
      //           bool foundRelayNode = false;
      //           int x = 0;
      //
      //           Neighbor tmpN = std::move(it.CopyNeighbor());
      //           ndn::util::StmpKF m_stmpInstance(&tmpN, control, scheduleOnOff);//, getNodeInfoSTMPVectorAddress()
      //           stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance);
      //
      //
      //           std::tie(neighborTmp, distance) = std::move(stmpCallback(&tmpN, control, scheduleOnOff));// , getNodeInfoSTMPVectorAddress()   -- neighbor->getNmsi()
      //           if(printToFileMobilityAwareStrategy)
      //             std::cout << "\t" << ++x << "- Neighbor Node_" << it.getNodeId() << " - current Node_" << currentNodeId1 << ": Distance = " << distance;// << std::endl;
      //
      //           auto distanceToDestination = toolsStrategy.CalculateDistance(neighborTmp.getGeoPosition().getLatPosition(), neighborTmp.getGeoPosition().getLonPosition(),
      //                                                                       it.getGeoPosition().getLatPosition(), it.getGeoPosition().getLonPosition());
      //           if(printToFileMobilityAwareStrategy)
      //             std::cout << " <---> Neighbor Node_" << it.getNodeId() << " - destination Node_" << endpointId << ": Distance = " << distanceToDestination << std::endl;
      //
      //           if (distanceToDestination < prevDistance){
      //             prevDistance = distanceToDestination; // Keeps the shortest calculated distance to the destination
      //             SelectedRelayNode = it.getNodeId();
      //             // foundRelayNode = true;
      //             pendingDownstreams.emplace(&inRecord.getFace(), SelectedRelayNode);
      //             break;
      //           }
      //
      //
      //
      //
      //         }
      //       }
      //       else{ // Existing neighbor but not active!
      //         // What to do? Send for now
      //         // 2024-01-13 17:07:01
      //         // pendingDownstreams.emplace(&inRecord.getFace(), endpointId);
      //
      //       }
      //     }
      //   }
      // }
      // else{ // m_neighborhoodTable->m_neighList.empty()
      //   // Do nothing... Well check the RSU
      // }

      pendingDownstreams.emplace(&inRecord.getFace(), inRecord.getEndpointId());// Elidio: inRecord.getEndpointId()
    }
  }





  // if(foundNeighbor == true){
  //   if(!printToFileMobilityAwareStrategy)
  //     std::cout << "\tFound " << neigb << " neighbor(s). Showing only valid: " << std::endl;
  //   float farthestDistance = 0;
  //   uint64_t farthestNode = 666;
  //
  //   uint16_t scheduleOnOff = 0;
  //   bool control = true; // If true, the naame of rmse.txt will be rmse_x.txt, the NodeX_STMP_Estimation.txt will be NodeX_x_STMP_Estimation.txt
  //   Neighbor neighborTmp;;
  //
  //
  //   ns3::Callback <std::tuple<Neighbor, double>, Neighbor*, bool, uint16_t > stmpCallback;//, std::shared_ptr<std::vector<std::vector<ndn::util::NodeIdSTMP>>>
  //   float distance = 0.0;
  //   float prevDistance = 32000.0; // Invalid value
  //   uint64_t SelectedRelayNode = 0;
  //
  //   bool foundRelayNode = false;
  //   int x = 0;
  //   for(auto & it: m_neighborhoodTable->m_neighList){
  //     if ((it.getStatus() == Neighbor::STATUS_ACTIVE) && (it.getNodeId() != fromId) && (it.getNodeId() != fromId)){
  //       Neighbor tmpN = std::move(it.CopyNeighbor());
  //       ndn::util::StmpKF m_stmpInstance(&tmpN, control, scheduleOnOff);//, getNodeInfoSTMPVectorAddress()
  //       stmpCallback = ns3::MakeCallback(&ndn::util::StmpKF::stmp, &m_stmpInstance);
  //
  //
  //       std::tie(neighborTmp, distance) = std::move(stmpCallback(&tmpN, control, scheduleOnOff));// , getNodeInfoSTMPVectorAddress()   -- neighbor->getNmsi()
  //       if(printToFileMobilityAwareStrategy)
  //         std::cout << "\t" << ++x << "- Neighbor Node_" << it.getNodeId() << " - current Node_" << currentNodeId1 << ": Distance = " << distance;// << std::endl;
  //
  //       auto distanceToDestination = toolsStrategy.CalculateDistance(neighborTmp.getGeoPosition().getLatPosition(), neighborTmp.getGeoPosition().getLonPosition(),
  //                                                                   it.getGeoPosition().getLatPosition(), it.getGeoPosition().getLonPosition());
  //       // if(printToFileMobilityAwareStrategy)
  //       //   std::cout << " <---> Neighbor Node_" << it.getNodeId() << " - destination Node_" << dest << ": Distance = " << distanceToDestination << std::endl;
  //
  //       if (distanceToDestination < prevDistance){
  //         prevDistance = distanceToDestination; // Keeps the shortest calculated distance to the destination
  //         SelectedRelayNode = it.getNodeId();
  //         // foundRelayNode = true;
  //         pendingDownstreams.emplace(&inRecord.getFace(), SelectedRelayNode);
  //       }
  //     }
  //   }
  //
  //   // if(foundRelayNode){
  //   //   if(printToFileMobilityAwareStrategy)
  //   //     std::cout << "\t>>> Node_" << SelectedRelayNode << " is the nearest (" << prevDistance << "m) relay to the destination neighbor. \n\t>>> Unicasting to it..." << std::endl;
  //   //   pendingDownstreams.emplace(&inRecord.getFace(), SelectedRelayNode);
  //   // }
  //   // else{ // Do nothing for now!
  //   // //   if(printToFileMobilityAwareStrategy)
  //   // //     std::cout << "\t>>> [0] No suitable Neighbor found... " << std::endl;
  //   // //   bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri(), InterestLife_ForContainer);
  //   // //   // bool inserted = addCN(container_0, interest.getName().getSubName(0,6).toUri());
  //   // //
  //   // //    if(inserted){//inserted
  //   // //     interest.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(broadcastId));
  //   // //     broadcastInterest(interest, FaceEndpoint(ingress.face, broadcastId), pitEntry);
  //   // //     if(printToFileMobilityAwareStrategy)
  //   // //       std::cout << "\t>>> [0] No suitable Neighbor found. \n\t>>> Broadcasting first Interest..." << std::endl;
  //   // //   }
  //   // //   else{
  //   // //     if(printToFileMobilityAwareStrategy)
  //   // //       std::cout << "\t>>> [0] No suitable Neighbor found. \n\t>>> Discarding Interest..." << std::endl;
  //   // //   }
  //   // //
  //   // //   std::cout.rdbuf(coutbuf);
  //   // //   return;
  //   // }
  // }



  // data.setTag<lp::fromIdTag>(make_shared<lp::fromIdTag>(currentNodeId1));

  for (const auto& pendingDownstream : pendingDownstreams) {
    data.setTag<lp::destIdTag>(make_shared<lp::destIdTag>(pendingDownstream.second)); // Elidio For each data<->endpoint
    this->sendData(pitEntry, data, FaceEndpoint(*pendingDownstream.first, pendingDownstream.second)); // Elidio: from *pendingDownstream to ...
  }
}









void
MobilityAwareStrategy::asyncProcessData(const shared_ptr<pit::Entry>& pitEntry, const FaceEndpoint& ingress, const Data& data)
{
  // Given that this processing is asynchronous, the PIT entry's expiry timer is extended first
  // to ensure that the entry will not be removed before the whole processing is finished
  // (the PIT entry's expiry timer was set to 0 before dispatching)
  this->setExpiryTimer(pitEntry, 1_s);
  Face& inFace = ingress.face;
//   const EndpointId endpointId = ingress.endpoint;
  // std::cout << "MobilityAwareStrategy::asyncProcessData.. [ingress.endpoint] " << ingress.endpoint << std::endl;

  runOnRibIoService([pitEntryWeak = weak_ptr<pit::Entry>{pitEntry}, inFaceId = inFace.getId(), data, this] {
    rib::Service::get().getRibManager().slFindAnn(data.getName(),
      [pitEntryWeak, inFaceId, data, this] (optional<ndn::PrefixAnnouncement> paOpt) {
        if (paOpt) {
          runOnMainIoService([pitEntryWeak, inFaceId, data, pa = std::move(*paOpt), this] {
            auto pitEntry = pitEntryWeak.lock();
            auto inFace = this->getFace(inFaceId);
            if (pitEntry && inFace) {
              NFD_LOG_DEBUG("found PrefixAnnouncement=" << pa.getAnnouncedName());
			        if(printToFileMobilityAwareStrategy) std::cout << "asyncProcessData: found PrefixAnnouncement=" << pa.getAnnouncedName() << std::endl;
              data.setTag(make_shared<lp::PrefixAnnouncementTag>(lp::PrefixAnnouncementHeader(pa)));
			        // this->sendDataToAll(pitEntry, FaceEndpoint(*inFace, 9), data);//ingress.endpoint
              this->sendDataToAllCafs(pitEntry, FaceEndpoint(*inFace, 0), data);//ingress.endpoint
              this->setExpiryTimer(pitEntry, 0_ms); //Elidio. Review this to point to the RSU???
            }
            else {
              NFD_LOG_DEBUG("PIT entry or Face no longer exists");
			        if(printToFileMobilityAwareStrategy) std::cout << "asyncProcessData: PIT entry or Face no longer exists" << std::endl;
            }
          });
        }
    });
  });
}

bool
MobilityAwareStrategy::needPrefixAnn(const shared_ptr<pit::Entry>& pitEntry)
{
  if(printToFileMobilityAwareStrategy)
    std::cout << "MobilityAwareStrategy::needPrefixAnn.. " << std::endl;
  bool hasDiscoveryInterest = false;
  bool directToConsumer = true;

  auto now = time::steady_clock::now();
  for (const auto& inRecord : pitEntry->getInRecords()) {
    if (inRecord.getExpiry() > now) {
      InRecordInfo* inRecordInfo = inRecord.getStrategyInfo<InRecordInfo>();
      if (inRecordInfo && !inRecordInfo->isNonDiscoveryInterest) {
        hasDiscoveryInterest = true;
      }
      if (inRecord.getFace().getScope() != ndn::nfd::FACE_SCOPE_LOCAL) {
        directToConsumer = false;
      }
    }
  }
  return hasDiscoveryInterest && !directToConsumer;
}


void
MobilityAwareStrategy::addRoute(const shared_ptr<pit::Entry>& pitEntry, const FaceEndpoint& ingress,
								const Data& data, const ndn::PrefixAnnouncement& pa)
{
  // std::cout << "MobilityAwareStrategy::addRoute. [ingress.endpoint] " << ingress.endpoint << std::endl;
  // Face inFace = ingress.face;
  if(printToFileMobilityAwareStrategy)
    std::cout << "pa.getAnnouncedName: " << pa.getAnnouncedName().toUri() << " and pa.getExpiration(): " << pa.getExpiration() << std::endl;

  runOnRibIoService([pitEntryWeak = weak_ptr<pit::Entry>{pitEntry}, inFaceId = ingress.face.getId(), data, pa] {
		rib::Service::get().getRibManager().slAnnounce(pa, inFaceId, ROUTE_RENEW_LIFETIME,
													   [] (RibManager::SlAnnounceResult res) {
														   NFD_LOG_DEBUG("Add route via PrefixAnnouncement with result=" << res);
														   if(printToFileMobilityAwareStrategy) std::cout << "addRoute: Add route via PrefixAnnouncement with result=" << res << std::endl;
													   });
	});
}


void
MobilityAwareStrategy::renewRoute(const Name& name, const FaceEndpoint& ingress, time::milliseconds maxLifetime)
{
  FaceId inFaceId = ingress.face.getId();
	// renew route with PA or ignore PA (if route has no PA)
  if(printToFileMobilityAwareStrategy)
    std::cout << "MobilityAwareStrategy::renewRoute. [ingress.endpoint] " << ingress.endpoint << std::endl;
	runOnRibIoService([name, inFaceId, maxLifetime] {
		rib::Service::get().getRibManager().slRenew(name, inFaceId, maxLifetime,
													[] (RibManager::SlAnnounceResult res) {
														NFD_LOG_DEBUG("Renew route with result=" << res);
														if(printToFileMobilityAwareStrategy) std::cout << "Renew route with result=" << res << std::endl;
													});
	});
}


//  print FIB
// void
// MobilityAwareStrategy::FIBManagement(ns3::Ptr<ns3::Node> node, const std::string& Ffile, bool control)


// void
// MobilityAwareStrategy::FIBManagement(ns3::Ptr<ns3::Node> node, bool control)
// {
//   // bool control = false;
// 	// shared_ptr<std::ostream> outputStream;
// 	// if (Ffile != "-") {
// 	// 	shared_ptr<std::ofstream> os(new std::ofstream());
// 	// 	if (control == false){
// 	// 		os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::trunc);
// 	// 		control = true;
// 	// 	}
// 	// 	else if (control == true)
// 	// 		os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::app);
//  //
// 	// 	if (!os->is_open()) {
// 	// 		std::cout << "File " << Ffile << " <FIB> cannot be opened for writing..." << std::endl;
// 	// 		return;
// 	// 	}
//  //
// 	// 	outputStream = os;
// 	// }
// 	// else {
// 	// 	outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
// 	// }
//
// 	ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
// 	// 	Only in the simulation: Get direct access to Fib instance on the node, during the simulation
// 	const nfd::Fib& fib = node->GetObject<ns3::ndn::L3Protocol>()->getForwarder()->getFib();
// 	// 	(*outputStream) << "FIB content on node" << node->GetId() << endl;
// 	for (nfd::Fib::const_iterator entry = fib.begin(); entry != fib.end(); entry++){
// 		// (*outputStream) << ns3::Simulator::Now ().ToDouble (ns3::Time::MS) << "\t[" << fib.size() << "]: "
// 		// /*(*outputStream)*/ << entry->getPrefix() << " - (";
// 		// bool isFirst = true;
// 		int numOfValidFIBEntries = 0;
//         int numOfFIBEntries = entry->getNextHops().size();
//         for (auto& nextHop : entry->getNextHops()) {
//           numOfValidFIBEntries++;
//           auto& face = nextHop.getFace();
//           auto endpointID = nextHop.getEndpointId();
//           auto Cost = nextHop.getCost();
//
//           auto Timestamp = nextHop.getTimestamp();
//           auto Px = nextHop.getPx();
//           auto Py = nextHop.getPy();
//           auto Vx = nextHop.getVx();
//           auto Vy = nextHop.getVy();
//
//           if ((numOfFIBEntries > 2) && (numOfValidFIBEntries > 2)){
//             ::ns3::ndn::FibHelper::RemoveRoute(currentNode, endpointID, Timestamp, Cost, Px, Py, Vx, Vy, entry->getPrefix(), currentNode->GetObject<ns3::ndn::L3Protocol>()->getFaceById(257), endpointID);
//
//             // (*outputStream) << "faceId: " << face.getId() << " - endpointId: " << endpointID << " - Cost: " << Cost << " - Timestamp: " << Timestamp << " - Px: " << Px << " - Py: " << Py << " - Vx: " << Vx << " - Vy: " << Vy << "; ";
//           }
//
//
//           // (*outputStream) << "faceId: " << face.getId() << " - endpointId: " << endpointID << " - Cost: " << Cost << " - Timestamp: " << Timestamp << " - Px: " << Px << " - Py: " << Py << " - Vx: " << Vx << " - Vy: " << Vy << "; ";
//
// // 			std::cout << "\n" << face.endpoint << "\n";
//           auto transport = dynamic_cast<ns3::ndn::NetDeviceTransport*>(face.getTransport());
// //             Ptr<WifiMac> wifinetdevice2 = dynamic_cast<ns3::WifiNetDevice>(transport->GetNetDevice());
//             if (transport == nullptr)
//               continue;
// 			// (*outputStream) << " towards ";
// // 			if (!isFirst)
// // 				(*outputStream) << ", ";
// 			// (*outputStream) <<  " Node ID " << transport->GetNetDevice()->GetChannel()->GetDevice(node->GetId())->GetNode()->GetId() << " with Address: " << transport->GetNetDevice()->GetChannel()->GetDevice(node->GetId())->GetAddress();  /*<< "\n\t";*/
//
// // 			if (!isFirst)
// // 				(*outputStream) << ", ";
//
// 			// isFirst = false;
// 		}
// 		// (*outputStream) << ")" << std::endl;
// 	}
//
//
//
//   // 		std:: cout << entry->getPrefix() << std::endl;
//   // (*outputStream) << std::endl << std::endl;
//
// // ns3::Simulator::Schedule (ns3::Seconds (5.0), MobilityAwareStrategy::FIBManagement, node, Ffile, control);
// // 	std::cout << "Does it come back here? " << endl;
//
//   // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   this->m_Fibtimer.Cancel();
//   this->m_Fibtimer.SetDelay (ns3::MilliSeconds(FIBManagementTimer));
//   this->m_Fibtimer.SetFunction (&MobilityAwareStrategy::FIBManagement, this);
//   // this->m_Fibtimer.SetArguments (currentNode, tmp_dir + "DeleteFIB_Node_" + std::to_string(currentNode->GetId()) + ".txt", control);
//   this->m_Fibtimer.SetArguments (currentNode, control);
//   this->m_Fibtimer.Schedule();
//
// }


} // namespace fw
} // namespace nfd

