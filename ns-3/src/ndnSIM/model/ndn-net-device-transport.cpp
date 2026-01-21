/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2018  Regents of the University of California.
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

#include "ndn-net-device-transport.hpp"

#include "../helper/ndn-stack-helper.hpp"
#include "ndn-block-header.hpp"
#include "../utils/ndn-ns3-packet-tag.hpp"

#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>

#include "ns3/queue.h"

// Elidio
#include "ns3/wifi-net-device.h"
#include "ns3/sta-wifi-mac.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/mac48-address.h"
#include <ndn-cxx/util/colors.hpp>// For coloring std::cout

#include <limits>
#include <map>
#include <boost/lexical_cast.hpp>
#include "ns3/string.h"

#include "ns3/ndnSIM/model/map-common.hpp"
// extern std::vector<std::tuple<std::string, int>> NodeIdMacMap;

NS_LOG_COMPONENT_DEFINE("ndn.NetDeviceTransport");

namespace ns3 {
namespace ndn {

NetDeviceTransport::NetDeviceTransport(Ptr<Node> node,
                                       const Ptr<NetDevice>& netDevice,
                                       const std::string& localUri,
                                       const std::string& remoteUri,
                                       ::ndn::nfd::FaceScope scope,
                                       ::ndn::nfd::FacePersistency persistency,
                                       ::ndn::nfd::LinkType linkType)
  : m_netDevice(netDevice)
  , m_node(node)
{
  this->setLocalUri(FaceUri(localUri));
  this->setRemoteUri(FaceUri(remoteUri));
  this->setScope(scope);
  this->setPersistency(persistency);
  this->setLinkType(linkType);
  this->setMtu(m_netDevice->GetMtu()); // Use the MTU of the netDevice

  // Get send queue capacity for congestion marking
  PointerValue txQueueAttribute;
  if (m_netDevice->GetAttributeFailSafe("TxQueue", txQueueAttribute)) {
    Ptr<ns3::QueueBase> txQueue = txQueueAttribute.Get<ns3::QueueBase>();
    // must be put into bytes mode queue

    auto size = txQueue->GetMaxSize();
    if (size.GetUnit() == BYTES) {
      this->setSendQueueCapacity(size.GetValue());
    }
    else {
      // don't know the exact size in bytes, guessing based on "standard" packet size
      this->setSendQueueCapacity(size.GetValue() * 1500);
    }
  }

  NS_LOG_FUNCTION(this << "Creating an ndnSIM transport instance for netDevice with URI"
                  << this->getLocalUri());

  NS_ASSERT_MSG(m_netDevice != 0, "NetDeviceFace needs to be assigned a valid NetDevice");

  m_node->RegisterProtocolHandler(MakeCallback(&NetDeviceTransport::receiveFromNetDevice, this),
                                  L3Protocol::ETHERNET_FRAME_TYPE, m_netDevice,
                                  true /*promiscuous mode*/);
}

NetDeviceTransport::~NetDeviceTransport()
{
  NS_LOG_FUNCTION_NOARGS();
}

ssize_t
NetDeviceTransport::getSendQueueLength()
{
  PointerValue txQueueAttribute;
  if (m_netDevice->GetAttributeFailSafe("TxQueue", txQueueAttribute)) {
    Ptr<ns3::QueueBase> txQueue = txQueueAttribute.Get<ns3::QueueBase>();
    return txQueue->GetNBytes();
  }
  else {
    return nfd::face::QUEUE_UNSUPPORTED;
  }
}

void
NetDeviceTransport::doClose()
{
  NS_LOG_FUNCTION(this << "Closing transport for netDevice with URI"
                  << this->getLocalUri());

  // set the state of the transport to "CLOSED"
  this->setState(nfd::face::TransportState::CLOSED);
}

// void
// NetDeviceTransport::doSend(const Block& packet, const nfd::EndpointId& endpoint)
// {
// 	NS_LOG_FUNCTION(this << "Sending packet from netDevice with URI"
// 	<< this->getLocalUri());
//
// 	// convert NFD packet to NS3 packet
// 	BlockHeader header(packet);
//
// 	Ptr<ns3::Packet> ns3Packet = Create<ns3::Packet>();
// 	ns3Packet->AddHeader(header);
//
// 	// send the NS3 packet
// 	m_netDevice->Send(ns3Packet, m_netDevice->GetBroadcast(),
// 					  L3Protocol::ETHERNET_FRAME_TYPE);
// }




void
NetDeviceTransport::doSend(const Block& packet, const nfd::EndpointId& endpointId)
{
  NS_LOG_FUNCTION(this << "Sending packet from netDevice with URI"
                  << this->getLocalUri());

  // convert NFD packet to NS3 packet
  BlockHeader header(packet);

  Ptr<ns3::Packet> ns3Packet = Create<ns3::Packet>();
  ns3Packet->AddHeader(header);

//   Elidio --begin--
  Address dest;

  // Ptr<ns3::StaWifiMac> staMac;

  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // auto nodeId = currentNode->GetId();
  // // ===============================================================================
  //
  // std::string Ffile = tmp_dir + "doSend_Context_Node_" + std::to_string(nodeId) + ".txt";
  // std::ofstream out;
  // out.open(Ffile, std::fstream::app);
  //
  // auto *coutbuf = std::cout.rdbuf();
  // std::cout.rdbuf(out.rdbuf());
  //
  // std::cout << ns3::Simulator::Now().GetMilliSeconds() << std::endl;
  // std::cout << "endpoint: " << endpointId << std::endl;

  /** reset cout buffer **/
  // std::cout.rdbuf(coutbuf);

  if(endpointId > 120){// Maximum nodes used... Before: endpointId == 666
    dest = m_netDevice->GetBroadcast();
  }
  else{
    auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                    [&endpointId](const std::tuple<std::string, int>& NodeIdMacMap)
                    {return std::get<1>(NodeIdMacMap) == endpointId;});
    if (it != NodeIdMacMap.end()) {
      std::string endereco = std::get<0>(*it); // e.g., "00:00:00:00:00:00:00"
      endereco.pop_back(); //to exclude the last \"
      const char * endereco2 = &endereco[1]; // Starts from 1, instead 0, o exclude the first \"
      dest = Mac48Address(endereco2);
    }
  }

