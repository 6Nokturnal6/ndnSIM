# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# def calculate_satisfaction_ratio(tracer_file):
#     satisfied_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "SatisfiedInterests" in line:
#                 satisfied_count += float(line.split("\t")[6])
#             if "InInterests" in line and len(line.split("\t")) > 5:
#                 total_count += float(line.split("\t")[5])
#     return satisfied_count/total_count if total_count > 0 else 0
#
# def calculate_cache_hit_ratio(tracer_file):
#     cache_hit_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "OutData" in line:
#                 cache_hit_count += float(line.split("\t")[5])
#             if "OutInterests" in line:
#                 total_count += float(line.split("\t")[5])
#     return cache_hit_count/total_count
#
# parent_dir = './'
# sim_dirs = [os.path.join(parent_dir, d) for d in os.listdir(parent_dir) if os.path.isdir(os.path.join(parent_dir, d))]
#
# satisfaction_ratios = []
# cache_hit_ratios = []
#
# for sim_dir in sim_dirs:
#     tracer_file = os.path.join(sim_dir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#     satisfaction_ratio = calculate_satisfaction_ratio(tracer_file)
#     cache_hit_ratio = calculate_cache_hit_ratio(tracer_file)
#     satisfaction_ratios.append(satisfaction_ratio)
#     cache_hit_ratios.append(cache_hit_ratio)
#
# mean_satisfaction_ratio = np.mean(satisfaction_ratios)
# std_satisfaction_ratio = np.std(satisfaction_ratios)
# mean_cache_hit_ratio = np.mean(cache_hit_ratios)
# std_cache_hit_ratio = np.std(cache_hit_ratios)
#
# print("Interest Satisfaction Ratio: {:.2f} +/- {:.2f}".format(mean_satisfaction_ratio, std_satisfaction_ratio))
# print("Cache Hit Ratio: {:.2f} +/- {:.2f}".format(mean_cache_hit_ratio, std_cache_hit_ratio))
#
# fig, ax = plt.subplots()
# ax.errorbar(range(len(satisfaction_ratios)), satisfaction_ratios, yerr=std_satisfaction_ratio, fmt='o', label='Interest Satisfaction Ratio')
# ax.errorbar(range(len(cache_hit_ratios)), cache_hit_ratios, yerr=std_cache_hit_ratio, fmt='o', label='Cache Hit Ratio')
# ax.set_xticks(range(len(satisfaction_ratios)))
# ax.set_xticklabels([str(i+1) for i in range(len(satisfaction_ratios))])
# ax.set_xlabel('Simulation Number')
# ax.set_ylabel('Ratio')
# ax.legend()
# plt.show()


# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# def calculate_satisfaction_ratio(tracer_file):
#     satisfied_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "SatisfiedInterests" in line:
#                 satisfied_count += float(line.split("\t")[6])
#             if "InInterests" in line and len(line.split("\t")) > 5:
#                 total_count += float(line.split("\t")[5])
#     return satisfied_count/total_count if total_count > 0 else 0
#
# def calculate_cache_hit_ratio(tracer_file):
#     cache_hit_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "OutData" in line:
#                 cache_hit_count += float(line.split("\t")[5])
#             if "OutInterests" in line:
#                 total_count += float(line.split("\t")[5])
#     return cache_hit_count/total_count
#
# parent_dir = './'
# sim_dirs = []
# for dir_name in ['60', '100', '70']:
#     for subdir in os.listdir(os.path.join(parent_dir, dir_name)):
#         if subdir.startswith("RngRun_"):
#             sim_dirs.append(os.path.join(parent_dir, dir_name, subdir))
#
# satisfaction_ratios = []
# cache_hit_ratios = []
#
# for sim_dir in sim_dirs:
#     tracer_file = os.path.join(sim_dir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#     satisfaction_ratio = calculate_satisfaction_ratio(tracer_file)
#     cache_hit_ratio = calculate_cache_hit_ratio(tracer_file)
#     satisfaction_ratios.append(satisfaction_ratio)
#     cache_hit_ratios.append(cache_hit_ratio)
#
# mean_satisfaction_ratio = np.mean(satisfaction_ratios)
# std_satisfaction_ratio = np.std(satisfaction_ratios)
# mean_cache_hit_ratio = np.mean(cache_hit_ratios)
# std_cache_hit_ratio = np.std(cache_hit_ratios)
#
# print("Interest Satisfaction Ratio: {:.2f} +/- {:.2f}".format(mean_satisfaction_ratio, std_satisfaction_ratio))
# print("Cache Hit Ratio: {:.2f} +/- {:.2f}".format(mean_cache_hit_ratio, std_cache_hit_ratio))
#
# fig, ax = plt.subplots()
# ax.errorbar(range(len(satisfaction_ratios)), satisfaction_ratios, yerr=std_satisfaction_ratio, fmt='o', label='Interest Satisfaction Ratio')
# ax.errorbar(range(len(cache_hit_ratios)), cache_hit_ratios, yerr=std_cache_hit_ratio, fmt='o', label='Cache Hit Ratio')
# ax.set_xticks(range(len(satisfaction_ratios)))
# ax.set_xticklabels([str(i+1) for i in range(len(satisfaction_ratios))])
# ax.set_xlabel('Simulation Number')
# ax.set_ylabel('Ratio')
# ax.legend()
# plt.show()

