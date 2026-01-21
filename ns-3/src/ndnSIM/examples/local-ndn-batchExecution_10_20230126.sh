#!/bin/bash

#
# Copyright (c) 2022 Centro ALGORITMI
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
# Author: Elidio <elidio.silva@unilurio.ac.mz>
#


start_time=$(date +%s)

tmp_dir="$1"
nodeNum="$2" #10
duration="$3" #471.00


#Catch abort signal
control_c()
{
  echo "Aborted, exiting..."
  exit $?
}
trap control_c SIGINT

scriptDir=`pwd`
cd ../../../
wafDir=`pwd`
# ${wafDir}/../../../mobility_20221217_1.tcl (~/mobility_20221217_1.tcl)
traceFile=mobility_10_20230126.tcl
logFile=ns2-mobility-trace_20221217.log
log=log_file

storage_dir="/home/dasilva/PDEEC2021/fromVM/OneConsumer${nodeNum}"
cd $scriptDir

file_to_run=ndn-simple-wifi3-WithBeacon_20221222 #19, 18
control=0

#Iteratively run simulation for all combinations
#Legacy combinations
cd $wafDir


# storage_dir="/home/dasilva/PDEEC2021/fromVM/OneConsumer"
for i in 0 #done
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

      echo "Moving ${tmp_dir}* $outputDir" >> log_run
      mv -v $tmp_dir/* $outputDir
#   ((control++))

      end_time=$(date +%s)
      echo "Total time elapsed: $(($end_time - $start_time)) seconds" > $outputDir/finishTime.txt
  done

echo "Finished"
echo "Total time elapsed: $(($end_time - $start_time)) seconds"
