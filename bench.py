import csv, os, subprocess, sys, tomllib
from datetime import datetime

HEADER = ["functionIdx", "matSize", "threadsNum", "blockSize", "parallel", "time"]

RESET = "\033[0m"
BOLD = "\033[1m"
RED = "\033[31m"
CYAN = "\033[36m"

PROG_BAR_LENGTH = 30

def runMatmulopt(fIdx, matSize, threads, blockSize, parallel):
    command = ["./matmulopt",
        "-f", str(fIdx),
        "-n", str(matSize), 
        "-t", str(threads),
        "-b", str(blockSize),
        "--csv"
    ]
    if parallel:
        command.append("-p")
        
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        print(result.stderr)
    return result.stdout.strip().split(",")
        
def expand(job):
    runs = []
    for matSize in job["matSize"]:
        for threads in job["threads"]:
            for blockSize in job["blockSize"]:
                runs.append({**job, **{"matSize":matSize,"threads":threads,"blockSize":blockSize}})
                #print("Job registered")
    return runs
    
def loadConfig(path):
    with open(path, "rb") as f:
        data = tomllib.load(f)
    return data["defaults"], data["job"]

def main():
    print(f"{BOLD + RED}╔" + "═" * 47 + "╗")
    print(f"║ {CYAN}MATMULOPT BENCHMARK ".ljust(53) + f"{RED}║")
    print(f"{RED}╚" + "═" * 47 + "╝")
    os.makedirs("results", exist_ok=True)
    csvPath = "results/"+os.path.splitext(os.path.basename(sys.argv[1]))[0]+".csv"
    defaults, jobs = loadConfig(sys.argv[1])
    with open(csvPath, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(HEADER)
    
        for job in jobs:
            full = {**defaults, **job}
            runs = expand(full)
            for run in runs:
                print(f"[{datetime.now():%H:%M:%S}] {run["matSize"]}x{run["matSize"]} | F{run["fIdx"]} "
                f"| {run["threads"]}T | {run["blockSize"]}B")
                for rep in range(0, run["reps"]):
                    output = runMatmulopt(run["fIdx"], run["matSize"], run["threads"], run["blockSize"], run["parallel"])
                    writer.writerow(output)
                    f.flush()
                    # Job progress bar
                    doneReps = rep + 1
                    totalReps = run["reps"]
                    filledBar = int(PROG_BAR_LENGTH * doneReps / totalReps)
                    bar = "█" * filledBar + "░" * (PROG_BAR_LENGTH - filledBar)
                    print(f"\r|{bar}| [{doneReps}/{totalReps}]", end="")
                print()
    print(RESET)
main()
