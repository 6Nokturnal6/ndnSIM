# import os
# import matplotlib.pyplot as plt
# import numpy as np
#
# from matplotlib.ticker import ScalarFormatter
#
# # Define the name of the trace file
# trace_file = "delay-ndn-simple-wifi3-WithBeacon_20230109.txt"
#
# # Define the columns to extract from the trace file
# columns = "Time Node AppId SeqNo Type DelayS DelayUS RetxCount HopCount"
#
# # Initialize lists to store the average latency and jitter values, and simulation IDs
# avg_latency_list = []
# jitter_list = []
# sim_id_list = []
# std_latency_list = []
#
# last_avg_latency_list = []
# last_jitter_list = []
#
# # Loop over each simulation directory (e.g., 100, 60, 70)
# for dir in os.listdir("."):
#     if not os.path.isdir(dir) or dir.startswith("."):
#         continue
#     print(f"Processing {dir}")
#
#     # Initialize variables for calculating latency and jitter
#     total_latency = 0
#     num_packets_received = 0
#     latency_values = []
#
#     last_total_latency = 0
#     last_num_packets_received = 0
#     last_latency_values = []
#
#     # Loop over each RngRun_* directory inside the current simulation directory
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)):
#             continue
#
#         # Read the trace file into an array
#         trace_data = []
#         with open(f"{dir}/{subdir}/{trace_file}") as f:
#             # next(f)  # skip header row
#             for line in f:
#                 fields = line.strip().split()
#                 if len(fields) >= 6:
#                     # Extract the DelayS field and calculate the latency
#                     delay_s = float(fields[5])
#                     if 'FullDelay' in line:
#                         latency_values.append(delay_s)
#                     elif 'LastDelay' in line:
#                         last_latency_values.append(delay_s)
#
#     # Calculate the average latency, jitter, and standard deviation for the current simulation directory for FullDelay
#     avg_latency = np.mean(latency_values)
#     var_latency = np.var(latency_values)
#     jitter = np.sqrt(var_latency)
#     std_latency = np.std(latency_values)
#
#     print(f"Average latency for {dir} FullDelay: {avg_latency:.6f} seconds")
#     print(f"Jitter for {dir} FullDelay: {jitter:.6f} seconds")
#     print(f"Standard deviation of latency for {dir} FullDelay: {std_latency:.6f} seconds")
#
#     # Append the average latency, jitter, and simulation ID to the lists for FullDelay
#     avg_latency_list.append(avg_latency)
#     jitter_list.append(jitter)
#     sim_id_list.append(dir)
#     std_latency_list.append(std_latency)
#
#     # Calculate the average latency, jitter, and standard deviation for the current simulation directory for LastDelay
#     last_avg_latency = np.mean(last_latency_values)
#     last_var_latency = np.var(last_latency_values)
#     last_jitter = np.sqrt(last_var_latency)
#     last_std_latency = np.std(last_latency_values)
#
#     print(f"Average latency for {dir} LastDelay: {last_avg_latency:.6f} seconds")
#     print(f"Jitter for {dir} LastDelay: {last_jitter:.6f} seconds")
#     print(f"Standard deviation of latency for {dir} LastDelay: {last_std_latency:.6f} seconds")
#
#     # Append the average latency, jitter, and simulation ID to the lists for LastDelay
#     last_avg_latency_list.append
#
#
#
#
#
#
#     import os
# import matplotlib.pyplot as plt
# import numpy as np
#
# from matplotlib.ticker import ScalarFormatter
#
# # Define the name of the trace file
# trace_file = "delay-ndn-simple-wifi3-WithBeacon_20230109.txt"
#
# # Define the columns to extract from the trace file
# columns = "Time Node AppId SeqNo Type DelayS DelayUS RetxCount HopCount"
#
# # Initialize lists to store the average latency and jitter values, and simulation IDs
# avg_latency_list = []
# jitter_list = []
# sim_id_list = []
# std_latency_list = []
#
# last_avg_latency_list = []
# last_jitter_list = []
#
# # Loop over each simulation directory (e.g., 100, 60, 70)
# for dir in os.listdir("."):
#     if not os.path.isdir(dir) or dir.startswith("."):
#         continue
#     print(f"Processing {dir}")
#
#     # Initialize variables for calculating latency and jitter
#     total_latency = 0
#     num_packets_received = 0
#     latency_values = []
#
#     last_total_latency = 0
#     last_num_packets_received = 0
#     last_latency_values = []
#
#     # Loop over each RngRun_* directory inside the current simulation directory
#     for subdir in os.listdir(dir):
#         if not os.path.isdir(os.path.join(dir, subdir)):
#             continue
#
#         # Read the trace file into an array
#         trace_data = []
#         with open(f"{dir}/{subdir}/{trace_file}") as f:
#             # next(f)  # skip header row
#             for line in f:
#                 fields = line.strip().split()
#                 if len(fields) >= 6:
#                     # Extract the DelayS field and calculate the latency
#                     delay_s = float(fields[5])
#                     if 'FullDelay' in line:
#                         latency_values.append(delay_s)
#                     elif 'LastDelay' in line:
#                         last_latency_values.append(delay_s)
#
#     # Calculate the average latency, jitter, and standard deviation for the current simulation directory for FullDelay
#     avg_latency = np.mean(latency_values)
#     var_latency = np.var(latency_values)
#     jitter = np.sqrt(var_latency)
#     std_latency = np.std(latency_values)
#
#     print(f"Average latency for {dir} FullDelay: {avg_latency:.6f} seconds")
#     print(f"Jitter for {dir} FullDelay: {jitter:.6f} seconds")
#     print(f"Standard deviation of latency for {dir} FullDelay: {std_latency:.6f} seconds")
#
#     # Append the average latency, jitter, and simulation ID to the lists for FullDelay
#     avg_latency_list.append(avg_latency)
#     jitter_list.append(jitter)
#     sim_id_list.append(dir)
#     std_latency_list.append(std_latency)
#
#     # Calculate the average latency, jitter, and standard deviation for the current simulation directory for LastDelay
#     last_avg_latency = np.mean(last_latency_values)
#     last_var_latency = np.var(last_latency_values)
#     last_jitter = np.sqrt(last_var_latency)
#     last_std_latency = np.std(last_latency_values)
#
#     print(f"Average latency for {dir} LastDelay: {last_avg_latency:.6f} seconds")
#     print(f"Jitter for {dir} LastDelay: {last_jitter:.6f} seconds")
#     print(f"Standard deviation of latency for {dir} LastDelay: {last_std_latency:.6f} seconds")
#
#     # Append the average latency, jitter, and simulation ID to the lists for LastDelay
#     last_avg_latency_list.append(last_avg_latency)
#     last_jitter_list
#



