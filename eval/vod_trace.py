import csv
import numpy as np
import matplotlib.pyplot as plt
import operator

users_count = 220000
groups_count = 150000
total_events = 9000000


g = []
# each group has at least a member
left_users = users_count - groups_count
for i in range(0, groups_count):
    g.append(1)

# model group popularity by zipf's law
a = 2.2 # parameter
id_max = 0
for i in range(0, left_users + 1):
    s = np.random.zipf(a, 1) - 1
    if s>id_max:
        id_max=s
    g[s[0] % groups_count] += 1

print("Quality of sampling : 15000 vs", id_max)
print(g[0:15])

# sample group add from discrete distribution
p = []
for i in g:
    p.append(i / users_count)
over_flow = sum(p) - 1
p[0] -= over_flow
print(sum(p))
events = np.random.choice(groups_count, total_events, True, p)

# construct trace
trace = []
total_ms = 11 * 7 * 24 * 3600 * 1000 # i.e. 11 weeks
days30_ms = 30 * 24 * 3600 * 1000
dist_add_ms = total_ms / total_events
print("Ticks between adds :", dist_add_ms)
for i in range(len(events)):
    user = np.random.randint(users_count, size=1)[0]
    trace.append((i * dist_add_ms, "a", events[i], user))
    trace.append((i * dist_add_ms + days30_ms, "r", events[i], user))

# serialize to CSV
trace.sort(key=lambda x: x[0])
with open('vod_trace.csv', mode='w') as trace_file:
    trace_writer = csv.writer(trace_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
    trace_writer.writerow(["tick", "operation", "group", "user"])
    i = 0
    for t in trace:
        trace_writer.writerow([i, t[1], t[2], t[3]])
        i += 1
        if (i%100000 == 0):
            print(i)

print(events[0:30])
print(trace[0:5])
