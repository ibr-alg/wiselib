#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from pylab import setp
import glob
import os
import os.path

import re
import io

from matplotlib import rc
#rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
## for Palatino and other serif fonts use:

fig = None
rc('font',**{'family':'serif','serif':['Palatino'], 'size': 8})
rc('text', usetex=True)

fs = (8, 3)

def get_style(d):
    l = '-'
    lw = 1

    if d['h']:
        c = '#dd7777'
        lw = 2
    else:
        c = '#88bbbb'

    c = {
            '20': '#dd7777',
            '40': '#88bbbb',
            '100': '#bbaa88',
            '200': 'black',
    }.get(d['mtu'], 'black')

   # u) not in (20, 40, 100, 200): return
    return {
            'plot': {
                'linestyle': l,
                'linewidth': lw,
                'marker': ' ',
                'color': c,
            }
        }

def get_style__(k):
    c = 'black'
    l = '-'

    if k.database == 'tuplestore':
        #c = '#ddccaa'
        c = 'black'
        l = '-'
        if k.ts_dict == 'tree':
            c = '#88bbbb'
            l = '-'
        elif k.ts_dict == 'avl':
            c = '#bbaa88'
            l = '-'
    elif k.database == 'teeny':
        c = '#dd7777'
        l = '-'

    return {
        'plot': {
            'linestyle': l,
            'color': c,
        },
        'box': {
            'linestyle': l,
            'color': c,
        }
    }

def style_box(bp, k):
    s = get_style(k)['box']
    for key in ('boxes', 'whiskers', 'fliers', 'caps', 'medians'):
        plt.setp(bp[key], **s)
    plt.setp(bp['fliers'], marker='+')

def parse(f):
    global data
    
    data = {}
    
    # Form:
    # data = {
    #   "100": {
    #       "tablesize": [10, 20, 30. 40. ...],
    #       "ratio": [0.1, 0.2, 0.3, 0.4, ...],
    #   }
    # }
    
    for line in f:
        origline = line
        comment_idx = line.find('//')
        if comment_idx != -1: line = line[:comment_idx]
        
        ls = line.split()
        kv = dict(zip(ls, ls[1:]))
        
        if kv['bufsize'] not in data: data[kv['bufsize']] = { 'tablesize': [], 'ratio': []}
        data[kv['bufsize']]['tablesize'].append(int(kv['tablesize']))
        data[kv['bufsize']]['ratio'].append(1.0 - float(kv['ratio']))

def parse_filesize(d):
    global data
    data = {}

    x = 'incontextsensing'
    x = 'ssp'
    x = 'btcsample0'
    
    os.chdir(d)
    for fn in glob.glob('*.hshdt'):
        parse_file(fn, x)
    for fn in glob.glob('*.shdt'):
        parse_file(fn, x)

    print(data.keys())
    os.chdir('..')

def parse_file(fn, x):
    global data
    print(fn)
    m = re.match(r'([^.]+)\.(\d+).(\d+).(h?)shdt', fn)
    #print(m.groups())
    dataset, mtu, tablesize, h = m.groups()

    if dataset != x: return
    if int(mtu) not in (20, 40, 100, 500): return

    #k = dataset + ' ' + mtu + (' H' if h else '')

    k = '{}{}'.format(mtu, '/H' if h else '')

    if k not in data:
        data[k] = { 'tablesize': [], 'ratio': [], 'h': h, 'dataset': dataset, 'mtu': mtu }
    d = data[k]
    d['tablesize'].append(int(tablesize))
    sz = os.path.getsize(fn)
    print('../' + os.path.basename(fn) + '.rdf')
    origsize = os.path.getsize('../' + dataset + '.rdf')
    d['ratio'].append(sz / origsize)




        
def fig_ratio():
    global fig
    global data
    
    fig = plt.figure(figsize=fs)
    ax = plt.subplot(111)
    #ax.set_yscale('log')
    ax.set_ylim((0.0, 1))
    ax.set_xlim((0, 128))
    
    ax.set_xlabel('Table Size (\#entries)')
    ax.set_ylabel('Compression Ratio')
    
    #markers = ['^', 'D', 'o', 's']
    
    print(data.keys())

    def labelkey(x):
        k, v = x
        return int(v['mtu']), v['h']

    for bufsize, d in sorted(data.items(), key=labelkey):
        print(bufsize)
        # sort by table size
        d['tablesize'], d['ratio'] = zip(*sorted(zip(d['tablesize'], d['ratio'])))

        ax.plot(d['tablesize'], d['ratio'], label=bufsize, **get_style(d)['plot'])
        
    #_, d = data.items()[0]
    
    # incontextsensing
    #ax.plot(d['tablesize'], [1.0 - 999.0 / 7686.0] * len(d['tablesize']), label='BZIP2', linestyle='--')
    #ax.plot(d['tablesize'], [1.0 - 951.0 / 7686.0] * len(d['tablesize']), label='GZIP', linestyle='--')

    # ssp
    #ax.plot(d['tablesize'], [1.0 - 25199.0 / 904039.0] * len(d['tablesize']), label='BZIP2', linestyle='--')
    #ax.plot(d['tablesize'], [1.0 - 51277.0 / 904039.0] * len(d['tablesize']), label='GZIP', linestyle='--')
        
        
    # 4703 / 27596  4939 /
    #ax.plot(d['tablesize'], [1.0 - 4703.0 / 27596.0] * len(d['tablesize']), label='BZIP2', linestyle='--')
    #ax.plot(d['tablesize'], [1.0 - 4939.0 / 27596.0] * len(d['tablesize']), label='GZIP', linestyle='--')
        
    #fig.legend()
    ax.grid()
    ax.legend(loc='upper right', ncol=int(len(data)/2))
    fig.savefig('ratio.pdf', bbox_inches='tight', pad_inches=0.1)

parse_filesize('shdt_test')
#data = None
#parse(open('log.txt', 'r'))
fig_ratio()

