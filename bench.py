import csv, os, subprocess, sys, tomllib, time, random
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
    return data["defaults"], data["warmup"], data["job"]
    
def printBar(done, total, start):
    elapsed = time.monotonic() - start
    if done == 0:
        eta = 0
    else:
        eta = (elapsed / done) * (total - done)
    filledBar = int(PROG_BAR_LENGTH * done / total)
    bar = "█" * filledBar + "░" * (PROG_BAR_LENGTH - filledBar)
    print(f"\r|{bar}| [{done}/{total}] ETA {int(eta/60)}m{int(eta%60)}s   ", end="")
    return

def main():
    print(f"{BOLD + RED}╔" + "═" * 47 + "╗")
    print(f"║ {CYAN}MATMULOPT BENCHMARK ".ljust(53) + f"{RED}║")
    print(f"{RED}╚" + "═" * 47 + "╝")
    os.makedirs("results", exist_ok=True)
    csvPath = "results/"+os.path.splitext(os.path.basename(sys.argv[1]))[0]+".csv"
    defaults, warmup, jobs = loadConfig(sys.argv[1])
    with open(csvPath, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(HEADER)
        # Warmup
        startTime = time.monotonic()
        warmup = {**defaults, **warmup}
        print(f"[{datetime.now():%H:%M:%S}] WARMUP {warmup['seconds']}s ")
        printBar(0, warmup["seconds"], startTime)
        while time.monotonic() - startTime < warmup["seconds"]:
            runMatmulopt(warmup["fIdx"], warmup["matSize"], warmup["threads"], warmup["blockSize"], warmup["parallel"])
            printBar(min(int(time.monotonic() - startTime),warmup["seconds"]), warmup["seconds"], startTime)
        print("")
        # Jobs
        allRuns = []
        for job in jobs:
            full = {**defaults, **job}
            runs = expand(full)
            for run in runs:
                for rep in range(0, run["reps"]):
                    allRuns.append(run)
        
        totalRuns = len(allRuns)
        doneRuns = 0
        random.seed(42)
        random.shuffle(allRuns)
        print(f"[{datetime.now():%H:%M:%S}] RUNNING BENCHMARK...")
        startTime = time.monotonic()
        printBar(doneRuns, totalRuns, startTime)
        for run in allRuns:     
            output = runMatmulopt(run["fIdx"], run["matSize"], run["threads"], run["blockSize"], run["parallel"])
            writer.writerow(output)
            f.flush()
            # Runs progress bar
            doneRuns = doneRuns + 1
            printBar(doneRuns, totalRuns, startTime)
        print()
    print(f"[{datetime.now():%H:%M:%S}] BENCHMARK COMPLETE" + RESET)
main()
