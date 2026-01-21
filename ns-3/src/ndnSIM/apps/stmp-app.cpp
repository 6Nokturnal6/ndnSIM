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

// stmp-app.cpp

#include "stmp-app.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"

#include <math.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>


#include "ndn-app.hpp"


#include <ns3/node-list.h>
#include <ns3/node.h>
#include "ns3/mobility-model.h"
#include "ns3/core-module.h"
// For redirecting std::cout
#include <bits/stdc++.h>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

string base_dir_ = "/var/tmp/ns-3/testingVM_logs/";

// const float PI2 = 2 * M_PI;

NS_LOG_COMPONENT_DEFINE("StmpApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(StmpApp);

// register NS-3 type
TypeId
StmpApp::GetTypeId()
{
  static TypeId tid = TypeId("StmpApp").SetParent<ndn::App>().AddConstructor<StmpApp>();
  return tid;
}

// Processing upon start of the application
void
StmpApp::StartApplication()
{
  // Create a name components object for name
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  uint64_t nodeId = node->GetId();


  // initialize ndn::App
  ndn::App::StartApplication();
  // std::cout << "Initializing STMP algorithm on Node_" << nodeId << std::endl;
  NS_LOG_DEBUG("Initializing STMP algorithm on Node_" << nodeId);

  // Add entry to FIB for `/prefix/sub`
  ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);

  // Schedule send of first interest
  Simulator::Schedule(Seconds(0.0), &StmpApp::stmp, this);
}

// Processing when application is stopped
void
StmpApp::StopApplication()
{
//   Simulator::Schedule(Seconds(0.0), &StmpApp::stmp, this);
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
StmpApp::SendInterest()
{
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////

  // Create and configure ndn::Interest
  auto interest = std::make_shared<ndn::Interest>("/prefix/sub");
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setInterestLifetime(ndn::time::seconds(1));

  NS_LOG_DEBUG("Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
//   m_transmittedInterests(interest, this, m_face);

//   m_appLink->onReceiveInterest(*interest);
  NS_LOG_DEBUG("Reaches here?.. ");
  Simulator::Schedule(Seconds(1.0), &StmpApp::stmp, this);
}

// Callback that will be called when Interest arrives
void
StmpApp::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  // Note that Interests send out by the app will not be sent back to the app !

//   auto data = std::make_shared<ndn::Data>(interest->getName());
//   data->setFreshnessPeriod(ndn::time::milliseconds(1000));
//   data->setContent(std::make_shared< ::ndn::Buffer>(1024));
//   ndn::StackHelper::getKeyChain().sign(*data);

//   NS_LOG_DEBUG("Sending Data packet for " << data->getName());
//   NS_LOG_DEBUG("Only for testing... STMP would send data...");
  // Call trace (for logging purposes)
//   m_transmittedDatas(data, this, m_face);

//   m_appLink->onReceiveData(*data);
}

// Callback that will be called when Data arrives
void
StmpApp::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  // std::cout << "DATA received for name " << data->getName() << std::endl;
}

// Tracking stuff

KalmanFilter::KalmanFilter() {}

KalmanFilter::~KalmanFilter() {}

void KalmanFilter::Init(VectorXd &x_in, MatrixXd &P_in, MatrixXd &F_in,
                        MatrixXd &H_in, MatrixXd &R_in, MatrixXd &Q_in) {
  x_ = x_in;
  P_ = P_in;
  F_ = F_in;
  H_ = H_in;
  R_ = R_in;
  Q_ = Q_in;
}

void KalmanFilter::Predict() {
  /**
  TODO:
    * predict the state
  */
  x_ = F_ * x_;
  MatrixXd Ft = F_.transpose();
  P_ = F_ * P_ * Ft + Q_;
}

void KalmanFilter::Update(const VectorXd &z) {
  /**
  TODO:
    * update the state by using Kalman Filter equations
  */

  VectorXd z_pred = H_ * x_;

  VectorXd y = z - z_pred;
  MatrixXd Ht = H_.transpose();
  MatrixXd PHt = P_ * Ht;
  MatrixXd S = H_ * PHt + R_;
  MatrixXd Si = S.inverse();
  MatrixXd K = PHt * Si;

  //new estimate
  x_ = x_ + (K * y);
  long x_size = x_.size();
  MatrixXd I = MatrixXd::Identity(x_size, x_size);
  P_ = (I - K * H_) * P_;

}


