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
import time
from matplotlib import ticker

# seconds
MEASUREMENT_INTERVAL = 64.0 / 3000.0

# mA
CURRENT_FACTOR = 70.0 / 4095.0

# mA * 3300mV = microJoule (uJ)
CURRENT_DISPLAY_FACTOR = 3300.0

# mA
IDLE_CONSUMPTION = (0.55 + 0.62 + 0.78 + 0.45 + 1.1 + 0.9 + 0.85 + 0.55 + 0.85\
        + 0.81 + 1.05 + 0.39 + 0.41 + 0.76 + 0.8 + 0.55 + 0.82 + 0.61 + 0.75 + 0.6) / 20.0
#print(IDLE_CONSUMPTION)

rc('font',**{'family':'serif','serif':['Palatino'], 'size': 12})
rc('text', usetex=True)
fs = (12, 5)

#blacklist = set([10019, 10029])

class Timer:
    def __init__(self, name):
        self.name = name

    def __enter__(self):
        print('{}...'.format(self.name))
        self.t = time.time()

    def __exit__(self, *args):
        print(' {} done ({}s)'.format(self.name, time.time() - self.t))


def parse(fn):
    with Timer("reading {}...".format(fn)):
        data = np.genfromtxt(fn, delimiter = '\t', skip_header=0, names=True, usecols=('avg','motelabMoteID'),
                dtype=[('avg', 'i4'), ('motelabMoteID', 'i4')])

    blacklist = set([10019]) | {
            '25458.csv': set([10029]),
            '25464.csv': set([10005, 10010]),
            '25465.csv': set([10005, 10017]),
            '25466.csv': set([10200]),
            '25469.csv': set([10035, 10056, 10037, 104117, 10199, 10200]),
            '25470.csv': set([10033, 10030]),
            '25473.csv': set([10039]),
            '25474.csv': set([10027, 10041]),
            '25475.csv': set([10031, 10027, 10037, 10044]),
            '25476.csv': set([]),
            '25477.csv': set([10007,10010,10023,10046,10047]),
            '25478.csv': set([10042,10018,10008,10025]),
            '25479.csv': set([10035]),
            '25480.csv': set([]),
            '25487.csv': set([10037, 10031, 10009, 10038]),
            '25488.csv': set([10009, 10199]),
            '25489.csv': set([]),
            '25493.csv': set([10037, 10044]),
            '25516.csv': set([10027, 10050, 10042, 10037, 10044]),
            '25515.csv': set([10010]),
    }.get(fn, set())

    with Timer('refudelling'):
        # split into columns
        d = defaultdict(list)
        for avg, mote_id in data:
            if mote_id not in blacklist:
                d[mote_id].append(avg)

    with Timer('converting'):
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
    #ret[n:] = ret[n:] / n
    #return ret[n - 1:] / n
    return ret / n

def plot(ax, vs, name, style):
    print("plotting {}...".format(name))
    #ax.set_xlim((50000 * MEASUREMENT_INTERVAL, 100000 * MEASUREMENT_INTERVAL))
    #ax.set_xlim((2000, 2100))
    #ax.set_ylim((0, 2))
    #for k, vs in d.items():
    ts = np.arange(len(vs)) * MEASUREMENT_INTERVAL
    vs = vs * CURRENT_FACTOR
    #ax.plot(ts, vs, color='#aadddd')

    vs = moving_average(vs, int(60.0 / MEASUREMENT_INTERVAL)) # avg over 10s
    ax.plot(ts, vs, label=name, **style)

    #ax.plot([0, ts[-1]], [IDLE_CONSUMPTION, IDLE_CONSUMPTION], color='#ff9999', linewidth=3)


#d = parse('25446.csv')
#d = parse('25458.csv.tmp')


def plotone(vs, name):
    #print("plotting {}...".format(name))
    fig = plt.figure(figsize=fs)
    ax = fig.add_subplot(111)
    #ax.set_ylim((0, 10))
    ax.set_ylabel('$I$ / mA')
    ax.set_xlabel('$t$ / s')
    #ax.set_xlim((50000 * MEASUREMENT_INTERVAL, 100000 * MEASUREMENT_INTERVAL))
    #ax.set_xlim((600, 1000))
    #ax.set_xlim((760, 850))
    #ax.set_ylim((0, 2))
    #for k, vs in d.items():
    ts = np.arange(len(vs)) * MEASUREMENT_INTERVAL
    vs = vs * CURRENT_FACTOR
    #ax.plot(ts, vs, color='#aadddd')

    vs = moving_average(vs, int(1.0 / MEASUREMENT_INTERVAL)) # avg over 10s
    #ax.set_xlim((500, 510))
    ax.plot(ts, vs, 'k-')
    ax.grid()
    #fig.savefig('energy_{}.pdf'.format(name), bbox_inches='tight', pad_inches=.1)
    fig.savefig('energy_{}.png'.format(name))
    plt.close(fig)

