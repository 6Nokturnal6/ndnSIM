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

//cout setprecision stuff
#include <iomanip>

#include <cmath>

// Defined in "ns3/ndnSIM/model/map-common.hpp"
extern std::vector<std::tuple<std::string, int>> NodeIdMacMap;
std::string tmp_dir;

bool printToFileFIB = false;
bool printToFilePIT = false;
bool printToFileCS = false;
bool printToFilePosition = false;
bool printToFileAll = true;//false;

using namespace std;
namespace ns3 {

// NS_LOG_COMPONENT_DEFINE("ndn.WifiExample");

//
// DISCLAIMER:  Note that this is an extremely simple example, containing just 2 wifi vehicles
// communicating directly over AdHoc channel.
//


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
		if (interest_name.getSubName(0, 1) != "/localhost"){
      // std::cout << interest_name.getSubName(0,1) << std::endl;
      (*outputStream) << Simulator::Now ().ToDouble (Time::S) << "\t"
      << interest_name << "\n" /*entry->getName()*/ /*<< "\t"
      << entry->GetId()*/ /*<< std::endl*/;
      // remember pending downstreams
  // 		auto pitEntry = entry;
      for (const nfd::pit::InRecord& inRecord : entry->getInRecords()) {
        (*outputStream) << "\t\t inRecord Face: " << inRecord.getFace() << std::endl;
  // 			(*outputStream) << "\n\t inRecord Face: " << inRecord.getFace().getLocalUri() << "\n";
              (*outputStream) << "\t\t inRecord fromId: " << inRecord.getEndpointId() << std::endl;
      }
  // 		for (const nfd::pit::InRecord& inRecord : entry->getInRecords()) {
  // 			(*outputStream) << "\n\t inRecord endpointID: " << inRecord.getEndpointId() << std::endl;
  // 			// 			(*outputStream) << "\n\t inRecord Face: " << inRecord.getFace().getLocalUri() << "\n";
  // 		}
      for (const nfd::pit::OutRecord& outRecord : entry->getOutRecords()) {
        (*outputStream) << "\n\t\t outRecord Face: " << outRecord.getFace() << std::endl;
        // 			(*outputStream) << "\t outRecord Face: " << outRecord.getFace().getLocalUri() << std::endl;//getRemoteUri()
      }
      (*outputStream) << std::endl;
    }
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
		(*outputStream) << Simulator::Now ().ToDouble (Time::S) << "\t" << "Content name: " << entry->getName().toUri() << "\t" << "Node Speed: " << entry->getNodeSpeed() << "\t" << "Geo-Position: " << entry->getNodePosition() /*<< "\t" << "Prefix: " << entry->getPrefix()*/ << "\t" << "Hops: " << entry->getNodeHops() << "\t" << "Timestamp: " << entry->getNodeTimestamp() << std::endl;// getFullName

	(*outputStream) << std::endl;
	Simulator::Schedule (Seconds (5.0), printCS, node, Ffile, control);
}


void showPosition(Ptr<Node> node, double deltaTime, const std::string& Ffile, bool control)
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

  (*outputStream).setf(ios::right);
  (*outputStream).setf(ios::showpoint);
  (*outputStream)<<setprecision(4)<<std::setw(8);


  uint64_t nodeId = node->GetId ();
  Ptr<MobilityModel> mobModel = node->GetObject<MobilityModel> ();
  Vector3D pos = mobModel->GetPosition ();
  Vector3D speed = mobModel->GetVelocity ();

  // auto radial_distance = std::hypot(pos.x, pos.y);
  // auto radial_speed = (speed.x * (pos.x - 0) + speed.y * (pos.y - 0)) / radial_distance;// RSU position: 828.2 1003. See: 3DToRadialSpeed in /home/dasilva/PDEEC2021/testingENV




