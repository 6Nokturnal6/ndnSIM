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

#include "fib-entry.hpp"

#include <ns3/node-list.h>
#include <ns3/node.h>
#include "ns3/ndnSIM/model/map-common.hpp"
// extern uint64_t neighborhoodTableSize;
static uint32_t packets_sequenceDataFIB = 0;
static uint32_t partial_sizesDataFIB = 0;
static double currentTimeDataFIB;
static double elapsedSecondsDataFIB;
static double lastUpdateTimeDataFIB;
static bool controlDataFIB = false;
static bool controlFIB = false;


uint32_t thresholdDataFIB = 10;

namespace nfd {
namespace fib {

Entry::Entry(const Name& prefix)
  : m_prefix(prefix)
{
}

NextHopList::iterator
Entry::findNextHop(const Face& face)
{
  return std::find_if(m_nextHops.begin(), m_nextHops.end(),
					  [&face] (const NextHop& nexthop) {
                        // std::cout << "Ever Invoked 1?" << std::endl; // Yes 20240329 once all nodes
                        return &nexthop.getFace() == &face;
                      });
}

NextHopList::iterator
Entry::findNextHop(const uint64_t endpointId)
{
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  auto nodeId = node->GetId();

  // for (const auto& x: m_nextHops){
    // std::cout << "Vamos ver! faceId: " << x.getFace().getId() << " - endpointId: " << endpointId << " -- Node_" << nodeId << std::endl;
  // }


  // std::cout << "Ever Invoked 1-11?" << std::endl;
  return std::find_if(m_nextHops.begin(), m_nextHops.end(),
					  [endpointId, nodeId] (const NextHop& nexthop) {
                        // std::cout << "Ever Invoked 1-1? -- endpointId: " << endpointId << " - Node_" << nodeId << std::endl; // Yes 20240329 once all nodes
                        return nexthop.getEndpointId() == endpointId;
                      });
}


NextHopList::iterator
Entry::findNextHop(const Face& face, const uint64_t endpointId) /*Elidio*/
{
  return std::find_if(m_nextHops.begin(), m_nextHops.end(),
					  [&face, endpointId] (const NextHop& nexthop) {
                        // std::cout << "Ever Invoked 2?" << std::endl;
                        return ((&nexthop.getFace() == &face) && (nexthop.getEndpointId() == endpointId) /*&& (nexthop.getEndpointId() == endpointId)*/);
                      });
}

NextHopList::iterator
Entry::findNextHop(const Face& face,
                   const uint64_t endpointId,
                   const uint64_t timestamp,
                   const uint32_t hops,
                   const double px,
                   const double py,
                   const double vx,
                   const double vy,
                   const uint64_t cost) /*Elidio*/
{
  return std::find_if(m_nextHops.begin(), m_nextHops.end(),
					  [&face, endpointId, cost] (const NextHop& nexthop) {
                        // std::cout << "Entry::findNextHop... Endpoint: " << nexthop.getEndpointId() << " -- Cost: " << nexthop.getCost() << std::endl;
                        return ((&nexthop.getFace() == &face) && (nexthop.getEndpointId() == endpointId) /*&& (nexthop.getCost() == cost)*/ /*&& (nexthop.getTimestamp() == timestamp) && (nexthop.getHops() == hops) && (nexthop.getPx() == px) && (nexthop.getPy() == py) && (nexthop.getVx() == vx) && (nexthop.getVy() == vy) */);
                      });
}

bool
Entry::hasNextHop(const Face& face) const
{
  // std::cout << "Aqui??? 11" << std::endl;
  return const_cast<Entry*>(this)->findNextHop(face) != m_nextHops.end();
}

// Elidio
bool
Entry::hasNextHop(const Face& face, const uint64_t endpointId) const
{
  // std::cout << "Aqui??? 12" << std::endl;
  return const_cast<Entry*>(this)->findNextHop(face, endpointId) != m_nextHops.end();//face, endpointId
}

bool
Entry::hasNextHop(const uint64_t endpointId) const
{
  // std::cout << "Aqui??? 12-1" << std::endl;
  return const_cast<Entry*>(this)->findNextHop(endpointId) != m_nextHops.end();
}


std::pair<NextHopList::iterator, bool>
Entry::addOrUpdateNextHop(Face& face, uint64_t cost)
{
  auto it = this->findNextHop(face);
  bool isNew = false;
  if (it == m_nextHops.end()) {
    m_nextHops.emplace_back(face);
    it = std::prev(m_nextHops.end());
    isNew = true;
  }

  it->setCost(cost);
  this->sortNextHops();

  // std::cout << "Aqui??? 0" << std::endl;
  return std::make_pair(it, isNew);
}

// Elidio
std::pair<NextHopList::iterator, bool>
Entry::addOrUpdateNextHop(Face& face, const uint64_t endpointId, uint64_t cost)
{
  auto it = this->findNextHop(endpointId);//endpointId
  bool isNew = false;
  if (it == m_nextHops.end()) {
    m_nextHops.emplace_back(face);//endpointId
    it = std::prev(m_nextHops.end());
    isNew = true;
  }

  it->setEndpointId(endpointId);
  it->setCost(cost);
  this->sortNextHops();

  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // std::cout << "Node_" << currentNode->GetId() << " Aqui??? 1" << std::endl; // Ok See ndn-l3-protocol.cpp 20240331
  return std::make_pair(it, isNew);
}

std::pair<NextHopList::iterator, bool>
Entry::addOrUpdateNextHop(Face& face,
           uint64_t endpointId,
           uint64_t timestamp,
           uint32_t hops,
           double px,
           double py,
           double vx,
           double vy,
           uint64_t cost)
{
  // if ((face.getId() == 257)/* && (endpointId == 997)*/){

    // auto isRemoved = removeNextHop(face, 997);// Elidio 20241111
  auto isRemoved = removeNextHop(face, endpointId, timestamp, hops, px, py, vx, vy, cost);
    // if(isRemoved)
      // std::cout << "Face removed from Entry::addOrUpdateNextHop... " << " ---endpointId: "<< endpointId << " ---Cost: " << cost << std::endl;
  // }

      // Added for routing only. 20240228

    if(!controlDataFIB){
      lastUpdateTimeDataFIB = ns3::Simulator::Now().GetMilliSeconds();
      controlDataFIB = true;
    }
    // thresholdData = 5;

    // std::cout << "Node (" << currentNodeId1 << ") Data prefix: " << data.getName().getSubName(0, 7).toUri()
    //           << "\t>>> NT size: " << m_neighborhoodTable->m_neighList.size() << std::endl;

    partial_sizesDataFIB = neighborhoodTableSize;
    packets_sequenceDataFIB++;


    currentTimeDataFIB = ns3::Simulator::Now().GetMilliSeconds();
    elapsedSecondsDataFIB = currentTimeDataFIB - lastUpdateTimeDataFIB;

    if (elapsedSecondsDataFIB >= 50.0) { // 5000 milliseconds is equivalent to 5 seconds // 12500ms = 12.5s = 250m/20mps
        // Calculate the mean of m_neighborhoodTable->m_neighList.size()
        int meanNeighListSizeDataFIB = (packets_sequenceDataFIB > 0) ? partial_sizesDataFIB / packets_sequenceDataFIB : thresholdDataFIB; //calculateMeanNeighListSize();

        // std::cout << "partial_sizesDataFIB: " << partial_sizesDataFIB << " - packets_sequenceDataFIB: " << packets_sequenceDataFIB << " - meanNeighListSizeDataFIB: " << meanNeighListSizeDataFIB << " - elapsedSecondsDataFIB: " << elapsedSecondsDataFIB << std::endl;
        partial_sizesDataFIB = 0;
        packets_sequenceDataFIB = 0;

        // Update the thresholdDataFIB and reset the update time
        thresholdDataFIB = meanNeighListSizeDataFIB;
        // lastUpdateTimeDataFIB = currentTimeDataFIB;
    }
    lastUpdateTimeDataFIB = currentTimeDataFIB;
    // lastUpdateTimeInterest = ns3::Simulator::Now().GetMilliSeconds();

    if (thresholdDataFIB > 14)
      thresholdDataFIB -= 2; //Just to correct ISR for scenarios with higher traffic.

    if (hops <= thresholdDataFIB){//thresholdInterestBeacon
      controlFIB = true;
      // std::cout << "OUT!  - hopCount: " << hopCount << " - thresholdDataFIB: " << thresholdDataFIB << std::endl;
      // if(printToFileMobilityAwareStrategy)
        // std::cout << "\t>>> Should Not be sent downstream... " << " - hopCount: " << hopCount << " - DestFinalId: " << DestFinalId << std::endl;
      // std::cout.rdbuf(coutbuf);
      // return;
    }
    else{
      controlFIB = false;
      if (face.getId() == 257)
        controlFIB = true;
    }


  auto it = this->findNextHop(face, endpointId, timestamp, hops, px, py, vx, vy, cost);//, Only uses face and endpointId
  bool isNew = false;
  if ((it == m_nextHops.end()) && ((m_nextHops.size() <= 20) && (controlFIB == true)) /*&& ((cost <= 3) || (cost >= 20))*/) {// (cost <= 2)  -- 20241112
    m_nextHops.emplace_back(face, endpointId, timestamp, hops, px, py, vx, vy, cost);//endpointId
    it = std::prev(m_nextHops.end());
    isNew = true;
  }
  else{
    it = std::prev(m_nextHops.end()); // New NextHop but with cost > 3, then point to the last item
  }

  it->setEndpointId(endpointId);
  it->setTimestamp(timestamp);
  it->setHops(hops);
  it->setPx(px);
  it->setPy(py);
  it->setVx(vx);
  it->setVy(vy);

  it->setCost(cost);
  this->sortNextHops();

  ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // std::cout << "Entry::addOrUpdateNextHop - currentNode.GetId(): " << currentNode->GetId() << " it->getEndpointId(): " << it->getEndpointId() << " - it->getTimestamp(): " << it->getTimestamp() << " - it->getPx(): " << it->getPx() << " - it->getPy(): " << it->getPy() << " - it->getVx(): " << it->getVx() << " - it->getVy(): " << it->getVy() << " -- Hops: " << hops << " -- Cost: " << cost << std::endl; // OK 20240329
  // this->sortNextHops();
  // std::cout << "Node_" << currentNode->GetId() << " Aqui??? 66" << std::endl; // // Ok See ndn-l3-protocol.cpp 20240331
  return std::make_pair(it, isNew);
}


bool
Entry::removeNextHop(const Face& face)
{
  auto it = this->findNextHop(face);
  if (it != m_nextHops.end()) {
    m_nextHops.erase(it);
    // std::cout << "Aqui??? 100" << std::endl; // No
    return true;
  }
  // std::cout << "Aqui??? 200" << std::endl; // No
  return false;
}

bool
Entry::removeNextHop(const Face& face, const uint64_t endpointId)  // Elidio
{
  auto it = this->findNextHop(endpointId);  // endpointId Elidio
  if (it != m_nextHops.end()) {
    m_nextHops.erase(it);
    // std::cout << "Aqui??? 300" << std::endl; // No
    return true;
  }
  // std::cout << "Aqui??? 3000" << std::endl; // No
  return false;
}

bool
Entry::removeNextHop(const Face& face,
           uint64_t endpointId,
           uint64_t timestamp,
           uint32_t hops,
           double px,
           double py,
           double vx,
           double vy,
           uint64_t cost)  // Elidio
{
  auto it = this->findNextHop(face, endpointId, timestamp, hops, px, py, vx, vy, cost);  // Only face, endpointId...

  if ((it != m_nextHops.end()) /*&& (it->getCost() >= cost)*/) { // if existing has cost >= than actual new nexthop remove...
    m_nextHops.erase(it);
    // std::cout << "Entry::removeNextHop... EndpointId: " << endpointId << " -- Cost: " << cost << std::endl; // No
    return true;
  }
  // std::cout << "Aqui??? 3000x" << std::endl; // No
  return false;
}


// bool
// Entry::removeNextHop(const Face& face, const uint64_t endpointId)  // Elidio
// {
//   auto it = this->findNextHop(face, endpointId);  // Elidio
//   if (it != m_nextHops.end()) {
//     m_nextHops.erase(it);
//     // std::cout << "Aqui??? 300" << std::endl; // No
//     return true;
//   }
//   // std::cout << "Aqui??? 300" << std::endl; // No
//   return false;
// }


void
Entry::sortNextHops()
{
  std::sort(m_nextHops.begin(), m_nextHops.end(),
            [] (const NextHop& a, const NextHop& b) { return a.getCost() < b.getCost(); });
}

} // namespace fib
} // namespace nfd
