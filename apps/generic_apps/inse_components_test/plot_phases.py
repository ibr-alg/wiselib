#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt
from pylab import setp
import math
import re
import io
from matplotlib import rc
import sys

from pandas import DataFrame, Series, to_datetime

rc('font',**{'family':'serif','serif':['Palatino'], 'size': 8})
rc('text', usetex=True)

parents = {}
gnodes = {}
nhood = set()
tmax = None

#re_begin_iteration = re.compile('-+ BEGIN ITERATION (\d+)')
re_enter_sync = re.compile(r'@([a-zA-Z0-9]+) Sc:esp.*')
re_time_sN = re.compile(r'^T([0-9]+)\.([0-9]+)\|(.*)$')

fs = (8, 3)
PERIOD = 10.0
t0 = None

def parse(f):
    global gnodes, parents
    global t0
    gnodes = {}
    nodes = gnodes
    parents = {}
    t = 0
    t_report = 1000
    
    garbage = open('garbage_lines.txt', 'w')
    
    def extract_time_sN(line):
        m = re.match(re_time_sN, line)
        if m is None:
            return None, line
        else:
            s, N, line = m.groups()
            return float(s) + 10**-9 * float(N), line
    
    def extract_time_shawn(line):
        m = re.match(re_begin_iteration, line)
        if m is not None:
            return t, None
        else:
            return None, line
    
    t0 = None
    
    for line in f:
        
        t_, line = extract_time_sN(line)
        if t_ is not None:
            t = t_

        
        #t_, line = extract_time_shawn(line)
        #if t_ is not None: t = t_
        
        if (t is not None) and (t != 0) and ((t0 is None) or (t < t0)):
            print("NEW T0 {} LINE {}".format(t, line))
            t0 = t
        if t > t_report:
            print(t)
            t_report = t + 1000
        
        #if (t - t0) > tmax: break
        
        if not line: continue
        m = re.match(re_enter_sync, line)

        if m is not None:
            nodename, = m.groups()
            name = nodename
            if name not in nodes: nodes[name] = {}
            if 'phase' not in nodes[name]:
                nodes[name]['phase'] = {'t':[], 'v': []}
            #print(t, float(t) % PERIOD)
            nodes[name]['phase']['t'].append(t)
            nodes[name]['phase']['v'].append(float(t) % PERIOD)

def avg(l):
    return sum(l) / len(l)

def fig_phases():
    global fs
    global PERIOD
    global gnodes
    nodes = gnodes
    fig = plt.figure(figsize=fs)
    ax = fig.add_subplot(111)#, polar=True)
    #PERIOD = 20000.0
    F = 2.0 * math.pi / PERIOD

    #ax.set_xticklabels([x * PERIOD/8 for x in range(8)])
    ax.set_ylim((-.2,.2))
    
    #for name, node in [('52570', nodes['52570'])]: #sorted(nodes.items(), cmp=namesort):

    roots = ('inode019', '1')

    phase_shift = 0
    for name in roots:
        if name in nodes:
            phase_shift = avg(nodes[name]['phase']['v'])
            break

    print("phase shift:", phase_shift)

    for name, node in nodes.items():
        for i in range(len(node['phase']['v'])):
            node['phase']['v'][i] -= phase_shift
            if node['phase']['v'][i] < -5:
                node['phase']['v'][i] = 10 + node['phase']['v'][i] 


    for name, node in sorted(nodes.items()):
        if 'phase' in node : #and name in ('inode019','inode015', 'inode018'):
        #if 'phase' in node and name in ('1','10', '38', '23'):
            #print(len(node['phase']['v']), len(node['phase']['t']))
            #if name in roots:
                #r, = ax.plot([F*x for x in node['phase']['v']], node['phase']['t'], 'k-', label=name, alpha=1,
                        #linewidth=2)

            #r, = ax.plot([F*x for x in node['phase']['v']], node['phase']['t'], color='#ff9999', linestyle='-', label=name, alpha=.5)
            r, = ax.plot(node['phase']['t'], [1 * x for x in node['phase']['v']] , marker='+', linestyle='', color='#cc7777', alpha=.5, label=name,
                    )

        else:
            #print(dir(node))
            #print(node.index[:5])
            print(dir(node.index[0]))
            v = node.values * F
            #print(v)
            #print([x.toordinal() for x in node.index])
            print(node.index)
            print(type(node.index[0]))
            print([x.timestamp() for x in node.index])
            r, = ax.plot(v, [x.timestamp() for x in node.index], '-', label=name)

    for name in roots:
        if name in nodes:
            node = nodes[name]
            #r, = ax.plot([F*x for x in node['phase']['v']], node['phase']['t'], 'k-', label=name, alpha=1,
                    #linewidth=2)
            r, = ax.plot(node['phase']['t'], [1 * x for x in node['phase']['v']] , linestyle='-',
                    linewidth=2, color='black', alpha=1, label=name)
        
    #ax.legend(bbox_to_anchor=(1.2, 1.2), loc='upper right')
    ax.grid()
    fig.savefig('phases.pdf', bbox_inches='tight', pad_inches=.1)
    fig.savefig('phases.png')

parse(open(sys.argv[1], 'r'))
print("adjusting time shift... t0={}".format(t0))

def to_timeseries():
    global gnodes
    for name, d in gnodes.items():
        gnodes[name] = Series(
                d['phase']['v'],
                index=to_datetime(d['phase']['t'], unit='s')
        )
                #.resample('100ms')
        d = gnodes[name] #.resample('10L', fill_method='ffill')
        gnodes[name] = d.shift(-t0 + 3600.0, 'S')


def shift(d):
    ks = list(d.keys())
    for k in ks:
        if k == 't': d[k] = [float(x) - t0 for x in d[k]]
        elif type(d[k]) == dict: shift(d[k])

shift(gnodes)
#to_timeseries()

fig_phases()

