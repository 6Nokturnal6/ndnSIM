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

#include "ndn-fib-helper.hpp"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/names.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/callback.h"
// #include "ns3/node.h"
#include "ns3/core-config.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/callback.h"
#include "ns3/node-list.h"
#include "ns3/data-rate.h"

#include "daemon/mgmt/fib-manager.hpp"
// #include "ns3/ndnSIM/model/ndn-l3-protocol.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"

#include <ns3/node-list.h>
#include <ns3/node.h>

#include "ndn-stack-helper.hpp"

#include "ns3/ndnSIM/NFD/daemon/face/generic-link-service.hpp"

namespace ns3 {
namespace ndn {

NS_LOG_COMPONENT_DEFINE("ndn.FibHelper");

void
FibHelper::AddNextHop(const ControlParameters& parameters, Ptr<Node> node)
{
  Block encodedParameters(parameters.wireEncode());

  Name commandName;
  // std::cout << "parameters.getName().getSubName(3, 1).toUri(): " << parameters.getName().getSubName(3, 1).toUri() << std::endl;
  if (1/*parameters.getName().getSubName(3, 1).toUri() != "/consumer"*/){
    commandName = "/localhost/nfd/fib";
    commandName.append("add-nexthop");
  }
  else{
    commandName = parameters.getName().getSubName(0, 6).toUri();// from 4 to 6
    commandName.append("add-nexthop");
  }
  commandName.append(encodedParameters);

  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // std::cout << "FibHelper_AddNextHop: currentNode->GetId():" << currentNode->GetId() << " - " << parameters << std::endl; // Ok. 20240331
  // bool x = false;
  // if(parameters.hasExpirationPeriod() != 0){
  //   x = true;
  // }
  //
  // if(x)
  //   std::cout << "See add commandName: " << commandName << " - Expiration: " << parameters.getExpirationPeriod () << std::endl;
  // else
  //   std::cout << "See add commandName: " << commandName << " - Expiration: " << 0 << std::endl;


  shared_ptr<Interest> command(make_shared<Interest>(commandName));
  command->setCanBePrefix(false);
  StackHelper::getKeyChain().sign(*command);

  Ptr<L3Protocol> l3protocol = node->GetObject<L3Protocol>();
  l3protocol->injectInterest(*command);
}

void
FibHelper::AddNextHop(const ControlParameters& parameters, Ptr<Node> node, bool justForControl)
{
  Block encodedParameters(parameters.wireEncode());

  Name commandName(parameters.getName());
  // std::cout << "See add parameters.getName(): " << parameters.getName() << std::endl;
  // commandName.append("add-nexthop");
  commandName.append(encodedParameters);

  // ns3::Ptr<ns3::Node> currentNode = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // std::cout << "FibHelper_AddNextHop: currentNode->GetId():" << currentNode->GetId() << " - " << parameters << "\n\n - commandName: " << commandName << std::endl; // Ok. 20240331
  shared_ptr<Interest> command(make_shared<Interest>(commandName));
  command->setCanBePrefix(false);
  StackHelper::getKeyChain().sign(*command);

  Ptr<L3Protocol> l3protocol = node->GetObject<L3Protocol>();
  l3protocol->injectInterest(*command);
}

void
FibHelper::RemoveNextHop(const ControlParameters& parameters, Ptr<Node> node)
{
  Block encodedParameters(parameters.wireEncode());

  Name commandName("/localhost/nfd/fib");
  commandName.append("remove-nexthop");
  commandName.append(encodedParameters);

  shared_ptr<Interest> command(make_shared<Interest>(commandName));
  command->setCanBePrefix(false);
  StackHelper::getKeyChain().sign(*command);

  Ptr<L3Protocol> l3protocol = node->GetObject<L3Protocol>();
  l3protocol->injectInterest(*command);
}


void
FibHelper::RemoveNextHop(const ControlParameters& parameters, Ptr<Node> node, bool justForControl)
{
  Block encodedParameters(parameters.wireEncode());

  Name commandName(parameters.getName());
  // std::cout << "See remove parameters.getName(): " << parameters.getName() << std::endl;
  // commandName.append("remove-nexthop");
  commandName.append(encodedParameters);

  shared_ptr<Interest> command(make_shared<Interest>(commandName));
  command->setCanBePrefix(false);
  StackHelper::getKeyChain().sign(*command);

  Ptr<L3Protocol> l3protocol = node->GetObject<L3Protocol>();
  l3protocol->injectInterest(*command);
}
void
FibHelper::AddRoute(Ptr<Node> node, const Name& prefix, shared_ptr<Face> face, int32_t metric)
{
  NS_LOG_LOGIC("[" << node->GetId() << "]$ route add " << prefix << " via " << face->getLocalUri()
                   << " metric " << metric);
  ns3::Ptr<ns3::Node> NodeToAdd = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // std::cout << "[" << node->GetId() << "]$ route add " << prefix << " via " << face->getLocalUri()
                   // << " metric " << metric << " - NodeToAdd->GetId(): " << NodeToAdd->GetId() << std::endl;

  ControlParameters parameters;
  parameters.setName(prefix);
  parameters.setFaceId(face->getId());
  parameters.setCost(metric);

  AddNextHop(parameters, node);
}

// ---------------- Elidio -------------------
void
FibHelper::AddRoute(Ptr<Node> node, const uint64_t neighborNodeId, const Name& prefix, shared_ptr<Face> face, int32_t metric)
{
  NS_LOG_LOGIC("[" << node->GetId() << "]$ route add " << prefix << " via " << face->getLocalUri()
                   << " metric " << metric);
  ns3::Ptr<ns3::Node> NodeToAdd = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // std::cout << "[" << node->GetId() << "]$ route add " << prefix << " via " << face->getLocalUri()
                   // << " metric " << metric << " - neighborNodeId: " << neighborNodeId << std::endl;

  ControlParameters parameters;
  parameters.setName(prefix);
  parameters.setFaceId(face->getId());
  parameters.setCost(metric);
  parameters.setEndpointId(neighborNodeId);// Elidio: 20240228

  AddNextHop(parameters, node);
}

void
FibHelper::AddRoute(Ptr<Node> node,
           const uint64_t neighborNodeId,
           const uint64_t timestamp,
           const uint32_t hops,
           const double px,
           const double py,
           const double vx,
           const double vy,
           const Name& prefix, shared_ptr<Face> face, int32_t metric)
{

  // static int countingAddRoute = 0;
  NS_LOG_LOGIC("[" << node->GetId() << "]$ route add " << prefix << " via " << face->getLocalUri()
                   << " metric " << metric);
  // ns3::Ptr<ns3::Node> NodeToAdd = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  // std::cout << "[" << node->GetId() << "]$ route add " << prefix  << " - neighborNodeId: " << neighborNodeId << " - countingAddRoute: " << countingAddRoute++ << std::endl;

  ControlParameters parameters;
  parameters.setName(prefix);
  parameters.setFaceId(face->getId());
  parameters.setCost(metric);
  // Elidio: 20240305
  parameters.setEndpointId(neighborNodeId);//neighborNodeId
  parameters.setTimestamp(timestamp);
  parameters.setHops(hops);
  parameters.setPx(px);
  parameters.setPy(py);
  parameters.setVx(vx);
  parameters.setVy(vy);
  parameters.setExpirationPeriod(ndn::time::seconds(260));

  // std::cout << "FIB Helper - AddRoute - parameters: " << parameters << std::endl; // Ok 20240331
  // AddNextHop(parameters, node, true);
  AddNextHop(parameters, node);
}

void
FibHelper::AddRoute(Ptr<Node> node, const uint64_t neighborNodeId, const Name& prefix, uint32_t faceId, int32_t metric)
{
  Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
  NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

  shared_ptr<Face> face = ndn->getFaceById(faceId);
  NS_ASSERT_MSG(face != 0, "Face with ID [" << faceId << "] does not exist on node ["
                                            << node->GetId() << "]");

  AddRoute(node, neighborNodeId, prefix, face, metric);
}

void
FibHelper::AddRoute(Ptr<Node> node,
           const uint64_t neighborNodeId,
           const uint64_t timestamp,
           const uint32_t hops,
           const double px,
           const double py,
           const double vx,
           const double vy,
           const Name& prefix, uint32_t faceId, int32_t metric)
{
  Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
  NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

  shared_ptr<Face> face = ndn->getFaceById(faceId);
  NS_ASSERT_MSG(face != 0, "Face with ID [" << faceId << "] does not exist on node ["
                                            << node->GetId() << "]");

  // std::cout << "Face with ID [" << faceId << "] does not exist on node ["
                                            // << node->GetId() << "]" << std::endl;

  AddRoute(node, neighborNodeId, timestamp, hops, px, py, vx, vy, prefix, face, metric);
}

// -------------------------------------------------------------

void
FibHelper::AddRoute(Ptr<Node> node, const Name& prefix, uint32_t faceId, int32_t metric)
{
  Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
  NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

  shared_ptr<Face> face = ndn->getFaceById(faceId);
  NS_ASSERT_MSG(face != 0, "Face with ID [" << faceId << "] does not exist on node ["
                                            << node->GetId() << "]");

  AddRoute(node, prefix, face, metric);
}

void
FibHelper::AddRoute(const std::string& nodeName, const Name& prefix, uint32_t faceId,
                    int32_t metric)
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  NS_ASSERT_MSG(node != 0, "Node [" << nodeName << "] does not exist");

  Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
  NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

