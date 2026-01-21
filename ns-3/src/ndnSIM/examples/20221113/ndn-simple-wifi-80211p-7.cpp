//  To RUN
// NS_LOG=ndn.Consumer:ndn.Producer ./waf --run "ndn-simple-wifi-80211p-7 --traceFile=/home/dasilva/mobility_20210106.tcl --nodeNum=12  --duration=490.0 --logFile=ns2-mobility-trace_20210123.log"


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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"

#include "ns3/ndnSIM-module.h"

//
#include "ns3/ns2-mobility-helper.h"
#include "ns3/netanim-module.h"
#include "face/face.hpp"
#include "core/common.hpp"

#include "ns3/ndnSIM/model/ndn-net-device-transport.hpp"
// #include <ndn-net-device-face.h>

#include <iostream>
#include <fstream>
#include <cstdio>
// #include "ndn-cxx/name.hpp"

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

#include "boost/variant.hpp"

using namespace std;
namespace ns3 {

// NS_LOG_COMPONENT_DEFINE("ndn.WifiExample.20201019");


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
			(*outputStream) << "\n\t inRecord Face: " << inRecord.getFace() << std::endl;
// 			(*outputStream) << "\n\t inRecord Face: " << inRecord.getFace().getLocalUri() << "\n";
		}
		for (const nfd::pit::InRecord& inRecord : entry->getInRecords()) {
			(*outputStream) << "\n\t inRecord endpointID: " << inRecord.getEndpointId() << std::endl;
			// 			(*outputStream) << "\n\t inRecord Face: " << inRecord.getFace().getLocalUri() << "\n";
		}
		for (const nfd::pit::OutRecord& outRecord : entry->getOutRecords()) {
			(*outputStream) << "\t outRecord Face: " << outRecord.getFace() << std::endl;
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
			(*outputStream) << endpointID << " - "/*<< " Face: " */<< face /*<< " Face ID: " << nextHop.getFace().getId()*/;

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
		(*outputStream) << Simulator::Now ().ToDouble (Time::S) << "\t" << "Content name: " << entry->getName().toUri() /*<< "Content size:" << entry->getSize()*/ << std::endl;// getFullName
	
	(*outputStream) << std::endl;
	Simulator::Schedule (Seconds (5.0), printCS, node, Ffile, control);
}	

// Prints actual position and velocity when a course change event occurs
static void
CourseChange (std::ostream *os, std::string foo, Ptr<const MobilityModel> mobility)
{
	Vector pos = mobility->GetPosition (); // Get position
	Vector vel = mobility->GetVelocity (); // Get velocity
	
	// Prints position and velocities
	*os << Simulator::Now () << " POS: x=" << pos.x << ", y=" << pos.y
	<< ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
	<< ", z=" << vel.z << std::endl;
}



int
main(int argc, char* argv[])
{
	
// 	LogComponentEnable ("ndn.WifiExample.20201019",LOG_LEVEL_DEBUG);
	NS_LOG_COMPONENT_DEFINE ("ndn.WifiExample.80211p.20201019");
	
	// disable fragmentation
	Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
// 	Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
// 					   StringValue("OfdmRate24Mbps"));
	

	Wifi80211pHelper wifi80211p;// = Wifi80211pHelper::Default (); // Do not load Default (avoid "NonUnicastMode", StringValue ...)
	bool verbose = false;
	if (verbose)
	{
		wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
	}
	
	std::string phyMode ("OfdmRate6MbpsBW10MHz");
	wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
										"DataMode",StringValue (phyMode),
										"ControlMode",StringValue (phyMode));
	
	
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
	wifiPhyHelper.SetChannel(wifiChannel.Create());
	// ns-3 supports generate a pcap trace
	wifiPhyHelper.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
	NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default();

	
	std::string traceFile;
	std::string logFile;
	
	int    nodeNum;
	double duration;

	CommandLine cmd;
	cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
	cmd.AddValue ("nodeNum", "Number of nodes", nodeNum);
	cmd.AddValue ("duration", "Duration of Simulation", duration);
	cmd.AddValue ("logFile", "Log file", logFile);
	cmd.Parse (argc,argv);

