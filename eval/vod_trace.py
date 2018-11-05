import numpy as np
import matplotlib.pyplot as plt

users_count = 220
groups_count = 15

g = []
# each group has at least a member
left_users = users_count - groups_count
for i in range(0, groups_count):
    g.append(1)

a = 2.2 # parameter
id_max = 0
for i in range(0, left_users + 1):
    s = np.random.zipf(a, 1) - 1
    if s>id_max:
        id_max=s
    g[s[0] % groups_count] += 1

print("Quality of sampling : 15000 vs", id_max)
print(g[0:15])

# -------------------------------------------------------------
total_events = 9000000
# sample total_events from G as a discrete distribution

# the samples correspond to user adds
# for each add, insert a remove operation in 30 days