  shared_ptr<Face> face = ndn->getFaceById(faceId);
  NS_ASSERT_MSG(face != 0, "Face with ID [" << faceId << "] does not exist on node [" << nodeName
                                            << "]");

  AddRoute(node, prefix, face, metric);
}

// Elidio
void
FibHelper::AddRoute(const std::string& nodeName,
                    const uint64_t neighborNodeId,
                    const uint64_t timestamp,
                    const uint32_t hops,
                    const double px,
                    const double py,
                    const double vx,
                    const double vy,
                    const Name& prefix, uint32_t faceId, int32_t metric)
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  NS_ASSERT_MSG(node != 0, "Node [" << nodeName << "] does not exist");

  Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
  NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

  shared_ptr<Face> face = ndn->getFaceById(faceId);
  NS_ASSERT_MSG(face != 0, "Face with ID [" << faceId << "] does not exist on node [" << nodeName
                                            << "]");

  AddRoute(node, neighborNodeId, timestamp, hops, px, py, vx, vy, prefix, face, metric);
}

void
FibHelper::AddRoute(Ptr<Node> node, const Name& prefix, Ptr<Node> otherNode, int32_t metric)
{
  for (uint32_t deviceId = 0; deviceId < node->GetNDevices(); deviceId++) {
    Ptr<PointToPointNetDevice> netDevice =
      DynamicCast<PointToPointNetDevice>(node->GetDevice(deviceId));
    if (netDevice == 0)
      continue;

    Ptr<Channel> channel = netDevice->GetChannel();
    if (channel == 0)
      continue;

    if (channel->GetDevice(0)->GetNode() == otherNode
        || channel->GetDevice(1)->GetNode() == otherNode) {
      Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
      NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

      shared_ptr<Face> face = ndn->getFaceByNetDevice(netDevice);
      NS_ASSERT_MSG(face != 0, "There is no face associated with the p2p link");

      AddRoute(node, prefix, face, metric);

      return;
    }
  }

  NS_FATAL_ERROR("Cannot add route: Node# " << node->GetId() << " and Node# " << otherNode->GetId()
                                            << " are not connected");
}

