#!/bin/bash

# Arguments
HOST=${1:-127.0.0.1}
PORT=${2:-8080}
TEAM=${3:-team1}


echo "Starting load test for zappy..."
echo "Host: $HOST"
echo "Port: $PORT"
echo "Team: $TEAM"
echo "Be sure that the current server have a nbClients parameter at 10_000 or more"
echo "\n"

echo =========================================
echo "Start of the first test : 1_000 clients"
echo =========================================

for ((i = 1; i < 1000; i++)); do
    echo "Start of Client $i"
    (
        clientid = $i
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
        echo "End of Client $clientid"
    ) &
done
wait

echo "End of the first test"
echo "========================================="
echo "Start of the Second test : 10_000 clients"
echo "========================================="

for ((i = 1; i < 10000; i++)); do
    echo "Client $i"
    (
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
    ) &
    echo "Client $i started"
done
wait


echo "End of the Second test"
echo "========================================="
