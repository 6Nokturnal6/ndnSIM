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
                trace_file_path = os.path.join(rngrun_subdir, 'rate-ndn-simple-wifi3-WithBeacon_20230109.txt')
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
                        # Find the timestamp of the first line
                        first_timestamp = int(lines[0].split()[0])
                        # Compute the last timestamp to read
                        last_timestamp = first_timestamp + 199
                        if last_timestamp > first_timestamp and len(lines) >= last_timestamp-first_timestamp:
                            # Loop through the lines and keep lines with timestamps from first_timestamp to last_timestamp
                            for line in lines:
                                timestamp = int(line.split()[0]) # Assuming the timestamp is the first word in each line
                                if first_timestamp <= timestamp <= last_timestamp:
                                    valid_lines.append(line)
                                # Stop processing after 200 lines
                                if len(valid_lines) == 200:
                                    break
                            # Write the valid lines back to the trace file
                            with open(trace_file_path, 'w') as file:
                                file.writelines(valid_lines)
                            print(f"Modified trace file: {trace_file_path}. Read from {first_timestamp} to {last_timestamp}")
                        else:
                            print(f"Not enough lines in trace file '{trace_file_path}' to read from {first_timestamp} to {last_timestamp}")
