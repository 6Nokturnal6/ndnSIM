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

// ndn-simple.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

/* Elidio */
#include "ns3/ndnSIM/model/ndn-net-device-transport.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>


namespace ns3 {

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-simple
 */


/* Elidio */

//  print PIT
void
printPIT(Ptr<Node> node, const std::string& Ffile, bool control)
{
	shared_ptr<std::ostream> outputStream;
	if (Ffile != "-") {
		shared_ptr<std::ofstream> os(new std::ofstream());
		if (control == false){
			os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::trunc);
			control = true;
		}
		else if (control == true)
			os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::app);

		if (!os->is_open()) {
			std::cout << "File " << Ffile << " <PIT> cannot be opened for writing..." << std::endl;
			return;
		}

		outputStream = os;
	}
	else {
		outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
	}
// 	shared_ptr<nfd::pit::Entry> pitEntry;
	const nfd::Pit& pit = node->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
	for (nfd::Pit::const_iterator entry = pit.begin(); entry != pit.end(); entry++){
		auto interest_name = entry->getInterest().getName();//OR entry->getName();
		(*outputStream) << Simulator::Now ().ToDouble (Time::S) << "\t"
		<< interest_name /*entry->getName()*/ /*<< "\t"
		<< entry->GetId()*/ /*<< std::endl*/;
		// remember pending downstreams
// 		auto pitEntry = entry;
		for (const nfd::pit::InRecord& inRecord : entry->getInRecords()) {
			(*outputStream) << "\t inRecord Face: " << inRecord.getFace() << std::endl;
// 			(*outputStream) << "\n\t inRecord Face: " << inRecord.getFace().getLocalUri() << "\n";
            (*outputStream) << "\t inRecord endpointID: " << inRecord.getEndpointId() << std::endl;
		}
// 		for (const nfd::pit::InRecord& inRecord : entry->getInRecords()) {
// 			(*outputStream) << "\n\t inRecord endpointID: " << inRecord.getEndpointId() << std::endl;
// 			// 			(*outputStream) << "\n\t inRecord Face: " << inRecord.getFace().getLocalUri() << "\n";
// 		}
		for (const nfd::pit::OutRecord& outRecord : entry->getOutRecords()) {
			(*outputStream) << "\n\t outRecord Face: " << outRecord.getFace() << std::endl;
			// 			(*outputStream) << "\t outRecord Face: " << outRecord.getFace().getLocalUri() << std::endl;//getRemoteUri()
		}
		(*outputStream) << std::endl;
	}


	(*outputStream) << std::endl;
	Simulator::Schedule (Seconds (5.0), printPIT, node, Ffile, control);
}

//  print FIB
void
// printFIBmoreData = [](Ptr<Node> node, Time nextTime)
printFIBmoreData(Ptr<Node> node, const std::string& Ffile, bool control)
{
	shared_ptr<std::ostream> outputStream;
	if (Ffile != "-") {
		shared_ptr<std::ofstream> os(new std::ofstream());
		if (control == false){
			os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::trunc);
			control = true;
		}
		else if (control == true)
			os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::app);

		if (!os->is_open()) {
			std::cout << "File " << Ffile << " <FIB> cannot be opened for writing..." << std::endl;
			return;
		}

		outputStream = os;
	}
	else {
		outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
	}
	// 	Only in the simulation: Get direct access to Fib instance on the node, during the simulation
	const nfd::Fib& fib = node->GetObject<ndn::L3Protocol>()->getForwarder()->getFib();
	// 	(*outputStream) << "FIB content on node" << node->GetId() << endl;
	for (nfd::Fib::const_iterator entry = fib.begin(); entry != fib.end(); entry++){
		(*outputStream) << Simulator::Now ().ToDouble (Time::S) << "\t"
		/*(*outputStream)*/ << entry->getPrefix() << " - (";
		bool isFirst = true;
		for (auto& nextHop : entry->getNextHops()) {
			auto& face = nextHop.getFace();
			auto endpointID = nextHop.getEndpointId();
            auto Cost = nextHop.getCost();
			(*outputStream) << endpointID << " - "/*<< " Face: " */<< face /*<< " Face ID: " << nextHop.getFace().getId()*/ << " - " << Cost; // Cost, new element added

// 			std::cout << "\n" << face.endpoint << "\n";
			auto transport = dynamic_cast<ndn::NetDeviceTransport*>(face.getTransport());
			if (transport == nullptr)
				continue;
			(*outputStream) << " towards ";
// 			if (!isFirst)
// 				(*outputStream) << ", ";
			(*outputStream) <<  " Node ID " << transport->GetNetDevice()->GetChannel()->GetDevice(0)->GetNode()->GetId() << " with Address: " << transport->GetNetDevice()->GetChannel()->GetDevice(0)->GetAddress();  /*<< "\n\t";*/

// 			if (!isFirst)
// 				(*outputStream) << ", ";

			isFirst = false;
		}
		(*outputStream) << ")" << std::endl;
	}



