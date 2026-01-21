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

/* Elidio */
#include "ns3/ndnSIM/model/ndn-net-device-transport.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include "ns3/ndnSIM/model/map-common.hpp"

#include "ns3/netanim-module.h"

// Defined in "ns3/ndnSIM/model/map-common.hpp"
extern std::vector<std::tuple<std::string, int>> NodeIdMacMap;

using namespace std;
namespace ns3 {

// NS_LOG_COMPONENT_DEFINE("ndn.WifiExample");

//
// DISCLAIMER:  Note that this is an extremely simple example, containing just 2 wifi nodes
// communicating directly over AdHoc channel.
//

// Ptr<ndn::NetDeviceFace>
// MyNetDeviceFaceCallback (Ptr<Node> node, Ptr<ndn::L3Protocol> ndn, Ptr<NetDevice> device)
// {
//   // NS_LOG_DEBUG ("Create custom network device " << node->GetId ());
//   Ptr<ndn::NetDeviceFace> face = CreateObject<ndn::MyNetDeviceFace> (node, device);
//   ndn->AddFace (face);
//   return face;
// }

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
//             Ptr<WifiMac> wifinetdevice2 = dynamic_cast<ns3::WifiNetDevice>(transport->GetNetDevice());
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
		(*outputStream) << Simulator::Now ().ToDouble (Time::S) << "\t" << "Content name: " << entry->getName().toUri() << "\t" << "Node Speed: " << *(entry->getNodeSpeed()) << "\t" << "Geo-Position: " << *(entry->getNodePosition()) /*<< "\t" << "Prefix: " << entry->getPrefix()*/ << "\t" << "Direction: " << entry->getNodeDirection() << "\t" << "Timestamp: " << entry->getNodeTimestamp() << std::endl;// getFullName

	(*outputStream) << std::endl;
	Simulator::Schedule (Seconds (5.0), printCS, node, Ffile, control);
}


void showPosition (Ptr<Node> node, double deltaTime, const std::string& Ffile, bool control)
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
			  std::cout << "File " << Ffile << " <Mobility Printing> cannot be opened for writing..." << std::endl;
			  return;
		  }

	  outputStream = os;
	  }
	  else {
		  outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
	  }


  uint32_t nodeId = node->GetId ();
  Ptr<MobilityModel> mobModel = node->GetObject<MobilityModel> ();
  Vector3D pos = mobModel->GetPosition ();
  Vector3D speed = mobModel->GetVelocity ();
  (*outputStream) << "At timestamp (s): " << /*Simulator::Now().GetHours() << "-" << Simulator::Now().GetMinutes() << "-" <<*/ Simulator::Now().GetSeconds() << "\tnode " << nodeId
            << ":\tPosition(" << pos.x << ", " << pos.y << ", " << pos.z
            << ");\tSpeed(" << speed.x << ", " << speed.y << ", " << speed.z
            << ")" << std::endl;

  Simulator::Schedule (Seconds (deltaTime), showPosition, node, deltaTime, Ffile, control);
}

bool
getFileContent(std::string fileName, std::vector<std::tuple<std::string, int>> & NodeIdMacMap)
{
  // Open the File
  std::ifstream inFile(fileName.c_str());
  // Check if object is valid
  if(!inFile)
  {
    std::cerr << "Cannot open the File : " << fileName << std::endl;
    //         return nullptr;
  }
  std::string mac;
  int nodeId;
  int k = 0;

  while (std::getline(inFile, mac, '-') >> nodeId)// copy to mac till find separator, then dump remaining to nodeId
  {
    // Line contains string of length > 0 then save it in vector
    if ((k++))
      mac.erase(mac.begin()); //Remove "\n" from mac, BUT not from the first string

      if(mac.size() > 0)
        NodeIdMacMap.push_back(std::tuple<std::string, int>(mac,nodeId));
  }

//   std::string macToReturn;
//   auto it = std::find_if(NodeIdMacMap.begin(), NodeIdMacMap.end(),
//                          [&in_node](const std::tuple<std::string, int>& NodeIdMacMap)
//                          {return std::get<1>(NodeIdMacMap) == in_node;});
//   if (it != NodeIdMacMap.end()) {
//     macToReturn = std::get<0>(*it);
//     //         std::cout << "Found at address: " << macToReturn << std::endl;
//   }

  inFile.close();
  return true;
}


