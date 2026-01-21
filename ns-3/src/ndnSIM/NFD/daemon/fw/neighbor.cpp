/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018,  The University of Memphis,
 *                           Regents of the University of California
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

#include "neighbor.hpp"
// #include "common/logger.hpp"

#include <iostream>
#include <string>
#include <cmath>
#include <limits>

#include "ns3/log.h"

#include "ns3/node-list.h"
// #include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/timer.h"

namespace nfd {
namespace fw {

NS_LOG_COMPONENT_DEFINE ("Neighbor");

const float Neighbor::DEFAULT_LINK_COST = 10.0;

Neighbor::Neighbor() // Invalid value
  : m_ntimer_1 (ns3::Timer::CANCEL_ON_DESTROY)
  // , m_nmsi()
  , m_status (STATUS_ACTIVE)
  {
  }

Neighbor::Neighbor(const ndn::util::Nmsi nmsi, ns3::Time delay)
  : m_ntimer_1 (ns3::Timer::CANCEL_ON_DESTROY)
  , m_nmsi(nmsi)
  , m_status (STATUS_ACTIVE)
  {
  }

// Neighbor::Neighbor(const ndn::util::Nmsi nmsi)
//   : m_ntimer_1 (ns3::Timer::CANCEL_ON_DESTROY)
//   , m_nmsi(nmsi)
//   {
//     // m_nmsi.setNmsi(nmsi);
//   }

Neighbor::~Neighbor(){
    // std::cout << "Left Timer before die: " << this->m_ntimer_1.GetDelayLeft() << std::endl;
    m_ntimer_1.Cancel();
    m_ntimer_1.Remove();
  }

bool
Neighbor::operator==(const Neighbor& neighbor) const
{
  return (this->getNodeId() == neighbor.getNodeId());
}

bool
Neighbor::operator<(const Neighbor& neighbor) const
{
  auto linkCost = neighbor.getLinkCost();
  uint64_t n1 = this->getNodeId();
  uint64_t n2 = neighbor.getNodeId();

  return std::tie(n1, m_linkCost) <
         std::tie(n1, linkCost);
}

std::ostream&
operator<<(std::ostream& os, const Neighbor& neighbor)
{
  // os << "Neighbor: " << neighbor.getNodeId() << "\n Status: " << neighbor.m_status << "\n Expire Time: " << neighbor.m_expireTime << "\n Distance: " << neighbor.getDistance() << "\n nmsi: \n" << neighbor.getNmsi() << std::endl;

  os << "\n\tNeighbor: " << neighbor.getNodeId() << "\n\t Status: " << neighbor.m_status << "\n\t Distance: " << neighbor.getDistance() << "\n\t nmsi: \n\t" << neighbor.getNmsi() << std::endl;

  return os;
}

void
Neighbor::writeLog()
{
//   NLSR_LOG_DEBUG(*this);
}

void
Neighbor::ScheduleTimer ()
{
//   this->m_ntimer_1.Cancel ();
  this->m_ntimer_1.Schedule ();
}


}
} // namespace cromo2


