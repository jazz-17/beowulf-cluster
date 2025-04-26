#!/bin/bash

# --- Configuration ---
# Directory containing the source code (.c file), relative to this script's location
SOURCE_SUBDIR="src/mpi"
# Base output directory for the compiled executable, relative to this script's location
# We'll put the executable alongside the source code.
OUTPUT_SUBDIR="build"

FLAGS="-Wall"                           # Compiler flags (e.g., -Wall, -O2)
NODES=("node2" "node3")                 # List of worker nodes to copy the executable to (node1 is local)
HOSTFILE_PATH="~/hostfile"              # Path to the MPI hostfile

# --- Argument Handling ---
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <source_filename_no_extension> <num_processes>"
  echo "  Example: $0 mpi_hello 3"
  echo "  Assumes source file is located at: ./${SOURCE_SUBDIR}/<source_filename>.c"
  echo "  Compiled executable will be placed at: ./${OUTPUT_SUBDIR}/<source_filename>"
  echo "  Requires hostfile at: ${HOSTFILE_PATH}"
  exit 1
fi

FILENAME_NO_EXT="$1"
NUM_OF_PROCESSES="$2"

# Basic check for positive integer for number of processes
if ! [[ "$NUM_OF_PROCESSES" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: Number of processes must be a positive integer."
    exit 1
fi


# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Construct full paths relative to the script's location
SOURCE_FILE="${SCRIPT_DIR}/${SOURCE_SUBDIR}/${FILENAME_NO_EXT}.c"
OUTPUT_EXECUTABLE="${SCRIPT_DIR}/${OUTPUT_SUBDIR}/${FILENAME_NO_EXT}"
# Path relative to home dir, needed for remote execution and hostfile resolution
OUTPUT_EXECUTABLE_REL_HOME="./${OUTPUT_SUBDIR}/${FILENAME_NO_EXT}" # Assumes script is run from ~ or similar relative structure exists
HOSTFILE_PATH_RESOLVED=$(eval echo ${HOSTFILE_PATH}) # Resolve ~ in hostfile path for checks


# --- Pre-checks ---
if [ ! -f "${SOURCE_FILE}" ]; then
  echo "Error: Source file not found at ${SOURCE_FILE}"
  exit 1
fi

if [ ! -f "${HOSTFILE_PATH_RESOLVED}" ]; then
  echo "Error: Hostfile not found at ${HOSTFILE_PATH_RESOLVED}"
  echo "Please create it with your nodes (e.g., node1 slots=1, node2 slots=1, etc.)"
  exit 1
fi


# --- Ensure output directory exists (locally) ---
# Get the directory part of the output executable path
OUTPUT_DIR_ABS=$(dirname "${OUTPUT_EXECUTABLE}")
if [ ! -d "${OUTPUT_DIR_ABS}" ]; then
  echo "Creating output directory: ${OUTPUT_DIR_ABS}"
  mkdir -p "${OUTPUT_DIR_ABS}"
  if [ $? -ne 0 ]; then
    echo "Failed to create output directory: ${OUTPUT_DIR_ABS}"
    exit 1
  fi
fi

# --- Compilation ---
echo "Compiling ${SOURCE_FILE} -> ${OUTPUT_EXECUTABLE}"
if ! mpicc "${SOURCE_FILE}" -o "${OUTPUT_EXECUTABLE}" ${FLAGS}; then
  echo "Compilation failed!"
  exit 1
fi
echo "Compilation successful."

# --- Distribution (Copy to other nodes) ---
echo "Distributing executable to worker nodes..."
COPY_FAILED=0
for NODE in "${NODES[@]}"; do
  # Ensure remote directory exists first (robustness)
  echo "  Ensuring directory exists on ${NODE}..."
  ssh "${NODE}" "mkdir -p ~/${OUTPUT_SUBDIR}"
  if [ $? -ne 0 ]; then
      echo "  Warning: Failed to ensure directory exists on ${NODE}. Copy might fail."
      # Decide if this should be a fatal error or just a warning
  fi

  echo "  Copying ${OUTPUT_EXECUTABLE} to ${NODE}:~/${OUTPUT_SUBDIR}/"
  scp "${OUTPUT_EXECUTABLE}" "${NODE}:~/${OUTPUT_SUBDIR}/"
  if [ $? -ne 0 ]; then
    echo "  ERROR: Failed to copy executable to ${NODE}!"
    COPY_FAILED=1
  fi
done

if [ ${COPY_FAILED} -eq 1 ]; then
    echo "Distribution failed. Aborting execution."
    exit 1
fi
echo "Distribution successful."

# --- Execution ---
echo "Executing ${OUTPUT_EXECUTABLE_REL_HOME} with ${NUM_OF_PROCESSES} processes..."
# Execute the mpi program using the path relative to the home directory,
# as mpirun will start processes in the user's home directory on remote nodes.
# Use the resolved hostfile path.
mpirun -np "${NUM_OF_PROCESSES}" --hostfile "${HOSTFILE_PATH_RESOLVED}" "${OUTPUT_EXECUTABLE_REL_HOME}"
EXECUTION_STATUS=$? # Capture the exit status of mpirun

if [ ${EXECUTION_STATUS} -ne 0 ]; then
  echo "Execution failed with status: ${EXECUTION_STATUS}"
  exit ${EXECUTION_STATUS} # Exit with the same status as the failed command
fi

echo "Execution finished successfully."
exit 0