/**********/

int
main(int argc, char* argv[])
{
  NS_LOG_COMPONENT_DEFINE ("ndn.simple.wifi2");
//   LogComponentEnable("ndn.simple.wifi2", LOG_LEVEL_INFO);

  // disable fragmentation
  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                     StringValue("OfdmRate24Mbps"));

  CommandLine cmd;
  cmd.Parse(argc, argv);

  //////////////////////
  //////////////////////
  //////////////////////
  WifiHelper wifi;
  // wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  wifi.SetStandard(WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
                               StringValue("OfdmRate24Mbps"));

  YansWifiChannelHelper wifiChannel; // = YansWifiChannelHelper::Default ();
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::ThreeLogDistancePropagationLossModel");
  wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");

  // YansWifiPhy wifiPhy = YansWifiPhy::Default();
  YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
  wifiPhyHelper.SetChannel(wifiChannel.Create());
  wifiPhyHelper.Set("TxPowerStart", DoubleValue(5));
  wifiPhyHelper.Set("TxPowerEnd", DoubleValue(5));

  WifiMacHelper wifiMacHelper;
  wifiMacHelper.SetType("ns3::AdhocWifiMac");

  Ptr<UniformRandomVariable> randomizer = CreateObject<UniformRandomVariable>();
  randomizer->SetAttribute("Min", DoubleValue(10));
  randomizer->SetAttribute("Max", DoubleValue(100));

  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator", "X", PointerValue(randomizer),
                                "Y", PointerValue(randomizer), "Z", PointerValue(randomizer));
/****************************************************************************/
//   mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");


//   mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
// 								"MinX", DoubleValue (1.0),
// 								"MinY", DoubleValue (1.0),
// 								"DeltaX", DoubleValue (5.0),
// 								"DeltaY", DoubleValue (5.0),
// 								"GridWidth", UintegerValue (3),
// 								"LayoutType", StringValue ("RowFirst"));
/****************************************************************************/

  // Another model - tested
  mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel",
                              "Bounds", RectangleValue (Rectangle (0, 50, 0, 50)),
                              "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=20]"),
                              "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"));


  /***************************************************************************/






  NodeContainer nodes;
  nodes.Create(7); // one is static... RSU... ?

  ////////////////
  // 1. Install Wifi
  NetDeviceContainer wifiNetDevices = wifi.Install(wifiPhyHelper, wifiMacHelper, nodes);

  // 2. Install Mobility model
  // Using constant velocity model and assign individual velocity for each node
  // Set a constant velocity mobility model

//   mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (nodes);

//   // setup a uniform random variable for the speed
//   Ptr<UniformRandomVariable> rvar = CreateObject<UniformRandomVariable>();
//
//   // for each node set up its speed according to the random variable
//   for (NodeContainer::Iterator iter= nodes.Begin(); iter!=nodes.End(); ++iter){
// 	  Ptr<Node> tmp_node = (*iter);
//       // select the speed from (15,25) m/s
// 	  double speed = rvar->GetValue(15, 25);
// 	  tmp_node->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(Vector(speed, speed, 0));
//   }


  // Populate the Map
  getFileContent("NodeIdMacMapping.txt", NodeIdMacMap);

  // 3. Install NDN stack
  NS_LOG_INFO("Installing NDN stack");
  ndn::StackHelper ndnHelper;
  // ndnHelper.AddNetDeviceFaceCreateCallback (WifiNetDevice::GetTypeId (), MakeCallback
  // (MyNetDeviceFaceCallback));
  ndnHelper.setPolicy("nfd::cs::cromo2");
  ndnHelper.setCsSize(2000);//1000
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.Install(nodes);

  double deltaTime = 5; // Periodicity for printing speed

  // Set BestRoute strategy
//   ndn::StrategyChoiceHelper::Install(nodes, "/", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::Install(nodes, "/", "/localhost/nfd/strategy/mobility-aware");

  // 4. Set up applications
  NS_LOG_INFO("Installing Applications");

