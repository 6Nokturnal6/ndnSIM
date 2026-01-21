// Global MAP
// #pragma once
#ifndef MAP_COMMON_HPP
#define MAP_COMMON_HPP

#include "ns3/nstime.h"
#include "ns3/timer.h"
#include "ndn-cxx/util/nmsi.hpp"
// #include <string>

extern std::vector<std::tuple<std::string, int>> NodeIdMacMap;
extern std::string tmp_dir;
extern uint64_t nodeNum;

extern int BloomFilter_size;
extern int BloomFilter_NumberOfHashFunctions;

extern bool invokeConsumer;
extern bool invokeConsumerCromo; // for cromo and beacon
extern bool invokeConsumerClient;
extern bool invokeFromClient;
extern bool invokeFromClientBeacon;
extern bool invokeConsumerBeacon;
extern uint64_t vehicleBeaconIntervalGlobal;
// extern bool clearToLauchVehicleBeaconing;

extern std::shared_ptr<ns3::Timer> nodeBeaconTimerGlobal;

// For setting the Interest destination as the last Data provider
extern uint64_t destinationID;

// These refers the Data origins
extern ndn::util::Nmsi DataOrigNmsi;
// extern std::string DataOrigId;
extern std::string DataOrigId/* = "ff:ff:ff:ff:ff:77"*/;



extern uint64_t neighborhoodTableSize;
// ::ndn::util::Tools toolsX;
// uint64_t nodeNum = toolsX.GetNodeNum (tmp_dir);
#endif //MAP_COMMON_HPP
