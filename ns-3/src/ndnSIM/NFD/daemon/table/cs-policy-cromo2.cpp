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

#include "cs-policy-cromo2.hpp"
#include "cs.hpp"
#include "common/global.hpp"

/**** For getting Node ID ****/
#include <ns3/simulator.h>
#include <ns3/node-list.h>
#include <ns3/node.h>

// For redirecting std::cout
#include <bits/stdc++.h>

// For node position
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
/*****************************/

namespace nfd {
namespace cs {
namespace cromo2 {

const std::string CRoMo2Policy::POLICY_NAME = "cromo2";
NFD_REGISTER_CS_POLICY(CRoMo2Policy);

CRoMo2Policy::CRoMo2Policy()
  : Policy(POLICY_NAME)
{
}

CRoMo2Policy::~CRoMo2Policy()
{
  for (auto entryInfoMapPair : m_entryInfoMap) {
    delete entryInfoMapPair.second;
  }
}

void
CRoMo2Policy::doAfterInsert(EntryRef i)
{
//   std::cout << " Name of the packet: " << Policy::getCs()->begin()->Entry::getFullName() << std::endl; //i->Entry::getFullName()

  this->attachQueue(i);

  /***************************************************************************************************************/
  // auto node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   auto nodeId = node->GetId();
//   std::cout << "Name of the inserted packet: " << i->Entry::getName() << std::endl;

  // ndn::util::NodeSpeed nodespeed(i->Entry::getNodeSpeed());

  // ndn::util::GeoPosition geoposition(i->Entry::getNodePosition());

  // auto name = i->Entry::getPrefix();
  // auto nodeId = i->Entry::getNodeId();
  // uint64_t timestamp = i->Entry::getNodeTimestamp();
  // auto nodeDirection = i->Entry::getNodeDirection();

  // ndn::util::Nmsi nmsi(nodeId, name, geoposition, nodespeed, nodeDirection, timestamp);

  /**
   * Store NMSI into local CCT, from CS ---------- commented out 20230118
   */

  // LcctQueue::iterator it = find2(name);
  // static int control = 1;
  // if (it == m_lcctqueue.end()){
  //   // std::cout << "[CRoMo2Policy::doAfterInsert]. Enqueuing NMSI with prefix: " << name << std::endl;
  //   this->m_lcctqueue.push_back(nmsi);
  // }
  // else if ((control % 15) == 0){// Updates some duplicated
  //   // std::cout << "[CRoMo2Policy::doAfterInsert]. Re-enqueuing NMSI with prefix: " << name << ". Control = " << control << std::endl;
  //   it = this->m_lcctqueue.erase(it);
  //   this->m_lcctqueue.insert(it, nmsi);
  //   control++;
  // }



  // }
  // k = 0;

//   lcct.push_back(*nmsi);

  /**********************************************************************************************************/

  this->evictEntries(); //TODO Elidio. Remove entries from lcct as well

  /**********************************************************************************************************/

}

void
CRoMo2Policy::doAfterRefresh(EntryRef i)
{
  this->detachQueue(i);
  this->attachQueue(i);
}

void
CRoMo2Policy::doBeforeErase(EntryRef i)
{
//   std::cout << " Name of the evicted packet: " << i->Entry::getName() << std::endl;
  this->detachQueue(i);
}

void
CRoMo2Policy::doBeforeUse(EntryRef i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());
}

void
CRoMo2Policy::evictEntries()
{
  BOOST_ASSERT(this->getCs() != nullptr);

  while (this->getCs()->size() > this->getLimit()) {
    this->evictOne();
  }
}

void
CRoMo2Policy::evictOne()
{
  BOOST_ASSERT(!m_queues[QUEUE_UNSOLICITED].empty() ||
               !m_queues[QUEUE_STALE].empty() ||
               /* !m_queues[QUEUE_CCT].empty() || Elidio 20220514 */
               !m_queues[QUEUE_FIFO].empty());

  EntryRef i;
  if (!m_queues[QUEUE_UNSOLICITED].empty()) {
    i = m_queues[QUEUE_UNSOLICITED].front();
  }
  else if (!m_queues[QUEUE_STALE].empty()) {
    i = m_queues[QUEUE_STALE].front();
  }
  else if (!m_queues[QUEUE_FIFO].empty()) {
    i = m_queues[QUEUE_FIFO].front();
  }
//   else if (!m_queues[QUEUE_CCT].empty()) { /* Elidio 20220514*/
//     i = m_queues[QUEUE_CCT].front();
//   }

//   Entry entrada;

//   auto val = i->Entry::getNodePosition();

//   std::cout << " Name of the evicted packet: " << i->Entry::getName() << std::endl;



  this->detachQueue(i);
  this->emitSignal(beforeEvict, i);
}

void
CRoMo2Policy::attachQueue(EntryRef i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) == m_entryInfoMap.end());

  EntryInfo* entryInfo = new EntryInfo();
  if (i->isUnsolicited()) {
    entryInfo->queueType = QUEUE_UNSOLICITED;
  }
  else if (!i->isFresh()) {
    entryInfo->queueType = QUEUE_STALE;
  }
  else {
    entryInfo->queueType = QUEUE_FIFO;
    entryInfo->moveStaleEventId = getScheduler().schedule(i->getData().getFreshnessPeriod(),
                                                          [=] { moveToStaleQueue(i); });
  }

  Queue& queue = m_queues[entryInfo->queueType];
  entryInfo->queueIt = queue.insert(queue.end(), i);

  /* Elidio 20220514 */
//   if (entryInfo->queueType == QUEUE_FIFO){
//     queue = m_queues[QUEUE_CCT];
//     queue.insert(queue.end(), i); // Insert to CCT queue
//     queue = m_queues[QUEUE_FIFO];
//   }

  m_entryInfoMap[i] = entryInfo;
}

void
CRoMo2Policy::detachQueue(EntryRef i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());

  EntryInfo* entryInfo = m_entryInfoMap[i];
  if (entryInfo->queueType == QUEUE_FIFO) {
    entryInfo->moveStaleEventId.cancel();
  }

  m_queues[entryInfo->queueType].erase(entryInfo->queueIt);
//   if (entryInfo->queueType == QUEUE_FIFO) /* Elidio 20220514*/
//     m_queues[QUEUE_CCT].erase(entryInfo->queueIt);

  m_entryInfoMap.erase(i);
  delete entryInfo;
}

void
CRoMo2Policy::moveToStaleQueue(EntryRef i)
{
  BOOST_ASSERT(m_entryInfoMap.find(i) != m_entryInfoMap.end());

  EntryInfo* entryInfo = m_entryInfoMap[i];
  BOOST_ASSERT(entryInfo->queueType == QUEUE_FIFO);

  m_queues[QUEUE_FIFO].erase(entryInfo->queueIt);
//   m_queues[QUEUE_CCT].erase(entryInfo->queueIt); /* Elidio 20220514*/

  entryInfo->queueType = QUEUE_STALE;
  Queue& queue = m_queues[QUEUE_STALE];
  entryInfo->queueIt = queue.insert(queue.end(), i);
  m_entryInfoMap[i] = entryInfo;
}

LcctQueue::iterator
CRoMo2Policy::find2(const std::string name)
{
  LcctQueue::iterator it = std::find_if(m_lcctqueue.begin(),
                                                  m_lcctqueue.end(),
                                                  [&name](const ndn::util::Nmsi& a){return a.getPrefix() == name;});
  return it;
}



} // namespace cromo2
} // namespace cs
} // namespace nfd
