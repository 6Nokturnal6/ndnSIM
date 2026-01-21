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

#ifndef NFD_DAEMON_TABLE_CS_POLICY_CROMO2_HPP
#define NFD_DAEMON_TABLE_CS_POLICY_CROMO2_HPP

#include "cs-policy.hpp"

#include <list>
#include <vector>

/*For friendship and LCCT access*/
// #include "apps/ndn-producer-cromo2.hpp"
// #include "apps/ndn-producer.hpp"

// class ProducerCRoMo2;

namespace nfd {
namespace cs {
namespace cromo2 {

using Queue = std::list<Policy::EntryRef>;
using LcctQueue = std::list<ndn::util::Nmsi>;

// using LcctQueue = std::list<std::set<ndn::util::Nmsi, std::less<>>::const_iterator>;



// using NmsiEntryRef = NmsiTable::const_iterator; // Elidio For CCT
// nfd::cs::cromo2::LcctQueue lcct; //functioning
// std::set<ndn::util::Nmsi, std::less<>> lcct;


enum QueueType {
  QUEUE_UNSOLICITED,
  QUEUE_STALE,
  QUEUE_FIFO,
/*  QUEUE_CCT,  Elidio: 20220514*/
  QUEUE_MAX
};

struct EntryInfo
{
  QueueType queueType;
  Queue::iterator queueIt;
  scheduler::EventId moveStaleEventId;
};



// LCCTQueue::iterator lcctqueueIt;
/** \brief Priority FIFO replacement policy
 *
 *  This policy maintains a set of cleanup queues to decide the eviction order of CS entries.
 *  The cleanup queues are three doubly linked lists that store EntryRefs.
 *  The three queues keep track of unsolicited, stale, and fresh Data packet, respectively.
 *  EntryRef is placed into, removed from, and moved between suitable queues
 *  whenever an Entry is added, removed, or has other attribute changes.
 *  Each Entry should be in exactly one queue at any moment.
 *  Within each queue, the EntryRefs are kept in first-in-first-out order.
 *  Eviction procedure exhausts the first queue before moving onto the next queue,
 *  in the order of unsolicited, stale, and fresh queue.
 */
class CRoMo2Policy : public Policy
{
public:
  CRoMo2Policy();

  ~CRoMo2Policy() override;

public:
  static const std::string POLICY_NAME;

  // inline bool
  // compare(const std::string name) const
  // {
  //   return it->getPrefix() == name;
  // }

  LcctQueue::iterator
  find2(const std::string name);

private:
  void
  doAfterInsert(EntryRef i) override;

  void
  doAfterRefresh(EntryRef i) override;

  void
  doBeforeErase(EntryRef i) override;

  void
  doBeforeUse(EntryRef i) override;

  void
  evictEntries() override;

private:
  /** \brief evicts one entry
   *  \pre CS is not empty
   */
  void
  evictOne();

  /** \brief attaches the entry to an appropriate queue
   *  \pre the entry is not in any queue
   */
  void
  attachQueue(EntryRef i);

  /** \brief detaches the entry from its current queue
   *  \post the entry is not in any queue
   */
  void
  detachQueue(EntryRef i);

  /** \brief moves an entry from FIFO queue to STALE queue
   */
  void
  moveToStaleQueue(EntryRef i);


private:
  Queue m_queues[QUEUE_MAX];
  LcctQueue m_lcctqueue;//[QUEUE_MAX];
  std::map<EntryRef, EntryInfo*> m_entryInfoMap;

//   friend class ProducerCRoMo2;// To grant access to LCCT

public: /* Elidio*/
//   LcctQueue m_lcctqueue;//[QUEUE_MAX];
  LcctQueue::iterator lccqueueIt;

  void
  PrintLCCT (){
  for (lccqueueIt = this->m_lcctqueue.begin(); lccqueueIt != this->m_lcctqueue.end(); lccqueueIt++)
    std::cout << *lccqueueIt << "$" << std::endl;// The $ terminator is used on the process of NMSI extraction by the producer-cromo2 for sharing caching content
  };

//   auto
//   ObjectCRoMo2(){
//     return this;
//   }
//   friend class ProducerCRoMo2;// To grant access to LCCT
};

// void
// PrintLCCT (){
//   LcctQueue::iterator lccqueueIt;
//
//   for (lccqueueIt = m_lcctqueue.begin(); lccqueueIt != m_lcctqueue.end(); lccqueueIt++)
//     std::cout << *lccqueueIt << std::endl;
// };





} // namespace cromo2


using cromo2::CRoMo2Policy;

} // namespace cs
} // namespace nfd

#endif // NFD_DAEMON_TABLE_CS_POLICY_CROMO2_HPP
