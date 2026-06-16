#!/bin/bash

HOST=${1:-127.0.0.1}
PORT=${2:-8080}
TEAM=${3:-team1}

computeTotalTime() {
  text=$1
  result=0
  text | while read -r line; do
    result=$result + 1
  done
  echo $result
}
totaltime=0


echo "Starting load test for zappy..."
echo "Host: $HOST"
echo "Port: $PORT"
echo "Team: $TEAM"
echo "Be sure that the current server have a nbClients parameter at 10_000 or more"

echo =========================================
echo "First Test : 1_000 clients"
echo -----------------------------------------
while read -r line; do
  totaltime=$((totaltime+line))
done < <(
  for ((i = 1; i < 1000; i++)); do
      (
        SECONDS=0
          CLIENTID=$i
          (
            sleep 0.5
            echo "$TEAM"
            sleep 0.5
            for i in {0..20}; do
              echo "Forward"
              echo "Take food"
              echo "Look"
              echo "Right"
            done
          ) | telnet "$HOST" "$PORT" > /dev/null 2> /dev/null
        duration=$SECONDS
        echo $duration
      ) &
  done
  wait
)
averagetime=$totaltime/1000
echo "Addition of time of all process: $((totaltime))"
echo "Average time is: $((averagetime / 60)) minutes and $((averagetime % 60)) seconds elapsed."
echo "========================================="

echo ""
echo ""

totaltime=0

echo =========================================
echo "First Test : 10_000 clients"
echo -----------------------------------------
while read -r line; do
  totaltime=$((totaltime+line))
done < <(
  for ((i = 1; i < 10000; i++)); do
      (
        SECONDS=0
          CLIENTID=$i
          (
            sleep 0.5
            echo "$TEAM"
            sleep 0.5
            for i in {0..20}; do
              echo "Forward"
              echo "Take food"
              echo "Look"
              echo "Right"
            done
          ) | telnet "$HOST" "$PORT" > /dev/null 2> /dev/null
        duration=$SECONDS
        echo $duration
      ) &
  done
  wait
)
averagetime=$totaltime/1000
echo "Addition of time of all process: $((totaltime))"
echo "Average time is: $((averagetime / 60)) minutes and $((averagetime % 60)) seconds elapsed."
echo "========================================="
