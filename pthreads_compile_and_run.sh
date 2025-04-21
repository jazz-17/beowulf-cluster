#!/bin/bash

# --- Configuration ---
BASE_OUTPUT_DIR="./pthreads"              # HARDCODED output directory path
FLAGS="-Wall"                  # Additional compiler flags (e.g., -Wall, -O2)
COMPILER="gcc"                        # Compiler to use
FILENAME="$1"
NUM_OF_CORES="$2"

# --- Argument Handling ---
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <filename>"
  echo "  Example: $0 my_program"
  echo "  This will compile ${FILENAME} into ${BASE_OUTPUT_DIR}/<filename>"
  echo "  and then execute ${BASE_OUTPUT_DIR}/<filename}"
  exit 1
fi

# Construct the full path for the executable
FULL_OUTPUT_PATH="${BASE_OUTPUT_DIR}/${FILENAME}"

# --- Ensure output directory exists ---
if [ ! -d "${BASE_OUTPUT_DIR}" ]; then
  echo "Creating output directory: ${BASE_OUTPUT_DIR}"
  mkdir -p "${BASE_OUTPUT_DIR}"
  if [ $? -ne 0 ]; then
    echo "Failed to create output directory: ${BASE_OUTPUT_DIR}"
    exit 1
  fi
fi

# --- Compilation ---
echo "Compiling ${FILENAME}.c -> ${FULL_OUTPUT_PATH}"
if ! "${COMPILER}" "${FULL_OUTPUT_PATH}.c" -o "${FULL_OUTPUT_PATH}" ${FLAGS}; then
  echo "Compilation failed!"
  exit 1  
fi
echo "Compilation successful."

# --- Execution ---
echo "Executing: ${FULL_OUTPUT_PATH}"
# Execute the compiled program directly
"${FULL_OUTPUT_PATH}" "${NUM_OF_CORES}"
EXECUTION_STATUS=$? # Capture the exit status of the command

if [ ${EXECUTION_STATUS} -ne 0 ]; then
  echo "Execution failed with status: ${EXECUTION_STATUS}"
  exit ${EXECUTION_STATUS} # Exit with the same status as the failed command
fi

exit 0