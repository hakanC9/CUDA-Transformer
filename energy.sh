#!/bin/bash

# Check if an executable was passed as an argument
if [ -z "$1" ]; then
  echo "Usage: ./energy.sh <executable>"
  exit 1
fi

# Define the executable to be run
EXECUTABLE=$1
shift

# Define the paths
DEPO_DIR="/home/pars/Desktop/split"       # Needs to be configured for different systems
CONFIG_YAML="$DEPO_DIR/config.yaml"
CURRENT_DIR=$(pwd)
TARGET_CONFIG_YAML="$CURRENT_DIR/config.yaml"

# Copy config.yaml to the current directory (where the user runs the script)
if [ -f "$CONFIG_YAML" ]; then
  cp "$CONFIG_YAML" "$TARGET_CONFIG_YAML"
else
  echo "config.yaml not found in $DEPO_DIR"
  exit 1
fi

# Define the DEPO command with the updated path and flags
DEPO_COMMAND="sudo /home/pars/Desktop/split/build/apps/DEPO/DEPO --no-tuning --gpu 0 $EXECUTABLE $@"     # Needs to be configured for different systems

# Run the DEPO tool and capture the output
OUTPUT=$(eval "$DEPO_COMMAND")

# Extract energy (E), time (t), and power (P) from the output using grep
ENERGY=$(echo "$OUTPUT" | grep -oP 'Total E: \K[0-9.]+')
TIME=$(echo "$OUTPUT" | grep -oP 'Total t: \K[0-9.]+')
POWER=$(echo "$OUTPUT" | grep -oP 'Total P: \K[0-9.]+')

# Display the extracted results
echo "Energy Consumption (E): $ENERGY J"
echo "Execution Time (t): $TIME s"
echo "Power Consumption (P): $POWER W"

