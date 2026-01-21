/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef NDN_BFCLIENT_CROMO2_H
#define NDN_BFCLIENT_CROMO2_H

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/model/map-common.hpp"
#include "ndn-consumer.hpp"

// extern bool invokeConsumer;
namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * @brief Ndn application for sending out Interest packets at a "constant" rate (Poisson process)
 */
class BFClientCRoMo2 : virtual public Consumer {
public:
  static TypeId
  GetTypeId();

  /**
   * \brief Default constructor
   * Sets up randomizer function and packet sequence number
   */
  BFClientCRoMo2();
  virtual ~BFClientCRoMo2();


  // void
  // ScheduleBFquerying(){
  //
  //
  // void
  // StopApplication();
  //
  // // void
  // // ShareBF(uint64_t requestedNode);
  //
  // void
  // StartApplication();
  //   // if (!m_sendEvent.IsRunning())
  //     // Simulator::Schedule(Seconds(0.0), ScheduleNextPacket());
  //
  //   // Simulator::Cancel (m_sendEvent);
  //   // double retxTimeout = 5;
  //   // SetRetxTimer(Seconds(retxTimeout));
  //
  //   // std::cout << "invokeConsumerClient: " << invokeConsumerClient << std::endl;
  //   // if (invokeConsumerClient) {
  //   //
  //   //   // SetStartTime (Seconds(0));
  //   //   // DoInitialize();
  //   //   // StartApplication();
  //   //   // SetStartTime (Seconds(0));
  //   //   ScheduleNextPacket();
  //   //   // invokeConsumer = 0;
  //   // }
  //   // else{
  //   //   std::cout << "Not initializing BFClientCRoMo2." << std::endl;
  //   // }
  //
  //   // invokeConsumerClient = true;
  //   // invokeFromClient = true;
  //   // this->ScheduleNextPacket();
  //   // StartApplication();
  //   // SendPacket2();
  //   // invokeFromClient = false;
  //   // invokeConsumerClient = false;
  //
  // };

  void
  StartApplication();

  void
  ScheduleNextPacket();

  void
  ShareBF(uint64_t requestedNode);

  void
  WillSendOutInterest(uint32_t sequenceNumber);

  void
  OnTimeout(uint32_t sequenceNumber);

  void
  OnData(shared_ptr<const Data> data);

protected:
  /**
   * \brief Constructs the Interest packet and sends it using a callback to the underlying NDN
   * protocol
   */
  // virtual void
  // ScheduleNextPacket();

  /**
   * @brief Set type of frequency randomization
   * @param value Either 'none', 'uniform', or 'exponential'
   */
  void
  SetRandomize(const std::string& value);

  /**
   * @brief Get type of frequency randomization
   * @returns either 'none', 'uniform', or 'exponential'
   */
  std::string
  GetRandomize() const;

  // void
  // SendPacket();

    // From App, consumer
  // virtual void
  // OnData(shared_ptr<const Data> contentObject);


protected:
  double m_frequency; // Frequency of interest packets (in hertz)
  bool m_firstTime;
  Ptr<RandomVariableStream> m_random;
  std::string m_randomType;
};

} // namespace ndn
} // namespace ns3

#endif
