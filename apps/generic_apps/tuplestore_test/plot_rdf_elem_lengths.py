#!/usr/bin/env python3

import rdflib
import sys
import os
import os.path
import matplotlib.pyplot as plt
import csv
from matplotlib import rc
import gzip
import itertools

rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)


def parse(fn):
    lens = []
    g = rdflib.Graph()
    g.parse(fn, format='n3')
    for s, p, o in g:
        lens.append(len(s))
        lens.append(len(p))
        lens.append(len(o))
    return lens

def make_cdf(l):
    rx = []
    ry = []

    l.sort()
    n = len(l)
    y = 0
    c = 0
    x_ = None
    for x in l:
        if x_ is None or x != x_:
            y += c / n
            ry.append(y)
            rx.append(x_ or 0.0)
            x_ = x
            c = 1
        else:
            c += 1
    y += c / n
    ry.append(y)
    rx.append(x_ or 0.0)
    return rx, ry

def plot_cdf(x, y):
    fig = plt.figure(figsize=(4,3))
    ax = plt.subplot(111)
    ax.plot(x, y, 'k-')
    fig.savefig('rdf_elem_lengths.pdf', bbox_inches='tight', pad_inches=.1)

fig = plt.figure(figsize=(4,3))
ax = plt.subplot(111)

styles = ['k--', 'k-', 'k:']

names = {
    'btcsample0.rdf': 'BTCSAMPLE',
    'ssp.rdf': 'SSP',
    'incontextsensing.rdf': 'NODE'
}

for fn, s in zip(sys.argv[1:], styles):
    l = parse(fn)
    xs, ys = make_cdf(l)
    #plot_cdf(xs, ys)
    ax.plot(xs, ys, s, label=names[fn])

ax.legend(loc='lower right')
ax.set_xlabel('element length')
ax.set_ylabel(r'\#elems with len $\le x$ / \#elems')
fig.savefig('rdf_elem_lengths.pdf', bbox_inches='tight', pad_inches=.1)