//   ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCRoMo2");

  consumerHelper.SetPrefix("/test/prefix/consumer1");
  consumerHelper.SetAttribute("Frequency", DoubleValue(50.0));//100
  auto consumer1 = nodes.Get(0);
  auto appConsumer1 = consumerHelper.Install(consumer1);

  consumerHelper.SetPrefix("/test/prefix/consumer2");
  consumerHelper.SetAttribute("Frequency", DoubleValue(50.0));//100
  auto consumer2 = nodes.Get(2);
  auto appConsumer2 = consumerHelper.Install(consumer2);
  appConsumer2.Stop(Seconds(6.0));

  consumerHelper.SetPrefix("/test/prefix/consumer3");
  consumerHelper.SetAttribute("Frequency", DoubleValue(50.0));//100
  auto consumer3 = nodes.Get(4);
  auto appConsumer3 = consumerHelper.Install(consumer3);
  appConsumer3.Start(Seconds(6.0));

  consumerHelper.SetPrefix("/test/prefix/consumer4");
  consumerHelper.SetAttribute("Frequency", DoubleValue(70.0));//100
  auto consumer4 = nodes.Get(5);
  auto appConsumer4 = consumerHelper.Install(consumer4);
  appConsumer4.Start(Seconds(0.0));

  // For Beacon
  ndn::AppHelper consumerHelper1("ns3::ndn::ConsumerCRoMo2Beacon");
  consumerHelper1.SetPrefix("/beacon/RSU0");
  consumerHelper1.SetAttribute("Frequency", DoubleValue(100.0));//100
  auto RSU0 = nodes.Get(6);
  auto appRSU0 = consumerHelper1.Install(RSU0);
  appRSU0.Start(Seconds(0.0));

//   std::string tmp_dir = "/home/dasilva/PDEEC2021/testingENV/ns-3/testingVM_logs/";
  std::string tmp_dir = "/var/tmp/ns-3/testingVM_logs/";
  const bool control = false;
  //int i = 0;
  printFIBmoreData (consumer1, tmp_dir + "FIB_Consumer1_WiFi.txt", control);
  printPIT (consumer1, tmp_dir + "PIT_Consumer1_WiFi.txt", control);
  printCS(consumer1, tmp_dir +  "CS_Consumer1_WiFi.txt", control);
  showPosition (consumer1, deltaTime, tmp_dir + "Mobility_Consumer1_WiFi.txt", control);

  printFIBmoreData (consumer2, tmp_dir + "FIB_Consumer2_WiFi.txt", control);
  printPIT (consumer2, tmp_dir + "PIT_Consumer2_WiFi.txt", control);
  printCS(consumer2, tmp_dir + "CS_Consumer2_WiFi.txt", control);
  showPosition (consumer2, deltaTime, tmp_dir + "Mobility_Consumer2_WiFi.txt", control);

  printFIBmoreData (consumer3, tmp_dir + "FIB_Consumer3_WiFi.txt", control);
  printPIT (consumer3, tmp_dir + "PIT_Consumer3_WiFi.txt", control);
  printCS(consumer3, tmp_dir + "CS_Consumer3_WiFi.txt", control);
  showPosition (consumer3, deltaTime, tmp_dir + "Mobility_Consumer3_WiFi.txt", control);

  printFIBmoreData (consumer4, tmp_dir + "FIB_Consumer4_WiFi.txt", control);
  printPIT (consumer4, tmp_dir + "PIT_Consumer4_WiFi.txt", control);
  printCS(consumer4, tmp_dir + "CS_Consumer4_WiFi.txt", control);
  showPosition (consumer4, deltaTime, tmp_dir + "Mobility_Consumer4_WiFi.txt", control);

  printFIBmoreData (RSU0, tmp_dir + "FIB_RSU0_WiFi.txt", control);
  printPIT (RSU0, tmp_dir + "PIT_RSU0_WiFi.txt", control);
  printCS(RSU0, tmp_dir + "CS_RSU0_WiFi.txt", control);
  showPosition (RSU0, deltaTime, tmp_dir + "Mobility_RSU0_WiFi.txt", control);


