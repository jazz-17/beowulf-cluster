#!/bin/bash

# --- Configuration ---
# Directory containing the source code (.c file), relative to this script's location
SOURCE_SUBDIR="src/mpi"
# Base output directory for the compiled executable, relative to this script's location
# We'll put the executable alongside the source code.
OUTPUT_SUBDIR="build"

FLAGS="-Wall -Wextra -g"                 # Compiler flags (e.g., -Wall, -O2, -g for debugging)
HOSTFILE_PATH="~/hostfile"               # Path to the MPI hostfile (tilde expansion is handled)

# --- Argument Handling ---
if [ "$#" -lt 2 ]; then
  echo "Usage: $0 <source_filename_no_extension> <num_processes> [args_for_mpi_program...]"
  echo "  Example 1 (no args for program): $0 mpi_hello 4"
  echo "  Example 2 (with args for program): $0 mpi_trapezoid 4 0.0 1.0 10000"
  echo "  Assumes source file is located at: ./${SOURCE_SUBDIR}/<source_filename>.c"
  echo "  Compiled executable will be placed at: ./${OUTPUT_SUBDIR}/<source_filename>"
  echo "  Requires hostfile at: ${HOSTFILE_PATH}"
  exit 1
fi

FILENAME_NO_EXT="$1"
NUM_OF_PROCESSES="$2"

# Basic check for positive integer for number of processes
if ! [[ "$NUM_OF_PROCESSES" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: Number of processes must be a positive integer, got '${NUM_OF_PROCESSES}'."
    exit 1
fi

# Remove the first two arguments (script name handled implicitly, filename, num_processes)
# so that $@ contains only the arguments intended for the MPI program.
shift 2
MPI_PROGRAM_ARGS=("$@") # Store remaining arguments in an array for clarity/safety

# --- Path Management ---
# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Construct full paths relative to the script's location
SOURCE_FILE="${SCRIPT_DIR}/${SOURCE_SUBDIR}/${FILENAME_NO_EXT}.c"
OUTPUT_EXECUTABLE_ABS="${SCRIPT_DIR}/${OUTPUT_SUBDIR}/${FILENAME_NO_EXT}"
# Path relative to home dir. This relies on mpirun starting processes in the user's home directory on remote nodes (default OpenMPI+SSH behavior).
OUTPUT_EXECUTABLE_REL_HOME="./${OUTPUT_SUBDIR}/${FILENAME_NO_EXT}"
HOSTFILE_PATH_RESOLVED=$(eval echo "${HOSTFILE_PATH}") # Resolve ~ in hostfile path

# --- Pre-checks ---
echo "--- Pre-checks ---"
if [ ! -f "${SOURCE_FILE}" ]; then
  echo "Error: Source file not found at ${SOURCE_FILE}"
  exit 1
fi
echo "Source file found: ${SOURCE_FILE}"

if [ ! -f "${HOSTFILE_PATH_RESOLVED}" ]; then
  echo "Error: Hostfile not found at ${HOSTFILE_PATH_RESOLVED}"
  echo "Please create it with your nodes (e.g., node1 slots=1, node2 slots=1, etc.)"
  exit 1
fi
echo "Hostfile found: ${HOSTFILE_PATH_RESOLVED}"
echo "--------------------"


# --- Ensure output directory exists (locally) ---
OUTPUT_DIR_ABS=$(dirname "${OUTPUT_EXECUTABLE_ABS}")
if [ ! -d "${OUTPUT_DIR_ABS}" ]; then
  echo "Creating local output directory: ${OUTPUT_DIR_ABS}"
  mkdir -p "${OUTPUT_DIR_ABS}"
  if [ $? -ne 0 ]; then
    echo "Error: Failed to create local output directory: ${OUTPUT_DIR_ABS}"
    exit 1
  fi
fi

# --- Compilation ---
echo "--- Compilation ---"
echo "Compiling ${SOURCE_FILE} -> ${OUTPUT_EXECUTABLE_ABS}"
if ! mpicc "${SOURCE_FILE}" -o "${OUTPUT_EXECUTABLE_ABS}" ${FLAGS}; then
  echo "Error: Compilation failed!"
  exit 1
fi
echo "Compilation successful."
echo "--------------------"

# --- Execution ---
echo "--- Execution ---"
echo "Executing ${OUTPUT_EXECUTABLE_REL_HOME} with ${NUM_OF_PROCESSES} processes..."
echo "MPI Program Arguments: ${MPI_PROGRAM_ARGS[*]}" # Show arguments being passed

# Execute the mpi program using the path relative to the home directory.
# Pass any additional arguments captured earlier using "${MPI_PROGRAM_ARGS[@]}"
# The quotes around "${MPI_PROGRAM_ARGS[@]}" are important to handle arguments with spaces correctly.
mpirun -np "${NUM_OF_PROCESSES}" --hostfile "${HOSTFILE_PATH_RESOLVED}" "${OUTPUT_EXECUTABLE_REL_HOME}" "${MPI_PROGRAM_ARGS[@]}"
EXECUTION_STATUS=$? # Capture the exit status of mpirun

if [ ${EXECUTION_STATUS} -ne 0 ]; then
  echo "Error: Execution failed with status: ${EXECUTION_STATUS}"
  echo "--------------------"
  exit ${EXECUTION_STATUS} # Exit with the same status as the failed command
fi

echo "Execution finished successfully."
echo "--------------------"
exit 0