//  To RUN
// NS_LOG=ndn.Consumer:ndn.Producer LD_LIBRARY_PATH=/home/dasilva/PDEEC2021/boost_1_74_0/lib ./waf --run "ndn-simple-wifi3-WithBeacon_20220817 --traceFile=/home/dasilva/mobility_20220831_2.tcl --nodeNum=15 --duration=350.00 --logFile=ns2-mobility-trace_20220831.log"


// export LD_LIBRARY_PATH="/home/dasilva/PDEEC2021/boost_1_74_0/lib:/usr/local/lib64"
// ./build/src/ndnSIM/examples/ns3-dev-ndn-simple-wifi3-WithBeacon_20220819-debug --traceFile=/home/dasilva/mobility_20220831_2.tcl --nodeNum=15 --duration=350.00 --logFile=ns2-mobility-trace_20220831.log --command-template=/home/dasilva/PDEEC2021/testingENV/ns-3/gdbString.txt




// Without NS_LOG
// LD_LIBRARY_PATH=/home/dasilva/PDEEC2021/boost_1_74_0/lib ./waf --run "ndn-simple-wifi3-WithBeacon_20220819 --traceFile=/home/dasilva/mobility_20220831_2.tcl --nodeNum=15 --duration=350.00 --logFile=ns2-mobility-trace_20220831.log --command-template=/home/dasilva/PDEEC2021/testingENV/ns-3/gdbString.txt" 2> log


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
#include "ns3/ns2-mobility-helper.h"
#include "face/face.hpp"

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

#include "boost/variant.hpp"

// Defined in "ns3/ndnSIM/model/map-common.hpp"
extern std::vector<std::tuple<std::string, int>> NodeIdMacMap;

using namespace std;
namespace ns3 {

// NS_LOG_COMPONENT_DEFINE("ndn.WifiExample");

//
// DISCLAIMER:  Note that this is an extremely simple example, containing just 2 wifi nodes
// communicating directly over AdHoc channel.
//


std::tuple<bool, shared_ptr<std::ostream>>
printToFile(const std::string& Ffile, bool control){
    shared_ptr<std::ostream> outputStream;

    /*
     * control == false -> return false ---> No printing
     * control == true && file == '-' Or file open == false --> std::cout
     * control == true && file open == true --> print to file
     */
    // static uint64_t x = 0;
    // bool trunc_app = false;

    if (control == false){
      outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
      return make_tuple(false, outputStream); // The calling function/method should know NO printing requested
    }
    else{
      if (Ffile != "-") { // outputs to file
        shared_ptr<std::ofstream> os(new std::ofstream());
        // if ((x == 0) && (trunc_app == false)){
        //   os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::trunc);
        //   trunc_app = true;
        //   x++;
        // }
        // else
        if (os->is_open()){
          os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::app);
          outputStream = os;
          return make_tuple(true, outputStream); // To file
        }
        else /*(!os->is_open())*/ {// error
          std::cout << "File " << Ffile << " Cannot be opened for writing..." << std::endl;
          outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
          return make_tuple(false, outputStream);
        }
      }
      else {// The calling function/method should know that std::cout MUST be used.
        outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
        return make_tuple(false, outputStream);
      }
  }
}

