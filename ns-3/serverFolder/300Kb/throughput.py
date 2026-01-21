import os
import numpy as np
import matplotlib.pyplot as plt

parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
sim_dirs = ['30', '40', '50', '60', '70', '80', '100', '120']  # Update simulation directory names accordingly

throughput_avgs = []
throughput_stds = []
interest_satisfaction_ratios = []
# content_hit_ratios = []
interest_satisfaction_ratio_errors = []
# content_hit_ratio_errors = []

for sim_dir in sim_dirs:
    dir = os.path.join(parent_dir, sim_dir)
    throughput_avg = 0
    throughput_std = 0
    interest_satisfaction_ratio_avg = 0
    interest_satisfaction_ratio_std = 0
    num_runs = 0
    for subdir in os.listdir(dir):
        if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
            continue

        trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')

        throughput_sum = 0
        throughput_count = 0

        satisfied_count = 0
        timedout_count = 0
        total_count = 0
        cache_hit_count = 0
        total_hit_count = 0

        with open(trace_file, 'r') as f:
            lines = f.readlines()
            for line in lines:
                if "OutData" in line and "netdev://[00:" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
                    cache_hit_count += float(line.split("\t")[5])
                    throughput_sum += float(line.split("\t")[6])/8
                    throughput_count += 1
                if "SatisfiedInterests" in line:
                    satisfied_count += float(line.split("\t")[5])
                if "TimedOutInterests" in line and len(line.split("\t")) > 4:
                    timedout_count += float(line.split("\t")[5])
                if "InInterests" in line and "netdev://[00:" in line and len(line.split("\t")) > 4:
                    total_count += float(line.split("\t")[5])
                if "OutInterests" in line and "netdev://[00:" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
                    total_hit_count += float(line.split("\t")[5])
            start_time = float(lines[0].split('\t')[0])
            end_time = float(lines[-1].split('\t')[0])
            duration = end_time - start_time

        # throughput_avg += throughput_sum/throughput_count if throughput_count > 0 else 0
        throughput_avg += throughput_sum/duration if duration > 0 else 0
        interest_satisfaction_ratio_avg += satisfied_count/total_hit_count if timedout_count > 0 else 0
        num_runs += 1

    if num_runs > 0:
        throughput_avg /= num_runs
        throughput_stds.append(np.std(throughput_avgs))
        interest_satisfaction_ratio_avg /= num_runs
        interest_satisfaction_ratio_errors.append(np.std(interest_satisfaction_ratios))
    else:
        throughput_stds.append(0)
        interest_satisfaction_ratio_errors.append(0)
    throughput_avgs.append(throughput_avg)
    interest_satisfaction_ratios.append(interest_satisfaction_ratio_avg * 100)

if throughput_avgs and interest_satisfaction_ratios:
    fig, ax = plt.subplots(2, 1, sharex=True, figsize=(8, 8))

    # Plot Interest Satisfaction Ratio (ISR)
    ax[0].errorbar(sim_dirs, interest_satisfaction_ratios, yerr=interest_satisfaction_ratio_errors, marker='o', label='Average Interest Satisfaction Ratio (ISR)')
    ax[0].set_ylabel('ISR (%)')
    ax[0].legend()
    # ax[0].grid(True)
    ax[0].axhline(y=0, color='gray', linestyle='--') # Add horizontal grid line at y=0
    ax[0].yaxis.grid(True, which='both', linestyle='-', linewidth=0.5) # Plot only horizontal grid lines for CHR
    ax[0].xaxis.grid(False) # Turn off vertical grid lines

    # Plot average throughput
    ax[1].errorbar(sim_dirs, throughput_avgs, yerr=throughput_stds, marker='o', label='Average Throughput')
    ax[1].set_xlabel('Number of vehicles (in a period of 470s - 220s = 250s)')
    ax[1].set_ylabel('Throughput (Mbps)')
    ax[1].legend()
    ax[1].axhline(y=0, color='gray', linestyle='--') # Add horizontal grid line at y=0
    ax[1].yaxis.grid(True, which='both', linestyle='-', linewidth=0.5) # Plot only horizontal grid lines for throughput
    ax[1].xaxis.grid(False) # Turn off vertical grid lines

    plt.savefig('isr_throughput.png', dpi=600)
    plt.show()
else:
    print("No data to plot.")












# *******************************************************