void
FibHelper::AddRoute(const std::string& nodeName, const Name& prefix,
                    const std::string& otherNodeName, int32_t metric)
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  NS_ASSERT_MSG(node != 0, "Node [" << nodeName << "] does not exist");

  Ptr<Node> otherNode = Names::Find<Node>(otherNodeName);
  NS_ASSERT_MSG(otherNode != 0, "Node [" << otherNodeName << "] does not exist");

  AddRoute(node, prefix, otherNode, metric);
}

void
FibHelper::RemoveRoute(Ptr<Node> node, const Name& prefix, shared_ptr<Face> face)
{
  NS_LOG_LOGIC("[" << node->GetId() << "]$ route del " << prefix << " via " << face->getLocalUri());

  // Get L3Protocol object
  Ptr<L3Protocol> L3protocol = node->GetObject<L3Protocol>();
  // Get the forwarder instance
  shared_ptr<nfd::Forwarder> m_forwarder = L3protocol->getForwarder();

  ControlParameters parameters;
  parameters.setName(prefix);
  parameters.setFaceId(face->getId());

  RemoveNextHop(parameters, node);
}

// Elidio
void
FibHelper::RemoveRoute(Ptr<Node> node, const Name& prefix, shared_ptr<Face> face, const uint64_t endpointId)
{
  NS_LOG_LOGIC("[" << node->GetId() << "]$ route del " << prefix << " via " << face->getLocalUri());
  // std::cout << "[" << node->GetId() << "]$ route del " << prefix << " via " << face->getLocalUri() << " - endpointId: " << endpointId << std::endl;
  // std::cout << "Chamada para remover...11" << std::endl;
  // Get L3Protocol object
  Ptr<L3Protocol> L3protocol = node->GetObject<L3Protocol>();
  // Get the forwarder instance
  shared_ptr<nfd::Forwarder> m_forwarder = L3protocol->getForwarder();

  ControlParameters parameters;
  parameters.setName(prefix);
  parameters.setFaceId(face->getId());
  parameters.setEndpointId(endpointId);

  RemoveNextHop(parameters, node);
}