// Producer
  ndn::AppHelper producerHelper("ns3::ndn::ProducerCRoMo2");
  producerHelper.SetPrefix("/test");
  producerHelper.SetAttribute("PayloadSize", StringValue("1200"));

  auto producer1 = nodes.Get(1);
  auto appProducer1 = producerHelper.Install(producer1);
  appProducer1.Stop(Seconds(10.0));

  // A producer for push-based emergency messages ...  and for responding Beacons
  ndn::AppHelper producerHelper1("ns3::ndn::ProducerCRoMo2Emergency");
  producerHelper1.SetPrefix("/push-message");
  producerHelper1.SetAttribute("PayloadSize", StringValue("1200"));

  auto producer2 = nodes.Get(3);
  auto appProducer2 = producerHelper1.Install(producer2);
  appProducer2.Start(Seconds(0.0));

  // ...  and for responding Beacons
  ndn::AppHelper producerHelper2("ns3::ndn::ProducerCRoMo2Beacon");
  producerHelper2.SetPrefix("/beacon");
  producerHelper2.SetAttribute("PayloadSize", StringValue("1200"));

  auto producer3 = nodes.Get(3);
  auto appProducer3 = producerHelper2.Install(producer3);
  appProducer3.Start(Seconds(0.0));

  // We're installing a consumer at node nodes.Get(3) which is also a producer. This node will force this node to produce push-based emergency messages with the prefix /push-based/info-type/sender-ID/sender-geo-coordinates/
  ndn::AppHelper consumerHelper2("ns3::ndn::ConsumerCRoMo2Emergency");
  consumerHelper2.SetPrefix("/push-message/active-safety");
  consumerHelper2.SetAttribute("Frequency", DoubleValue(50.0));//100
  auto consumer5 = nodes.Get(3);
  auto appConsumer5 = consumerHelper2.Install(consumer5);
  appConsumer5.Start(Seconds(0.0));

  printFIBmoreData (consumer5, tmp_dir + "FIB_Consumer5_WiFi.txt", control);
  printPIT (consumer5, tmp_dir + "PIT_Consumer5_WiFi.txt", control);
  printCS(consumer5, tmp_dir + "CS_Consumer5_WiFi.txt", control);
  showPosition (consumer5, deltaTime, tmp_dir + "Mobility_Consumer5_WiFi.txt", control);



  // For producer (s)
  printFIBmoreData (producer1, tmp_dir + "FIB_Producer1_WiFi.txt", false);
  printPIT (producer1, tmp_dir + "PIT_Producer1_WiFi.txt", false);
  printCS (producer1, tmp_dir + "CS_Producer1_WiFi.txt", false);
  showPosition (producer1, deltaTime, tmp_dir + "Mobility_Producer1_WiFi.txt", false);

  printFIBmoreData (producer2, tmp_dir + "FIB_Producer2_WiFi.txt", false);
  printPIT (producer2, tmp_dir + "PIT_Producer2_WiFi.txt", false);
  printCS (producer2, tmp_dir + "CS_Producer2_WiFi.txt", false);
  showPosition (producer2, deltaTime, tmp_dir + "Mobility_Producer2_WiFi.txt", false);

  printFIBmoreData (producer3, tmp_dir + "FIB_Producer3_WiFi.txt", false);
  printPIT (producer3, tmp_dir + "PIT_Producer3_WiFi.txt", false);
  printCS (producer3, tmp_dir + "CS_Producer3_WiFi.txt", false);
  showPosition (producer3, deltaTime, tmp_dir + "Mobility_Producer3_WiFi.txt", false);

  /**************/
//   double deltaTime = 2;
//   Simulator::Schedule (Seconds (0.0), &showPosition, producer, deltaTime); // Schedule to print node velocity status
  ////////////////

  Simulator::Stop(Seconds(30.0));

//   ndn::L3AggregateTracer::InstallAll ("aggregate-ndn-simple-wifi.txt", Seconds (2));
  ndn::L3RateTracer::InstallAll (tmp_dir + "rate-ndn-simple-wifi.txt", Seconds (5));
  AnimationInterface anim(tmp_dir + "vehicularmobility_20220817.xml");


//   for (uint32_t i=0; i<wifiSta ; i++) {
//       string str = "simple-wifi-trace" + std::to_string(i+1) + ".txt";
//       ndn::L3RateTracer::Install(consumers.Get(i), str, Seconds(endtime-0.5));
//   }


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