import os
import matplotlib.pyplot as plt
import numpy as np

from matplotlib.ticker import ScalarFormatter

# Define the name of the trace file
trace_file = "delay-ndn-simple-wifi3-WithBeacon_20230109.txt"

# Define the columns to extract from the trace file
columns = "Time Node AppId SeqNo Type DelayS DelayUS RetxCount HopCount"

# Initialize lists to store the average latency and jitter values, and simulation IDs
avg_latency_list = []
jitter_list = []
sim_id_list = []
std_latency_list = []

last_avg_latency_list = []
last_jitter_list = []

###
last_std_latency_list = []

# Loop over each simulation directory (e.g., 100, 60, 70)
for dir in os.listdir("."):
    if not os.path.isdir(dir) or dir.startswith("."):
        continue
    print(f"Processing {dir}")

    # Initialize variables for calculating latency and jitter
    total_latency = 0
    num_packets_received = 0
    latency_values = []

    last_total_latency = 0
    last_num_packets_received = 0
    last_latency_values = []

    # Loop over each RngRun_* directory inside the current simulation directory
    for subdir in os.listdir(dir):
        if not os.path.isdir(os.path.join(dir, subdir)):
            continue

        # Read the trace file into an array
        trace_data = []
        with open(f"{dir}/{subdir}/{trace_file}") as f:
            # next(f)  # skip header row
            for line in f:
                fields = line.strip().split()
                if len(fields) >= 6:
                    # Extract the DelayS field and calculate the latency
                    delay_s = float(fields[5])
                    if 'FullDelay' in line:
                        latency_values.append(delay_s)
                    elif 'LastDelay' in line:
                        last_latency_values.append(delay_s)

    # Calculate the average latency, jitter, and standard deviation for the current simulation directory for FullDelay
    avg_latency = np.mean(latency_values)
    var_latency = np.var(latency_values)
    jitter = np.sqrt(var_latency)
    std_latency = np.std(latency_values)

    print(f"Average latency for {dir} FullDelay: {avg_latency:.6f} seconds")
    print(f"Jitter for {dir} FullDelay: {jitter:.6f} seconds")
    print(f"Standard deviation of latency for {dir} FullDelay: {std_latency:.6f} seconds")

    # Append the average latency, jitter, and simulation ID to the lists for FullDelay
    avg_latency_list.append(avg_latency)
    jitter_list.append(jitter)
    sim_id_list.append(dir)
    std_latency_list.append(std_latency*0.05) # MUST be 1000

    # Calculate the average latency, jitter, and standard deviation for the current simulation directory for LastDelay
    last_avg_latency = np.mean(last_latency_values)
    last_var_latency = np.var(last_latency_values)
    last_jitter = np.sqrt(last_var_latency)
    last_std_latency = np.std(last_latency_values)

    print(f"Average latency for {dir} LastDelay: {last_avg_latency:.6f} seconds")
    print(f"Jitter for {dir} LastDelay: {last_jitter:.6f} seconds")
    print(f"Standard deviation of latency for {dir} LastDelay: {last_std_latency:.6f} seconds")

    # Append the average latency, jitter, and simulation ID to the lists for LastDelay
    last_avg_latency_list.append(last_avg_latency)
    last_jitter_list.append(last_jitter)
    ##
    last_std_latency_list.append(last_std_latency) # MUST be 1000


