#!/usr/bin/env python3

import numpy as np
from collections import defaultdict
import matplotlib.pyplot as plt
from pylab import setp
import math
import re
import io
from matplotlib import rc
import sys

# seconds
MEASUREMENT_INTERVAL = 64.0 / 3000.0

# mA
CURRENT_FACTOR = 70.0 / 4095.0

# mA * 3300mV = microJoule (uJ)
CURRENT_DISPLAY_FACTOR = 3300.0

# mA
IDLE_CONSUMPTION = (0.55 + 0.62 + 0.78 + 0.45 + 1.1 + 0.9 + 0.85 + 0.55 + 0.85\
    + 0.81 + 1.05 + 0.39 + 0.41 + 0.76 + 0.8 + 0.55 + 0.82 + 0.61 + 0.75 + 0.6) #/ 20.0

print(IDLE_CONSUMPTION)

rc('font',**{'family':'serif','serif':['Palatino'], 'size': 8})
rc('text', usetex=True)
fs = (8, 3)

blacklist = set([10019, 10029])

def parse(fn):
    print("reading csv...")
    data = np.genfromtxt(fn, delimiter = '\t', skip_header=0, names=True, usecols=('avg','motelabMoteID'),
            dtype=[('avg', 'i4'), ('motelabMoteID', 'i4')])

    #print(data)

    print("refuddeling...")
    # split into columns
    d = defaultdict(list)
    for avg, mote_id in data:
        if mote_id not in blacklist:
            d[mote_id].append(avg)


    #print("printing...")

    for k in d.keys():
        d[k] = np.array(d[k])
        #print(k, d[k])

    return d

def moving_average(a, n=3) :
    """
    sauce: http://stackoverflow.com/questions/14313510/moving-average-function-on-numpy-scipy
    """
    ret = np.cumsum(a, dtype=float)
    #ret[n:] = ret[n:] - ret[:-n]
    ret[n:] = ret[n:] - ret[:-n]
    #return ret[n - 1:] / n
    return ret / n

def plot(vs, name):
    print("plotting {}...".format(name))
    fig = plt.figure(figsize=fs)
    ax = fig.add_subplot(111)
    #ax.set_xlim((50000 * MEASUREMENT_INTERVAL, 100000 * MEASUREMENT_INTERVAL))
    #ax.set_xlim((2000, 2100))
    #ax.set_ylim((0, 2))
    #for k, vs in d.items():
    ts = np.arange(len(vs)) * MEASUREMENT_INTERVAL
    vs = vs * CURRENT_FACTOR
    ax.plot(ts, vs, color='#aadddd')

    vs = moving_average(vs, int(10.0 / MEASUREMENT_INTERVAL)) # avg over 10s
    ax.plot(ts, vs, 'k-', linewidth=1)

    ax.plot([0, ts[-1]], [IDLE_CONSUMPTION, IDLE_CONSUMPTION], color='#ff9999',
            linewidth=3)

    ax.grid()

    fig.savefig('energy_{}.png'.format(name))
    fig.savefig('energy_{}.pdf'.format(name), bbox_inches='tight', pad_inches=.1)
    plt.close(fig)

#d = parse('25446.csv')
d = parse('25458.csv.tmp')

l = min((len(x) for x in d.values() if len(x)))
print("l=", l)
print([len(x) for x in d.values()])
sums = np.zeros(l)
for k,v in d.items():
    #plot(v, str(k))
    sums += v[:l]

plot(sums, 'sum')