// 	sumoD -c osm.sumocfg --step-length 0.001 --eager-insert true --fcd-output trace_20201017.xml
// 	Global options that affect Departure
// 	--random-depart-offset <TIME>: randomly delays departure time for all vehicles
// 	--max-num-vehicles <INT>: limits the total amount of vehicles that may exist in the network. Setting this may cause delayed insertion
// 	--max-depart-delay <TIME>: removes vehicles from the insertion queue after a set amount of time
// 	--eager-insert <BOOL>: tries to insert all vehicles that are insertion-delayed on each edge. By default, insertion on an edge stops after the first failure in each time step
// 	--step-length <FLOAT>: the frequency of insertion attempts can be raised and also the number of possible headways (for a fixed departPos) is increased by setting a lower step-length.
// 	--extrapolate-departpos <BOOL>: Moves the default depart position of a vehicle downstream if it's departure time is a fraction of the simulation step-length. The offset in depart position is proportional to the depart delay from step-length discretization.
	
  //////////////////////
	if (traceFile.empty () || nodeNum <= 0 || duration <= 0 || logFile.empty ())
	{
		std::cout << "Usage of " << argv[0] << " :\n\n"
		"./waf --run \"ns2-mobility-trace"
		" --traceFile=src/mobility/examples/default.ns_movements"
		" --nodeNum=2 --duration=100.0 --logFile=ns2-mob.log\" \n\n"
		"NOTE: ns2-traces-file could be an absolute or relative path. You could use the file default.ns_movements\n"
		"      included in the same directory of this example file.\n\n"
		"NOTE 2: Number of nodes present in the trace file must match with the command line argument and must\n"
		"        be a positive number. Note that you must know it before to be able to load it.\n\n"
		"NOTE 3: Duration must be a positive number. Note that you must know it before to be able to load it.\n\n";
		
		return 0;
	}
	
	
	// open log file for output
	std::ofstream os;
	os.open (logFile.c_str ());
	
	// Create Ns2MobilityHelper with the specified trace log file as parameter
  Ns2MobilityHelper ns2 = Ns2MobilityHelper(traceFile);

  NodeContainer nodes;
  nodes.Create(nodeNum);

  ////////////////
  // 1. Install Wifi
//   NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, nodes);
//   bool RecursiveCall = false;
  NetDeviceContainer wifiNetDevices = wifi80211p.Install(wifiPhyHelper, wifi80211pMac, nodes);
//   One more device installed on Node 3
//   wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, nodes[0]));
  
//   if (RecursiveCall == false){
// 	  std::cout << "RecursiveCall " << RecursiveCall << std::endl;
// 	  RecursiveCall = true;
// 	  wifi80211p.Install(true, wifiPhyHelper, wifi80211pMac, nodes[0]); //ACTIVATE...
// 	}
	
  //   One more device installed on Node 2
//   wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, nodes[1]));
//   //   One more device installed on Node 3
//   wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, nodes[2]));
//   //   One more device installed on Node 2
//   wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, nodes[3]));
//   //   One more device installed on Node 3
//   wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, nodes[4]));
//   //   One more device installed on Node 2
//   wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, nodes[5]));
//   //   One more device installed on Node 3
//   wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, nodes[6]));
  //   One more device installed on Node 2
//   wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, nodes[2]));
 
  // Tracing
  wifiPhyHelper.EnablePcap ("V2V-pdeec-simple-80211p", wifiNetDevices);
  
  
  
  // 2. Install Mobility model
  ns2.Install (); // configure movements for each node, while reading trace file

  // 3. Install NDN stack
  NS_LOG_INFO("Installing NDN stack");
  ndn::StackHelper ndnHelper;
  // ndnHelper.AddNetDeviceFaceCreateCallback (WifiNetDevice::GetTypeId (), MakeCallback
  // (MyNetDeviceFaceCallback));
  ndnHelper.setPolicy("nfd::cs::lru");
  ndnHelper.setCsSize(1000);//1000
