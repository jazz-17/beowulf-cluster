#!/bin/bash

# --- Configuration ---
SOURCE_FILE="2.c"  # Change this to your C source file
OUTPUT_NAME="output"    # Name of the executable
OPENMP_FLAG="-fopenmp"            # OpenMP flag (usually -fopenmp for GCC/Clang)
ADDITIONAL_FLAGS="-lm"              # Any additional compiler flags you want to use (e.g., -Wall, -O2)

# --- Compilation ---
if ! "gcc" "${SOURCE_FILE}" -o "${OUTPUT_NAME}" ${OPENMP_FLAG} ${ADDITIONAL_FLAGS}; then
  echo "Compilation failed!"
  exit 1 
fi
# --- Execution ---
./"${OUTPUT_NAME}"
exit 0 