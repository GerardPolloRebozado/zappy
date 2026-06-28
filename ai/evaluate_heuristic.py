#!/usr/bin/env python3
"""Evaluate heuristic AI performance across multiple multi-agent runs.

Launches concurrent clients across multiple teams so they can coordinate
incantations and compete for resources on the same map.

Usage:
  # Auto-start server (recommended):
  python3 ai/evaluate_heuristic.py --server-cmd ./build/zappy_server

  # Connect to existing server:
  python3 ai/evaluate_heuristic.py -p 8080
"""

import argparse
import os
import signal
import socket
import subprocess
import sys
import threading
import time
from collections import Counter, defaultdict
from statistics import mean, median, stdev

sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))

from src.client import ZappyAiClient
from src.strategy import run_client


class EvalClient(ZappyAiClient):
    def __init__(self, port, name, ip):
        super().__init__(port, name, ip)
        self.action_count = 0
        self.peak_level = 1

    def wait_for_response(self):
        self.action_count += 1
        result = super().wait_for_response()
        if self.level > self.peak_level:
            self.peak_level = self.level
        return result


def run_client_thread(port, team, host, results, index):
    client = EvalClient(port, team, host)
    if client.connect() != 0:
        results[index] = None
        return
    run_client(client)
    results[index] = {
        "team": team,
        "actions": client.action_count,
        "peak_level": client.peak_level,
        "final_level": client.level,
    }


def run_batch(port, host, teams, clients_per_team):
    threads = []
    n_total = len(teams) * clients_per_team
    results = [None] * n_total
    idx = 0

    for team in teams:
        for _ in range(clients_per_team):
            t = threading.Thread(
                target=run_client_thread,
                args=(port, team, host, results, idx),
            )
            threads.append(t)
            t.start()
            idx += 1
            time.sleep(0.05)

    for t in threads:
        t.join()

    return [r for r in results if r is not None]


def wait_for_server(host, port, timeout=15):
    start = time.time()
    while time.time() - start < timeout:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(1)
            s.connect((host, port))
            s.close()
            return True
        except (ConnectionRefusedError, OSError):
            time.sleep(0.3)
    return False


def build_server_cmd(server_cmd, port, teams, clients_per_team, server_args):
    team_list = " ".join(teams)
    return f"{server_cmd} -p {port} -n {team_list} -c {clients_per_team} {server_args}"