//   (*outputStream) << "At timestamp (s): " << /*Simulator::Now().GetHours() << "-" << Simulator::Now().GetMinutes() << "-" <<*/ Simulator::Now().GetSeconds() << "\tnode " << nodeId
//             << ":\tPosition(" << pos.x << ", " << pos.y << ", " << pos.z
//             << ");\tSpeed(" << speed.x << ", " << speed.y << ", " << speed.z
//             << ")" << std::endl;

  // For Kalman Filter
  (*outputStream) << pos.x << "\t" << pos.y << "\t" << speed.x << "\t" << speed.y /*<< "\t" << radial_speed*/ << "\t" <<  Simulator::Now().GetSeconds() << std::endl;//Simulator::Now().GetMilliSeconds()


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


void check_files(/*ifstream& in_file, string& in_name,*/
ofstream& out_file, string& out_name) {

  if (!out_file.is_open()) {
    cerr << "Cannot open output file: " << out_name << endl;
    exit(EXIT_FAILURE);
  }
}

/**********/

int
main(int argc, char* argv[])
{
  NS_LOG_COMPONENT_DEFINE ("ndn.simple.wifi2");
//   LogComponentEnable("ndn.simple.wifi2", LOG_LEVEL_INFO);

  // disable fragmentation
  // Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
  // Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));

  // Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue("OfdmRate24Mbps"));

  /***/ //Delete all this
  //   wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
  // QosWaveMacHelper wifi80211pMac = QosWaveMacHelper::Default ();
  // Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  // if (verbose)
  //   {
  //     wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
  //   }
  //
  // wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
  //                                     "DataMode",StringValue (phyMode),
  //                                     "ControlMode",StringValue (phyMode));
  // NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);
  /*****/


 Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default (); // Do not load Default (avoid "NonUnicastMode", StringValue ...)
	bool verbose = false; // Turn on (true) or off (false) all Wifi 802.11p logging
	if (verbose)
	{
		wifi80211p.EnableLogComponents ();
	}

	std::string phyMode ("OfdmRate12MbpsBW10MHz");
    // std::string phyMode ("OfdmRate12MbpsBW10MHz");
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
    // std::string tmp_dir;
    // std::string gdbString;

	int    nodeNum;
	double duration;

	CommandLine cmd;
    cmd.AddValue ("tmp_dir", "Temporary directory name", tmp_dir);
	cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
	cmd.AddValue ("nodeNum", "Number of vehicles", nodeNum);
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
		"NOTE 2: Number of vehicles present in the trace file must match with the command line argument and must\n"
		"        be a positive number. Note that you must know it before to be able to load it.\n\n"
		"NOTE 3: Duration must be a positive number. Note that you must know it before to be able to load it.\n\n";

		return 0;
	}



	// Register the number of nodes for the actual scenario. Each other module can read the file
	// std::ofstream file;
  // file.open("nodeNum.txt", ios::out | ios::trunc | O_SYNC);
  // file << nodeNum + 1; // Include RSU
  // file.flush();
  // file.close();


  // #include <fcntl.h>
  // #include <unistd.h>
  // const char* filename = "nodeNum.txt";
  // int value = nodeNum + 1; // Includes RSU

  // int fd = open(filename, O_WRONLY | O_CREAT | O_SYNC, 0644);
  // if (fd != -1) {
  //     write(fd, &value, sizeof(value));
  //     close(fd);
  // } else {
  //     perror("Failed to open file");
  // }

  // open log file for output
  std::ofstream os;
  os.open (logFile.c_str ());

	// Create Ns2MobilityHelper with the specified trace log file as parameter
  Ns2MobilityHelper ns2 = Ns2MobilityHelper(traceFile);

  NodeContainer vehicles;
  vehicles.Create(nodeNum);

  NodeContainer rsu;
  rsu.Create(1); //1 RSU

  NodeContainer allNodes;
  allNodes.Add(vehicles);
  allNodes.Add(rsu);

  ////////////////
  // 1. Install Wifi
  NetDeviceContainer wifiNetDevices = wifi80211p.Install(wifiPhyHelper, wifi80211pMac, vehicles);//vehicles
  // wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, vehicles[0])); // One more device
  // wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, vehicles)); // One more device for all
  NetDeviceContainer rsuDevices = wifi80211p.Install(wifiPhyHelper, wifi80211pMac, rsu);//RSU

  // 2. Install Mobility model
  // Using constant velocity model and assign individual velocity for each node
  // Set a constant velocity mobility model