  // dest = m_netDevice->GetBroadcast();
  // std::cout << "endpoint: " << endpointId << " dest:" << dest << std::endl;
  m_netDevice->Send(ns3Packet, dest,
				  L3Protocol::ETHERNET_FRAME_TYPE);

  // std::cout << /*FBLU(*/"From NetDevice: "/*)*/ << source  << " to Address (EndpointId): " << dest << " (" << /*uri*/  endpoint << ")" << std::endl << std::endl;

  /** reset cout buffer **/
  // std::cout.rdbuf(coutbuf);
}



// callback
void
NetDeviceTransport::receiveFromNetDevice(Ptr<NetDevice> device,
                                         Ptr<const ns3::Packet> p,
                                         uint16_t protocol,
                                         const Address& from, const Address& to,
                                         NetDevice::PacketType packetType)
{
	NS_LOG_FUNCTION(device << p << protocol << from << to << packetType);

	ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
    auto nodeId = currentNode->GetId();

  // IMPORTANT:
  // const Address& to -- carries the destination defined in NetDeviceTransport::doSend(const Block& packet, const nfd::EndpointId& endpoint
  // const Address& from -- from which node the received packet comes from


  // std::string Ffile = tmp_dir + "receiveFrom_Context_Node_" + std::to_string(nodeId) + ".txt";
  // std::ofstream out;
  // out.open(Ffile, std::fstream::app);
  //
  // auto *coutbuf = std::cout.rdbuf();
  // std::cout.rdbuf(out.rdbuf());
  //
  // std::cout << ns3::Simulator::Now().GetMilliSeconds() << std::endl;
  // std::cout << "\nNode: " << nodeId << "\nCurrent device: " << device->GetAddress() << "\nfrom: " << from << "\nTo: " << to << std::endl << std::endl;
  //


  // Convert NS3 packet to NFD packet
  Ptr<ns3::Packet> packet = p->Copy();
  // packet.addTag(make_shared<lp::fromIdTag>(endpoint));

  BlockHeader header;
  packet->RemoveHeader(header);

  nfd::EndpointId endpoint = 999;

  //===============================================================

  Address dest = m_netDevice->GetBroadcast();

  auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
                    [&from](const std::tuple<std::string, int>& NodeIdMacMap)
                    {
                      std::string endereco = std::get<0>(NodeIdMacMap);
                      endereco.pop_back(); //do not include the last \"
                      const char * endereco2 = &endereco[1];
                      return Mac48Address(endereco2) == Mac48Address::ConvertFrom (from);// NOT to but from
                    });
  if (it != NodeIdMacMap.end()) {
    auto endP = std::get<1>(*it);
    endpoint = endP;//607;//
  }


  // std::cout << "\nNode: " << nodeId << "\nCurrent device: " << device->GetAddress() << "\nfrom: " << from << "\nTo: " << to << "\nendpoint: " << endpoint << std::endl;

  // if(endpoint == nodeId){
  //   std::cout << "Exiting from NetDeviceTransport::receiveFromNetDevice... endpoint: " << endpoint << " - nodeId: " << nodeId << std::endl;
  //   return;
  // }
  // else
    // std::cout << "Exiting from NetDeviceTransport::receiveFromNetDevice... endpoint: " << endpoint << " - nodeId: " << nodeId << std::endl;






  // std::cout << "from: " << from << " to: " << to << " assigned endpoint: " << endpoint << std::endl;
  // auto nfdPacket = header.getBlock();
  // nfdPacket.addTag(make_shared<lp::fromIdTag>(endpoint));
  this->receive(std::move(header.getBlock()), endpoint);



  // header.addTag(make_shared<lp::fromIdTag>(endpoint));
  /*this->receive(std::move(header.getBlock()), endpoint); */// Transport::receive(const Block& packet, const EndpointId& endpoint), the endpoint have been initialized to 0 in this function in transport.hpp
// }


  /** reset cout buffer **/
  // std::cout.rdbuf(coutbuf);
}
Ptr<NetDevice>
NetDeviceTransport::GetNetDevice() const
{
  return m_netDevice;
}

} // namespace ndn
} // namespace ns3