// Tracking

Tracking::Tracking() {
  is_initialized_ = false;
  previous_timestamp_ = 0;

  //create a 4D state vector, we don't know yet the values of the x state
  kf_.x_ = VectorXd(4);

  //state covariance matrix P
  kf_.P_ = MatrixXd(4, 4);
  kf_.P_ << 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1000, 0,
            0, 0, 0, 1000;


  //measurement covariance
  kf_.R_ = MatrixXd(2, 2);
  kf_.R_ << 0.0225, 0,
            0,      0.0225;

  //measurement matrix
  kf_.H_ = MatrixXd(2, 4);
  kf_.H_ << 1, 0, 0, 0,
            0, 1, 0, 0;

  //the initial transition matrix F_
  kf_.F_ = MatrixXd(4, 4);
  kf_.F_ << 1, 0, 1, 0,
            0, 1, 0, 1,
            0, 0, 1, 0,
            0, 0, 0, 1;

  //set the acceleration noise components
  noise_ax = 5;
  noise_ay = 5;

}

Tracking::~Tracking() {

}

// Process a single measurement
void Tracking::ProcessMeasurement(const MeasurementPackage &measurement_pack, const GroundTruthPackage &gt_pack) {
  if (!is_initialized_) {
//     cout << "Kalman Filter Initialization " << endl;

    //set the state with the initial location and velocity
    kf_.x_ << measurement_pack.raw_measurements_[0], measurement_pack.raw_measurements_[1], gt_pack.gt_values_(2), gt_pack.gt_values_(3);

//     std::cout << "vx: " << gt_pack.gt_values_(2) << std::endl;
    previous_timestamp_ = measurement_pack.timestamp_;
    is_initialized_ = true;
    return;
  }

  //compute the time elapsed between the current and previous measurements
  float dt = (measurement_pack.timestamp_ - previous_timestamp_)/1000.0;// / 1000000.0;	//dt - expressed in seconds
  previous_timestamp_ = measurement_pack.timestamp_;

  static int i = 0;
  // std::cout << "dt: " << dt << ", i = " << i++ << std::endl;
  float dt_2 = dt * dt;
  float dt_3 = dt_2 * dt;
  float dt_4 = dt_3 * dt;

  //Modify the F matrix so that the time is integrated
  kf_.F_(0, 2) = dt;
  kf_.F_(1, 3) = dt;

  //set the process covariance matrix Q
  kf_.Q_ = MatrixXd(4, 4);
  kf_.Q_ <<  dt_4/4*noise_ax, 0,               dt_3/2*noise_ax, 0,
             0,               dt_4/4*noise_ay, 0,               dt_3/2*noise_ay,
             dt_3/2*noise_ax, 0,               dt_2*noise_ax,   0,
             0,               dt_3/2*noise_ay, 0,               dt_2*noise_ay;

  //predict
  kf_.Predict();

  //measurement update
  kf_.Update(measurement_pack.raw_measurements_);

//   std::cout << "x_= " << kf_.x_ << std::endl;
//   std::cout << "P_= " << kf_.P_ << std::endl;

}

//RMSE

Tools::Tools() {}

Tools::~Tools() {}

VectorXd Tools::CalculateRMSE(const vector<VectorXd> &estimations,
                              const vector<VectorXd> &ground_truth) {
  /**
    * Calculate the RMSE here.
  */

  VectorXd rmse(4);
  rmse << 0,0,0,0;

  // check the validity of the following inputs:
  //  * the estimation vector size should not be zero
  //  * the estimation vector size should equal ground truth vector size
  if(estimations.size() != ground_truth.size() || estimations.size() == 0){
    // std::cout << "Invalid estimation or ground_truth data" << std::endl;
    return rmse;
  }

  //accumulate squared residuals
  for(unsigned int i=0; i < estimations.size(); ++i){

    VectorXd residual = estimations[i] - ground_truth[i];

    //coefficient-wise multiplication
    residual = residual.array()*residual.array();
    rmse += residual;
  }

  //calculate the mean
  rmse = rmse/estimations.size();

  //calculate the squared root
  rmse = rmse.array().sqrt();

  // Not for now
//   ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
//   uint32_t nodeId = node->GetId();
//
//   string out_file_name_ = base_dir_+"Node_" + std::to_string(nodeId) + "_RMSE.txt";
//   ofstream out_file_(out_file_name_.c_str(), ofstream::out);
//
//   out_file_ << rmse << "\n";
//
//
//   // close files
//   if (out_file_.is_open()) {
//     out_file_.close();
//   }


  //return the result
  return rmse;
}


