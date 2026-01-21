#!/bin/bash

#
# Copyright (c) 2017 Orange Labs
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Author: Rediet <getachew.redieteab@orange.com>
#

# This script runs wifi-trans-example for all combinations, plots transmitted spectra
# using the generated .plt file and puts result png images in wifi-trans-results folder.
#
# Inspired from Tom's and Nicola's scripts for LAA-WiFi coexistence

start_time=$(date +%s)

tmp_dir="$1"
nodeNum=50
duration=689.00
traceFile=mobility_50_2023010.tcl
logFile=ns2-mobility-trace_20221217.log
log=log_file


#Catch abort signal
control_c()
{
  echo "Aborted, exiting..."
  exit $?
}
trap control_c SIGINT

#Save waf and script locations. Have to be run from where they are (otherwise won't find executables)
# tmp_dir=$@

# if [ -d $tmp_dir ]; then
#   echo "$tmp_dir directory exists."
# else
#   mkdir -p "$tmp_dir"
#   echo "$tmp_dir directory created."
# fi


# '/var/tmp/ns-3/testingVM_logs/' + RngRun_${i}_OneConsumer
scriptDir=`pwd`
cd ../../../
wafDir=`pwd`
# ${wafDir}/../../../mobility_20221217_1.tcl (~/mobility_20221217_1.tcl)

storage_dir="/root/fromVM/OneConsumer${nodeNum}"
cd $scriptDir


# RngRun=(10 15 20 25)
file_to_run=ndn-simple-wifi3-WithBeacon_20221222
control=0

#Iteratively run simulation for all combinations
#Legacy combinations
cd $wafDir


# storage_dir="/home/dasilva/PDEEC2021/fromVM/OneConsumer"
for i in 50
  do
      outputDir="${storage_dir}/RngRun_${i}_OneConsumer_$(date +%4Y%m%d)_"
      if [ -d $outputDir ]; then
        echo "$outputDir directory exists."
      else
        mkdir -p "$outputDir"
        echo "$outputDir directory created."
      fi

#       tmp_dir=$1
#       if [ -d $tmp_dir ]; then
#         echo "$tmp_dir directory exists."
#       else
#         mkdir -p "$tmp_dir"
#         echo "$tmp_dir directory created."
#       fi


      echo "==============================================="
      echo "RngRun=\"${i}\" ./waf --run $file_to_run --tmp_dir=$tmp_dir --traceFile=$traceFile --nodeNum=$nodeNum --duration=$duration --logFile=$logFile 2>&1 | tee $log _RngRun_${i}"
#       cd $wafDir
      NS_GLOBAL_VALUE="RngRun=${i}" ${wafDir}/waf --run "$file_to_run --tmp_dir=$tmp_dir --traceFile=$traceFile --nodeNum=$nodeNum --duration=$duration --logFile=$logFile"


      echo "Finished running RngRun="$i" ./waf --run $file_to_run --tmp_dir=$tmp_dir --traceFile=$traceFile --nodeNum=$nodeNum --duration=$duration --logFile=$logFile with Time=$(($(date +%s) - $start_time)) seconds" >> log_run

      echo "Moving $tmp_dir/* $outputDir" >> log_run
      mv -v $tmp_dir/* $outputDir
#   ((control++))
      end_time=$(date +%s)
      echo "Total time elapsed: $(($end_time - $start_time)) seconds" > $outputDir/finishTime.txt
  done

echo "Finished"

end_time=$(date +%s)

echo "Total time elapsed: $(($end_time - $start_time)) seconds"
