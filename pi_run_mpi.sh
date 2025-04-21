#!/bin/bash

# --- Configuration ---
SOURCE_SUBDIR="MPI"
OUTPUT_SUBDIR="MPI"
COMPILER="mpicc"
FLAGS="-Wall -lm" # Added -lm to link the math library for M_PI and fabs
NODES=("node2" "node3")
HOSTFILE_PATH="~/hostfile"

# --- Argument Handling ---
if [ "$#" -ne 3 ]; then
  echo "Usage: $0 <source_filename_no_extension> <num_processes> <num_intervals>"
  echo "  Example: $0 mpi_pi 3 1000000"
  echo "  Assumes source file is located at: ./${SOURCE_SUBDIR}/<source_filename_no_extension>.c"
  echo "  Compiled executable will be placed at: ./${OUTPUT_SUBDIR}/<source_filename_no_extension>"
  echo "  Requires hostfile at: ${HOSTFILE_PATH}"
  exit 1
fi

FILENAME_NO_EXT="$1"
NUM_OF_PROCESSES="$2"
NUM_INTERVALS="$3" # New argument

# Basic check for positive integers
if ! [[ "$NUM_OF_PROCESSES" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: Number of processes must be a positive integer."
    exit 1
fi
if ! [[ "$NUM_INTERVALS" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: Number of intervals must be a positive integer."
    exit 1
fi

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Construct full paths relative to the script's location
SOURCE_FILE="${SCRIPT_DIR}/${SOURCE_SUBDIR}/${FILENAME_NO_EXT}.c"
OUTPUT_EXECUTABLE="${SCRIPT_DIR}/${OUTPUT_SUBDIR}/${FILENAME_NO_EXT}"
OUTPUT_EXECUTABLE_REL_HOME="./${OUTPUT_SUBDIR}/${FILENAME_NO_EXT}"
HOSTFILE_PATH_RESOLVED=$(eval echo ${HOSTFILE_PATH})

# --- Pre-checks ---
if [ ! -f "${SOURCE_FILE}" ]; then
  echo "Error: Source file not found at ${SOURCE_FILE}"
  exit 1
fi
if [ ! -f "${HOSTFILE_PATH_RESOLVED}" ]; then
  echo "Error: Hostfile not found at ${HOSTFILE_PATH_RESOLVED}"
  exit 1
fi

# --- Ensure output directory exists (locally) ---
OUTPUT_DIR_ABS=$(dirname "${OUTPUT_EXECUTABLE}")
if [ ! -d "${OUTPUT_DIR_ABS}" ]; then
  echo "Creating output directory: ${OUTPUT_DIR_ABS}"
  mkdir -p "${OUTPUT_DIR_ABS}"
  if [ $? -ne 0 ]; then
    echo "Failed to create output directory: ${OUTPUT_DIR_ABS}"; exit 1; fi
fi

# --- Compilation ---
echo "Compiling ${SOURCE_FILE} -> ${OUTPUT_EXECUTABLE}"
if ! "${COMPILER}" "${SOURCE_FILE}" -o "${OUTPUT_EXECUTABLE}" ${FLAGS}; then
  echo "Compilation failed!"; exit 1; fi
echo "Compilation successful."

# --- Distribution (Copy to other nodes) ---
echo "Distributing executable to worker nodes..."
COPY_FAILED=0
for NODE in "${NODES[@]}"; do
  ssh "${NODE}" "mkdir -p ~/${OUTPUT_SUBDIR}" # Ensure remote directory exists
  scp "${OUTPUT_EXECUTABLE}" "${NODE}:~/${OUTPUT_SUBDIR}/"
  if [ $? -ne 0 ]; then
    echo "  ERROR: Failed to copy executable to ${NODE}!"; COPY_FAILED=1; fi
done
if [ ${COPY_FAILED} -eq 1 ]; then echo "Distribution failed."; exit 1; fi
echo "Distribution successful."

# --- Execution ---
echo "Executing ${OUTPUT_EXECUTABLE_REL_HOME} with ${NUM_OF_PROCESSES} processes and ${NUM_INTERVALS} intervals..."
# Pass the number of intervals as a command-line argument to the MPI program
mpirun -np "${NUM_OF_PROCESSES}" --hostfile "${HOSTFILE_PATH_RESOLVED}" "${OUTPUT_EXECUTABLE_REL_HOME}" "${NUM_INTERVALS}"
EXECUTION_STATUS=$?

if [ ${EXECUTION_STATUS} -ne 0 ]; then
  echo "Execution failed with status: ${EXECUTION_STATUS}"; exit ${EXECUTION_STATUS}; fi

echo "Execution finished successfully."
exit 0