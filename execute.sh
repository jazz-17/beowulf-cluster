#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
# Treat unset variables as an error when substituting.
# set -eu # Uncomment these for stricter error checking if desired

# --- Configuration ---
# Use $HOME for reliable home directory expansion
HOSTFILE_PATH="$HOME/hostfile"                  # Path to the MPI hostfile
BIN_DIR="$HOME/beowulf-cluster/build"           # Directory containing the MPI executables
DEFAULT_NUM_PROCESSES=3                         # Default number of processes if not specified

# Silence PMIX warning if needed (keep this if it solves a problem for you)
export PMIX_MCA_pcompress_base_silence_warning=1

# --- Argument Handling ---
if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <executable_name> [num_processes] [args_for_mpi_program...]"
  echo "  Example 1 (default processes, no args): $0 mpi_hello"
  echo "  Example 2 (specify processes, no args): $0 mpi_hello 4"
  echo "  Example 3 (default processes, with args): $0 mpi_trapezoid 0.0 1.0 10000"
  echo "  Example 4 (specify processes, with args): $0 mpi_trapezoid 4 0.0 1.0 10000"
  echo ""
  echo "  <executable_name>: The name of the executable file located in ${BIN_DIR}"
  echo "  [num_processes]: Optional. Number of processes to run (default: ${DEFAULT_NUM_PROCESSES})."
  echo "  [args_for_mpi_program...]: Optional. Arguments passed directly to the MPI executable."
  echo "  Requires hostfile at: ${HOSTFILE_PATH}"
  exit 1
fi

EXECUTABLE_NAME="$1"
shift # Remove executable name from args list

# Check if the *new* first argument looks like a number (for num_processes)
# If it is, use it and shift again. Otherwise, use the default.
NUM_PROCESSES=${DEFAULT_NUM_PROCESSES} # Assume default
if [[ "$1" =~ ^[0-9]+$ ]]; then        # Check if $1 is purely digits
    NUM_PROCESSES="$1"
    shift # Remove num_processes from args list
# Else: $1 is not a number, so it must be the first arg for the MPI program
fi

# All remaining arguments are for the MPI program
MPI_PROGRAM_ARGS=("$@")

# Construct the full path to the executable
FULL_EXECUTABLE_PATH="${BIN_DIR}/${EXECUTABLE_NAME}"

# --- Pre-checks ---
echo "--- Pre-checks ---"

# Check if the binary directory exists
if [ ! -d "${BIN_DIR}" ]; then
  echo "Error: Binary directory not found at '${BIN_DIR}'"
  exit 1
fi
echo "Binary directory found: ${BIN_DIR}"

# Check if the executable file exists *and* is executable in the BIN_DIR
if [ ! -x "${FULL_EXECUTABLE_PATH}" ]; then
  # Provide a more specific error if it exists but isn't executable
  if [ -f "${FULL_EXECUTABLE_PATH}" ]; then
    echo "Error: File found at '${FULL_EXECUTABLE_PATH}', but it is not executable."
    echo "       Hint: Use 'chmod +x ${FULL_EXECUTABLE_PATH}'"
  else
    echo "Error: Executable file not found at '${FULL_EXECUTABLE_PATH}'"
  fi
  exit 1
fi
echo "Executable found and is executable: ${FULL_EXECUTABLE_PATH}"

# Check if the hostfile exists and is readable
if [ ! -f "${HOSTFILE_PATH}" ] || [ ! -r "${HOSTFILE_PATH}" ]; then
  echo "Error: Hostfile not found or not readable at '${HOSTFILE_PATH}'"
  echo "       Please create it with your nodes (e.g., 'node1 slots=2')."
  exit 1
fi
echo "Hostfile found: ${HOSTFILE_PATH}"
echo "--------------------"

# --- Execution ---
echo "--- Execution ---"
echo "Running MPI job:"
echo "  Executable: ${FULL_EXECUTABLE_PATH}"
echo "  Processes:  ${NUM_PROCESSES}"
echo "  Hostfile:   ${HOSTFILE_PATH}"
# Display arguments clearly, handling the case where there are none
if [ ${#MPI_PROGRAM_ARGS[@]} -gt 0 ]; then
    # Use printf for safer printing of potentially weird arguments
    printf "  Arguments:  "
    printf "'%s' " "${MPI_PROGRAM_ARGS[@]}"
    printf "\n"
else
    echo "  Arguments:  (none)"
fi
echo "--------------------"

# Execute the mpi program using the full path.
# Pass any additional arguments captured earlier using "${MPI_PROGRAM_ARGS[@]}"
# The quotes around "${MPI_PROGRAM_ARGS[@]}" handle arguments with spaces correctly.
mpirun -np "${NUM_PROCESSES}" --hostfile "${HOSTFILE_PATH}" "${FULL_EXECUTABLE_PATH}" "${MPI_PROGRAM_ARGS[@]}"
EXECUTION_STATUS=$? # Capture the exit status of mpirun

echo "--------------------"
if [ ${EXECUTION_STATUS} -ne 0 ]; then
  echo "Error: Execution failed with status: ${EXECUTION_STATUS}"
  exit ${EXECUTION_STATUS} # Exit with the same status as the failed command
fi

echo "Execution finished successfully."
exit 0