void
printPIT(Ptr<Node> node, const std::string& Ffile, bool control)
{
	// shared_ptr<std::ostream> outputStream;
	// if (Ffile != "-") {
	// 	shared_ptr<std::ofstream> os(new std::ofstream());
	// 	if (control == false){
	// 		os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::trunc);
	// 		control = true;
	// 	}
	// 	else if (control == true)
	// 		os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::app);
 //
	// 	if (!os->is_open()) {
	// 		std::cout << "File " << Ffile << " <PIT> cannot be opened for writing..." << std::endl;
	// 		return;
	// 	}
 //
	// 	outputStream = os;
	// }
	// else {
	// 	outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
	// }

  shared_ptr<std::ostream> outputStream;
  bool valid = false;
  std::tie(valid, outputStream) = printToFile(Ffile, control);

// 	shared_ptr<nfd::pit::Entry> pitEntry;
	const nfd::Pit& pit = node->GetObject<ndn::L3Protocol>()->getForwarder()->getPit();
	for (nfd::Pit::const_iterator entry = pit.begin(); entry != pit.end(); entry++){
		auto interest_name = entry->getInterest().getName();//OR entry->getName();
		if (interest_name.getSubName(0,1) != "/localhost"){
      // std::cout << interest_name.getSubName(0,1) << std::endl;
      (control && valid) ? std::cout : (*outputStream) << Simulator::Now ().ToDouble (Time::S) << "\t"
      << interest_name << "\n" /*entry->getName()*/ /*<< "\t"
      << entry->GetId()*/ /*<< std::endl*/;
      // remember pending downstreams
  // 		auto pitEntry = entry;
      for (const nfd::pit::InRecord& inRecord : entry->getInRecords()) {
        (control && valid) ? std::cout : (*outputStream) << "\t\t inRecord Face: " << inRecord.getFace() << std::endl;
  // 			(*outputStream) << "\n\t inRecord Face: " << inRecord.getFace().getLocalUri() << "\n";
              (control && valid) ? std::cout : (*outputStream) << "\t\t inRecord fromId: " << inRecord.getEndpointId() << std::endl;
      }
  // 		for (const nfd::pit::InRecord& inRecord : entry->getInRecords()) {
  // 			(*outputStream) << "\n\t inRecord endpointID: " << inRecord.getEndpointId() << std::endl;
  // 			// 			(*outputStream) << "\n\t inRecord Face: " << inRecord.getFace().getLocalUri() << "\n";
  // 		}
      for (const nfd::pit::OutRecord& outRecord : entry->getOutRecords()) {
        (control && valid) ? std::cout : (*outputStream) << "\n\t\t outRecord Face: " << outRecord.getFace() << std::endl;
        // 			(*outputStream) << "\t outRecord Face: " << outRecord.getFace().getLocalUri() << std::endl;//getRemoteUri()
      }
      (control && valid) ? std::cout : (*outputStream) << std::endl;
    }
  }


	(control && valid) ? std::cout : (*outputStream) << std::endl;
	Simulator::Schedule (Seconds (5.0), printPIT, node, Ffile, control);
}

//  print FIB
void
// printFIBmoreData = [](Ptr<Node> node, Time nextTime)
printFIBmoreData(Ptr<Node> node, const std::string& Ffile, bool control)
{
	// shared_ptr<std::ostream> outputStream;
	// if (Ffile != "-") {
	// 	shared_ptr<std::ofstream> os(new std::ofstream());
	// 	if (control == false){
	// 		os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::trunc);
	// 		control = true;
	// 	}
	// 	else if (control == true)
	// 		os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::app);
 //
	// 	if (!os->is_open()) {
	// 		std::cout << "File " << Ffile << " <FIB> cannot be opened for writing..." << std::endl;
	// 		return;
	// 	}
 //
	// 	outputStream = os;
	// }
	// else {
	// 	outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
	// }

    shared_ptr<std::ostream> outputStream;
    bool valid = false;
    std::tie(valid, outputStream) = printToFile(Ffile, control);

	// 	Only in the simulation: Get direct access to Fib instance on the node, during the simulation
	const nfd::Fib& fib = node->GetObject<ndn::L3Protocol>()->getForwarder()->getFib();
	// 	(control && valid) ? std::cout : (*outputStream) << "FIB content on node" << node->GetId() << endl;
	for (nfd::Fib::const_iterator entry = fib.begin(); entry != fib.end(); entry++){
		(control && valid) ? std::cout : (*outputStream) << Simulator::Now ().ToDouble (Time::S) << "\t"
		/*(control && valid) ? std::cout : (*outputStream)*/ << entry->getPrefix() << " - (";
		bool isFirst = true;
		for (auto& nextHop : entry->getNextHops()) {
			auto& face = nextHop.getFace();
			auto endpointID = nextHop.getEndpointId();
            auto Cost = nextHop.getCost();
			(control && valid) ? std::cout : (*outputStream) << endpointID << " - "/*<< " Face: " */<< face /*<< " Face ID: " << nextHop.getFace().getId()*/ << " - " << Cost; // Cost, new element added

// 			std::cout << "\n" << face.endpoint << "\n";
			auto transport = dynamic_cast<ndn::NetDeviceTransport*>(face.getTransport());
//             Ptr<WifiMac> wifinetdevice2 = dynamic_cast<ns3::WifiNetDevice>(transport->GetNetDevice());
			if (transport == nullptr)
				continue;
			(control && valid) ? std::cout : (*outputStream) << " towards ";
// 			if (!isFirst)
// 				valid ? std::cout : (*outputStream) << ", ";
			(control && valid) ? std::cout : (*outputStream) <<  " Node ID " << transport->GetNetDevice()->GetChannel()->GetDevice(0)->GetNode()->GetId() << " with Address: " << transport->GetNetDevice()->GetChannel()->GetDevice(0)->GetAddress();  /*<< "\n\t";*/

// 			if (!isFirst)
// 				(control && valid) ? std::cout : (*outputStream) << ", ";

			isFirst = false;
		}
		(control && valid) ? std::cout : (*outputStream) << ")" << std::endl;
	}



  // 		std:: cout << entry->getPrefix() << std::endl;
  (control && valid) ? std::cout : (*outputStream) << std::endl;

Simulator::Schedule (Seconds (5.0), printFIBmoreData, node, Ffile, control);
// 	std::cout << "Does it come back here? " << endl;
}

//  print CS
void
printCS(Ptr<Node> node, const std::string& Ffile, bool control)
{
	// shared_ptr<std::ostream> outputStream;
	// if (Ffile != "-") {
	// 	shared_ptr<std::ofstream> os(new std::ofstream());
	// 	if (control == false){
	// 		os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::trunc);
	// 		control = true;
	// 	}
	// 	else if (control == true)
	// 		os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::app);
 //
	// 	if (!os->is_open()) {
	// 		std::cout << "File " << Ffile << " <CS> cannot be opened for writing..." << std::endl;
	// 		return;
	// 	}
 //
	// 	outputStream = os;
	// }
	// else {
	// 	outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
	// }

    shared_ptr<std::ostream> outputStream;
    bool valid = false;
    std::tie(valid, outputStream) = printToFile(Ffile, control);

    const nfd::Cs& cs = node->GetObject<ndn::L3Protocol>()->getForwarder()->getCs();
	(control && valid) ? std::cout : (*outputStream) << "Number of stored contents: " << cs.size() << std::endl;// getFullName
	for (nfd::Cs::const_iterator entry = cs.begin(); entry != cs.end(); entry++)
		(control && valid) ? std::cout : (*outputStream) << Simulator::Now ().ToDouble (Time::S) << "\t" << "Content name: " << entry->getName().toUri() << "\t" << "Node Speed: " << entry->getNodeSpeed() << "\t" << "Geo-Position: " << entry->getNodePosition() /*<< "\t" << "Prefix: " << entry->getPrefix()*/ << "\t" << "Direction: " << entry->getNodeDirection() << "\t" << "Timestamp: " << entry->getNodeTimestamp() << std::endl;// getFullName

	(control && valid) ? std::cout : (*outputStream) << std::endl;
	Simulator::Schedule (Seconds (5.0), printCS, node, Ffile, control);
}


void showPosition (Ptr<Node> node, double deltaTime, const std::string& Ffile, bool control)
{
  // shared_ptr<std::ostream> outputStream;
  // if (Ffile != "-") {
	 //  shared_ptr<std::ofstream> os(new std::ofstream());
	 //  if (control == false){
		//   os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::trunc);
		//   control = true;
		// }
	 //  else if (control == true)
	 //      os->open(Ffile.c_str(), std::ios_base::out | std::ios_base::app);
  //
		//   if (!os->is_open()) {
		// 	  std::cout << "File " << Ffile << " <Mobility Printing> cannot be opened for writing..." << std::endl;
		// 	  return;
		//   }
  //
	 //  outputStream = os;
	 //  }
	 //  else {
		//   outputStream = shared_ptr<std::ostream>(&std::cout, std::bind([]{}));
	 //  }

  shared_ptr<std::ostream> outputStream;
  bool valid = false;
  std::tie(valid, outputStream) = printToFile(Ffile, control);

  uint32_t nodeId = node->GetId ();
  Ptr<MobilityModel> mobModel = node->GetObject<MobilityModel> ();
  Vector3D pos = mobModel->GetPosition ();
  Vector3D speed = mobModel->GetVelocity ();
//   (control && valid) ? std::cout : (*outputStream) << "At timestamp (s): " << /*Simulator::Now().GetHours() << "-" << Simulator::Now().GetMinutes() << "-" <<*/ Simulator::Now().GetSeconds() << "\tnode " << nodeId
//             << ":\tPosition(" << pos.x << ", " << pos.y << ", " << pos.z
//             << ");\tSpeed(" << speed.x << ", " << speed.y << ", " << speed.z
//             << ")" << std::endl;

  // For Kalman Filter
  (control && valid) ? std::cout : (*outputStream) << pos.x << " " <<  pos.y << " " << Simulator::Now().GetSeconds()  << " "  <<  pos.x << " " <<  pos.y << " " << speed.x << " " << speed.y << std::endl;//Simulator::Now().GetMilliSeconds()


  Simulator::Schedule (Seconds (deltaTime), showPosition, node, deltaTime, Ffile, control);
}

bool
getFileContent(std::string fileName, std::vector<std::tuple<std::string, int>> & NodeIdMacMap)//File is used everywhere to manage nodeId
{
  // Open the File
  std::ifstream inFile(fileName.c_str());
  // Check if object is valid
  if(!inFile)
  {
    std::cerr << "[scenario]. Cannot open the File : " << fileName << std::endl;
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
//   Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue("OfdmRate24Mbps"));

 Wifi80211pHelper wifi80211p;// = Wifi80211pHelper::Default (); // Do not load Default (avoid "NonUnicastMode", StringValue ...)
	bool verbose = false; // Turn on (true) or off (false) all Wifi 802.11p logging
	if (verbose)
	{
		wifi80211p.EnableLogComponents ();
	}

	std::string phyMode ("OfdmRate12MbpsBW10MHz");
	wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
										"DataMode",StringValue (phyMode),
										"ControlMode",StringValue (phyMode));


    YansWifiChannelHelper wifiChannel;// = YansWifiChannelHelper::Default ();// Loads defaul but then it configures what's necessary
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

    // wifiChannel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel");
    // wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");

    // wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel",
    //                                 "MaxRange", DoubleValue (250.0));//
    wifiChannel.AddPropagationLoss("ns3::TwoRayGroundPropagationLossModel",
                                   "SystemLoss", DoubleValue(1), /*L = 1*/
                                   "Frequency", DoubleValue(5.89e9),
                                   "HeightAboveZ", DoubleValue(1.0));



    YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();// Loads defaul but then it configures what's necessary

    //used: https://github.com/CSVNetLab/PA-GPSR/blob/master/examples/pagpsr-main.cc

    // https://groups.google.com/g/ns-3-users/c/zjMmq7y6Ab0   25dBm = 250m? 30dBm = 550m?
    // For range near 250m
    // https://groups.google.com/g/ns-3-users/c/Q1D_2Ode8Dc/m/6EvUt7jcBQAJ
    // https://github.com/cjamadagni/Wormhole-Attack-ns-3/blob/master/wormhole2.cc


    wifiPhyHelper.Set ("TxPowerStart", DoubleValue(12));//33 - 16.7?
    wifiPhyHelper.Set ("TxPowerEnd", DoubleValue(12));
    wifiPhyHelper.Set ("TxPowerLevels", UintegerValue(1));
    wifiPhyHelper.Set ("TxGain", DoubleValue(1));//12? 2 -> 355m
    wifiPhyHelper.Set ("RxGain", DoubleValue(1));//12? 2 -> 355m

    // wifiPhyHelper.Set ("EnergyDetectionThreshold", DoubleValue(-61.8));//-61.8 - 96
    // wifiPhyHelper.Set ("CcaMode1Threshold", DoubleValue(-64.8));//-64.8 - 99


	// ns-3 supports generate a pcap trace
	wifiPhyHelper.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
	wifiPhyHelper.SetChannel(wifiChannel.Create());

    NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default();

	std::string traceFile;
	std::string logFile;
    std::string tmp_dir;
    // std::string gdbString;

	int    nodeNum;
	double duration;

	CommandLine cmd;
    cmd.AddValue ("tmp_dir", "Temporary directory name", tmp_dir);
	cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
	cmd.AddValue ("nodeNum", "Number of nodes", nodeNum);
	cmd.AddValue ("duration", "Duration of Simulation", duration);
	cmd.AddValue ("logFile", "Log file", logFile);
    // cmd.AddValue ("command-template", "gdb", gdbString);//--command-template="gdb --args %s <args>"
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

 // NodeContainer rsu;
 // rsu.Create(2); //2 RSU

  ////////////////
  // 1. Install Wifi
  NetDeviceContainer wifiNetDevices = wifi80211p.Install(wifiPhyHelper, wifi80211pMac, nodes);

  // 2. Install Mobility model
  // Using constant velocity model and assign individual velocity for each node
  // Set a constant velocity mobility model

//   mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
//   mobility.Install (nodes);
  ns2.Install ();

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


  // Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  // positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  // positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  // mobility.SetPositionAllocator (positionAlloc);
  // mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // mobility.Install (c);//nodes


  // https://github.com/ms-van3t-devs/ms-van3t/blob/master/src/automotive/examples/v2i-areaSpeedAdvisor-80211p.cc
  // MobilityHelper mobility;
  // mobility.Install (rsu);
  /* Set the RSU to a fixed position (i.e. on the center of the map, in this case) */
  // Ptr<MobilityModel> mobilityRSU = rsu.Get (0)->GetObject<MobilityModel> ();
  // mobilityRSU->SetPosition (Vector (0, 0, 20.0)); // Normally, in SUMO, (0,0) is the center of the map


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
  ndn::StrategyChoiceHelper::Install(nodes, "/cromo", "/localhost/nfd/strategy/mobility-aware");

  // 4. Set up applications
  NS_LOG_INFO("Installing Applications");

    // STMP on all nodes
//   ndn::AppHelper app1("StmpApp");
//   app1.Install(nodes);


//   ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCRoMo2");

  Ptr<UniformRandomVariable> randomNum = CreateObject<UniformRandomVariable> ();
  // std::cout << "Generated random nr: " << (uint32_t) randomNum->GetValue(0, nodeNum) << std::endl;

  uint32_t selectedProducer = (uint32_t) randomNum->GetValue(0, nodeNum);
  uint32_t selectedProducerBeacon = (uint32_t) randomNum->GetValue(0, nodeNum);
  uint32_t selectedConsumer = (uint32_t) randomNum->GetValue(0, nodeNum);

  while ((selectedProducer == selectedConsumer)
                || (selectedProducerBeacon == selectedConsumer)
                || (selectedProducer == selectedProducerBeacon)
                || (nodes.Get(7)->GetId() == selectedProducer)
                || (nodes.Get(7)->GetId() == selectedConsumer)
                || (nodes.Get(7)->GetId() == selectedProducerBeacon))
  {// nodes.Get(7) <= RSU
    std::cout << "Re-generating Consumer Id... " << std::endl;
    selectedConsumer = (uint32_t) randomNum->GetValue(0, nodeNum);
    selectedProducer = (uint32_t) randomNum->GetValue(0, nodeNum);
    selectedProducerBeacon = (uint32_t) randomNum->GetValue(0, nodeNum);
  }
  std::cout << "Selected Consumer: Node_" << selectedConsumer <<" Selected Producer: Node_" << selectedProducer <<" Selected ProducerBeacon: Node_" << selectedProducerBeacon << std::endl;

  consumerHelper.SetPrefix("/cromo/test/prefix/consumer_"+std::to_string(nodes.Get(selectedConsumer)->GetId()));
  consumerHelper.SetAttribute("Frequency", DoubleValue(50.0));//100
  auto consumer1 = nodes.Get(selectedConsumer);
  auto appConsumer1 = consumerHelper.Install(consumer1);
  appConsumer1.Start(Seconds(0.0));

  // consumerHelper.SetPrefix("/cromo/test/prefix/consumer_"+std::to_string(nodes.Get(30)->GetId()));
  // consumerHelper.SetAttribute("Frequency", DoubleValue(50.0));//100
  // auto consumer2 = nodes.Get(30);
  // auto appConsumer2 = consumerHelper.Install(consumer2);
  // appConsumer2.Start(Seconds(6.0));

  // consumerHelper.SetPrefix("/cromo/test/prefix/consumer_"+std::to_string(nodes.Get(4)->GetId()));
  // consumerHelper.SetAttribute("Frequency", DoubleValue(50.0));//100
  // auto consumer3 = nodes.Get(4);
  // auto appConsumer3 = consumerHelper.Install(consumer3);
  // appConsumer3.Start(Seconds(0.0));

  // consumerHelper.SetPrefix("/cromo/test/prefix/consumer_"+std::to_string(nodes.Get(5)->GetId()));
  // consumerHelper.SetAttribute("Frequency", DoubleValue(70.0));//100
  // auto consumer4 = nodes.Get(5);
  // auto appConsumer4 = consumerHelper.Install(consumer4);
  // appConsumer4.Start(Seconds(0.0));

  // For Beacon
  ndn::AppHelper consumerHelper1("ns3::ndn::ConsumerCRoMo2Beacon");
  consumerHelper1.SetPrefix("/cromo/beacon/RSU_"+std::to_string(nodes.Get(7)->GetId()));
  consumerHelper1.SetAttribute("Frequency", DoubleValue(100.0));//100
  auto RSU0 = nodes.Get(7);// Instead of 6!
  auto appRSU0 = consumerHelper1.Install(RSU0);
  appRSU0.Start(Seconds(0.0));

//   std::string tmp_dir = "/home/dasilva/PDEEC2021/testingENV/ns-3/testingVM_logs/";
  // std::string tmp_dir = "/var/tmp/ns-3/testingVM_logs/";
  const bool control = true;
  //int i = 0;
  printFIBmoreData (nodes.Get(0), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(0)->GetId()) + ".txt", control);
  printPIT (nodes.Get(0), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(0)->GetId()) + ".txt", control);
  printCS(nodes.Get(0), tmp_dir +  "CS_Node_" + std::to_string(nodes.Get(0)->GetId()) + ".txt", control);
  showPosition (nodes.Get(0), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(nodes.Get(0)->GetId()) + ".txt", control);



  // Intermediate nodes
  printFIBmoreData (nodes.Get(2), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(2)->GetId()) + ".txt", control);
  printPIT (nodes.Get(2), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(2)->GetId()) + ".txt", control);
  printCS(nodes.Get(2), tmp_dir + "CS_Node_" + std::to_string(nodes.Get(2)->GetId()) + ".txt", control);
  showPosition (nodes.Get(2), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(nodes.Get(2)->GetId()) + ".txt", control);

  printFIBmoreData (nodes.Get(4), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(4)->GetId()) + ".txt", control);
  printPIT (nodes.Get(4), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(4)->GetId()) + ".txt", control);
  printCS(nodes.Get(4), tmp_dir + "CS_Node_" + std::to_string(nodes.Get(4)->GetId()) + ".txt", control);
  showPosition (nodes.Get(4), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(nodes.Get(4)->GetId()) + ".txt", control);

  printFIBmoreData (nodes.Get(5), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(5)->GetId()) + ".txt", control);
  printPIT (nodes.Get(5), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(5)->GetId()) + ".txt", control);
  printCS(nodes.Get(5), tmp_dir + "CS_Node_" + std::to_string(nodes.Get(5)->GetId()) + ".txt", control);
  showPosition (nodes.Get(5), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(nodes.Get(5)->GetId()) + ".txt", control);

  printFIBmoreData (nodes.Get(7), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(7)->GetId()) + ".txt", control);
  printPIT (nodes.Get(7), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(7)->GetId()) + ".txt", control);
  printCS(nodes.Get(7), tmp_dir + "CS_Node_" + std::to_string(nodes.Get(7)->GetId()) + ".txt", control);
  showPosition (nodes.Get(7), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(nodes.Get(7)->GetId()) + ".txt", control);


// Producer
  ndn::AppHelper producerHelper("ns3::ndn::ProducerCRoMo2");
  producerHelper.SetPrefix("/cromo/test");
  producerHelper.SetAttribute("PayloadSize", StringValue("1200"));

  auto producer1 = nodes.Get(selectedProducer);
  auto appProducer1 = producerHelper.Install(producer1);
  appProducer1.Start(Seconds(0.0));

  // A producer for push-based emergency messages ...  and for responding Beacons

  // // ndn::AppHelper producerHelper1("ns3::ndn::ProducerCRoMo2Emergency");
  // // producerHelper1.SetPrefix("/cromo/push-message");
  // // producerHelper1.SetAttribute("PayloadSize", StringValue("1200"));
  // //
  // // auto producer2 = nodes.Get(3);
  // // auto appProducer2 = producerHelper1.Install(producer2);
  // // appProducer2.Start(Seconds(0.0));
  // //
  // ...  and for responding Beacons
  ndn::AppHelper producerHelper2("ns3::ndn::ProducerCRoMo2Beacon");
  producerHelper2.SetPrefix("/cromo/beacon");
  producerHelper2.SetAttribute("PayloadSize", StringValue("1200"));

  // auto producer3 = nodes.Get(9);// 3 to 9
  auto appProducer3 = producerHelper2.Install(nodes.Get(selectedProducerBeacon));// selectedProducerBeacon
  appProducer3.Start(Seconds(0.0));




  // We're installing a consumer at node nodes.Get(3) which is also a producer. This node will force this node to produce push-based emergency messages with the prefix /push-based/info-type/sender-ID/sender-geo-coordinates/

  // ndn::AppHelper consumerHelper2("ns3::ndn::ConsumerCRoMo2Emergency");
  // consumerHelper2.SetPrefix("/cromo/push-message/active-safety");
  // consumerHelper2.SetAttribute("Frequency", DoubleValue(50.0));//100
  // auto consumer5 = nodes.Get(11); // 3 to 12
  // auto appConsumer5 = consumerHelper2.Install(consumer5);
  // appConsumer5.Start(Seconds(0.0));

  printFIBmoreData (nodes.Get(11), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(11)->GetId()) + ".txt", control);
  printPIT (nodes.Get(11), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(11)->GetId()) + ".txt", control);
  printCS(nodes.Get(11), tmp_dir + "CS_Node_" + std::to_string(nodes.Get(11)->GetId()) + ".txt", control);
  showPosition (nodes.Get(11), deltaTime, tmp_dir + "Mobility_Node_"+std::to_string(nodes.Get(11)->GetId()) + ".txt", control);


  // For producer (s)
  printFIBmoreData (nodes.Get(1), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(1)->GetId()) + ".txt", false);
  printPIT (nodes.Get(1), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(1)->GetId()) + ".txt", false);
  printCS (nodes.Get(1), tmp_dir + "CS_Node_" + std::to_string(nodes.Get(1)->GetId()) + ".txt", false);
  showPosition (nodes.Get(1), deltaTime, tmp_dir + "Mobility_Node_"+std::to_string(nodes.Get(1)->GetId()) + ".txt", false);

  // NOT TO BE EXTRACTED... 2 APPS INSTALLED
  printFIBmoreData (nodes.Get(3), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(3)->GetId()) + ".txt", false);
  printPIT (nodes.Get(3), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(3)->GetId()) + ".txt", false);
  printCS (nodes.Get(3), tmp_dir + "CS_Node_" + std::to_string(nodes.Get(3)->GetId()) + ".txt", false);
  showPosition (nodes.Get(3), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(nodes.Get(3)->GetId()) + ".txt", false);

  printFIBmoreData (nodes.Get(9), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(9)->GetId()) + ".txt", false);
  printPIT (nodes.Get(9), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(9)->GetId()) + ".txt", false);
  printCS (nodes.Get(9), tmp_dir + "CS_Node_" + std::to_string(nodes.Get(9)->GetId()) + ".txt", false);
  showPosition (nodes.Get(9), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(nodes.Get(9)->GetId()) + ".txt", false);


  /**** Random router(s) *****/

  printFIBmoreData (nodes.Get(10), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(10)->GetId()) + ".txt", false);
  printPIT (nodes.Get(10), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(10)->GetId()) + ".txt", false);
  printCS (nodes.Get(10), tmp_dir + "CS_Node_" + std::to_string(nodes.Get(10)->GetId()) + ".txt", false);
  showPosition (nodes.Get(10), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(nodes.Get(10)->GetId()) + ".txt", false);// 3 seconds

  printFIBmoreData (nodes.Get(29), tmp_dir + "FIB_Node_" + std::to_string(nodes.Get(29)->GetId()) + ".txt", false);
  printPIT (nodes.Get(29), tmp_dir + "PIT_Node_" + std::to_string(nodes.Get(29)->GetId()) + ".txt", false);
  printCS (nodes.Get(29), tmp_dir + "CS_Node_" + std::to_string(nodes.Get(29)->GetId()) + ".txt", false);
  showPosition (nodes.Get(29), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(nodes.Get(29)->GetId()) + ".txt", false);// 3 seconds


  /**************/
//   double deltaTime = 2;
//   Simulator::Schedule (Seconds (0.0), &showPosition, producer, deltaTime); // Schedule to print node velocity status
  ////////////////

  Simulator::Stop(Seconds(duration));

  ndn::AppDelayTracer::InstallAll(tmp_dir + "delay-ndn-simple-wifi3-WithBeacon_20230109.txt");
//   ndn::L3AggregateTracer::InstallAll ("aggregate-ndn-simple-wifi.txt", Seconds (2));
  ndn::L3RateTracer::InstallAll (tmp_dir + "rate-ndn-simple-wifi3-WithBeacon_20230109.txt", Seconds (5));
  // AnimationInterface anim(tmp_dir + "vehicularmobility_ndn-simple-wifi3-WithBeacon_20221212.xml");


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
