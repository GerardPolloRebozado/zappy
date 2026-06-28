PORT=8080
IP="localhost"
COUNT=1

usage() {
    echo "Usage: $0 [-p port] [-h host] [-n count] team1 [team2 ...]"
    echo "  -p: port (default 8080)"
    echo "  -h: host (default localhost)"
    echo "  -n: number of clients per team (default 1)"
    exit 1
}

# parse options
while getopts "p:h:n:" opt; do
    case $opt in
        p) PORT=$OPTARG ;;
        h) IP=$OPTARG ;;
        n) COUNT=$OPTARG ;;
        *) usage ;;
    esac
done
shift $((OPTIND-1))

# check if at least one team name is provided
if [ $# -eq 0 ]; then
    usage
fi

TEAMS=("$@")

trap "echo 'Stopping all AI clients...'; kill 0" EXIT

for TEAM in "${TEAMS[@]}"; do
    for i in $(seq 1 "$COUNT"); do
        echo "Starting AI client $i for team '$TEAM' on $IP:$PORT (heuristic mode)"
        python3 ai/src/main.py -p "$PORT" -n "$TEAM" -ip "$IP" &
    done
done

echo "All clients started. Press Ctrl+C to stop."

# Wait for all background processes
wait
