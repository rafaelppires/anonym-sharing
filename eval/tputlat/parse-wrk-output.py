#!/usr/bin/env python3

import re
from fileinput import input

def parse(lines):
    print("Req/s\tLatency(ms)")
    for line in lines:
        match = re.search(r"\s*Latency\s*(\d+(\.\d+)?)ms\s*.+", line)
        if match is not None:
            latency_ms = match.group(1)
        match = re.search(r"\s*Latency\s*(\d+(\.\d+)?)s\s*.+", line)
        if match is not None:
            latency_ms = str(float(match.group(1)) * 1000)
        match = re.search(r"^Requests/sec:\s*(\d+(\.\d+)?)$", line)
        if match is not None:
            tput_req_s = match.group(1)
            print("\t".join((tput_req_s, latency_ms)))

if __name__ == '__main__':
    parse(input())

