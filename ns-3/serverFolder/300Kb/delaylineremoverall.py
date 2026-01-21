# import os
#
# # Set the directory path
# directory = './'
#
# # Loop through all subdirectories in the directory
# for root, dirs, files in os.walk(directory):
#     for dir in dirs:
#         # Check if the subdirectory name is a number (e.g., 40, 50, 60)
#         if dir.isdigit():
#             # Construct the trace file path
#             trace_file_path = os.path.join(root, dir, 'trace.txt')
#             # Check if the trace file exists
#             if os.path.exists(trace_file_path):
#                 # Read the lines from the trace file
#                 with open(trace_file_path, 'r') as file:
#                     next(file)
#                     lines = file.readlines()
#                 # Create a new list to store the lines with valid timestamps
#                 valid_lines = []
#                 # Loop through the lines and keep lines with timestamps from 200 to 450
#                 for line in lines:
#                     timestamp = int(line.split()[0]) # Assuming the timestamp is the first word in each line
#                     if 200 <= timestamp <= 450:
#                         valid_lines.append(line)
#                 # Write the valid lines back to the trace file
#                 with open(trace_file_path, 'w') as file:
#                     file.writelines(valid_lines)
#                 print(f"Modified trace file: {trace_file_path}")



# #  Only files with Header
# import os
# import glob
#
# # Set the directory path
# directory = './'
#
# # Loop through all subdirectories in the directory
# for root, dirs, files in os.walk(directory):
#     for dir in dirs:
#         # Check if the subdirectory name is a number (e.g., 40, 50, 60)
#         if dir.isdigit():
#             # Construct the RngRun subdirectory path
#             rngrun_dir = os.path.join(root, dir, 'RngRun_*')
#             # Get the list of RngRun subdirectories
#             rngrun_subdirs = glob.glob(rngrun_dir)
#             # Loop through the RngRun subdirectories
#             for rngrun_subdir in rngrun_subdirs:
#                 # Construct the trace file path
#                 trace_file_path = os.path.join(rngrun_subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#                 # Check if the trace file exists
#                 if os.path.exists(trace_file_path):
#                     # Read the lines from the trace file
#                     with open(trace_file_path, 'r') as file:
#                         next(file)
#                         lines = file.readlines()
#                     # Create a new list to store the lines with valid timestamps
#                     valid_lines = []
#                     # Loop through the lines and keep lines with timestamps from 200 to 450
#                     for line in lines:
#                         timestamp = int(line.split()[0]) # Assuming the timestamp is the first word in each line
#                         if 200 <= timestamp <= 450:
#                             valid_lines.append(line)
#                     # Write the valid lines back to the trace file
#                     with open(trace_file_path, 'w') as file:
#                         file.writelines(valid_lines)
#                     print(f"Modified trace file: {trace_file_path}")



## For files with or without Header

# import os
# import glob
#
# # Set the directory path
# directory = './'
#
# # Loop through all subdirectories in the directory
# for root, dirs, files in os.walk(directory):
#     for dir in dirs:
#         # Check if the subdirectory name is a number (e.g., 40, 50, 60)
#         if dir.isdigit():
#             # Construct the RngRun subdirectory path
#             rngrun_dir = os.path.join(root, dir, 'RngRun_*')
#             # Get the list of RngRun subdirectories
#             rngrun_subdirs = glob.glob(rngrun_dir)
#             # Loop through the RngRun subdirectories
#             for rngrun_subdir in rngrun_subdirs:
#                 # Construct the trace file path
#                 trace_file_path = os.path.join(rngrun_subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
#                 # Check if the trace file exists
#                 if os.path.exists(trace_file_path):
#                     # Read the lines from the trace file
#                     with open(trace_file_path, 'r') as file:
#                         lines = file.readlines()
#                         # Check if the first line is a header
#                         if lines[0].startswith('Time'):
#                             # Skip the header line
#                             lines = lines[1:]
#                             print(f"Trace file '{trace_file_path}' has a header.")
#                         # Create a new list to store the lines with valid timestamps
#                         valid_lines = []
#                         # Loop through the lines and keep lines with timestamps from 200 to 450
#                         for line in lines:
#                             timestamp = int(line.split()[0]) # Assuming the timestamp is the first word in each line
#                             if 200 <= timestamp <= 450:
#                                 valid_lines.append(line)
#                                 # print(f"Trace file '{trace_file_path}' has been modified.")
#                         # Write the valid lines back to the trace file
#                         with open(trace_file_path, 'w') as file:
#                             file.writelines(valid_lines)
#                         print(f"Modified trace file: {trace_file_path}")




## Corrected version with/without Header
import os
import glob

# Set the directory path
directory = './'

# Loop through all subdirectories in the directory
for root, dirs, files in os.walk(directory):
    for dir in dirs:
        # Check if the subdirectory name is a number (e.g., 40, 50, 60)
        if dir.isdigit():
            # Construct the RngRun subdirectory path
            rngrun_dir = os.path.join(root, dir, 'RngRun_*')
            # Get the list of RngRun subdirectories
            rngrun_subdirs = glob.glob(rngrun_dir)
            # Loop through the RngRun subdirectories
            for rngrun_subdir in rngrun_subdirs:
                # Construct the trace file path
                trace_file_path = os.path.join(rngrun_subdir, 'delay-ndn-simple-wifi3-WithBeacon_20230109.txt')
                # Check if the trace file exists
                if os.path.exists(trace_file_path):
                    # Read the lines from the trace file
                    with open(trace_file_path, 'r') as file:
                        lines = file.readlines()
                        # Check if the first line is a header
                        if lines and lines[0].startswith('Time\tNode'):
                            # Skip the header line
                            lines = lines[1:]
                            print(f"Trace file '{trace_file_path}' has a header.")
                        # Create a new list to store the lines with valid timestamps
                        valid_lines = []
                        # Loop through the lines and keep lines with timestamps from 200 to 450
                        for line in lines:
                            timestamp = float(line.split()[0]) # Assuming the timestamp is the first word in each line
                            if 220 <= timestamp <= 470:
                                valid_lines.append(line)
                            # print(f"Trace file '{trace_file_path}' has been modified.")
                        # Write the valid lines back to the trace file
                        with open(trace_file_path, 'w') as file:
                            file.writelines(valid_lines)
                        print(f"Modified trace file: {trace_file_path}")












#
# import os
#
# # Set the directory path
# directory = './'
#
# # Loop through all subdirectories in the directory
# for root, dirs, files in os.walk(directory):
#     for dir in dirs:
#         # Check if the subdirectory name is a number (e.g., 40, 50, 60)
#         if dir.isdigit():
#             # Construct the RngRun subdirectory path
#             rngrun_dir = os.path.join(root, dir, 'RngRun_*')
#             # Get the list of RngRun subdirectories
#             rngrun_subdirs = glob.glob(rngrun_dir)
#             # Loop through the RngRun subdirectories
#             for rngrun_subdir in rngrun_subdirs:
#                 # Construct the trace file path
#                 trace_file_path = os.path.join(rngrun_subdir, 'trace.txt')
#                 # Check if the trace file exists
#                 if os.path.exists(trace_file_path):
#                     # Read the lines from the trace file
#                     with open(trace_file_path, 'r') as file:
#                         lines = file.readlines()
#                     # Check if the first line is a header
#                     if lines[0].startswith('Timestamp\tEvent'):
#                         # Header is present
#                         print(f"Trace file '{trace_file_path}' has a header.")
#                     else:
#                         # Header is not present
#                         print(f"Trace file '{trace_file_path}' does not have a header.")