void
FibHelper::RemoveRoute(Ptr<Node> node,
           const uint64_t neighborNodeId,
           const uint64_t timestamp,
           const uint32_t hops,
           const double px,
           const double py,
           const double vx,
           const double vy,
           const Name& prefix,
           shared_ptr<Face> face,
           const uint64_t cost)
{
  NS_LOG_LOGIC("[" << node->GetId() << "]$ route del " << prefix << " via " << face->getLocalUri());
 // std::cout << "[" << node->GetId() << "]$ route del 22" << prefix << " via " << face->getLocalUri() << std::endl;
 // std::cout << "Chamada para remover...22" << std::endl;
  // Get L3Protocol object
  Ptr<L3Protocol> L3protocol = node->GetObject<L3Protocol>();
  // Get the forwarder instance
  shared_ptr<nfd::Forwarder> m_forwarder = L3protocol->getForwarder();

  ControlParameters parameters;
  parameters.setName(prefix);
  parameters.setFaceId(face->getId());

  //
  parameters.setEndpointId(neighborNodeId);//neighborNodeId
  parameters.setTimestamp(timestamp);
  parameters.setHops(hops);
  parameters.setPx(px);
  parameters.setPy(py);
  parameters.setVx(vx);
  parameters.setVy(vy);
  parameters.setCost(cost);

  // std::cout << "Node (" << node->GetId() << ") Removing route with prefix: " << prefix << ", cost: " << hops << " - With timestamp: " << timestamp << std::endl;
  // RemoveNextHop(parameters, node, true);
  RemoveNextHop(parameters, node);
}


void
FibHelper::RemoveRoute(Ptr<Node> node, const Name& prefix, uint32_t faceId)
{
  Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
  NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

  shared_ptr<Face> face = ndn->getFaceById(faceId);
  NS_ASSERT_MSG(face != 0, "Face with ID [" << faceId << "] does not exist on node ["
                                            << node->GetId() << "]");

  RemoveRoute(node, prefix, face);
}

// Elidio
void
FibHelper::RemoveRoute(Ptr<Node> node, const Name& prefix, uint32_t faceId, const uint64_t endpointId)
{
  Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
  NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

  shared_ptr<Face> face = ndn->getFaceById(faceId);
  NS_ASSERT_MSG(face != 0, "Face with ID [" << faceId << "] does not exist on node ["
                                            << node->GetId() << "]");

  RemoveRoute(node, prefix, face, endpointId);
}

