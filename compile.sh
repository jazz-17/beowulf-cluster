#!/bin/bash

# --- Configuration ---
# Directory containing the source code (.c file), relative to this script's location
SOURCE_SUBDIR="src/mpi"
# Base output directory for the compiled executable, relative to this script's location
# We'll put the executable alongside the source code.
OUTPUT_SUBDIR="build"

FLAGS="-Wall -Wextra -g"                 # Compiler flags (e.g., -Wall, -O2, -g for debugging)
HOSTFILE_PATH="~/hostfile"               # Path to the MPI hostfile (tilde expansion is handled)
export PMIX_MCA_pcompress_base_silence_warning=1

FILENAME_NO_EXT="$1"
NUM_OF_PROCESSES="$2"

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