//   mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
//   mobility.Install (vehicles);
  ns2.Install ();


    // Print the selected nodes Id to file
  string out_file_name_ = tmp_dir + "Selected_Nodes.txt";
  ofstream out_file_;//(out_file_name_.c_str(), ofstream::out);
  out_file_.open(out_file_name_.c_str(), ofstream::out | std::ios::trunc);
  check_files(out_file_, out_file_name_);




  // For RSU
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject <ListPositionAllocator>();
  // positionAlloc->Add(Vector(1459.22, 294.14, 0));// Set the RSU to a fixed position (from QGis: 4754480.7,-720798.2) Near Node 45: 828.16 1003.02


   Ptr<UniformRandomVariable> randomNum = CreateObject<UniformRandomVariable> ();
  // std::cout << "Generated random nr: " << (uint64_t) randomNum->GetValue(0, nodeNum) << std::endl;

  uint64_t selectedRSU = (uint64_t) randomNum->GetValue(0, 16); // random RSU positions
  // auto positionVector = Vector(828.16, 1003.02, 0.0);
  switch(selectedRSU){

    case 0: positionAlloc->Add(Vector(608.179, 653.983, 0.0));
            out_file_ << "RSU location: 608.179, 653.983" << std::endl;
            break;
    case 1: positionAlloc->Add(Vector(1099.3, 381.702, 0.0));
            out_file_ << "RSU location: 1099.3, 381.702" << std::endl;
            break;
    case 2: positionAlloc->Add(Vector(1005.15, 1425.02, 0.0));
            out_file_ << "RSU location: 1005.15, 1425.02" << std::endl;
            break;
    case 3: positionAlloc->Add(Vector(712.511, 111.966, 0.0));
            out_file_ << "RSU location: 712.511, 111.966" << std::endl;
            break;
    case 4: positionAlloc->Add(Vector(262.102, 1361.4, 0.0));
            out_file_ << "RSU location: 262.102, 1361.4" << std::endl;
            break;
    case 5: positionAlloc->Add(Vector(699.787, 302.817, 0.0));
            out_file_ << "RSU location: 699.787, 302.817" << std::endl;
            break;
    case 6: positionAlloc->Add(Vector(587.821, 900.817, 0.0));
            out_file_ << "RSU location: 587.821, 980.817" << std::endl;
            break;
    case 7: positionAlloc->Add(Vector(709.6, 1895.79, 0.0));
            out_file_ << "RSU location: 709.6, 1895.79" << std::endl;
            break;
    case 8: positionAlloc->Add(Vector(951.711, 1053.5, 0.0));
            out_file_ << "RSU location: 951.711, 1053.5" << std::endl;
            break;
    case 9: positionAlloc->Add(Vector(1394.49, 1605.69, 0.0));
            out_file_ << "RSU location: 1394.49, 1605.69" << std::endl;
            break;
    case 10: positionAlloc->Add(Vector(1145.11, 776.128, 0.0));
            out_file_ << "RSU location: 1145.11, 776.128" << std::endl;
            break;
    case 11: positionAlloc->Add(Vector(828.16, 1003.02, 0.0));
            out_file_ << "RSU location: 828.16 , 1003.02" << std::endl;
            break;
    case 12: positionAlloc->Add(Vector(201.03, 272.281, 0.0));
            out_file_ << "RSU location: 201.03, 272.281" << std::endl;
            break;
    case 13: positionAlloc->Add(Vector(1119.66, 944.077, 0.0));
            out_file_ << "RSU location: 1119.66, 944.077" << std::endl;
            break;
    case 14: positionAlloc->Add(Vector(1855.07, 432.596, 0.0));
            out_file_ << "RSU location: 1855.07, 432.596" << std::endl;
            break;
    case 15: positionAlloc->Add(Vector(997.515, 709.966, 0.0));
            out_file_ << "RSU location: 997.515, 709.966" << std::endl;
            break;
    case 16: positionAlloc->Add(Vector(1598.06, 1710.03, 0.0));
            out_file_ << "RSU location: 1598.06, 1710.03" << std::endl;
            break;
    default: positionAlloc->Add(Vector(828.16, 1003.02, 0.0));
            out_file_ << "RSU location: 828.16 , 1003.02" << std::endl;
  }

  if (out_file_.is_open()) {
    out_file_.close();
  }

  // positionAlloc->Add(Vector(828.16, 1003.02, 0.0));// Set the RSU to a fixed position near node 45 (from scenario with 50 nodes): 828.16 1003.02


  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install (rsu);




