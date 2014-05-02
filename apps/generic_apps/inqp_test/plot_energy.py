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
import os.path
import pickle

sys.path.append('/home/henning/bin')
from experiment_utils import quantize, fold, median, join_boxes, average, materialize

PLOT_DIR = 'plots'

# seconds
MEASUREMENT_INTERVAL = 64.0 / 3000.0

# mA
CURRENT_FACTOR = 70.0 / 4095.0

# mA * 3300mV = microJoule (uJ)
CURRENT_DISPLAY_FACTOR = 3300.0

# mA
IDLE_CONSUMPTION = (0.55 + 0.62 + 0.78 + 0.45 + 1.1 + 0.9 + 0.85 + 0.55 + 0.85\
        + 0.81 + 1.05 + 0.39 + 0.41 + 0.76 + 0.8 + 0.55 + 0.82 + 0.61 + 0.75 + 0.6) / 20.0


EXPERIMENT_INTERVAL = 300.0
BOX_INTERVAL = 5.0



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
    pickled_fn = fn + '.p'
    if os.path.exists(pickled_fn):
        with Timer("unpickling {}".format(pickled_fn)):
            return pickle.load(open(pickled_fn, 'rb'))

    with Timer("reading {}".format(fn)):
        data = np.genfromtxt(fn, delimiter = '\t', skip_header=0, names=True, usecols=('avg','motelabMoteID'),
                dtype=[('avg', 'i4'), ('motelabMoteID', 'i4')])

    energy_measurements_broken = set([
        10004, 10037, 10044
    ])

    # Nodes that did not connect (permanently) in 3600s
    always_active = set([
        10008, 10009, 10013, 10050
    ])

    # Nodes that consume energy as if the radio was permanently on when
    # a MAC layer should have been active
    high_energy =  set([
        10042
    ])

    root = set([
        #10029
    ])

    blacklist = root | energy_measurements_broken | always_active | {
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
            '25577.csv': set([10014, 10007, 10004, 10037, 10044]),
            '26034.csv': set([10200]),
            '26045.csv': set([10200]),
            '26064.csv': set([10007, 10011, 10027, 10029, 10039]),
            '26073.csv': set([10012, 10027, 10029, 10033, ]),
    }.get(fn, set())

    with Timer('refudelling'):
        # split into columns
        d = defaultdict(list)
        for avg, mote_id in data:
            if mote_id not in blacklist:
                d[mote_id].append(avg)

    print("  considered {} motes".format(len(d)))

    with Timer('converting'):
        for k in d.keys():
            d[k] = np.array(d[k])
            #print(k, d[k])

    with Timer("pickling {}".format(pickled_fn)):
        pickle.dump(d, open(pickled_fn, 'wb'))

    return d

def moving_average(a, n=3) :
    """
    sauce: http://stackoverflow.com/questions/14313510/moving-average-function-on-numpy-scipy
    """
    ret = np.cumsum(a, dtype=float)
    ret[n:] = (ret[n:] - ret[:-n]) / n
    ret[:n] = ret[:n] / (np.arange(n) + 1)
    return ret

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


def plotone(vs, name, style):
    #print("plotting {}...".format(name))
    fig = plt.figure(figsize=fs)
    ax = fig.add_subplot(111)
    #ax.set_ylim((0, 10))
    ax.set_ylabel('$I$ / mA')
    ax.set_xlabel('$t$ / s')
    ax.set_ylim((0, 5))
    #ax.set_xlim((50000 * MEASUREMENT_INTERVAL, 100000 * MEASUREMENT_INTERVAL))
    #ax.set_xlim((1000, 1500))
    #ax.set_xlim((760, 850))
    #ax.set_ylim((0, 2))
    #for k, vs in d.items():
    ts = np.arange(len(vs)) * MEASUREMENT_INTERVAL
    vs = vs * CURRENT_FACTOR
    #ax.plot(ts, vs, color='#aadddd')

    vs = moving_average(vs, int(5.0 / MEASUREMENT_INTERVAL)) # avg over 10s
    #ax.set_xlim((500, 510))
    ax.plot(ts, vs, **style)
    ax.grid()
    #fig.savefig(PLOT_DIR + '/energy_{}.pdf'.format(name), bbox_inches='tight', pad_inches=.1)
    fig.savefig(PLOT_DIR + '/energy_{}.png'.format(name), bbox_inches='tight', pad_inches=.1)
    plt.close(fig)


