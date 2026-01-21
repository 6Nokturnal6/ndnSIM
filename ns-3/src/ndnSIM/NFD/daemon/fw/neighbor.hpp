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

#include <string>
#include <cmath>
#include <boost/cstdint.hpp>

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/net/face-uri.hpp>
#include "ndn-cxx/util/tools.hpp"

#include "ns3/timer.h"
#include "ns3/nstime.h"
#include <ndn-cxx/util/scheduler.hpp>
#include "ns3/callback.h"


#include "ndn-cxx/util/stmp-kf.hpp"


// #include <tuple>
#ifndef NFD_DAEMON_FW_NEIGHBOR_HPP
#define NFD_DAEMON_FW_NEIGHBOR_HPP


// int neighborTimer = 5; // Timer for updating neigbor info in NT
// int neighborhoodTimer = 16; // Timer for cleaning neigbor info up in NT

// using namespace ns3;
namespace nfd {
namespace fw {

/*! \brief A neighbor reachable over a Face.
 *
 * Represents another node that we expect to be running CROMO2 that we
 * should be able to reach over a direct Face connection.
 */
class Neighbor
{

public:
  enum Status
  {
    STATUS_UNKNOWN = -1,
    STATUS_INACTIVE = 0,
    STATUS_ACTIVE = 1
  };

  Neighbor();

//   Neighbor(ns3::Time t, ns3::Time delay = ns3::Seconds(6));//{}

  Neighbor(const ndn::util::Nmsi nmsi, ns3::Time delay = ns3::Seconds(5));
  // Neighbor(const ndn::util::Nmsi nmsi);

//   Neighbor(const shared_ptr<ndn::util::Nmsi> nmsi, ns3::Time delay = ns3::Seconds(6));

  // Copy constructor
  Neighbor(const Neighbor& other)
    : m_linkCost(other.m_linkCost),
      m_status(other.m_status),
      m_expireTime(other.m_expireTime),
      m_distance(other.m_distance),
      // m_hops(other.m_hops),
      m_nmsi(other.m_nmsi),
      m_ntimer_1(other.m_ntimer_1)
  {
    m_ntimer_1.Cancel();
  }

  ~Neighbor();

  const uint64_t
  getNodeId() const
  {
    return m_nmsi.getNodeId();
  }

  const uint64_t
  getHops() const
  {
    return m_nmsi.getHops();
  }

  const std::string
  getPrefix() const
  {
    return m_nmsi.getPrefix();
  }

  const uint64_t
  getTimestamp() const
  {
    return m_nmsi.getTimestamp();
  }

  const ndn::util::GeoPosition
  getGeoPosition() const
  {
    return m_nmsi.getGeoPosition();
  }


  void//ndn::util::Nmsi*
  setPosition(double latitude, double longitude, double altitude)
  {
    m_nmsi.setPosition(latitude,longitude,altitude);
//     return this;
  }


  const ndn::util::Nmsi
  getNmsi() const
  {
    return m_nmsi.getNmsi();
  }

  std::shared_ptr<ndn::util::Nmsi>
  getAdressNmsi() const
  {
    return m_nmsi.getNmsi2();
  }

  void
  setNmsi(const uint64_t nodeId, const std::string prefix, const ndn::util::GeoPosition position, const ndn::util::NodeSpeed speed, const uint64_t hops, const uint64_t timestamp)
  {
    m_nmsi.setNmsi(nodeId, prefix, position, speed, hops, timestamp);
  }

  void
  setNmsi(const ndn::util::Nmsi nmsi)
  {
    m_nmsi.setNmsi(nmsi);
//     return this->getNmsi();
  }



  uint64_t
  getLinkCost() const
  {
    uint64_t linkCost = static_cast<uint64_t>(ceil(m_linkCost));
    return linkCost;
  }

  void
  setLinkCost(double lc)
  {
    m_linkCost = lc;
  }

  Status
  getStatus() const
  {
    return m_status;
  }

  void
  setStatus(Status s)
  {
    m_status = s;
  }


  ns3::Time
  getExpireTime() const
  {
    return m_expireTime;
  }

  // ns3::Time
  // getDelayLeft() const
  // {
  //   return m_ntimer_1;// .GetDouble();
  // }


  /// Get distante to the neighbor
  float
  getDistance() const
  {
    return m_distance;
  }

  /// Set distante to the neighbor
  void
  setDistance(float dist)
  {
    m_distance = dist;
  }

  /// Set # hops
  void
  setHops(uint64_t hops)
  {
    m_hops = hops;
  }

  /*! \brief Equality is when name, Face URI, and link cost are all equal. */
  bool
  operator==(const Neighbor& neighbor) const;

  bool
  operator!=(const Neighbor& neighbor) const
  {
    return !(*this == neighbor);
  }

  bool
  operator<(const Neighbor& neighbor) const;

  inline bool
  compare(const uint64_t &neighborhoodNodeId) const
  {
    return this->getNodeId() == neighborhoodNodeId;
  }

  void
  writeLog();

  // Copy method
  /**
   *
   * Neighbor original;
   * Neighbor copied = original.Copy();
   */
  Neighbor CopyNeighbor() const
  {
    Neighbor copy;
    copy.m_linkCost = m_linkCost;
    copy.m_status = m_status;
    copy.m_expireTime = m_expireTime;
    copy.m_distance = m_distance;
    // copy.m_hops = m_hops;
    copy.setNmsi(this->getNmsi());
    // copy.m_ntimer_1 = m_ntimer_1;
    copy.m_ntimer_1.Cancel();
    return std::move(copy);
  }


// public:
  static const float DEFAULT_LINK_COST;

  void ScheduleTimer ();


public:
  ns3::Timer m_ntimer_1;// ns3::Timer m_ntimer_1 (ns3::Timer::CANCEL_ON_DESTROY); // Scheduled on neighbor insertion -> neighborhood-list.cpp (insertOrUpdate(Neighbor& neighbor))
//   ndn::util::StmpKF m_stmpInstance;

private:
  /*! m_linkCost The semi-arbitrary cost to traverse the link. */
  double m_linkCost;
  /*! m_status Whether the neighbor is active or not */
  Status m_status = STATUS_ACTIVE;

// Neighbor expire time
  ns3::Time m_expireTime = ns3::Seconds(10);

  /// Neighbor distance from actual node
  double m_distance;

  /// Neighbor hops from actual node
  uint64_t m_hops;


  /// Node mobiity status information
  ndn::util::Nmsi m_nmsi;

  friend std::ostream&
  operator<<(std::ostream& os, const Neighbor& neighbor);
};

std::ostream&
operator<<(std::ostream& os, const Neighbor& neighbor);

}
}// namespace nlsr

#endif // CROMO2_NEIGHBOR_HPP
