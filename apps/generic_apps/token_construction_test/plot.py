#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from pylab import setp

import re
import io

from matplotlib import rc
#rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
## for Palatino and other serif fonts use:
rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)


fig = None
nodes = {}

properties = ('on', 'awake', 'active')

def parse(f):
	global nodes
	nodes = {}
	t = 0
	
	for line in f:
		def get_value(s):
			m = re.search(s + r'\s*(=|\s)\s*([^= ]+)', line)
			if m is None: return None
			return m.group(2)
		
		m = re.match('-+ BEGIN ITERATION (\d+)', line)
		if m is not None:
			t = int(m.group(1))
			
		if 'node' not in line:
			continue
		
		name = get_value('node')
		se = get_value('SE')
		if se is not None:
			name += ':' + se.rstrip('.')
		if name not in nodes:
			nodes[name] = {}
		
		for k in properties:
			v = get_value(k)
			if v is not None:
				if k not in nodes[name]: nodes[name][k] = {'t': [], 'v': []}
				
				try: int(v)
				except ValueError: pass
				else:
					nodes[name][k]['t'].append(t)
					nodes[name][k]['v'].append(int(v))

def make_figure():
	global fig
	global nodes
	
	fig = plt.figure()
	#fig.subplots_adjust(
	
	property_styles = {}
	
	i = 0
	first_ax = None
	last_ax = None
	
	def namesort(kva, kvb):
		for a, b in zip(kva[0], kvb[0]):
			if a != b: return cmp(a, b)
		return cmp(len(kva[0]), len(kvb[0]))
	
	for name, node in sorted(nodes.items(), cmp=namesort):
		if first_ax is None:
			ax = plt.subplot(len(nodes), 1, i + 1)
			first_ax = ax
		else:
			ax = plt.subplot(len(nodes), 1, i + 1, sharex=first_ax)
			
		ax.spines['bottom'].set_visible(False)
		ax.spines['top'].set_visible(False)
		#ax.spines['left'].set_visible(False)
		#ax.spines['right'].set_visible(False)
		setp(ax.get_xticklabels(), visible = False)
		ax.get_yaxis().set_visible(False)
		ax.get_xaxis().set_tick_params(size=0)
		last_ax = ax

		if 'on' in node:
			r, = ax.plot(node['on']['t'], node['on']['v'], 'b-', label='on', drawstyle='steps-post')
			property_styles['on'] = r
		if 'awake' in node:
			r, = ax.plot(node['awake']['t'], node['awake']['v'], 'r--', label='awake', drawstyle='steps-post')
			property_styles['awake'] = r
		if 'active' in node:
			r, = ax.plot(node['active']['t'], node['active']['v'], 'k:', label='active', drawstyle='steps-post')
			property_styles['active'] = r
		
		#ax.set_title(name)
		pos = list(ax.get_position().bounds)
		fig.text(pos[0] - 0.01, pos[1], name, fontsize = 8,
				horizontalalignment = 'right')
		i += 1
			
	last_ax.spines['bottom'].set_visible(True)
	last_ax.set_xlim((-1, 43))
	setp(last_ax.get_xticklabels(), visible = True)
	
	kv = list(property_styles.items())
	fig.legend(tuple(x[1] for x in kv), tuple(x[0] for x in kv))
	
	fig.savefig('plot.pdf', bbox_inches='tight') #, pad_inches=.1)
	#plt.show()
	

s = """
----- BEGIN ITERATION 0
node 1 awake 1
node 2 awake 1
node 3 awake 0
node 1 active 1
node 3:1.1 awake 0
node 3:1.2 awake 0
node 1 on 1
node 3:1.1 awake 0
node 3:1.2 on 0
node 1:1.1 active 1
----- BEGIN ITERATION 1
node 1 awake 0
node 2 active 1
node 3 awake 1
node 1 active 0
node 3:1.1 on 1
node 3:1.2 active 0
node 1:1.1 active 1
----- BEGIN ITERATION 2
node 1 awake 1
node 2 active 0
node 3 on 1
node 1 active 1
node 3:1.1 awake 0
node 3:1.2 active 1
node 1:1.1 active 0
----- BEGIN ITERATION 3
node 1 awake 1
node 2 awake 1
node 3 on 0
node 1 active 1
node 3:1.1 awake 0
node 3:1.2 awake 0
node 1:1.1 active 1
----- BEGIN ITERATION 4
node 1 awake 0
node 2 active 1
node 3 awake 1
node 1 active 0
node 3:1.1 awake 1
node 3:1.2 on 0
node 1:1.1 active 1
----- BEGIN ITERATION 5
node 1 awake 1
node 2 active 0
node 3 awake 1
node 1 active 1
node 3:1.1 awake 0
node 3:1.2 active 1
node 1:1.1 on 0
"""

#parse(io.StringIO(s))
parse(open('/home/henning/repos/wiselib/apps/generic_apps/token_construction_test/log.txt', 'r'))
make_figure()