# xxxx
# import os
# # import glob
# import numpy as np
# import matplotlib.pyplot as plt
#
#
# def calculate_interest_satisfaction_ratio(tracer_file):
#     satisfied_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "SatisfiedInterests" in line:
#                 satisfied_count += float(line.split("\t")[6])
#             if "InInterests" in line and len(line.split("\t")) > 5:
#                 total_count += float(line.split("\t")[5])
#     return satisfied_count/total_count if total_count > 0 else 0
#
# def calculate_content_hit_ratio(tracer_file):
#     cache_hit_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "OutData" in line:
#                 cache_hit_count += float(line.split("\t")[5])
#             if "OutInterests" in line:
#                 total_count += float(line.split("\t")[5])
#     return cache_hit_count/total_count
#
# parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
# sim_dirs = ['60', '70', '100']  # Update simulation directory names accordingly
#
# interest_satisfaction_ratios = []
# content_hit_ratios = []
#
# for sim_dir in sim_dirs:
#     dir = os.path.join(parent_dir, sim_dir)
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
#             continue
#
#         trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#
#         # Read the trace file into an array
#         trace_data = []
#         with open(trace_file) as f:
#             next(f)  # skip header row
#             for line in f:
#                 trace_data.append(line.strip().split())
#
#         interest_satisfaction_ratio = calculate_interest_satisfaction_ratio(trace_file)
#         content_hit_ratio = calculate_content_hit_ratio(trace_file)
#         interest_satisfaction_ratios.append(interest_satisfaction_ratio)
#         content_hit_ratios.append(content_hit_ratio)
#
# if interest_satisfaction_ratios and content_hit_ratios:
#     fig, ax = plt.subplots()
#     ax.plot(range(len(interest_satisfaction_ratios)), interest_satisfaction_ratios, marker='o', label='Interest Satisfaction Ratio')
#     ax.plot(range(len(content_hit_ratios)), content_hit_ratios, marker='o', label='Content Hit Ratio')
#     # ax.set_xticks(range(len(sim_dirs)))
#     # ax.set_xticklabels(sim_dirs)
#     ax.set_xticks(range(len(sim_dirs)))
#     ax.set_xticklabels(sim_dirs, rotation=45)  # Specify the rotation angle for x-axis labels
#     ax.set_xlabel('Simulation Directory')
#     ax.set_ylabel('Ratio')
#     ax.legend()
#     plt.show()
# else:
#     print("No data to plot.")