def compute_boxplot(vs):
    #box_entries = int(10.0 / MEASUREMENT_INTERVAL)
    box_entries = int(BOX_INTERVAL / MEASUREMENT_INTERVAL)

    ts = np.arange(len(vs)) * MEASUREMENT_INTERVAL
    return fold(quantize(zip(ts, vs), box_entries), int(EXPERIMENT_INTERVAL / (box_entries * MEASUREMENT_INTERVAL)), skip = 1)

def style_box(bp, s):
    for key in ('boxes', 'whiskers', 'fliers', 'caps', 'medians'):
        plt.setp(bp[key], **s)
    plt.setp(bp['fliers'], marker='+')

def data_to_boxes(d, label, style):
    #l = min((len(x) for x in d.values() if len(x)))
    #l = None
    #for k, v in d.values():

    l, k = max(((len(v), k) for k, v in d.items() if len(v)))
    print("max l={} k={}".format(l, k))
    l, k = min(((len(v), k) for k, v in d.items() if len(v)))
    print("min l={} k={}".format(l, k))

    #print("l({})={}".format(label, l))
    with Timer("summing up"):
        sums = np.zeros(l)
        for k,v in d.items():
            #plotone(v, label + '_' + str(k), style)
            sums += v[:l]
    with Timer("computing box plot"):
        r = compute_boxplot(CURRENT_FACTOR * sums / len(d))
    r = materialize(r)
    return r

def plot_boxes(ax, it, label, style):
    #print(list(it))
    vs2 = [v for (t, v) in it]
    print("-- {} {}".format(label, ' '.join(str(len(x)) for x in vs2)))
    bp = ax.boxplot(vs2)
    style_box(bp, style)
    ax.plot(range(1, len(vs2) + 1), [average(x) for x in vs2], label=label, **style)

#data = [
        ##('Test', parse('26034.csv'), {'color': '#bbaa88'}), #{'color': '#88bbbb'}),
        ##('Simple Temperature', parse('26034.csv'), {'color': '#dd7777'}), #{'color': '#88bbbb'}),
        #('Simple Temperature II', parse('26052.csv'), {'color': '#dd7777'}), #{'color': '#88bbbb'}),
        #('Simple Temperature III', parse('26064.csv'), {'color': 'black'}), #{'color': '#88bbbb'}),
        ##('Idle', parse('26053.csv'), {'color': '#88bbbb'}), #{'color': '#88bbbb'}),
        ##('Collect-All', parse('26045.csv'), {'color': '#bbaa88'}),
#]

fig = plt.figure(figsize=fs)
ax = fig.add_subplot(111)
ax.set_ylabel('$I$ / mA')
ax.set_xlabel('$t$ / s')
ax.set_ylim((0, 2.2))

# 
# Experiments for simple temperature query
#
kws = { 'style': { 'color': 'black', 'linestyle': '-'}, 'label': 'Temperature Old' }
boxes = materialize(data_to_boxes(parse(x + '.csv'), **kws) for x in ('26052', '26064'))
#it = join_boxes(boxes)
j = list(join_boxes(*boxes))
plot_boxes(ax, j, **kws)

kws = { 'style': { 'color': '#88bbbb', 'linestyle': ':'}, 'label': 'Idle Old' }
plot_boxes(ax, data_to_boxes(parse('26053.csv'), **kws), **kws)

kws = { 'style': { 'color': '#dd7777', 'linestyle': ':'}, 'label': 'Collect Old' }
plot_boxes(ax, data_to_boxes(parse('26045.csv'), **kws), **kws)

kws = { 'style': { 'color': '#bbaa88', 'linestyle': '-'}, 'label': 'Temperature' }
plot_boxes(ax, data_to_boxes(parse('26073.csv'), **kws), **kws)

kws = { 'style': { 'color': '#dd7777', 'linestyle': '-'}, 'label': 'Collect' }
plot_boxes(ax, data_to_boxes(parse('26074.csv'), **kws), **kws)

ax.set_xticks(range(0, int(EXPERIMENT_INTERVAL / BOX_INTERVAL) + 1, int(60.0 / BOX_INTERVAL)))
ax.set_xticklabels(range(0, int(EXPERIMENT_INTERVAL + BOX_INTERVAL), 60))
ax.grid(True, which='both')
ax.legend(bbox_to_anchor=(1.0, .95), loc='upper right')
#ax.legend(loc='upper right')

fig.savefig(PLOT_DIR + '/energy_sum.png')
fig.savefig(PLOT_DIR + '/energy_sum.pdf', bbox_inches='tight', pad_inches=.1)
plt.close(fig)

# vim: set ts=4 sw=4 expandtab:

