#!/bin/bash

# --- Configuration ---
# Directory containing the source code (.c file), relative to this script's location
SOURCE_SUBDIR="src/mpi"
# <<< CHANGE: Define the absolute path for the output directory >>>
OUTPUT_BASE_DIR="$HOME/SHARED" # Use $HOME for reliable home directory path

FLAGS="-Wall -Wextra -g"                 # Compiler flags (e.g., -Wall, -O2, -g for debugging)
HOSTFILE_PATH="~/hostfile"               # Path to the MPI hostfile (not used in compilation, but kept from original)
export PMIX_MCA_pcompress_base_silence_warning=1

# --- Argument Handling ---
if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <source_filename_no_extension>"
  echo "  Example: $0 mpi_hello"
  echo "  Compiles ./${SOURCE_SUBDIR}/<source_filename>.c"
  echo "  Outputs executable to ${OUTPUT_BASE_DIR}/<source_filename>"
  exit 1
fi

FILENAME_NO_EXT="$1"
# NUM_OF_PROCESSES="$2" # This argument is not used in the compilation script

# --- Path Management ---
# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Construct full path to the source file relative to the script's location
SOURCE_FILE="${SCRIPT_DIR}/${SOURCE_SUBDIR}/${FILENAME_NO_EXT}.c"

# <<< CHANGE: Construct absolute path for the output executable in the new directory >>>
OUTPUT_EXECUTABLE_ABS="${OUTPUT_BASE_DIR}/${FILENAME_NO_EXT}"

# HOSTFILE_PATH_RESOLVED=$(eval echo "${HOSTFILE_PATH}") # Resolve ~ (kept from original, but unused here)

# --- Pre-checks ---
echo "--- Pre-checks ---"
if [ ! -f "${SOURCE_FILE}" ]; then
  echo "Error: Source file not found at ${SOURCE_FILE}"
  exit 1
fi
echo "Source file found: ${SOURCE_FILE}"
echo "Target output directory: ${OUTPUT_BASE_DIR}" # Inform user of output location

# <<< CHANGE: Ensure the *new* output directory exists >>>
if [ ! -d "${OUTPUT_BASE_DIR}" ]; then
  echo "Creating output directory: ${OUTPUT_BASE_DIR}"
  # Use mkdir -p to create parent directories if needed and avoid errors if it already exists
  mkdir -p "${OUTPUT_BASE_DIR}"
  if [ $? -ne 0 ]; then
    echo "Error: Failed to create output directory: ${OUTPUT_BASE_DIR}"
    exit 1
  fi
fi
echo "Output directory checked/created."
echo "--------------------"


# --- Compilation ---
echo "--- Compilation ---"
# <<< CHANGE: Use the new OUTPUT_EXECUTABLE_ABS path >>>
echo "Compiling ${SOURCE_FILE} -> ${OUTPUT_EXECUTABLE_ABS}"
if ! mpicc "${SOURCE_FILE}" -o "${OUTPUT_EXECUTABLE_ABS}" ${FLAGS}; then
  echo "Error: Compilation failed!"
  exit 1
fi
echo "Compilation successful. Output executable: ${OUTPUT_EXECUTABLE_ABS}"
echo "--------------------"

exit 0 # Indicate success