data = [
        #('NullRDC @20', parse('25458.csv.tmp'), {}),
        #('ContikiMAC 16 @20', parse('25464.csv.tmp'), {'color': 'black', 'linestyle': '--'}),
        #('AINSE @40', parse('25466.csv'), {'color': 'black'}),
        #('AINSE \& ContikiMAC/16 @40', parse('25469.csv'), {'color': 'black', 'linestyle': '--'}),
        #('AINSE \& XMAC/16 @40', parse('25470.csv'), {'color': 'black', 'linestyle': ':'}),
        #('NullRDC @40', parse('25465.csv'), {}),
        #('No Scheduling @40', parse('25471.csv'), {'color': 'black', 'linestyle': ':', 'linewidth': 1}),
        #('ContikiMAC @40', parse('25472.csv'), {'color': 'black', 'linestyle': '--', 'linewidth': 1}),
        #('ContikiMAC Quiet @40', parse('25473.csv'), {'color': 'black', 'linestyle': '--', 'linewidth': 1}),
        #('AINSE \& ContikiMAC CSMA Quiet NOLED @40', parse('25476.csv'), {'color': 'black', 'linestyle': '-', 'linewidth': 1}),
        #('AINSE NOLED CSMA @40', parse('25477.csv'), {'color': 'black', 'linestyle': '-', 'linewidth': 2}),

        #('ContikiMAC Quiet NOLED @40', parse('25481.csv'), {'color': 'black', 'linestyle': '-', 'linewidth': 1}),
        #('ContikiMAC Quiet NOLED @40', parse('25474.csv'), {'color': 'black', 'linestyle': '-', 'linewidth': 1}),

        # XXX
        ('No Scheduling', parse('25515.csv'), {'color': 'black', 'linestyle': '--', 'linewidth': 2}),
        ('AINSE', parse('25516.csv'), {'color': 'black', 'linestyle': '-', 'linewidth': 2}),
        ('ContikiMAC', parse('25493.csv'), {'color': 'black', 'linestyle': '--', 'linewidth': 1}),
        ('AINSE \& ContikiMAC', parse('25475.csv'), {'color': 'black', 'linestyle': '-', 'linewidth': 1}),

        #('LPP', parse('25520.csv'), {'color': 'black', 'linestyle': '--', 'linewidth': 1}),
        #('NullRDC Loud 2 @40', parse('25518.csv'), {}),
        #('AINSE NOLED @40', parse('25480.csv'), {'color': 'black', 'linestyle': '-', 'linewidth': 2}),

        # XMAC
        # Problem: XMAC seems to sometimes leave the radio *completely* on for
        # a whole token phase --> BUG?
        #('XMAC Quiet NOLED @40', parse('25482.csv'), {'color': 'black', 'linestyle': '-', 'linewidth': 1}),
        #('AINSE \& XMAC', parse('25487.csv'), {'color': 'black', 'linestyle': '-', 'linewidth': 1}),
        #('AINSE \& XMAC CSMA NOLED @40', parse('25488.csv'), {'color': 'black', 'linestyle': '-', 'linewidth': 1}),
        #('XMAC', parse('25489.csv'), {'color': 'black', 'linestyle': '--', 'linewidth': 1}),
]

fig = plt.figure(figsize=fs)
ax = fig.add_subplot(111)
#ax.set_ylim((0, 10))
ax.set_ylabel('$I$ / mA')
ax.set_xlabel('$t$ / s')

ax.set_xlim((0, 3600))
ax.set_yscale('log')
ax.set_ylim((.7, 21))
ax.yaxis.set_major_formatter(ticker.ScalarFormatter())
ax.yaxis.set_minor_formatter(ticker.FormatStrFormatter('%.1f'))

for label, d, style in data:
    l = min((len(x) for x in d.values() if len(x)))
    print("l({})={}".format(label, l))
    print('\n'.join(['{}:{}'.format(k, len(x)) for k, x in d.items()]))
    sums = np.zeros(l)
    for k,v in d.items():
        plotone(v, label + '_' + str(k))
        sums += v[:l]

    plot(ax, sums / len(d), label, style)

#ax.plot([0, 3600], [IDLE_CONSUMPTION, IDLE_CONSUMPTION], ':', linewidth=2, label='idle')
#ax.plot([0, 3600], [IDLE_CONSUMPTION, IDLE_CONSUMPTION], ':', linewidth=2, label='idle 2')
#ax.plot([0, 3600], [IDLE_CONSUMPTION, IDLE_CONSUMPTION], ':', linewidth=2, label='idle 3')
#ax.plot([0, 3600], [IDLE_CONSUMPTION, IDLE_CONSUMPTION], ':', linewidth=2, label='idle 4')

ax.grid(True, which='both')
ax.legend(bbox_to_anchor=(1.0, 0.9), loc='upper right')
#ax.legend(loc='upper right')

fig.savefig('energy_sum.png')
fig.savefig('energy_sum.pdf', bbox_inches='tight', pad_inches=.1)
plt.close(fig)

# vim: set ts=4 sw=4 expandtab:

