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
			name += ':' + se
		if name not in nodes:
			nodes[name] = {}
		
		for k in properties:
			v = get_value(k)
			if v is not None:
				if k not in nodes[name]: nodes[name][k] = {'t': [], 'v': []}
				
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
	for name, node in nodes.items():
		if first_ax is None:
			ax = plt.subplot(len(nodes), 1, i + 1)
			first_ax = ax
			setp(ax, yticklabels=[])
		else:
			ax = plt.subplot(len(nodes), 1, i + 1, sharex=first_ax)
		setp(ax.get_yticklabels(), visible = False)

		#plt.axis('off')
		if 'on' in node:
			r, = ax.plot(node['on']['t'], node['on']['v'], 'b-', label='on',
					drawstyle='steps-post')
			property_styles['on'] = r
		if 'awake' in node:
			r, = ax.plot(node['awake']['t'], node['awake']['v'], 'r--', label='awake',
					drawstyle='steps-post')
			property_styles['awake'] = r
		if 'active' in node:
			r, = ax.plot(node['active']['t'], node['active']['v'], 'k:', label='active',
					drawstyle='steps-post')
			property_styles['active'] = r
		
		ax.set_ylim((-0.1, 1.1))
		#ax.legend()
		ax.set_title(name)
		i += 1
			
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