//   // setup a uniform random variable for the speed
//   Ptr<UniformRandomVariable> rvar = CreateObject<UniformRandomVariable>();
//
//   // for each node set up its speed according to the random variable
//   for (NodeContainer::Iterator iter= vehicles.Begin(); iter!=vehicles.End(); ++iter){
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
  // mobility.Install (c);//vehicles


  // https://github.com/ms-van3t-devs/ms-van3t/blob/master/src/automotive/examples/v2i-areaSpeedAdvisor-80211p.cc
  // MobilityHelper mobility;
  // mobility.Install (rsu);
  /* Set the RSU to a fixed position (i.e. on the center of the map, in this case) */
  // Ptr<MobilityModel> mobilityRSU = rsu.Get (0)->GetObject<MobilityModel> ();
  // mobilityRSU->SetPosition (Vector (0, 0, 20.0)); // Normally, in SUMO, (0,0) is the center of the map


  // Populate the Map
  // auto readNodesFromWifiHelper = nodeNum + 3;
  getFileContent("NodeIdMacMapping.txt", NodeIdMacMap);
  // std::cout << "readNodesFromWifiHelper: " << readNodesFromWifiHelper << "\n";

  // 3. Install NDN stack
  NS_LOG_INFO("Installing NDN stack");
  ndn::StackHelper ndnHelper;
  // ndnHelper.AddNetDeviceFaceCreateCallback (WifiNetDevice::GetTypeId (), MakeCallback
  // (MyNetDeviceFaceCallback));
  ndnHelper.setPolicy("nfd::cs::cromo2");
  ndnHelper.setCsSize(3000);//2000
  // ndnHelper.SetDefaultRoutes(false);//20230418 ndnHelper.SetDefaultRoutes(true);
  ndnHelper.Install(allNodes);//vehicles

  double deltaTime = 5; // Periodicity for printing speed

  // Set BestRoute strategy
//   ndn::StrategyChoiceHelper::Install(vehicles, "/", "/localhost/nfd/strategy/multicast");
  ndn::StrategyChoiceHelper::Install(allNodes, "/cromo", "/localhost/nfd/strategy/mobility-aware");//vehicles -> allNodes 20230418

  // 4. Set up applications
  NS_LOG_INFO("Installing Applications");

    // STMP on all vehicles
//   ndn::AppHelper app1("StmpApp");
//   app1.Install(vehicles);


