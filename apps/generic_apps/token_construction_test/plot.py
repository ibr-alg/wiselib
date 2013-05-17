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
parents = {}
gnodes = {}

properties = ('on', 'awake', 'active', 'window', 'interval', 'caffeine', 'count')

def getnodes(namepattern):
	global gnodes
	nodes = dict(
			(k, v) for (k, v) in gnodes.items() if re.match(namepattern, k)
	)
	return nodes
	

def parse(f):
	global gnodes, parents
	gnodes = {}
	nodes = gnodes
	parents = {}
	t = 0
	
	for line in f:
		def get_value(s):
			m = re.search(r'\b' + s + r'\s*(=|\s)\s*([^= ]+)', line)
			if m is None: return None
			return m.group(2).strip()
		
		# strip off comments
		
		origline = line
		comment_idx = line.find('//')
		if comment_idx != -1: line = line[:comment_idx]
		
		# is this a begin iteration line?
		
		m = re.match('-+ BEGIN ITERATION (\d+)', line)
		if m is not None:
			t = int(m.group(1))
		
		# which node is this line about?
		
		if 'node' not in line: continue
		name = nodename = get_value('node')
		if name is None:
			print ("[!!!] nodename is none in line: " + origline)
		
		# is it also about a SE?
		
		se = get_value('SE')
		if se is not None: name += ':' + se.rstrip('.')
		if name not in nodes: nodes[name] = {}
		
		# update SE graph
		
		parent = get_value('parent')
		if parent is not None and se is not None:
			if se not in parents:
				parents[se] = {}
			parents[se][nodename] = parent
			
		# track other properties
		
		for k in properties:
			v = get_value(k)
			if v is not None:
				if k not in nodes[name]: nodes[name][k] = {'t': [], 'v': []}
				
				try:
					t = float(get_value('t')) / 1000.0
				except: pass
				
				try: int(v)
				except ValueError: pass
				else:
					nodes[name][k]['t'].append(t)
					nodes[name][k]['v'].append(int(v))

def fig_count_onegraph(namepattern = '*'):
	global fig
	nodes = getnodes(namepattern)
	
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
	
	ax = plt.subplot(111)
	for name, node in sorted(nodes.items(), cmp=namesort):
		if 'count' in node:
			r, = ax.plot(node['count']['t'], node['count']['v'], label='count ' + name, drawstyle='steps-post')
			property_styles['count ' + name] = r
		
			
	ax.set_xlim((600, 901))
	ax.grid()
	
	kv = list(property_styles.items())
	fig.legend(tuple(x[1] for x in kv), tuple(x[0] for x in kv))
	
	fig.savefig('counts_onegraph.pdf') #, bbox_inches='tight', pad_inches=.1)
	#plt.show()
	
def fig_count(namepattern = '*'):
	global fig
	nodes = getnodes(namepattern)
	
	fig = plt.figure(figsize=(8, len(nodes)))
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
		ax.grid()
		#ax.spines['left'].set_visible(False)
		#ax.spines['right'].set_visible(False)
		setp(ax.get_xticklabels(), visible = False)
		#ax.get_yaxis().set_visible(False)
		ax.get_xaxis().set_tick_params(size=0)
		last_ax = ax

		if 'count' in node:
			r, = ax.plot(node['count']['t'], node['count']['v'], 'b-', label='count', drawstyle='steps-post')
			property_styles['count'] = r
		
		ax.set_title(name)
		#pos = list(ax.get_position().bounds)
		#fig.text(pos[0] - 0.01, pos[1], name, fontsize = 8,
				#horizontalalignment = 'right')
		i += 1
			
	last_ax.spines['bottom'].set_visible(True)
	last_ax.set_xlim((600, 901))
	setp(last_ax.get_xticklabels(), visible = True)
	
	kv = list(property_styles.items())
	fig.legend(tuple(x[1] for x in kv), tuple(x[0] for x in kv))
	
	fig.savefig('counts.pdf') #, bbox_inches='tight', pad_inches=.1)
	#plt.show()
	