//   ndnHelper.SetDefaultRoutes(true);
//   ndnHelper.Install(nodes);
  ndnHelper.InstallAll();

  
  // Installing global routing interface on all nodes
  
  // No route??
  
//   ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
//   ndnGlobalRoutingHelper.InstallAll();
  
  
//   ndnGlobalRoutingHelper.Install(nodes);
  
  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::Install(nodes, "/", "/localhost/nfd/strategy/mobility-aware");
//   ndn::StrategyChoiceHelper::Install(nodes, "/", "/localhost/nfd/strategy/self-learning");
//   ndn::StrategyChoiceHelper::Install(nodes, "/", "/localhost/nfd/strategy/best-route");
//   ndn::StrategyChoiceHelper::Install(nodes, "/", "/localhost/nfd/strategy/multicast");
//   ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route");

  // 4. Set up applications
  NS_LOG_INFO("Installing Applications");

  ApplicationContainer app;  
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/");
  producerHelper.SetAttribute("PayloadSize", StringValue("1200"));

  Ptr<Node> producer;
//   NODE 6 as producer
//   for (int i = 4; i < 7; i++){
	producer = nodes.Get(6);
	
// 	ndnGlobalRoutingHelper.AddOrigins("/", producer);
	
	app = producerHelper.Install(producer);
	app.Start(Seconds(0));
	std::cout << "Producer: " << producer->GetId() << "... started" << std::endl;
//   }

	
	// For producer
  printFIBmoreData (producer, "FIB_Node6.txt", false);
  printPIT (producer, "PIT_Node6.txt", false);
  printCS (producer, "CS_Node6.txt", false);
  
  
  
//     NODE 2 as producer
//   ndn::AppHelper producerHelper("ns3::ndn::Producer");
//   producerHelper.SetPrefix("/");
//   producerHelper.SetAttribute("PayloadSize", StringValue("1200"));
  
//   producer = nodes.Get(2);
//   ndnGlobalRoutingHelper.AddOrigins("/", producer);
//   app = producerHelper.Install(producer);
//   std::cout << "Producer: " << producer->GetId() << "... started" << std::endl;
//   //   }
//   // For producer
//   printFIBmoreData (producer, "FIB_Node2.txt", false);
//   printPIT (producer, "PIT_Node2.txt", false);
//   printCS (producer, "CS_Node2.txt", false);
  
  
  
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
//   consumerHelper.SetPrefix("/test/prefix");
  consumerHelper.SetAttribute("Frequency", DoubleValue(5.0));
  
//   boost::variant<int, double> variavel;
  
  for (int i = 0; i < 12; i++){
	if (i == 6)
		continue;
	else{
	  switch (i){
		case 0:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}  /*"/prefix/Node_0/"*/
		case 1:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}
		case 2:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}
		case 3:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}
		case 4:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}
		case 5:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}