def main():
    parser = argparse.ArgumentParser(
        description="Evaluate heuristic AI performance across multiple multi-agent runs"
    )
    parser.add_argument(
        "-p", type=int, dest="port", default=8080, help="port number (default: 8080)"
    )
    parser.add_argument(
        "-ip",
        type=str,
        dest="host",
        default="127.0.0.1",
        help="server host (default: 127.0.0.1)",
    )
    parser.add_argument(
        "--runs",
        type=int,
        default=5,
        help="number of evaluation runs (default: 5)",
    )
    parser.add_argument(
        "--teams",
        type=int,
        default=4,
        help="number of teams (default: 4)",
    )
    parser.add_argument(
        "--clients",
        type=int,
        default=6,
        help="concurrent clients per team (default: 6)",
    )
    parser.add_argument(
        "--server-cmd",
        type=str,
        default="",
        help="path to zappy_server binary (auto-starts server)",
    )
    parser.add_argument(
        "--server-args",
        type=str,
        default="-x 10 -y 10 -f 100",
        help="server CLI args (default: -x 10 -y 10 -f 100)",
    )

    args = parser.parse_args()
    port = args.port
    host = args.host
    n_runs = args.runs
    teams = [f"Team{i}" for i in range(1, args.teams + 1)]
    clients_per_team = args.clients

    server_proc = None

    if args.server_cmd:
        full_cmd = build_server_cmd(
            args.server_cmd, port, teams, clients_per_team, args.server_args
        )
        print(f"[INFO] Starting server: {full_cmd}")
        server_proc = subprocess.Popen(
            full_cmd.split(),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

        def cleanup(*_):
            if server_proc and server_proc.poll() is None:
                server_proc.terminate()
                try:
                    server_proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    server_proc.kill()

        signal.signal(signal.SIGINT, cleanup)
        signal.signal(signal.SIGTERM, cleanup)

        if not wait_for_server(host, port):
            print("[ERROR] Server did not start in time")
            server_proc.kill()
            return 1
        print("[INFO] Server is ready")
    else:
        print(f"[INFO] Connecting to existing server at {host}:{port}")
        print(f"[INFO] Teams: {', '.join(teams)}")

    all_runs = []

    try:
        for run_idx in range(n_runs):
            print(
                f"\n[INFO] Run {run_idx + 1}/{n_runs} "
                f"({args.teams} teams × {clients_per_team} clients = "
                f"{args.teams * clients_per_team} total)..."
            )
            results = run_batch(port, host, teams, clients_per_team)

            if not results:
                print("  FAILED — no clients connected")
                continue

            all_runs.append(results)

            by_team = defaultdict(list)
            for r in results:
                by_team[r["team"]].append(r)

            print(f"  Connected: {len(results)}/{args.teams * clients_per_team}")
            for team in teams:
                team_results = by_team.get(team, [])
                if team_results:
                    lvls = [r["final_level"] for r in team_results]
                    acts = [r["actions"] for r in team_results]
                    print(
                        f"    {team}: "
                        f"avg_lvl={mean(lvls):.2f} "
                        f"max_lvl={max(lvls)} "
                        f"avg_actions={mean(acts):.0f}"
                    )
                else:
                    print(f"    {team}: no connections")

            if server_proc and run_idx < n_runs - 1:
                print("[INFO] Restarting server for next run...")
                server_proc.terminate()
                try:
                    server_proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    server_proc.kill()
                time.sleep(3)

                server_proc = subprocess.Popen(
                    full_cmd.split(),
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
                if not wait_for_server(host, port):
                    print("[ERROR] Server restart failed")
                    return 1
                print("[INFO] Server is ready")

    finally:
        if server_proc and server_proc.poll() is None:
            server_proc.terminate()
            try:
                server_proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                server_proc.kill()

    if not all_runs:
        print("[ERROR] No successful runs completed")
        return 1

    # Aggregate
    all_actions = [r["actions"] for run in all_runs for r in run]
    all_final = [r["final_level"] for run in all_runs for r in run]
    all_peak = [r["peak_level"] for run in all_runs for r in run]
    total_clients = len(all_actions)

    print()
    print("=" * 62)
    print("  HEURISTIC AI EVALUATION RESULTS")
    print("=" * 62)
    print(f"  Runs completed:              {len(all_runs)}/{n_runs}")
    print(f"  Teams:                       {args.teams}")
    print(f"  Clients per team:            {clients_per_team}")
    print(f"  Total clients evaluated:     {total_clients}")
    print(f"  Server:                      {host}:{port}")
    print(f"  Map:                         {args.server_args}")
    print()
    print("  Actions (turns survived):")
    print(f"    Mean:                      {mean(all_actions):.1f}")
    print(f"    Median:                    {median(all_actions):.0f}")
    if len(all_actions) >= 2:
        print(f"    StdDev:                    {stdev(all_actions):.1f}")
    print(f"    Min:                       {min(all_actions)}")
    print(f"    Max:                       {max(all_actions)}")
    print()
    print("  Final level:")
    print(f"    Mean:                      {mean(all_final):.2f}")
    print(f"    Median:                    {median(all_final):.0f}")
    print(f"    Max:                       {max(all_final)}")
    print()
    print("  Peak level:")
    print(f"    Mean:                      {mean(all_peak):.2f}")
    print(f"    Median:                    {median(all_peak):.0f}")
    print(f"    Max:                       {max(all_peak)}")
    print()

    level_dist = Counter(all_final)
    print("  Final level distribution (all clients):")
    for level in sorted(level_dist):
        pct = level_dist[level] / total_clients * 100
        bar = "█" * max(1, int(pct / 4))
        print(f"    Level {level}: {level_dist[level]:>3} ({pct:5.1f}%) {bar}")
    print()

    print("  Per-run summary:")
    header = f"    {'Run':>4} | {'Connected':>9} | {'AvgLvl':>6} | {'MaxLvl':>6} | {'AvgActions':>10} | {'MaxActions':>10}"
    print(header)
    print("    " + "-" * (len(header) - 4))
    for idx, run_results in enumerate(all_runs):
        lvls = [r["final_level"] for r in run_results]
        acts = [r["actions"] for r in run_results]
        print(
            f"    {idx + 1:>4} | {len(run_results):>9} | "
            f"{mean(lvls):>6.2f} | {max(lvls):>6} | "
            f"{mean(acts):>10.0f} | {max(acts):>10}"
        )
    print("=" * 62)

    return 0


if __name__ == "__main__":
    sys.exit(main())