//   ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCRoMo2");

  // Ptr<UniformRandomVariable> randomNum = CreateObject<UniformRandomVariable> ();
  // std::cout << "Generated random nr: " << (uint64_t) randomNum->GetValue(0, nodeNum) << std::endl;

  uint64_t selectedProducer = (uint64_t) randomNum->GetValue(0, nodeNum);
  uint64_t selectedProducerBeacon = (uint64_t) randomNum->GetValue(0, nodeNum);
  uint64_t selectedConsumer = (uint64_t) randomNum->GetValue(0, nodeNum);

  while ((selectedProducer == selectedConsumer)
                || (selectedProducerBeacon == selectedConsumer)
                || (selectedProducer == selectedProducerBeacon))
  {// vehicles.Get(7) <= RSU
    std::cout << "Re-generating Consumer Id... " << std::endl;
    selectedConsumer = (uint64_t) randomNum->GetValue(0, nodeNum);
    selectedProducer = (uint64_t) randomNum->GetValue(0, nodeNum);
    selectedProducerBeacon = (uint64_t) randomNum->GetValue(0, nodeNum);
  }

  // Commented 20230418
  // wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, vehicles[selectedConsumer])); // One more device
  // wifiNetDevices.Add(wifi80211p.Install(wifiPhyHelper, wifi80211pMac, vehicles[selectedProducer])); // One more device








  // Print the selected nodes Id to file
  // string out_file_name_ = tmp_dir + "Selected_Nodes.txt";
  // ofstream out_file_;//(out_file_name_.c_str(), ofstream::out);
  out_file_.open(out_file_name_.c_str(), ofstream::out | std::ios::app);
  check_files(out_file_, out_file_name_);

  out_file_ << "Selected Consumer: Node_" << selectedConsumer <<"\nSelected Producer: Node_" << selectedProducer <<"\nSelected ProducerBeacon: Node_" << selectedProducerBeacon << std::endl;

  if (out_file_.is_open()) {
    out_file_.close();
  }

  consumerHelper.SetPrefix("/cromo/test/prefix/consumer_"+std::to_string(vehicles.Get(selectedConsumer)->GetId()));
  consumerHelper.SetAttribute("Frequency", DoubleValue(10.0));//100
  auto consumer1 = vehicles.Get(selectedConsumer);
  auto appConsumer1 = consumerHelper.Install(consumer1);
  appConsumer1.Start(Seconds(0.0));

  // For now desactivated
  // consumerHelper.SetPrefix("/cromo/beacon/OtherTmp_"+std::to_string(vehicles.Get(13)->GetId()));
  // consumerHelper.SetAttribute("Frequency", DoubleValue(1));//100
  // auto consumer2 = vehicles.Get(13);
  // auto appConsumer2 = consumerHelper.Install(consumer2);







  // consumerHelper.SetPrefix("/cromo/test/prefix/consumer_"+std::to_string(vehicles.Get(4)->GetId()));
  // consumerHelper.SetAttribute("Frequency", DoubleValue(50.0));//100
  // auto consumer3 = vehicles.Get(4);
  // auto appConsumer3 = consumerHelper.Install(consumer3);
  // appConsumer3.Start(Seconds(0.0));

  // consumerHelper.SetPrefix("/cromo/test/prefix/consumer_"+std::to_string(vehicles.Get(5)->GetId()));
  // consumerHelper.SetAttribute("Frequency", DoubleValue(70.0));//100
  // auto consumer4 = vehicles.Get(5);
  // auto appConsumer4 = consumerHelper.Install(consumer4);
  // appConsumer4.Start(Seconds(0.0));




  // For now desactivated
  // For Beacon
  // ndn::AppHelper consumerHelper1("ns3::ndn::ConsumerCRoMo2Beacon");
  // consumerHelper1.SetPrefix("/cromo/beacon/RSU_"+std::to_string(rsu.Get(0)->GetId()));
  // consumerHelper1.SetAttribute("Frequency", DoubleValue(1));//0.2
  // auto RSU0 = rsu.Get(0);
  // auto appRSU0 = consumerHelper1.Install(RSU0);
  // appRSU0.Start(Seconds(0.0));





//   std::string tmp_dir = "/home/dasilva/PDEEC2021/testingENV/ns-3/testingVM_logs/";
  // std::string tmp_dir = "/var/tmp/ns-3/testingVM_logs/";
  const bool control = false;
  //int i = 0;