// STMP

void StmpApp::stmp() {
  // Create a name components object for name
  ns3::Ptr<ns3::Node> node = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
  uint32_t nodeId = node->GetId();

  string in_file_name_ = base_dir_+"Mobility_Node_" + std::to_string(nodeId) + ".txt";
  ifstream in_file_(in_file_name_.c_str(), ifstream::in);

  string out_file_name_ = base_dir_+"Node" + std::to_string(nodeId) + "_STMP_Estimation.txt";
  ofstream out_file_(out_file_name_.c_str(), ofstream::out);

//   cout << "Whats hapenning? " << endl;
  NS_LOG_DEBUG("STMP on ACTION!... ");
//   check_files(in_file_, in_file_name_, out_file_, out_file_name_);

  vector<MeasurementPackage> measurement_pack_list;
  vector<GroundTruthPackage> gt_pack_list;

  string line;

  // prep the measurement packages (each line represents a measurement at a
  // timestamp)
  while (getline(in_file_, line)) {

    MeasurementPackage meas_package;
    GroundTruthPackage gt_package;
    istringstream iss(line);
    long long timestamp;

    // read measurements at this timestamp

    meas_package.raw_measurements_ = VectorXd(2);
    float x;
    float y;
    iss >> x;
    iss >> y;
    meas_package.raw_measurements_ << x, y; // The first two raw values (z on KF equation)
    iss >> timestamp; // Read timestamp
    meas_package.timestamp_ = timestamp;
    measurement_pack_list.push_back(meas_package);

    // read ground truth data to compare later
    float x_gt;
    float y_gt;
    float vx_gt;
    float vy_gt;
    iss >> x_gt;
    iss >> y_gt;
    iss >> vx_gt;
    iss >> vy_gt;
    gt_package.gt_values_ = VectorXd(4);
    gt_package.gt_values_ << x_gt, y_gt, vx_gt, vy_gt;
    gt_pack_list.push_back(gt_package);
  }

  // Create a KF instance
  Tracking trackingKF;


  // used to compute the root-mean-squared error (RMSE) later
  vector<VectorXd> estimations;
  vector<VectorXd> ground_truth;

  //Call the EKF-based fusion
  size_t N = measurement_pack_list.size();
  for (size_t k = 0; k < N; ++k) {
    // start filtering from the second frame (the speed is unknown in the first
    // frame)

    trackingKF.ProcessMeasurement(measurement_pack_list[k], gt_pack_list[k]);

    // output the estimation
    out_file_ << trackingKF.kf_.x_(0) << "\t"; //px
    out_file_ << trackingKF.kf_.x_(1) << "\t"; //py
    out_file_ << trackingKF.kf_.x_(2) << "\t"; //vx
    out_file_ << trackingKF.kf_.x_(3) << "\t" << "\t"; //vy

    // output the measurements **** FOR NOW this measurements need not to be printed
//     out_file_ << measurement_pack_list[k].raw_measurements_(0) << "\t";
//     out_file_ << measurement_pack_list[k].raw_measurements_(1) << "\t"  << "\t";

    // output the ground truth packages
    out_file_ << gt_pack_list[k].gt_values_(0) << "\t";
    out_file_ << gt_pack_list[k].gt_values_(1) << "\t";
    out_file_ << gt_pack_list[k].gt_values_(2) << "\t";
    out_file_ << gt_pack_list[k].gt_values_(3) << "\n";

//     estimations.push_back(fusionEKF.ekf_.x_);
    estimations.push_back(trackingKF.kf_.x_);
    ground_truth.push_back(gt_pack_list[k].gt_values_);
  }

  // compute the accuracy (RMSE)
  Tools tools;
  // cout << "Accuracy - RMSE ([px, py, vx, vy]):" << endl << tools.CalculateRMSE(estimations, ground_truth) << endl; // 20250310

  // close files
  if (out_file_.is_open()) {
    out_file_.close();
  }

  // if (in_file_.is_open()) {
  //   in_file_.close();
  // }

  Simulator::Schedule(Seconds(3.0), &StmpApp::stmp, this);
}


} // namespace ns3
