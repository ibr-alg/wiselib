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
from experiment_utils import (
        quantize, fold, median, join_boxes, average, materialize, PickleCache,
        t_quantize, shift, t_fold
    )

PLOT_DIR = 'plots'
EXPERIMENT_INTERVAL = 300.0
BOX_INTERVAL = 5.0
TICK_INTERVAL = 10.0
rc('font',**{'family':'serif','serif':['Palatino'], 'size': 12})
rc('text', usetex=True)
fs = (12, 5)
#fs = (8, 3)

prefix_re = re.compile(r'^T([0-9]+\.[0-9]+)\|@([0-9]+)[: ]')
acked_re = re.compile(r'ACKED f([0-9]+)$')

Acked = namedtuple('Acked', ['t', 'addr', 'f'])

def parse(fn):
    f = open(fn, 'r', encoding='latin1')
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
    #print(list(t_quantize(((x.t, 1) for x in xs), BOX_INTERVAL, align=EXPERIMENT_INTERVAL,
            #align_phase=12.0)))
    r = fold(
        t_quantize(((x.t, 1) for x in xs), BOX_INTERVAL, align=EXPERIMENT_INTERVAL,
            align_phase=12.0),
        int(EXPERIMENT_INTERVAL / BOX_INTERVAL)
    )

    #r = list(r)
    #print(r)

    return r

def style_box(bp, s):
    for key in ('boxes', 'whiskers', 'fliers', 'caps', 'medians'):
        plt.setp(bp[key], **s)
    plt.setp(bp['fliers'], marker='+')

def plot_boxes(ax, it, label, style):
    # materialize counts, ignoring timestamps,
    # throw out [0,...,0] elements
    vs2 = [(v if any(v) else []) for (t, v) in it]
    print("  exps {}".format(' '.join(str(len(x)) for x in vs2)))
    bp = ax.boxplot(vs2, positions=[x+.5 for x in range(len(vs2))])
    style_box(bp, style)
    #ax.plot(range(1, len(vs2) + 1), [average(x) for x in vs2], label=label, **style)
    ax.plot([0], [0], label=label, **style)

def print_last(acked):
    it = t_fold(((x.t, 1) for x in acked), EXPERIMENT_INTERVAL, align_phase=-12.0)
    it = list(it)
    #print ("--------", it)
    print("  {}".format(it[-1][0]))

fig = plt.figure(figsize=fs)
ax = fig.add_subplot(111)
ax.set_ylabel('\# Msgs')
ax.set_xlabel('$t$ / s')
ax.set_ylim((-1, 120))
#ax.set_yscale('log')
#ax.set_xscale('log')

label = 'Collect'
print(label)
kws = { 'style': { 'color': '#bbaa88', 'linestyle': '-'}, 'label': label }
acked = parse_dir('26235')['acked']
print_last(acked)
#plot_boxes(ax, compute_boxplot(acked), **kws)

acked2 = parse_dir('26244')['acked']
print_last(acked2)
b1 = compute_boxplot(acked)
b2 = compute_boxplot(acked2)
plot_boxes(ax, join_boxes(b1, b2), **kws)


label = 'Temperature'
print(label)
kws = { 'style': { 'color': '#88bbbb', 'linestyle': '-'}, 'label': label }
acked = parse_dir('26242')['acked']
print_last(acked)
plot_boxes(ax, compute_boxplot(acked), **kws)

# Roomlight 10
label = 'Light in Room 10'
print(label)
kws = { 'style': { 'color': '#dd7777', 'linestyle': '-'}, 'label': label }
acked = parse_dir('26243')['acked']
print_last(acked)
plot_boxes(ax, compute_boxplot(acked), **kws)



ax.set_xticks(range(0, int(EXPERIMENT_INTERVAL / BOX_INTERVAL) + 1, int(TICK_INTERVAL / BOX_INTERVAL)))
ax.set_xticklabels(range(0, int(EXPERIMENT_INTERVAL + BOX_INTERVAL), int(TICK_INTERVAL)))
ax.set_xlim((0, 210 / BOX_INTERVAL))

ax.grid(True, which='both')
ax.legend(ncol=1, bbox_to_anchor=(1.0, 1.0), loc='upper right')
#ax.legend(loc='upper right')

fig.savefig(PLOT_DIR + '/messages.png')
fig.savefig(PLOT_DIR + '/messages.pdf', bbox_inches='tight', pad_inches=.1)
plt.close(fig)
