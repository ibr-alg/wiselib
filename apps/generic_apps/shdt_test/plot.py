#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from pylab import setp

import re
import io

from matplotlib import rc
#rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
## for Palatino and other serif fonts use:

fig = None
rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)

def parse(f):
	global data
	
	data = {}
	
	for line in f:
		origline = line
		comment_idx = line.find('//')
		if comment_idx != -1: line = line[:comment_idx]
		
		ls = line.split()
		kv = dict(zip(ls, ls[1:]))
		
		if kv['bufsize'] not in data: data[kv['bufsize']] = { 'tablesize': [], 'ratio': []}
		data[kv['bufsize']]['tablesize'].append(int(kv['tablesize']))
		data[kv['bufsize']]['ratio'].append(float(kv['ratio']))
		
def fig_ratio():
	global fig
	global data
	
	fig = plt.figure()
	ax = plt.subplot(111)
	for bufsize, d in data.items():
		ax.plot(d['tablesize'], d['ratio'], label='bufsize='+bufsize)
		
	#fig.legend()
	ax.legend()
	fig.savefig('ratio.pdf')

data = None
parse(open('log.txt', 'r'))
fig_ratio()

