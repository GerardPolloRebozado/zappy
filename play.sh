#!/bin/bash

HOST=${1:-127.0.0.1}
PORT=${2:-8080}
TEAM=${3:-team1}

echo "Starting basic Zappy AI script..."
echo "Host: $HOST"
echo "Port: $PORT"
echo "Team: $TEAM"

(
  sleep 0.5
  echo "$TEAM"
  sleep 0.5
  while true; do
    echo "Forward"
    sleep 1
    echo "Take food"
    sleep 1
    echo "Look"
    sleep 1
    echo "Right"
    sleep 1
  done
) | telnet "$HOST" "$PORT"