def fig_timings():
	global fig
	global gnodes
	nodes = gnodes
	
	fig = plt.figure(figsize=(8, 40))
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
		#ax.get_yaxis().set_visible(False)
		ax.get_xaxis().set_tick_params(size=0)
		last_ax = ax

		if 'caffeine' in node:
			r, = ax.plot(node['caffeine']['t'], node['caffeine']['v'], 'g-', label='caffeine', drawstyle='steps-post')
			property_styles['caffeine'] = r
		if 'window' in node:
			r, = ax.plot(node['window']['t'], node['window']['v'], 'b-', label='window', drawstyle='steps-post')
			property_styles['window'] = r
		if 'interval' in node:
			r, = ax.plot(node['interval']['t'], node['interval']['v'], 'r-', label='interval', drawstyle='steps-post')
			property_styles['interval'] = r
		
		ax.set_title(name)
		#pos = list(ax.get_position().bounds)
		#fig.text(pos[0] - 0.01, pos[1], name, fontsize = 8,
				#horizontalalignment = 'right')
		i += 1
			
	last_ax.spines['bottom'].set_visible(True)
	last_ax.set_xlim((-1, 101))
	setp(last_ax.get_xticklabels(), visible = True)
	
	kv = list(property_styles.items())
	fig.legend(tuple(x[1] for x in kv), tuple(x[0] for x in kv))
	
	fig.savefig('timings.pdf') #, bbox_inches='tight', pad_inches=.1)
	#plt.show()
	
	
def fig_duty_cycle(namepattern = '.*'):
	global fig
	nodes = getnodes(namepattern)
	
	fig = plt.figure(figsize=(8, 0.3 * len(nodes)))
	#fig.subplots_adjust(
	
	property_styles = {}
	
	i = 0
	first_ax = None
	last_ax = None
	
	def nodesort(kva, kvb):
		for a, b in zip(kva[0], kvb[0]):
			if a != b: return cmp(a, b)
		return cmp(len(kva[0]), len(kvb[0]))
	
	def sesort(pa, pb):
		a = pa[0]
		b = pb[0]
		if ':' in a and ':' not in b: return 1
		if ':' in b and ':' not in a: return -1
		#if ':' not in a: return cmp(a, b)
		return cmp(tuple(reversed(a.split(':'))), tuple(reversed(b.split(':'))))
		#return cmp(a[a.find(':') + 1:], b[b.find(':') + 1:])
		
	for name, node in sorted(nodes.items(), cmp=sesort):
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
			r, = ax.plot(node['awake']['t'], node['awake']['v'], 'r-', label='awake', drawstyle='steps-post')
			property_styles['awake'] = r
		if 'active' in node:
			r, = ax.plot(node['active']['t'], node['active']['v'], 'k-', label='active', drawstyle='steps-post')
			property_styles['active'] = r
		
		#ax.set_title(name)
		pos = list(ax.get_position().bounds)
		fig.text(pos[0] - 0.01, pos[1], name, fontsize = 8,
				horizontalalignment = 'right')
		i += 1
			
	#last_ax.spines['bottom'].set_visible(True)
	last_ax.set_xlim((-1, 901))
	setp(last_ax.get_xticklabels(), visible = True)
	
	kv = list(property_styles.items())
	fig.legend(tuple(x[1] for x in kv), tuple(x[0] for x in kv))
	
	fig.savefig('duty_cycle.pdf', bbox_inches='tight') #, pad_inches=.1)
	#plt.show()
	

print("parsing data...")
parse(open('/home/henning/repos/wiselib/apps/generic_apps/token_construction_test/log.txt', 'r'))

for k, v in parents.items():
	print(k + ":")
	for src, tgt in v.items():
		print ("  " + src + " -> " + tgt)

#print("duty cycle graph...")
#fig_duty_cycle() #r'.*:1\.2')
print("timings graph...")
fig_timings()
#print("counts graph...")
#fig_count_onegraph(r'.*:1\.2')

