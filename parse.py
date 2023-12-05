import re, os, glob
import linecache

path = "output"

csv = open("re.csv", "a")

for filename in glob.glob(os.path.join(path, "*.txt")):
    #with open(filename, "r") as f:
       # 10000 2,4,5,6,10 
    line = linecache.getline(filename,10002)
    s = re.search(r"val_trace_(\w*)1", line)
    trace = s.group(1)

    line = linecache.getline(filename,10004)
    s = re.search(r"ROB_SIZE\s*=\s*(\d*)", line)
    rob = s.group(1)

    line = linecache.getline(filename,10005)
    s = re.search(r"IQ_SIZE\s*=\s*(\d*)", line)
    iq = s.group(1)

    line = linecache.getline(filename,10006)
    s = re.search(r"WIDTH\s*=\s*(\d*)", line)
    w = s.group(1)

    line = linecache.getline(filename,10010)
    s = re.search(r"\(IPC\)\s*=\s*(\d*\.\d*)", line)
    ipc = s.group(1)

    data = str(trace)+","+str(rob)+","+str(iq)+","+str(w)+","+str(ipc)+"\n"

    csv.write(data)

csv.close()

# === Simulator Command =========
# src/sim 16 8 1 traces/val_trace_gcc1
# === Processor Configuration ===
# ROB_SIZE = 16
# IQ_SIZE  = 8
# WIDTH    = 1
# === Simulation Results ========
# Dynamic Instruction Count    = 10000
# Cycles                       = 10263
# Instructions Per Cycle (IPC) = 0.97
