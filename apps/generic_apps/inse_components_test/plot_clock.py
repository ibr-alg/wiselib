#!/usr/bin/env python3

from pylab import setp
import io
import math
import matplotlib.pyplot as plt
import numpy as np
import os
import re
import sys

from matplotlib import rc

re_clk = re.compile(r'T([0-9]+)\.([0-9]+)\|@inode019 clk ([0-9]+) ([0-9]+)$')
re_clk2 = re.compile(r'T([0-9]+)\.([0-9]+)\|@inode055 Sc:esp ([0-9]+)$')
#re_clk = re.compile(r'T([0-9]+)\.([0-9]+)\|@1 clk ([0-9]+) ([0-9]+)$')

def parse(f):
    global t0
    global c0
    clock_t = []
    clock_clock = []
    t0 = 0
    c0 = 0
    for line in f:
        m = re.match(re_clk, line)
        if m is not None:
            print(m.groups())
            t_s, t_ns, count, clock = m.groups()
            t = int(t_s) + int(t_ns)* 10**(-9)
            c = int(clock) / 1000.0
            if t0 == 0 or t < t0: t0 = t
            if c0 == 0 or c < c0: c0 = c
            clock_t.append(t)
            clock_clock.append(c)

        m = re.match(re_clk2, line)
        if m is not None:
            print(m.groups())
            t_s, t_ns, clock = m.groups()
            t = int(t_s) + int(t_ns)* 10**(-9)
            c = int(clock) / 1000.0
            if t0 == 0 or t < t0: t0 = t
            if c0 == 0 or c < c0: c0 = c
            clock_t.append(t)
            clock_clock.append(c)

    return clock_t, clock_clock


xs, ys = parse(open(sys.argv[1], 'r'))

for i in range(len(xs)): xs[i] -= t0
for i in range(len(ys)): ys[i] -= c0

fig = plt.figure()
ax = plt.subplot(111)

ax.grid()
#ax.plot([0, 400], [0, 400], '--')
ax.plot(xs, ys, 'o-')

fig.savefig('clock.pdf')