void
FibHelper::RemoveRoute(const std::string& nodeName, const Name& prefix, uint32_t faceId)
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
  NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

  shared_ptr<Face> face = ndn->getFaceById(faceId);
  NS_ASSERT_MSG(face != 0, "Face with ID [" << faceId << "] does not exist on node ["
                                            << node->GetId() << "]");

  RemoveRoute(node, prefix, face);
}

// Elidio

void
FibHelper::RemoveRoute(const std::string& nodeName, const Name& prefix, uint32_t faceId, const uint64_t endpointId)
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
  NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

  shared_ptr<Face> face = ndn->getFaceById(faceId);
  NS_ASSERT_MSG(face != 0, "Face with ID [" << faceId << "] does not exist on node ["
                                            << node->GetId() << "]");

  RemoveRoute(node, prefix, face, endpointId);
}


void
FibHelper::RemoveRoute(Ptr<Node> node, const Name& prefix, Ptr<Node> otherNode)
{
  for (uint32_t deviceId = 0; deviceId < node->GetNDevices(); deviceId++) {
    Ptr<PointToPointNetDevice> netDevice =
      DynamicCast<PointToPointNetDevice>(node->GetDevice(deviceId));
    if (netDevice == 0)
      continue;

    Ptr<Channel> channel = netDevice->GetChannel();
    if (channel == 0)
      continue;

    if (channel->GetDevice(0)->GetNode() == otherNode
        || channel->GetDevice(1)->GetNode() == otherNode) {
      Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
      NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

      shared_ptr<Face> face = ndn->getFaceByNetDevice(netDevice);
      NS_ASSERT_MSG(face != 0, "There is no face associated with the p2p link");

      RemoveRoute(node, prefix, face);

      return;
    }
  }

  NS_FATAL_ERROR("Cannot remove route: Node# " << node->GetId() << " and Node# " << otherNode->GetId()
                                            << " are not connected");
}


// Elidio
void
FibHelper::RemoveRoute(Ptr<Node> node, const Name& prefix, Ptr<Node> otherNode, const uint64_t endpointId)
{
  for (uint32_t deviceId = 0; deviceId < node->GetNDevices(); deviceId++) {
    Ptr<PointToPointNetDevice> netDevice =
      DynamicCast<PointToPointNetDevice>(node->GetDevice(deviceId));
    if (netDevice == 0)
      continue;

    Ptr<Channel> channel = netDevice->GetChannel();
    if (channel == 0)
      continue;

    if (channel->GetDevice(0)->GetNode() == otherNode
        || channel->GetDevice(1)->GetNode() == otherNode) {
      Ptr<L3Protocol> ndn = node->GetObject<L3Protocol>();
      NS_ASSERT_MSG(ndn != 0, "Ndn stack should be installed on the node");

      shared_ptr<Face> face = ndn->getFaceByNetDevice(netDevice);
      NS_ASSERT_MSG(face != 0, "There is no face associated with the p2p link");

      RemoveRoute(node, prefix, face, endpointId);

      return;
    }
  }

  NS_FATAL_ERROR("Cannot remove route: Node# " << node->GetId() << " and Node# " << otherNode->GetId()
                                            << " are not connected");
}


void
FibHelper::RemoveRoute(const std::string& nodeName, const Name& prefix,
                    const std::string& otherNodeName)
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  Ptr<Node> otherNode = Names::Find<Node>(otherNodeName);
  RemoveRoute(node, prefix, otherNode);
}


// Elidio

void
FibHelper::RemoveRoute(const std::string& nodeName, const Name& prefix,
                    const std::string& otherNodeName, const uint64_t endpointId)
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  Ptr<Node> otherNode = Names::Find<Node>(otherNodeName);
  RemoveRoute(node, prefix, otherNode, endpointId);
}


} // namespace ndn

} // namespace ns
