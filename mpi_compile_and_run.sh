#!/bin/bash

# --- Configuration ---
SOURCE_FILE="hello_mpi2.c"  # Change this to your C source file
OUTPUT_NAME="hello_mpi2"    # Name of the executable
NUM_CORES="2"    # Name of the executable
FLAGS=""              # Any additional compiler flags you want to use (e.g., -Wall, -O2)

# --- Compilation ---
if ! "mpicc" "${SOURCE_FILE}" -o "${OUTPUT_NAME}" ${FLAGS}; then
  echo "Compilation failed!"
  exit 1 
fi
# --- Execution ---
"mpiexec" -n "${NUM_CORES}" ./"${OUTPUT_NAME}"
exit 0