# Sort the simulation IDs in ascending order
sim_id_list_sorted = [str(i) for i in sorted([int(x) for x in sim_id_list])]

# Create a figure with two subplots
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)
# error_bar_scale = 0.1

# Plot average latency with error bars
ax1.errorbar(sim_id_list_sorted, avg_latency_list, yerr=std_latency_list, fmt='-o', label='FullDelay')
ax1.errorbar(sim_id_list_sorted, last_avg_latency_list, yerr=last_std_latency_list, fmt='-o', label='LastDelay')

# Add a legend and axis labels
ax1.legend()
ax1.set_ylabel('Latency (s)')
ax1.set_title('Average Latency')

# # Plot jitter
# ax2.bar(sim_id_list_sorted, jitter_list, yerr=std_latency_list, label='FullDelay')
# ax2.bar(sim_id_list_sorted, last_jitter_list, yerr=last_std_latency_list, label='LastDelay')

# Plot jitter
ax2.errorbar(sim_id_list_sorted, jitter_list, yerr=std_latency_list, fmt='-o', label='FullDelay')
ax2.errorbar(sim_id_list_sorted, last_jitter_list, yerr=last_std_latency_list, fmt='-o', label='LastDelay')

# Add a legend and axis labels
ax2.legend()
ax2.set_ylabel('Jitter (s)')
ax2.set_title('Jitter')

# Set the x-axis tick labels to the simulation IDs
ax2.set_xticks(sim_id_list_sorted)
ax2.set_xticklabels(sim_id_list_sorted)

# Set the y-axis tick labels to use scalar formatter
ax1.yaxis.set_major_formatter(ScalarFormatter(useMathText=True))
# ax1.set_yscale("log") # Set y-axis as logarithmic scale
ax1.yaxis.grid(True, which='both', linestyle='-', linewidth=0.5)  # Plot only horizontal grid

ax2.yaxis.set_major_formatter(ScalarFormatter(useMathText=True))
# ax2.set_yscale("log") # Set y-axis as logarithmic scale
ax2.yaxis.grid(True, which='both', linestyle='-', linewidth=0.5)  # Plot only horizontal grid
ax2.set_xlabel("Number of Vehicles (in a period of 470s - 220s = 250s)")

plt.savefig('latency_jitter.png', dpi=600)
# Show the plot
plt.show()