// 		case 6:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}  
		case 7:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}
		case 8:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}
		case 9:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}
		case 10:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}
		case 11:	{consumerHelper.SetPrefix("/prefix/Node/"); break;}
		default: std::cout << "Unknown value! " << std::endl;	
	}
	// 	consumerHelper.SetPrefix("/test/prefix/node_" + Names::FindName(nodes.Get(i)));
	app = consumerHelper.Install(nodes.Get(i));
	app.Start(Seconds(0.01*i));
	std::cout << "Consumer[" << nodes.Get(i)->GetId() << "] ... started" << std::endl;	
		  
	//   Print Pit
	const bool control = false;// controls for trunc or append, in the tracing function
	switch (i){
		case 0: {			
// 			printFIBmoreData (nodes.Get(i), "FIB_Node0.txt", control);
// 			printPIT (nodes.Get(i), "PIT_Node0.txt", control);
// 			printCS(nodes.Get(i), "CS_Node0.txt", control);
			break;
		}
		case 1: {			
			printFIBmoreData (nodes.Get(i), "FIB_Node1.txt", control);
			printPIT (nodes.Get(i), "PIT_Node1.txt", control);
			printCS(nodes.Get(i), "CS_Node1.txt", control);
			break;
		}
		case 2: {			
			printFIBmoreData (nodes.Get(i), "FIB_Node2.txt", control);
			printPIT (nodes.Get(i), "PIT_Node2.txt", control);
			printCS(nodes.Get(i), "CS_Node2.txt", control);
			break;
		}
		case 3: {// producer now...			
// 			printFIBmoreData (nodes.Get(i), "FIB_Node3.txt", control);
// 			printPIT (nodes.Get(i), "PIT_Node3.txt", control);
// 			printCS(nodes.Get(i), "CS_Node3.txt", control);
			break;
		}
		case 4: {			
			printFIBmoreData (nodes.Get(i), "FIB_Node4.txt", control);
			printPIT (nodes.Get(i), "PIT_Node4.txt", control);
			printCS(nodes.Get(i), "CS_Node4.txt", control);
			break;
		}
		case 5: {			
// 			printFIBmoreData (nodes.Get(i), "FIB_Node5.txt", control);
// 			printPIT (nodes.Get(i), "PIT_Node5.txt", control);
// 			printCS(nodes.Get(i), "CS_Node5.txt", control);
			break;
		}
// 		case 6: {			
// 			// 			cout << "FIB content on node" << nodes.Get(i)->GetId() << endl;
// 			// 			Simulator::Schedule (Seconds (1.0), &printFIBmoreData, nodes.Get(i), "FIB_Node5.txt");
// 			printFIBmoreData (nodes.Get(i), "FIB_Node6.txt", control);
// 			printPIT (nodes.Get(i), "PIT_Node6.txt", control);
// 			printCS(nodes.Get(i), "CS_Node6.txt", control);
// 			break;
// 		}
		case 7: {			
// 			printFIBmoreData (nodes.Get(i), "FIB_Node7.txt", control);
// 			printPIT (nodes.Get(i), "PIT_Node7.txt", control);
// 			printCS(nodes.Get(i), "CS_Node7.txt", control);
			break;
		}
		case 8: {			
			printFIBmoreData (nodes.Get(i), "FIB_Node8.txt", control);
			printPIT (nodes.Get(i), "PIT_Node8.txt", control);
			printCS(nodes.Get(i), "CS_Node8.txt", control);
			break;
		}
		case 9: {			
// 			printFIBmoreData (nodes.Get(i), "FIB_Node9.txt", control);
// 			printPIT (nodes.Get(i), "PIT_Node9.txt", control);
// 			printCS(nodes.Get(i), "CS_Node9.txt", control);
// 			break;
		}
		case 10: {			
// 			printFIBmoreData (nodes.Get(i), "FIB_Node10.txt", control);
// 			printPIT (nodes.Get(i), "PIT_Node10.txt", control);
// 			printCS(nodes.Get(i), "CS_Node10.txt", control);
// 			break;
		}
		case 11: {			
// 			printFIBmoreData (nodes.Get(i), "FIB_Node11.txt", control);
// 			printPIT (nodes.Get(i), "PIT_Node11.txt", control);
// 			printCS(nodes.Get(i), "CS_Node11.txt", control);
			break;
		}
		default: 
			std::cout << "Unknown option! " << std::endl;
		}
	}
  }
  

  // Calculate and install FIBs
//   ndn::GlobalRoutingHelper::CalculateRoutes(); //Just one route!
//   ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes(); //All possible routes... Multipath?
//   ndn::FibHelper::AddRoute(nodes.Get(1), "/prefix3", 257, 1); // link to n1
  
  // Configure callback for logging
//   Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
// 				   MakeBoundCallback (&CourseChange, &os));
  
  Simulator::Stop(Seconds(duration));

  ndn::L3RateTracer::InstallAll("ndn-simple-wifi80211p-7.txt", Seconds(1));
//   AnimationInterface anim("vehicularmobility_20210102.xml");
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