// Producer
  ndn::AppHelper producerHelper("ns3::ndn::ProducerCRoMo2");
  producerHelper.SetPrefix("/cromo/test");
  producerHelper.SetAttribute("PayloadSize", StringValue("1450"));
  auto producer1 = vehicles.Get(selectedProducer);
  auto appProducer1 = producerHelper.Install(producer1);
  appProducer1.Start(Seconds(0.0));

  // A producer for push-based emergency messages ...  and for responding Beacons

  // // ndn::AppHelper producerHelper1("ns3::ndn::ProducerCRoMo2Emergency");
  // // producerHelper1.SetPrefix("/cromo/push-message");
  // // producerHelper1.SetAttribute("PayloadSize", StringValue("1200"));
  // //
  // // auto producer2 = vehicles.Get(3);
  // // auto appProducer2 = producerHelper1.Install(producer2);
  // // appProducer2.Start(Seconds(0.0));
  // //


  // For now desactivated
  // ...  and for responding Beacons
  // ndn::AppHelper producerHelper2("ns3::ndn::ProducerCRoMo2Beacon");
  // producerHelper2.SetPrefix("/cromo/beacon");
  // producerHelper2.SetAttribute("PayloadSize", StringValue("1200"));
  // auto appProducer3 = producerHelper2.Install(vehicles.Get(selectedProducerBeacon));// selectedProducerBeacon
  // appProducer3.Start(Seconds(0.0));







  // We're installing a consumer at node vehicles.Get(3) which is also a producer. This node will force this node to produce push-based emergency messages with the prefix /push-based/info-type/sender-ID/sender-geo-coordinates/

  // ndn::AppHelper consumerHelper2("ns3::ndn::ConsumerCRoMo2Emergency");
  // consumerHelper2.SetPrefix("/cromo/push-message/active-safety");
  // consumerHelper2.SetAttribute("Frequency", DoubleValue(50.0));//100
  // auto consumer5 = vehicles.Get(11); // 3 to 12
  // auto appConsumer5 = consumerHelper2.Install(consumer5);
  // appConsumer5.Start(Seconds(0.0));


  if(printToFileAll == false){ // Only print Info of selected nodes

    if(printToFileFIB) printFIBmoreData (vehicles.Get(selectedConsumer), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(selectedConsumer)->GetId()) + ".txt", control);
    if(printToFilePIT) printPIT (vehicles.Get(selectedConsumer), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(selectedConsumer)->GetId()) + ".txt", control);
    if(printToFileCS) printCS (vehicles.Get(selectedConsumer), tmp_dir +  "CS_Node_" + std::to_string(vehicles.Get(selectedConsumer)->GetId()) + ".txt", control);
    if(printToFilePosition) showPosition (vehicles.Get(selectedConsumer), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(vehicles.Get(selectedConsumer)->GetId()) + ".txt", control);



    // Intermediate vehicles
    if(printToFileFIB) printFIBmoreData (vehicles.Get(2), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(2)->GetId()) + ".txt", control);
    if(printToFilePIT) printPIT (vehicles.Get(2), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(2)->GetId()) + ".txt", control);
    if(printToFileCS) printCS (vehicles.Get(2), tmp_dir + "CS_Node_" + std::to_string(vehicles.Get(2)->GetId()) + ".txt", control);
    if(printToFilePosition) showPosition (vehicles.Get(2), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(vehicles.Get(2)->GetId()) + ".txt", control);

    if(printToFileFIB) printFIBmoreData (vehicles.Get(4), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(4)->GetId()) + ".txt", control);
    if(printToFilePIT) printPIT (vehicles.Get(4), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(4)->GetId()) + ".txt", control);
    if(printToFileCS) printCS (vehicles.Get(4), tmp_dir + "CS_Node_" + std::to_string(vehicles.Get(4)->GetId()) + ".txt", control);
    if(printToFilePosition) showPosition (vehicles.Get(4), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(vehicles.Get(4)->GetId()) + ".txt", control);

    if(printToFileFIB) printFIBmoreData (vehicles.Get(5), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(5)->GetId()) + ".txt", control);
    if(printToFilePIT) printPIT (vehicles.Get(5), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(5)->GetId()) + ".txt", control);
    if(printToFileCS) printCS (vehicles.Get(5), tmp_dir + "CS_Node_" + std::to_string(vehicles.Get(5)->GetId()) + ".txt", control);
    if(printToFilePosition) showPosition (vehicles.Get(5), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(vehicles.Get(5)->GetId()) + ".txt", control);

    if(printToFileFIB) printFIBmoreData (vehicles.Get(7), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(7)->GetId()) + ".txt", control);
    if(printToFilePIT) printPIT (vehicles.Get(7), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(7)->GetId()) + ".txt", control);
    if(printToFileCS) printCS (vehicles.Get(7), tmp_dir + "CS_Node_" + std::to_string(vehicles.Get(7)->GetId()) + ".txt", control);
    if(printToFilePosition) showPosition (vehicles.Get(7), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(vehicles.Get(7)->GetId()) + ".txt", control);


    // RSU0
    if(printToFileFIB) printFIBmoreData (rsu.Get(0), tmp_dir + "FIB_Node_" + std::to_string(rsu.Get(0)->GetId()) + ".txt", control);
    if(printToFilePIT) printPIT (rsu.Get(0), tmp_dir + "PIT_Node_" + std::to_string(rsu.Get(0)->GetId()) + ".txt", control);
    if(printToFileCS) printCS (rsu.Get(0), tmp_dir +  "CS_Node_" + std::to_string(rsu.Get(0)->GetId()) + ".txt", control);
    if(printToFilePosition) showPosition (rsu.Get(0), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(rsu.Get(0)->GetId()) + ".txt", control);


    if(printToFileFIB) printFIBmoreData (vehicles.Get(selectedProducerBeacon), tmp_dir + "FIB_ProducerBeacon_" + std::to_string(vehicles.Get(selectedProducerBeacon)->GetId()) + ".txt", control);
    if(printToFilePIT) printPIT (vehicles.Get(selectedProducerBeacon), tmp_dir + "PIT_ProducerBeacon_" + std::to_string(vehicles.Get(selectedProducerBeacon)->GetId()) + ".txt", control);
    if(printToFileCS) printCS (vehicles.Get(selectedProducerBeacon), tmp_dir + "CS_ProducerBeacon_" + std::to_string(vehicles.Get(selectedProducerBeacon)->GetId()) + ".txt", control);
    if(printToFilePosition) showPosition (vehicles.Get(selectedProducerBeacon), deltaTime, tmp_dir + "Mobility_ProducerBeacon_"+std::to_string(vehicles.Get(selectedProducerBeacon)->GetId()) + ".txt", control);


    // For producer (s)
    if(printToFileFIB) printFIBmoreData (vehicles.Get(1), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(1)->GetId()) + ".txt", false);
    if(printToFilePIT) printPIT (vehicles.Get(1), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(1)->GetId()) + ".txt", false);
    if(printToFileCS) printCS (vehicles.Get(1), tmp_dir + "CS_Node_" + std::to_string(vehicles.Get(1)->GetId()) + ".txt", false);
    if(printToFilePosition) showPosition (vehicles.Get(1), deltaTime, tmp_dir + "Mobility_Node_"+std::to_string(vehicles.Get(1)->GetId()) + ".txt", false);

    //
    if(printToFileFIB) printFIBmoreData (vehicles.Get(3), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(3)->GetId()) + ".txt", false);
    if(printToFilePIT) printPIT (vehicles.Get(3), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(3)->GetId()) + ".txt", false);
    if(printToFileCS) printCS (vehicles.Get(3), tmp_dir + "CS_Node_" + std::to_string(vehicles.Get(3)->GetId()) + ".txt", false);
    if(printToFilePosition) showPosition (vehicles.Get(3), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(vehicles.Get(3)->GetId()) + ".txt", false);

    if(printToFileFIB) printFIBmoreData (vehicles.Get(9), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(9)->GetId()) + ".txt", false);
    if(printToFilePIT) printPIT (vehicles.Get(9), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(9)->GetId()) + ".txt", false);
    if(printToFileCS) printCS (vehicles.Get(9), tmp_dir + "CS_Node_" + std::to_string(vehicles.Get(9)->GetId()) + ".txt", false);
    if(printToFilePosition) showPosition (vehicles.Get(9), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(vehicles.Get(9)->GetId()) + ".txt", false);


    /**** Random router(s) *****/

    if(printToFileFIB) printFIBmoreData (vehicles.Get(selectedProducer), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(selectedProducer)->GetId()) + ".txt", false);
    if(printToFilePIT) printPIT (vehicles.Get(selectedProducer), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(selectedProducer)->GetId()) + ".txt", false);
    if(printToFileCS) printCS (vehicles.Get(selectedProducer), tmp_dir + "CS_Node_" + std::to_string(vehicles.Get(selectedProducer)->GetId()) + ".txt", false);
    if(printToFilePosition) showPosition (vehicles.Get(selectedProducer), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(vehicles.Get(selectedProducer)->GetId()) + ".txt", false);// 3 seconds

    if(printToFileFIB) printFIBmoreData (vehicles.Get(8), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(29)->GetId()) + ".txt", false);
    if(printToFilePIT) printPIT (vehicles.Get(8), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(29)->GetId()) + ".txt", false);
    if(printToFileCS) printCS (vehicles.Get(8), tmp_dir + "CS_Node_" + std::to_string(vehicles.Get(29)->GetId()) + ".txt", false);
    if(printToFilePosition) showPosition (vehicles.Get(9), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(vehicles.Get(8)->GetId()) + ".txt", false);// 3 seconds
  }
  else {
    for (uint64_t i = 0; i < nodeNum; i++){// +1 for the RSU
       if(printToFileFIB) printFIBmoreData (vehicles.Get(i), tmp_dir + "FIB_Node_" + std::to_string(vehicles.Get(i)->GetId()) + ".txt", false);
       if(printToFilePIT) printPIT (vehicles.Get(i), tmp_dir + "PIT_Node_" + std::to_string(vehicles.Get(i)->GetId()) + ".txt", false);
       if(printToFileCS) printCS (vehicles.Get(i), tmp_dir + "CS_Node_" + std::to_string(vehicles.Get(i)->GetId()) + ".txt", false);
       if(printToFilePosition) showPosition (vehicles.Get(i), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(vehicles.Get(i)->GetId()) + ".txt", false);// 3 seconds
    }
    // RSU0
    if(printToFileFIB) printFIBmoreData (rsu.Get(0), tmp_dir + "FIB_Node_" + std::to_string(rsu.Get(0)->GetId()) + ".txt", control);
    if(printToFilePIT) printPIT (rsu.Get(0), tmp_dir + "PIT_Node_" + std::to_string(rsu.Get(0)->GetId()) + ".txt", control);
    if(printToFileCS) printCS (rsu.Get(0), tmp_dir +  "CS_Node_" + std::to_string(rsu.Get(0)->GetId()) + ".txt", control);
    if(printToFilePosition) showPosition (rsu.Get(0), deltaTime, tmp_dir + "Mobility_Node_" + std::to_string(rsu.Get(0)->GetId()) + ".txt", control);
  }

  /**************/
//   double deltaTime = 2;
//   Simulator::Schedule (Seconds (0.0), &showPosition, producer, deltaTime); // Schedule to print node velocity status
  ////////////////

  Simulator::Stop(Seconds(duration));

  ndn::AppDelayTracer::InstallAll(tmp_dir + "delay-ndn-simple-wifi3-WithBeacon_20230109.txt");
//   ndn::L3AggregateTracer::InstallAll ("aggregate-ndn-simple-wifi.txt", Seconds (2));
  ndn::L3RateTracer::InstallAll (tmp_dir + "rate-ndn-simple-wifi3-WithBeacon_20230109.txt", Seconds (5));
  // AnimationInterface anim(tmp_dir + "vehicularmobility_ndn-simple-wifi3-WithBeacon_20221212.xml");
  // anim.SetMobilityPollInterval(Seconds(1.0));//0.50//0.25*





  // anim.EnablePacketMetadata(true);

//   for (uint64_t i=0; i<wifiSta ; i++) {
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
