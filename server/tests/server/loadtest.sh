#!/bin/bash

HOST=${1:-127.0.0.1}
PORT=${2:-8080}
TEAM=${3:-team1}

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

totaltime=0

SECONDS=0
echo "Starting load test for zappy..."
echo -e "Host: ${GREEN}$HOST${NC}"
echo -e "Port: ${GREEN}$PORT${NC}"
echo -e "Team: ${GREEN}$TEAM${NC}"
echo -e "Be sure that the current server have a ${GREEN}nbClients${NC} parameter of ${RED}11_000${NC} or more"

echo -e ${BLUE}=========================================
echo -e "First Test : 1_000 clients"
echo -e ----------------------------------------- ${NC}
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
testduration=$SECONDS
echo -e "Duration of the test :  ${GREEN}$((testduration / 60)) minutes and $((testduration % 60)) elapsed.${NC}"
echo -e "Addition of time of all process: ${GREEN}$((totaltime))${NC}"
echo -e "Average time is: ${GREEN}$((averagetime / 60)) minutes and $((averagetime % 60)) seconds.${NC}"
echo -e "${BLUE}========================================="

echo ""
echo ""

totaltime=0
SECONDS=0

echo -e =========================================
echo -e "First Test : 10_000 clients"
echo -e -----------------------------------------${NC}
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
testduration=$SECONDS
echo -e "Duration of the test :  ${GREEN}$((testduration / 60)) minutes and $((testduration % 60)) seconds elapsed.${NC}"
echo -e "Addition of time of all process: ${GREEN}$((totaltime))${NC}"
echo -e "Average time is: ${GREEN}$((averagetime / 60)) minutes and $((averagetime % 60)) seconds.${NC}"
echo -e "${BLUE}=========================================${NC}"