// 		std:: cout << entry->getPrefix() << std::endl;
(*outputStream) << std::endl;

Simulator::Schedule (Seconds (5.0), printFIBmoreData, node, Ffile, control);
// 	std::cout << "Does it come back here? " << endl;
}

//  print CS
void
printCS(Ptr<Node> node, const std::string& Ffile, bool control)
{
	shared_ptr<std::ostream> outputStream;
	if (Ffile != "-") {
		shared_ptr<std::ofstream> os(new std::ofstream());
		if (control == false){
			os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::trunc);
			control = true;
		}
		else if (control == true)
			os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::app);

		if (!os->is_open()) {
			std::cout << "File " << Ffile << " <CS> cannot be opened for writing..." << std::endl;
			return;
		}

		outputStream = os;
	}
	else {
		outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
	}

	const nfd::Cs& cs = node->GetObject<ndn::L3Protocol>()->getForwarder()->getCs();
	(*outputStream) << "Number of stored contents: " << cs.size() << std::endl;// getFullName
	for (nfd::Cs::const_iterator entry = cs.begin(); entry != cs.end(); entry++)
		(*outputStream) << Simulator::Now ().ToDouble (Time::S) << "\t" << "Content name: " << entry->getName().toUri() << "\t" << "Node Speed: " << entry->getNodeSpeed() << "\t" << "Geo-Position: " << entry->getNodePosition() << "\t" << "Prefix: " << entry->getPrefix() << "\t" << "Direction: " << entry->getNodeDirection() << "\t" << "Timestamp: " << entry->getNodeTimestamp() << std::endl;// getFullName

	(*outputStream) << std::endl;
	Simulator::Schedule (Seconds (5.0), printCS, node, Ffile, control);
}


/**********/



int
main(int argc, char* argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create(3);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;

  ndnHelper.setPolicy("nfd::cs::cromo2");
  ndnHelper.setCsSize(10);//1000

  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCRoMo2");
  // Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper.SetPrefix("/prefix");
  consumerHelper.SetAttribute("Frequency", StringValue("100")); // 10 interests a second
  auto apps = consumerHelper.Install(nodes.Get(0));                        // first node
  apps.Stop(Seconds(5.0)); // stop the consumer app at 5 seconds mark
//   apps.Stop(Seconds(10.0)); // stop the consumer app at 5 seconds mark

  /************/
  const bool control = false;
  //int i = 0;
  printFIBmoreData (nodes.Get(0), "FIB_Consumer.txt", control);
  printPIT (nodes.Get(0), "PIT_Consumer.txt", control);
  printCS(nodes.Get(0), "CS_Consumer.txt", control);

  /************/
  // router stuff
  printFIBmoreData (nodes.Get(1), "FIB_Router.txt", control);
  printPIT (nodes.Get(1), "PIT_Router.txt", control);
  printCS(nodes.Get(1), "CS_Router.txt", control);
//   apps.Stop(Seconds(10.0)); // stop the consumer app at 5 seconds mark


  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::ProducerCRoMo2");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix("/prefix");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(nodes.Get(2)); // last node

  /**************/
  	// For producer
  printFIBmoreData (nodes.Get(2), "FIB_Producer.txt", false);
  printPIT (nodes.Get(2), "PIT_Producer.txt", false);
  printCS (nodes.Get(2), "CS_Producer.txt", false);

  /**************/


  Simulator::Stop(Seconds(20.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