# import matplotlib.pyplot as plt
# import os
#
# # List of simulation directories
# sim_dirs = ['100', '60', '70']
#
# # Create empty lists to store the average data and labels for each simulation
# avg_data = []
# labels = []
#
# # Loop through the simulation directories
# for sim_dir in sim_dirs:
#     # Loop through the RngRun subdirectories in the current simulation directory
#     sim_data = []
#     for subdir in os.listdir(sim_dir):
#         if subdir.startswith('RngRun'):
#             # Update this line with your code to read data from the trace file in the current subdirectory
#             # and append it to the sim_data list
#             trace_file = os.path.join(sim_dir, subdir, 'trace.txt')
#             # Assuming each line in the trace file contains a single data point, you can read it like this:
#             with open(trace_file, 'r') as f:
#                 for line in f:
#                     sim_data.append(float(line.strip()))  # Append the data point to sim_data list
#     avg = sum(sim_data) / len(sim_data)  # Calculate the average of the data for the current simulation
#     avg_data.append(avg)  # Append the average to avg_data list
#     labels.append(sim_dir)  # Append the simulation directory label to labels list
#
# # Create a figure and axis
# fig, ax = plt.subplots()
#
# # Plot the average data as individual points on the axis
# ax.plot(labels, avg_data, 'o')
#
# # Set y-axis label
# ax.set_ylabel('Average Data')
#
# # Show the plot
# plt.show()


# YY
# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# def calculate_interest_satisfaction_ratio(tracer_file):
#     satisfied_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "SatisfiedInterests" in line:
#                 satisfied_count += float(line.split("\t")[6])
#             if "InInterests" in line and len(line.split("\t")) > 5:
#                 total_count += float(line.split("\t")[5])
#     return satisfied_count/total_count if total_count > 0 else 0
#
# parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
# sim_dirs = ['60', '70', '100']  # Update simulation directory names accordingly
#
# interest_satisfaction_ratios = []
#
# for sim_dir in sim_dirs:
#     dir = os.path.join(parent_dir, sim_dir)
#     avg_interest_satisfaction_ratio = 0
#     num_runs = 0
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
#             continue
#
#         trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#         interest_satisfaction_ratio = calculate_interest_satisfaction_ratio(trace_file)
#         avg_interest_satisfaction_ratio += interest_satisfaction_ratio
#         num_runs += 1
#
#     if num_runs > 0:
#         avg_interest_satisfaction_ratio /= num_runs
#         interest_satisfaction_ratios.append(avg_interest_satisfaction_ratio)
#
# if interest_satisfaction_ratios:
#     fig, ax = plt.subplots()
#     ax.plot(sim_dirs, interest_satisfaction_ratios, marker='o', label='Interest Satisfaction Ratio')
#     ax.set_xlabel('Simulation Directory')
#     ax.set_ylabel('Ratio')
#     ax.legend()
#     plt.show()
# else:
#     print("No data to plot.")





























# # Does not use only the "All"
# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# def calculate_interest_satisfaction_ratio(tracer_file):
#     satisfied_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "SatisfiedInterests" in line:
#                 satisfied_count += float(line.split("\t")[6])
#             if "InInterests" in line and len(line.split("\t")) > 5:
#                 total_count += float(line.split("\t")[5])
#     return satisfied_count/total_count if total_count > 0 else 0
#
# def calculate_content_hit_ratio(tracer_file):
#     cache_hit_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "OutData" in line:
#                 cache_hit_count += float(line.split("\t")[5])
#             if "OutInterests" in line:
#                 total_count += float(line.split("\t")[5])
#     return cache_hit_count/total_count
#
# parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
# sim_dirs = ['60', '70', '100']  # Update simulation directory names accordingly
#
# interest_satisfaction_ratios = []
# content_hit_ratios = []
#
# for sim_dir in sim_dirs:
#     dir = os.path.join(parent_dir, sim_dir)
#     interest_satisfaction_ratio_avg = 0
#     content_hit_ratio_avg = 0
#     num_runs = 0
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
#             continue
#
#         trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#
#         # Read the trace file into an array
#         trace_data = []
#         with open(trace_file) as f:
#             next(f)  # skip header row
#             for line in f:
#                 trace_data.append(line.strip().split())
#
#         interest_satisfaction_ratio_avg += calculate_interest_satisfaction_ratio(trace_file)
#         content_hit_ratio_avg += calculate_content_hit_ratio(trace_file)
#         num_runs += 1
#
#     if num_runs > 0:
#         interest_satisfaction_ratio_avg /= num_runs
#         content_hit_ratio_avg /= num_runs
#     interest_satisfaction_ratios.append(interest_satisfaction_ratio_avg)
#     content_hit_ratios.append(content_hit_ratio_avg)
#
# if interest_satisfaction_ratios and content_hit_ratios:
#     fig, ax = plt.subplots()
#     ax.plot(sim_dirs, interest_satisfaction_ratios, marker='o', label='Interest Satisfaction Ratio')
#     ax.plot(sim_dirs, content_hit_ratios, marker='o', label='Content Hit Ratio')
#     ax.set_xlabel('Simulation Directory')
#     ax.set_ylabel('Ratio')
#     ax.legend()
#     plt.show()
# else:
#     print("No data to plot.")