#
# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
# sim_dirs = ['30', '40', '50', '60', '70', '80', '100', '120']  # Update simulation directory names accordingly
#
# throughput_avgs = []
# throughput_stds = []
#
# for sim_dir in sim_dirs:
#     dir = os.path.join(parent_dir, sim_dir)
#     throughput_avg = 0
#     throughput_std = 0
#     num_runs = 0
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
#             continue
#
#         trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#
#         throughput_sum = 0
#         throughput_count = 0
#
#         with open(trace_file, 'r') as f:
#             lines = f.readlines()
#             for line in lines:
#                 if "OutData" in line and "netdev://[00:" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     throughput_sum += float(line.split("\t")[5])
#                     throughput_count += 1
#
#         throughput_avg += throughput_sum/throughput_count if throughput_count > 0 else 0
#         num_runs += 1
#
#     if num_runs > 0:
#         throughput_avg /= num_runs
#         throughput_stds.append(np.std(throughput_avgs))
#     else:
#         throughput_stds.append(0)
#     throughput_avgs.append(throughput_avg)
#
# if throughput_avgs:
#     fig, ax = plt.subplots(1, 1, sharex=True, figsize=(8, 4))
#
#     # Plot average throughput
#     ax.errorbar(sim_dirs, throughput_avgs, yerr=throughput_stds, marker='o', label='Average Throughput')
#     ax.set_xlabel('Number of vehicles (in a period of 450s - 150s = 300s)')
#     ax.set_ylabel('Throughput (Mbps)')
#     ax.legend()
#     ax.axhline(y=0, color='gray', linestyle='--') # Add horizontal grid line at y=0
#     ax.yaxis.grid(True, which='both', linestyle='-', linewidth=0.5) # Plot only horizontal grid lines for throughput
#     ax.xaxis.grid(False) # Turn off vertical grid lines
#
#     plt.show()
# else:
#     print("No data to plot.")

# *****************************

# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
# sim_dirs = ['30', '40', '50', '60', '70', '80', '100', '120']  # Update simulation directory names accordingly
#
# interest_satisfaction_ratios = []
# interest_satisfaction_ratio_errors = []
# throughputs = []
# throughput_errors = []
#
# for sim_dir in sim_dirs:
#     dir = os.path.join(parent_dir, sim_dir)
#     isr_sum = 0
#     throughput_sum = 0
#     num_runs = 0
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
#             continue
#
#         trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#
#         with open(trace_file, 'r') as f:
#             lines = f.readlines()
#             start_time = float(lines[0].split('\t')[0])
#             end_time = float(lines[-1].split('\t')[0])
#             duration = end_time - start_time
#             isr_sum += float(lines[-1].split("\t")[3])
#             throughput_sum += float(lines[-1].split("\t")[5]) / duration  # Calculate throughput as bytes per second
#             num_runs += 1
#
#     if num_runs > 0:
#         isr_avg = isr_sum / num_runs
#         interest_satisfaction_ratio_errors.append(np.std(interest_satisfaction_ratios))
#         throughput_avg = throughput_sum / num_runs
#         throughput_errors.append(np.std(throughputs))
#     else:
#         interest_satisfaction_ratio_errors.append(0)
#         throughput_errors.append(0)
#     interest_satisfaction_ratios.append(isr_avg)
#     throughputs.append(throughput_avg)
#
# if interest_satisfaction_ratios and throughputs:
#     fig, ax = plt.subplots(2, 1, sharex=True, figsize=(8, 8))
#
#     # Plot Interest Satisfaction Ratio (ISR)
#     ax[0].errorbar(sim_dirs, interest_satisfaction_ratios, yerr=interest_satisfaction_ratio_errors, marker='o', label='Interest Satisfaction Ratio (ISR)')
#     ax[0].set_ylabel('ISR (%)')
#     ax[0].legend()
#     ax[0].axhline(y=0, color='gray', linestyle='--') # Add horizontal grid line at y=0
#     ax[0].yaxis.grid(True, which='both', linestyle='-', linewidth=0.5) # Plot only horizontal grid lines for ISR
#     ax[0].xaxis.grid(False) # Turn off vertical grid lines
#
#     # Plot Average Throughput
#     ax[1].errorbar(sim_dirs, throughputs, yerr=throughput_errors, marker='o', label='Average Throughput')
#     ax[1].set_ylabel('Throughput (bytes/s)')
#     ax[1].set_xlabel('Simulation Time (s)')
#     ax[1].legend()
#     ax[1].axhline(y=0, color='gray', linestyle='--') # Add horizontal grid line at y=0
#     ax[1].yaxis.grid(True, which='both', linestyle='-', linewidth=0.5) # Plot only horizontal grid lines for throughput
#     ax[1].xaxis.grid(True, linestyle='-', linewidth=0.5) # Plot
#
#     plt.show()
# else:
#     print("No data to plot.")



# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
# sim_dirs = ['30', '40', '50', '60', '70', '80', '100', '120']  # Update simulation directory names accordingly
#
# interest_satisfaction_ratios = []
# content_hit_ratios = []
# interest_satisfaction_ratio_errors = []
# content_hit_ratio_errors = []
# throughputs = []
#
# for sim_dir in sim_dirs:
#     dir = os.path.join(parent_dir, sim_dir)
#     interest_satisfaction_ratio_avg = 0
#     content_hit_ratio_avg = 0
#     interest_satisfaction_ratio_std = 0
#     content_hit_ratio_std = 0
#     num_runs = 0
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
#             continue
#
#         trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#
#         satisfied_count = 0
#         timedout_count = 0
#         total_count = 0
#         cache_hit_count = 0
#         total_hit_count = 0
#         throughput = 0
#
#         with open(trace_file, 'r') as f:
#             lines = f.readlines()
#             for line in lines:
#                 if "SatisfiedInterests" in line:
#                     satisfied_count += float(line.split("\t")[5])
#                 if "TimedOutInterests" in line and len(line.split("\t")) > 4:
#                     timedout_count += float(line.split("\t")[5])
#                 if "InInterests" in line and "netdev://[00:" in line and len(line.split("\t")) > 4:
#                     total_count += float(line.split("\t")[5])
#
#                 if "OutData" in line and "netdev://[00:" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     cache_hit_count += float(line.split("\t")[5])
#                 if "OutInterests" in line and "netdev://[00:" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     total_hit_count += float(line.split("\t")[5])
#
#             time = float(lines[-1].split('\t')[0])
#             if time > 0:
#                 throughput = (satisfied_count+cache_hit_count)/time
#             else:
#                 throughput = 0
#
#         # interest_satisfaction_ratio_avg += satisfied_count/timedout_count if timedout_count > 0 else 0
#         interest_satisfaction_ratio_avg += satisfied_count/total_hit_count if timedout_count > 0 else 0
#         content_hit_ratio_avg += cache_hit_count/total_hit_count if total_hit_count > 0 else 0
#         num_runs += 1
#
#         # Add calculated throughput to list
#         throughputs.append(throughput)
#
#     if num_runs > 0:
#         interest_satisfaction_ratio_avg /= num_runs
#         content_hit_ratio_avg /= num_runs
#         interest_satisfaction_ratio_errors.append(np.std(interest_satisfaction_ratios))
#         content_hit_ratio_errors.append(np.std(content_hit_ratios))
#     else:
#         interest_satisfaction_ratio_errors.append(0)
#         content_hit_ratio_errors.append(0)
#     interest_satisfaction_ratios.append(interest_satisfaction_ratio


# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
# sim_dirs = ['30', '40', '50', '60', '70', '80', '100', '120']  # Update simulation directory names accordingly
#
# interest_satisfaction_ratios = []
# content_hit_ratios = []
# throughputs = []
# interest_satisfaction_ratio_errors = []
# content_hit_ratio_errors = []
# throughput_errors = []
#
# for sim_dir in sim_dirs:
#     dir = os.path.join(parent_dir, sim_dir)
#     interest_satisfaction_ratio_avg = 0
#     content_hit_ratio_avg = 0
#     throughput_avg = 0
#     interest_satisfaction_ratio_std = 0
#     content_hit_ratio_std = 0
#     throughput_std = 0
#     num_runs = 0
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
#             continue
#
#         trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#
#         satisfied_count = 0
#         timedout_count = 0
#         total_count = 0
#         cache_hit_count = 0
#         total_hit_count = 0
#         received_bytes = 0
#         elapsed_time = 0
#
#         with open(trace_file, 'r') as f:
#             lines = f.readlines()
#             for line in lines:
#                 if "SatisfiedInterests" in line:
#                     satisfied_count += float(line.split("\t")[5])
#                 if "TimedOutInterests" in line and len(line.split("\t")) > 4:
#                     timedout_count += float(line.split("\t")[5])
#                 if "InInterests" in line and "netdev://[00:" in line and len(line.split("\t")) > 4:
#                     total_count += float(line.split("\t")[5])
#
#                 if "OutData" in line and "netdev://[00:" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     cache_hit_count += float(line.split("\t")[5])
#                 if "OutInterests" in line and "netdev://[00:" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     total_hit_count += float(line.split("\t")[5])
#
#                 if "node0" in line and "appFace: /localhost/nfd" in line:
#                     received_bytes += float(line.split("\t")[9])
#                     elapsed_time = 250 #float(line.split("\t")[1])
#
#         throughput = received_bytes * 8 / elapsed_time / 1000000  # convert bytes to bits and seconds to Megabits
#         interest_satisfaction_ratio_avg += satisfied_count/total_hit_count if timedout_count > 0 else 0
#         content_hit_ratio_avg += cache_hit_count/total_hit_count if total_hit_count > 0 else 0
#         throughput_avg += throughput
#         num_runs += 1
#
#         if num_runs > 0:
#                 interest_satisfaction_ratio_avg /= num_runs
#                 content_hit_ratio_avg /= num_runs
#                 throughput_avg /= num_runs
#                 interest_satisfaction_ratio_errors.append(np.std(interest_satisfaction_ratios))
#                 content_hit_ratio_errors.append(np.std(content_hit_ratios))
#                 throughput_errors.append(np.std(throughputs))
#
#                 print("\n==================RESULTS==================")
#                 print(f"Interest Satisfaction Ratio: {interest_satisfaction_ratio_avg:.2f} +/- {interest_satisfaction_ratio_errors[-1]:.2f}")
#                 print(f"Content Hit Ratio: {content_hit_ratio_avg:.2f} +/- {content_hit_ratio_errors[-1]:.2f}")
#                 print(f"Throughput (in queries per second): {throughput_avg:.2f} +/- {throughput_errors[-1]:.2f}")
#
#                 # return (interest_satisfaction_ratio_avg, content_hit_ratio_avg, throughput_avg), \
#                 #     (interest_satisfaction_ratio_errors, content_hit_ratio_errors, throughput_errors)
#         else:
#             print("No runs were successful.")
#             # return None, None
