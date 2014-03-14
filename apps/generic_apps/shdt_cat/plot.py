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
rc('font',**{'family':'serif','serif':['Palatino'], 'size': 10})
rc('text', usetex=True)

def parse(f):
	global data
	
	data = {}
	
	# Form:
	# data = {
	# 	"100": {
	# 		"tablesize": [10, 20, 30. 40. ...],
	# 		"ratio": [0.1, 0.2, 0.3, 0.4, ...],
	# 	}
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
		
def fig_ratio():
	global fig
	global data
	
	fig = plt.figure()
	ax = plt.subplot(111)
	ax.set_ylim((0, 1))
	ax.set_xlim((10, 128))
	
	ax.set_xlabel('Table Size (\#entries)')
	ax.set_ylabel('Compression Ratio')
	
	markers = ['^', 'D', 'o', 's']
	
	for (bufsize, d), m in zip(
			sorted(data.items(), key=lambda (a, b): int(a)),
			markers
	):
		ax.plot(d['tablesize'], d['ratio'], label='MTU='+bufsize, marker=' ', linestyle='-',
				markersize=5)
		
	_, d = data.items()[0]
	
	# incontextsensing
	#ax.plot(d['tablesize'], [1.0 - 999.0 / 7686.0] * len(d['tablesize']), label='BZIP2', linestyle='--')
	#ax.plot(d['tablesize'], [1.0 - 951.0 / 7686.0] * len(d['tablesize']), label='GZIP', linestyle='--')

	# ssp
	#ax.plot(d['tablesize'], [1.0 - 25199.0 / 904039.0] * len(d['tablesize']), label='BZIP2', linestyle='--')
	#ax.plot(d['tablesize'], [1.0 - 51277.0 / 904039.0] * len(d['tablesize']), label='GZIP', linestyle='--')
		
		
	# 4703 / 27596  4939 /
	ax.plot(d['tablesize'], [1.0 - 4703.0 / 27596.0] * len(d['tablesize']), label='BZIP2', linestyle='--')
	ax.plot(d['tablesize'], [1.0 - 4939.0 / 27596.0] * len(d['tablesize']), label='GZIP', linestyle='--')
		
	#fig.legend()
	ax.legend(loc='lower right')
	fig.savefig('ratio.pdf')

data = None
parse(open('log.txt', 'r'))
fig_ratio()

