#!/usr/bin/env python3

import numpy as np
from collections import defaultdict, namedtuple
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
from glob import glob

sys.path.append('/home/henning/bin')
from experiment_utils import quantize, fold, median, join_boxes, average, materialize, PickleCache, t_quantize

PLOT_DIR = 'plots'
EXPERIMENT_INTERVAL = 300.0
BOX_INTERVAL = 5.0
rc('font',**{'family':'serif','serif':['Palatino'], 'size': 12})
rc('text', usetex=True)
fs = (12, 5)

prefix_re = re.compile(r'^T([0-9]+\.[0-9]+)\|@([0-9]+)[: ]')
acked_re = re.compile(r'ACKED f([0-9]+)$')

Acked = namedtuple('Acked', ['t', 'addr', 'f'])

def parse(fn):
    f = open(fn)
    t = 0

    acked = []
    abrt = []
    for line in f:
        line = line.strip()
        m = re.match(prefix_re, line)
        if m is not None:
            t = float(m.groups()[0])
            addr = int(m.groups()[1])
            line = line[m.end():]
        else:
            #print(line)
            continue

        m = re.match(acked_re, line)
        if m is not None:
            f = int(m.groups()[0])
            acked.append(Acked(t, addr, f))

    return {
            'acked': acked
    }

def parse_dir(d):
    acked = []
    for fn in glob(d + '/*/output.txt'):
        d = parse(fn)
        acked.extend(d['acked'])
    acked.sort()
    return {
            'acked': acked
    }

def compute_boxplot(xs):
    """
    Combine the consecutive experiments into boxes
    """
    return fold(
        t_quantize(((x.t, 1) for x in xs), BOX_INTERVAL),
        int(EXPERIMENT_INTERVAL / BOX_INTERVAL)
    )

def style_box(bp, s):
    for key in ('boxes', 'whiskers', 'fliers', 'caps', 'medians'):
        plt.setp(bp[key], **s)
    plt.setp(bp['fliers'], marker='+')

def plot_boxes(ax, it, label, style):
    vs2 = [v for (t, v) in it]
    print("-- {} {}".format(label, ' '.join(str(len(x)) for x in vs2)))
    bp = ax.boxplot(vs2)
    style_box(bp, style)
    #ax.plot(range(1, len(vs2) + 1), [average(x) for x in vs2], label=label, **style)
    ax.plot([0], [0], label=label, **style)


fig = plt.figure(figsize=fs)
ax = fig.add_subplot(111)
ax.set_ylabel('\# Msgs')
ax.set_xlabel('$t$ / s')

# Collect
kws = { 'style': { 'color': 'black', 'linestyle': '-'}, 'label': 'Idle' }
plot_boxes(ax, compute_boxplot(parse_dir('26235')['acked']), **kws)

# test_both
#kws = { 'style': { 'color': 'red', 'linestyle': '-'}, 'label': 'Temperature' }
#plot_boxes(ax, compute_boxplot(parse_dir('26241')['acked']), **kws)

# Temperature
kws = { 'style': { 'color': 'red', 'linestyle': '-'}, 'label': 'Temperature' }
plot_boxes(ax, compute_boxplot(parse_dir('26242')['acked']), **kws)

ax.set_xticks(range(0, int(EXPERIMENT_INTERVAL / BOX_INTERVAL) + 1, int(60.0 / BOX_INTERVAL)))
ax.set_xticklabels(range(0, int(EXPERIMENT_INTERVAL + BOX_INTERVAL), 60))

ax.grid(True, which='both')
ax.legend(ncol=4, bbox_to_anchor=(1.0, 0), loc='lower right')
#ax.legend(loc='upper right')

fig.savefig(PLOT_DIR + '/messages.png')
fig.savefig(PLOT_DIR + '/messages.pdf', bbox_inches='tight', pad_inches=.1)
plt.close(fig)