# ## "Includes" "All"
#
# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# def calculate_interest_satisfaction_ratio(tracer_file):
#     satisfied_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "SatisfiedInterests" in line or "all\tSatisfiedInterests" in line:
#                 elements = line.strip().split("\t")
#                 if len(elements) > 6:
#                     satisfied_count += float(elements[6])
#             if "InInterests" in line and len(line.split("\t")) > 5:
#                 elements = line.strip().split("\t")
#                 if len(elements) > 5:
#                     total_count += float(elements[5])
#             if "TimedOutInterests" in line or "all\tTimedOutInterests" in line:
#                 elements = line.strip().split("\t")
#                 if len(elements) > 6:
#                     total_count += float(elements[6])
#     return satisfied_count/total_count if total_count > 0 else 0
#
# def calculate_content_hit_ratio(tracer_file):
#     cache_hit_count = 0
#     total_count = 0
#     with open(tracer_file, 'r') as f:
#         lines = f.readlines()
#         for line in lines:
#             if "OutData" in line:
#                 cache_hit_count += float(line.split("\t")[5])
#             if "OutInterests" in line:
#                 total_count += float(line.split("\t")[5])
#     return cache_hit_count/total_count
#
# parent_dir = './'  # Update parent directory to the correct directory where simulation results are stored
# sim_dirs = ['60', '70', '100']  # Update simulation directory names accordingly
#
# interest_satisfaction_ratios = []
# content_hit_ratios = []
#
# for sim_dir in sim_dirs:
#     dir = os.path.join(parent_dir, sim_dir)
#     interest_satisfaction_ratio_avg = 0
#     content_hit_ratio_avg = 0
#     num_runs = 0
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
#             continue
#
#         trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#
#         # Read the trace file into an array
#         trace_data = []
#         with open(trace_file) as f:
#             next(f)  # skip header row
#             for line in f:
#                 trace_data.append(line.strip().split())
#
#         interest_satisfaction_ratio_avg += calculate_interest_satisfaction_ratio(trace_file)
#         content_hit_ratio_avg += calculate_content_hit_ratio(trace_file)
#         num_runs += 1
#
#     if num_runs > 0:
#         interest_satisfaction_ratio_avg /= num_runs
#         content_hit_ratio_avg /= num_runs
#     interest_satisfaction_ratios.append(interest_satisfaction_ratio_avg)
#     content_hit_ratios.append(content_hit_ratio_avg)
#
# if interest_satisfaction_ratios and content_hit_ratios:
#     fig, ax = plt.subplots()
#     ax.plot(sim_dirs, interest_satisfaction_ratios, marker='o', label='Interest Satisfaction Ratio')
#     ax.plot(sim_dirs, content_hit_ratios, marker='o', label='Content Hit Ratio')
#     ax.set_xlabel('Simulation Directory')
#     ax.set_ylabel('Ratio')
#     ax.legend()
#     plt.show()
# else:
#     print("No data to plot.")




# #  Uses "only" the "All" lines
# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
# sim_dirs = ['30', '60', '70', '80', '100']  # Update simulation directory names accordingly
#
# interest_satisfaction_ratios = []
# content_hit_ratios = []
#
# for sim_dir in sim_dirs:
#     dir = os.path.join(parent_dir, sim_dir)
#     interest_satisfaction_ratio_avg = 0
#     content_hit_ratio_avg = 0
#     num_runs = 0
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
#             continue
#
#         trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#
#         satisfied_count = 0
#         total_count = 0
#         cache_hit_count = 0
#         total_hit_count = 0
#
#         with open(trace_file, 'r') as f:
#             lines = f.readlines()
#             for line in lines:
#                 if "SatisfiedInterests" in line:
#                     satisfied_count += float(line.split("\t")[6])
#                 if "InInterests" in line and len(line.split("\t")) > 5:
#                     total_count += float(line.split("\t")[5])
#                 if "OutData" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     cache_hit_count += float(line.split("\t")[5])
#                 if "OutInterests" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     total_hit_count += float(line.split("\t")[5])
#
#         interest_satisfaction_ratio_avg += satisfied_count/total_count if total_count > 0 else 0
#         content_hit_ratio_avg += cache_hit_count/total_hit_count if total_hit_count > 0 else 0
#         num_runs += 1
#
#     if num_runs > 0:
#         interest_satisfaction_ratio_avg /= num_runs
#         content_hit_ratio_avg /= num_runs
#     interest_satisfaction_ratios.append(interest_satisfaction_ratio_avg)
#     content_hit_ratios.append(content_hit_ratio_avg)
#
# if interest_satisfaction_ratios and content_hit_ratios:
#     fig, ax = plt.subplots()
#     ax.plot(sim_dirs, interest_satisfaction_ratios, marker='o', label='Interest Satisfaction Ratio (ISR)')
#     ax.plot(sim_dirs, content_hit_ratios, marker='o', label='Content Hit Ratio (CHR)')
#     ax.set_xlabel('Number of vehicles')
#     ax.set_ylabel('Ratio')
#     ax.legend()
#     plt.show()
# else:
#     print("No data to plot.")












# ISR to be updated -- Corrected...

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
#
#         with open(trace_file, 'r') as f:
#             lines = f.readlines()
#             for line in lines:
#                 if "SatisfiedInterests" in line:
#                     satisfied_count += float(line.split("\t")[5])
#                 if "TimedOutInterests" in line and len(line.split("\t")) > 4:
#                     timedout_count += float(line.split("\t")[5])
#                 if "InInterests" in line and len(line.split("\t")) > 4:
#                     total_count += float(line.split("\t")[5])
#                 if "OutData" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     cache_hit_count += float(line.split("\t")[5])
#                 if "OutInterests" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     total_hit_count += float(line.split("\t")[5])
#
#         interest_satisfaction_ratio_avg += satisfied_count/timedout_count if timedout_count > 0 else 0
#         content_hit_ratio_avg += cache_hit_count/total_hit_count if total_hit_count > 0 else 0
#         num_runs += 1
#
#     if num_runs > 0:
#         interest_satisfaction_ratio_avg /= num_runs
#         content_hit_ratio_avg /= num_runs
#         interest_satisfaction_ratio_errors.append(np.std(interest_satisfaction_ratios))
#         content_hit_ratio_errors.append(np.std(content_hit_ratios))
#     else:
#         interest_satisfaction_ratio_errors.append(0)
#         content_hit_ratio_errors.append(0)
#     interest_satisfaction_ratios.append(interest_satisfaction_ratio_avg)
#     content_hit_ratios.append(content_hit_ratio_avg)
#
# if interest_satisfaction_ratios and content_hit_ratios:
#     fig, ax = plt.subplots()
#     ax.errorbar(sim_dirs, interest_satisfaction_ratios, yerr=interest_satisfaction_ratio_errors, marker='o', label='Interest Satisfaction Ratio (ISR)')
#     ax.errorbar(sim_dirs, content_hit_ratios, yerr=content_hit_ratio_errors, marker='o', label='Content Hit Ratio (CHR)')
#     ax.set_xlabel('Number of vehicles')
#     ax.set_ylabel('Ratio')
#     ax.legend()
#     # ax.grid(True)
#     plt.show()
# else:
#     print("No data to plot.")



# #  Updating ISR formula (Not correct)

# import os
# import numpy as np
# import matplotlib.pyplot as plt
#
# parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
# sim_dirs = ['30', '60', '70', '80', '100']  # Update simulation directory names accordingly
#
# interest_satisfaction_ratios = []
# content_hit_ratios = []
# interest_satisfaction_ratio_errors = []
# content_hit_ratio_errors = []
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
#         total_count = 0
#         cache_hit_count = 0
#         total_hit_count = 0
#         total_isr_count = 0
#
#         with open(trace_file, 'r') as f:
#             lines = f.readlines()
#             for line in lines:
#                 if "InData" in line and 'appFace://' in line:
#                     total_count += float(line.split("\t")[5])
#                 if "OutInterests" in line and 'netdev://' in line:
#                     total_hit_count += 1
#                 if "OutInterest" in line and 'appFace://' in line:
#                     total_isr_count += float(line.split("\t")[5])
#                 if "SatisfiedInterests" in line:
#                     satisfied_count += float(line.split("\t")[6])
#                 if "OutData" in line and len(line.split("\t")) > 5 and line.split("\t")[5] == 'appFace://':
#                     cache_hit_count += 1
#
#         interest_satisfaction_ratio_avg += total_count/total_isr_count if total_isr_count > 0 else 0
#         content_hit_ratio_avg += cache_hit_count/total_hit_count if total_hit_count > 0 else 0
#         num_runs += 1
#
#     if num_runs > 0:
#         interest_satisfaction_ratio_avg /= num_runs
#         content_hit_ratio_avg /= num_runs
#         interest_satisfaction_ratio_errors.append(np.std(interest_satisfaction_ratios))
#         content_hit_ratio_errors.append(np.std(content_hit_ratios))
#     else:
#         interest_satisfaction_ratio_errors.append(0)
#         content_hit_ratio_errors.append(0)
#     interest_satisfaction_ratios.append(interest_satisfaction_ratio_avg)
#     content_hit_ratios.append(content_hit_ratio_avg)
#
# if interest_satisfaction_ratios and content_hit_ratios:
#     fig, ax = plt.subplots()
#     ax.errorbar(sim_dirs, interest_satisfaction_ratios, yerr=interest_satisfaction_ratio_errors, marker='o', label='Interest Satisfaction Ratio (ISR)')
#     ax.errorbar(sim_dirs, content_hit_ratios, yerr=content_hit_ratio_errors, marker='o', label='Content Hit Ratio (CHR)')
#     ax.set_xlabel('Number of vehicles')
#     ax.set_ylabel('Ratio')
#     ax.legend()
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
#
#         with open(trace_file, 'r') as f:
#             lines = f.readlines()
#             for line in lines:
#                 if "SatisfiedInterests" in line:
#                     satisfied_count += float(line.split("\t")[5])
#                 if "TimedOutInterests" in line and len(line.split("\t")) > 4:
#                     timedout_count += float(line.split("\t")[5])
#                 if "InInterests" in line and len(line.split("\t")) > 4:
#                     total_count += float(line.split("\t")[5])
#                 if "OutData" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     cache_hit_count += float(line.split("\t")[5])
#                 if "OutInterests" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
#                     total_hit_count += float(line.split("\t")[5])
#
#         interest_satisfaction_ratio_avg += satisfied_count/timedout_count if timedout_count > 0 else 0
#         content_hit_ratio_avg += cache_hit_count/total_hit_count if total_hit_count > 0 else 0
#         num_runs += 1
#
#     if num_runs > 0:
#         interest_satisfaction_ratio_avg /= num_runs
#         content_hit_ratio_avg /= num_runs
#         interest_satisfaction_ratio_errors.append(np.std(interest_satisfaction_ratios))
#         content_hit_ratio_errors.append(np.std(content_hit_ratios))
#     else:
#         interest_satisfaction_ratio_errors.append(0)
#         content_hit_ratio_errors.append(0)
#     interest_satisfaction_ratios.append(interest_satisfaction_ratio_avg * 100)
#     content_hit_ratios.append(content_hit_ratio_avg * 100)
#
# if interest_satisfaction_ratios and content_hit_ratios:
#     fig, ax = plt.subplots()
#     ax.errorbar(sim_dirs, interest_satisfaction_ratios, yerr=interest_satisfaction_ratio_errors, marker='o', label='Interest Satisfaction Ratio (ISR)')
#     ax.errorbar(sim_dirs, content_hit_ratios, yerr=content_hit_ratio_errors, marker='o', label='Content Hit Ratio (CHR)')
#     ax.set_xlabel('Number of vehicles (in a period of 450s - 150s = 300s)')
#     ax.set_ylabel('Ratio (%)')
#     ax.legend()
#     ax.grid(True)
#     plt.show()
# else:
#     print("No data to plot.")
#



import os
import numpy as np
import matplotlib.pyplot as plt

parent_dir = '/home/dasilva/PDEEC2021/testingENV/MobilityModels_OSM_Data_20230105/simulResultsTest'  # Update parent directory to the correct directory where simulation results are stored
sim_dirs = ['30', '40', '50', '60', '70', '80', '100', '120']  # Update simulation directory names accordingly

interest_satisfaction_ratios = []
content_hit_ratios = []
interest_satisfaction_ratio_errors = []
content_hit_ratio_errors = []

for sim_dir in sim_dirs:
    dir = os.path.join(parent_dir, sim_dir)
    interest_satisfaction_ratio_avg = 0
    content_hit_ratio_avg = 0
    interest_satisfaction_ratio_std = 0
    content_hit_ratio_std = 0
    num_runs = 0
    for subdir in os.listdir(dir):
        if not os.path.isdir(os.path.join(dir, subdir)) or not subdir.startswith("RngRun_"):
            continue

        trace_file = os.path.join(dir, subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')

        satisfied_count = 0
        timedout_count = 0
        total_count = 0
        cache_hit_count = 0
        total_hit_count = 0

        with open(trace_file, 'r') as f:
            lines = f.readlines()
            for line in lines:
                if "SatisfiedInterests" in line:
                    satisfied_count += float(line.split("\t")[5])
                if "TimedOutInterests" in line and len(line.split("\t")) > 4:
                    timedout_count += float(line.split("\t")[5])
                if "InInterests" in line and "netdev://[00:" in line and len(line.split("\t")) > 4:
                    total_count += float(line.split("\t")[5])

                if "OutData" in line and "netdev://[00:" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
                    cache_hit_count += float(line.split("\t")[5])
                if "OutInterests" in line and "netdev://[00:" in line and len(line.split("\t")) > 5 and line.split("\t")[5] != '':
                    total_hit_count += float(line.split("\t")[5])

        # interest_satisfaction_ratio_avg += satisfied_count/timedout_count if timedout_count > 0 else 0
        interest_satisfaction_ratio_avg += satisfied_count/total_hit_count if timedout_count > 0 else 0
        content_hit_ratio_avg += cache_hit_count/total_hit_count if total_hit_count > 0 else 0
        num_runs += 1

    if num_runs > 0:
        interest_satisfaction_ratio_avg /= num_runs
        content_hit_ratio_avg /= num_runs
        interest_satisfaction_ratio_errors.append(np.std(interest_satisfaction_ratios))
        content_hit_ratio_errors.append(np.std(content_hit_ratios))
    else:
        interest_satisfaction_ratio_errors.append(0)
        content_hit_ratio_errors.append(0)
    interest_satisfaction_ratios.append(interest_satisfaction_ratio_avg * 100)
    content_hit_ratios.append(content_hit_ratio_avg * 100)

if interest_satisfaction_ratios and content_hit_ratios:
    fig, ax = plt.subplots(2, 1, sharex=True, figsize=(8, 8))

    # Plot Interest Satisfaction Ratio (ISR)
    ax[0].errorbar(sim_dirs, interest_satisfaction_ratios, yerr=interest_satisfaction_ratio_errors, marker='o', label='Interest Satisfaction Ratio (ISR)')
    ax[0].set_ylabel('ISR (%)')
    ax[0].legend()
    # ax[0].grid(True)
    ax[0].axhline(y=0, color='gray', linestyle='--') # Add horizontal grid line at y=0
    ax[0].yaxis.grid(True, which='both', linestyle='-', linewidth=0.5) # Plot only horizontal grid lines for CHR
    ax[0].xaxis.grid(False) # Turn off vertical grid lines

    # Plot Content Hit Ratio (CHR)
    ax[1].errorbar(sim_dirs, content_hit_ratios, yerr=content_hit_ratio_errors, marker='o', label='Content Hit Ratio (CHR)')
    ax[1].set_xlabel('Number of vehicles (in a period of 470s - 220s = 250s)')
    ax[1].set_ylabel('CHR (%)')
    ax[1].legend()
    # ax[1].grid(True)
    ax[1].axhline(y=0, color='gray', linestyle='--') # Add horizontal grid line at y=0
    ax[1].yaxis.grid(True, which='both', linestyle='-', linewidth=0.5) # Plot only horizontal grid lines for CHR
    ax[1].xaxis.grid(False) # Turn off vertical grid lines

    plt.show()
else:
    print("No data to plot.")

