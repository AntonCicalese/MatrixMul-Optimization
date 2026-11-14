import csv, os, subprocess

DEFAULTS = {"fIdx": 1, "matSize": [1024], "threads": [1],
    "blockSize": [64], "parallel": 0, "reps": 2}

JOBS = [
    {"fIdx": 0, "matSize": [128]},
    {"fIdx": 1, "matSize": [128]},
    {"fIdx": 1, "matSize": [128], "threads": [1, 2], "parallel": 1}
]

HEADER = ["functionIdx", "matSize", "threadsNum", "blockSize", "parallel", "time"]

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
    return runs

def main():
    path = "results/bench.csv"
    os.makedirs("results", exist_ok=True)
    
    with open(path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(HEADER)
    
        for job in JOBS:
            full = {**DEFAULTS, **job}
            runs = expand(full)
            for run in runs:
                for rep in range(0, run["reps"]):
                    output = runMatmulopt(run["fIdx"], run["matSize"], run["threads"], run["blockSize"], run["parallel"])
                    writer.writerow(output)
                    f.flush()
    
main()