#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from pylab import setp
import math

import re
import io

from matplotlib import rc
#rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
## for Palatino and other serif fonts use:

import pydot

tmax = 3000000




rc('font',**{'family':'serif','serif':['Palatino'], 'size': 12})
rc('text', usetex=True)


fig = None
parents = {}
gnodes = {}
nhood = set()

properties = ('on', 'awake', 'active', 'window', 'interval', 'caffeine', 'count', 'node',
		'SE', 'parent', 'neighbor', 't', 'fwd_window', 'fwd_interval', 'fwd_from',
		'forwarding', 'waiting_for_broadcast')

def getnodes(namepattern):
	global gnodes
	nodes = dict(
			(k, v) for (k, v) in gnodes.items() if re.match(namepattern, k)
	)
	return nodes
	

re_begin_iteration = re.compile('-+ BEGIN ITERATION (\d+)')
re_properties = {}
for p in properties:
	re_properties[p] = re.compile(r'\b' + p + r'\s*(=|\s)\s*([^= ]+)')

re_kv = re.compile(r'([A-Za-z_]+) *[= ] *([0-9a-fA-Fx.-]+)')
re_onoff = re.compile(r'@([0-9]+) (on|off) t([0-9]+).*')
re_tok = re.compile(r'@([0-9]+) tok S([0-9a-f]+\.[0-9a-f]+) w([0-9]+) i([0-9]+) t([0-9]+) tr([0-9]+) d([0-9]+) e([0-9]+) c([0-9]+),([0-9]+) r([0-9]+) ri([0-9]+)')

re_ti = re.compile(r'@([0-9]+) TI< t([0-9]+) P([0-9]+) p([0-9]+).*')
re_rtt = re.compile(r'@([0-9]+) rtt t([0-9]+) F([0-9]+) d([0-9]+) e([0-9]+).*')
re_beacon = re.compile(r'@([0-9]+) SEND BEACON ([0-9]+) S[0-9]+ c([0-9]+) t([0-9]+).*')
#re_parent = re.compile(r'PARENT\(([0-9]+)\) := ([0-9]+)')
re_parent = re.compile(r'@([0-9]+) par([0-9]+) t([0-9]+)')
re_neigh_add = re.compile(r'@([0-9]+) N\+ ([0-9]+) l([0-9]+) t([0-9]+)')
re_neigh_stay = re.compile(r'@([0-9]+) N[=] ([0-9]+) l([0-9]+) L([0-9]+) t([0-9]+)')
re_neigh_erase = re.compile(r'@([0-9]+) N- ([0-9]+) l([0-9]+) L([0-9]+) t([0-9]+)')

re_time_sN = re.compile(r'^T([0-9]+)\.([0-9]+)\|(.*)$')

#re_tok = re.compile(r'@([0-9]+) tok ([v^]) F([0-9]+)

def parse(f):
	global gnodes, parents
	global t0
	gnodes = {}
	nodes = gnodes
	parents = {}
	t = 0
	t_report = 1000
	
	
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
		if t_ is not None: t = t_
		
		t_, line = extract_time_shawn(line)
		if t_ is not None: t = t_
		
		if t is not None and t0 is None or t < t0:
			t0 = t
		if t > t_report:
			print(t)
			t_report = t + 1000
		
		if (t - t0) > tmax: break
		
		if not line: continue
		
		m = re.match(re_onoff, line)
		if m is not None:
			nodename, onoff, t_ = m.groups()
			if nodename not in nodes: nodes[nodename] = {}
			if 'on' not in nodes[nodename]:
				nodes[nodename]['on'] = {'t': [], 'v': []}
			nodes[nodename]['on']['t'].append(t)
			nodes[nodename]['on']['v'].append(1 if onoff == 'on' else 0)
			continue
		
		#m = re.match(re_tok, line)
		#if m is not None:
			#nodename, S, w, i, t_, tr, d, e, c0, c1, r, ri = m.groups()
			#name = nodename + ':' + S
			##t_ = int(t_) // 1000
			#if name not in nodes: nodes[name] = {}
			#if 'window' not in nodes[name]:
				#nodes[name]['window'] = {'t': np.array((), dtype=np.int32), 'v': np.array((), dtype=np.int32)}
			#nodes[name]['window']['t'] = np.append(nodes[name]['window']['t'], t)
			#nodes[name]['window']['v'] = np.append(nodes[name]['window']['v'], int(w) // 1000)
			#if 'interval' not in nodes[name]:
				#nodes[name]['interval'] = {'t': np.array((), dtype=np.int32), 'v': np.array((), dtype=np.int32)}
			#nodes[name]['interval']['t'] = np.append(nodes[name]['interval']['t'], t)
			#nodes[name]['interval']['v'] = np.append(nodes[name]['interval']['v'], int(i) // 1000)
			#if 'real-interval' not in nodes[name]:
				#nodes[name]['real-interval'] = {'t': np.array((), dtype=np.int32), 'v': np.array((), dtype=np.int32)}
			#nodes[name]['real-interval']['t'] = np.append(nodes[name]['real-interval']['t'], t_)
			#nodes[name]['real-interval']['v'] = np.append(nodes[name]['real-interval']['v'], int(ri) // 1000)
			#if 'delay' not in nodes[name]:
				#nodes[name]['delay'] = {'t': np.array((), dtype=np.int32), 'v': np.array((), dtype=np.int32)}
			#nodes[name]['delay']['t'] = np.append(nodes[name]['delay']['t'], t_)
			#nodes[name]['delay']['v'] = np.append(nodes[name]['delay']['v'], int(d) // 1000)
			#continue
			
		m = re.match(re_ti, line)
		if m is not None:
			nodename, t_, P, p = m.groups()
			name = nodename
			#t_ = float(t_) / 1000.0
			P = float(P) #// 1000
			if name not in nodes: nodes[name] = {}
			if 'phase' not in nodes[name]:
				nodes[name]['phase'] = {'t': [], 'v': []}
			nodes[name]['phase']['t'].append(t)
			nodes[name]['phase']['v'].append(P)
			continue

		m = re.match(re_rtt, line)
		if m is not None:
			nodename, t_, F, d, e = m.groups()
			if int(e) > 200: continue
			
			name = nodename
			#t_ = int(t_) // 1000
			if name not in nodes: nodes[name] = {}
			if 'rtt' not in nodes[name]:
				nodes[name]['rtt'] = {'t': [], 'v': []}
			nodes[name]['rtt']['t'].append(t)
			nodes[name]['rtt']['v'].append(int(e))
			continue
			
		m = re.match(re_beacon, line)
		if m is not None:
			nodename, tgt, c, t_ = m.groups()
			#t_ = float(t_) / 1000.0
			name = nodename
			if name not in nodes: nodes[name] = {}
			if 'beacon' not in nodes[name]:
				nodes[name]['beacon'] = {'t': [], 'v': []}
			nodes[name]['beacon']['t'].append(t)
			nodes[name]['beacon']['v'].append(1)
			continue
			
		m = re.match(re_parent, line)
		if m is not None:
			nodename, parent, t_ = m.groups()
			#t_ = int(t_) // 1000
			parents[nodename] = parent
			continue

		m = re.match(re_neigh_add, line)
		if m is not None:
			nodename, addr, l, t_ = m.groups()
			#assert(int(l) < 1000)
			name = nodename
			#t_ = int(t_) // 1000
			if name not in nodes: nodes[name] = {}
			if 'link_metric' not in nodes[name]:
				nodes[name]['link_metric'] = {}
			if addr not in nodes[name]['link_metric']:
				nodes[name]['link_metric'][addr] = { 't': [], 'v': [] }
			nodes[name]['link_metric'][addr]['t'].append(t)
			nodes[name]['link_metric'][addr]['v'].append(int(l))
			continue
		
		m = re.match(re_neigh_stay, line)
		if m is not None:
			nodename, addr, l, L, t_ = m.groups()
			#assert(int(L) < 1000)
			name = nodename
			#t_ = int(t_) // 1000
			if name not in nodes: nodes[name] = {}
			if 'link_metric' not in nodes[name]:
				nodes[name]['link_metric'] = {}
			if addr not in nodes[name]['link_metric']:
				nodes[name]['link_metric'][addr] = { 't': [], 'v': [] }
			nodes[name]['link_metric'][addr]['t'].append(t)
			nodes[name]['link_metric'][addr]['v'].append(int(L))
			continue
			
		m = re.match(re_neigh_erase, line)
		if m is not None:
			nodename, addr, l, L, t_ = m.groups()
			#assert(int(L) < 1000)
			name = nodename
			#t_ = int(t_) // 1000
			if name not in nodes: nodes[name] = {}
			if 'link_metric' not in nodes[name]:
				nodes[name]['link_metric'] = {}
			if addr not in nodes[name]['link_metric']:
				nodes[name]['link_metric'][addr] = { 't': [], 'v': [] }
			nodes[name]['link_metric'][addr]['t'].append(t)
			nodes[name]['link_metric'][addr]['v'].append(int(L))
			continue
		
		print(line)
		
def fig_count_onegraph(namepattern = '.*'):
	# {{{
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
	# }}}
	
def fig_count(namepattern = '.*'):
	# {{{
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
	# }}}
	
def namesort(kva, kvb):
	for a, b in zip(kva[0], kvb[0]):
		if a != b: return cmp(a, b)
	return cmp(len(kva[0]), len(kvb[0]))
	
def fig_timings():
	global fig
	global gnodes
	nodes = gnodes
	
	fig = plt.figure(figsize=(16, 40))
	#fig.subplots_adjust(
	
	property_styles = {}
	
	i = 0
	first_ax = None
	last_ax = None
	
	for name, node in sorted(nodes.items(), cmp=namesort):
		if first_ax is None:
			ax = plt.subplot(len(nodes), 1, i + 1)
			first_ax = ax
		else:
			ax = plt.subplot(len(nodes), 1, i + 1, sharex=first_ax)
			
		#ax.spines['bottom'].set_visible(False)
		ax.spines['top'].set_visible(False)
		#ax.spines['left'].set_visible(False)
		#ax.spines['right'].set_visible(False)
		#setp(ax.get_xticklabels(), visible = False)
		#ax.set_xticks(range(0, tmax, 100))
		#ax.get_yaxis().set_visible(False)
		#ax.get_xaxis().set_tick_params(size=0)
		last_ax = ax

		if 'caffeine' in node:
			r, = ax.plot(node['caffeine']['t'], node['caffeine']['v'], 'g-', label='caffeine', drawstyle='steps-post')
			property_styles['caffeine'] = r
		if 'window' in node:
			r, = ax.plot(node['window']['t'], node['window']['v'], 'b-', label='window', drawstyle='steps-post')
			property_styles['window'] = r
		if 'real-interval' in node:
			r, = ax.plot(node['real-interval']['t'], node['real-interval']['v'], 'm:', label='real-interval', drawstyle='steps-post')
			property_styles['real-interval'] = r
		if 'interval' in node:
			r, = ax.plot(node['interval']['t'], node['interval']['v'], 'r-', label='interval', drawstyle='steps-post')
			property_styles['interval'] = r
		if 'delay' in node:
			r, = ax.plot(node['delay']['t'], node['delay']['v'], 'g-', label='delay', drawstyle='steps-post')
			property_styles['delay'] = r
		
		ax.set_title(name)
		#pos = list(ax.get_position().bounds)
		#fig.text(pos[0] - 0.01, pos[1], name, fontsize = 8,
				#horizontalalignment = 'right')
		i += 1
			
	last_ax.spines['bottom'].set_visible(True)
	last_ax.set_xlim((-1, tmax))
	setp(last_ax.get_xticklabels(), visible = True)
	
	kv = list(property_styles.items())
	fig.legend(tuple(x[1] for x in kv), tuple(x[0] for x in kv))
	
	fig.savefig('timings.pdf') #, bbox_inches='tight', pad_inches=.1)
	#plt.show()

def interpolate_phases():
	global gnodes
	nodes = gnodes
	PERIOD = 20000.0
	step = 0.001 * PERIOD
	
	for name, node in nodes.items():
		if 'phase' in node:
			vs = node['phase']['v']
			ts = node['phase']['t']
			
			vs_ = [vs[0]]
			ts_ = [ts[0]]
			for v, t in zip(vs, ts):
				delta = (v - vs_[-1])
				while delta < -PERIOD: delta += PERIOD
				while delta > PERIOD: delta -= PERIOD
				print("delta={}".format(delta))
				sgn = 1 if delta > 0 else -1
				for i in range(0, int((delta if delta > 0 else -delta) * (1.0 / step))):
					vs_.append(vs_[-1] + sgn * i / step)
					# equivalent to steps-post
					ts_.append(ts_[-1])
				vs_.append(v)
				ts_.append(t)
			
			node['phase']['v'] = vs_
			node['phase']['t'] = ts_

def fig_phases():
	global fig
	global gnodes
	nodes = gnodes
	fig = plt.figure() #figsize=(16, 40))
	ax = plt.subplot(111, polar=True)
	#ax.set_xlim((5000, 30000))
	#ax.set_ylim((250000, 350000))
	#ax.set_ylim((270, 400))
	
	PERIOD = 20000.0
	F = 2.0 * math.pi / PERIOD
	
	for name, node in sorted(nodes.items(), cmp=namesort):
		if 'phase' in node:# and name in ('0','1', '2', '3'):
			r, = ax.plot([F*x for x in node['phase']['v']], node['phase']['t'], '-', label=name)
		
	#ax.legend()
	fig.savefig('phases.pdf')
	fig.savefig('phases.png')
	
def fig_rtts():
	global fig
	global gnodes
	nodes = gnodes
	fig = plt.figure() #figsize=(16, 40))
	ax = plt.subplot(111)
	
	for name, node in sorted(nodes.items(), cmp=namesort):
		#if 'rtt' in node and name in ('47430','47442'): # and name in ('0', '1', '2'):
		if 'rtt' in node:
			#print (node['rtt'])
			r, = ax.plot(node['rtt']['t'], node['rtt']['v'], label=name,
					drawstyle='steps-post')
		
	#ax.legend()
	fig.savefig('rtts.pdf')

def fig_link_metric():
	global fig
	global gnodes
	nodes = gnodes
	fig = plt.figure()
	ax = plt.subplot(111)
	
	name, node = sorted([(k,v) for k,v in nodes.items() if 'link_metric' in v])[3]
	#print(name, node)
	for name2, node2 in node['link_metric'].items():
		ax.plot(node2['t'], node2['v'])
	fig.savefig('link_metric.pdf')

def fig_forward_timings():
	global fig
	global gnodes
	nodes = gnodes
	
	fig = plt.figure(figsize=(16, 40))
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
		if 'forward' not in node: continue
			
		for from_, forward in sorted(node['forward'].items(), cmp=namesort):
			if first_ax is None:
				ax = plt.subplot(len(nodes), 1, i + 1)
				first_ax = ax
			else:
				ax = plt.subplot(len(nodes), 1, i + 1, sharex=first_ax)
				
			#ax.spines['bottom'].set_visible(False)
			#ax.spines['top'].set_visible(False)
			#ax.spines['left'].set_visible(False)
			#ax.spines['right'].set_visible(False)
			#setp(ax.get_xticklabels(), visible = False)
			#ax.get_yaxis().set_visible(False)
			#ax.get_xaxis().set_tick_params(size=0)
			last_ax = ax

			if 'window' in forward:
				r, = ax.plot(forward['t'], forward['window'], 'b-', label='window', drawstyle='steps-post')
				property_styles['window'] = r
			if 'interval' in forward:
				r, = ax.plot(forward['t'], forward['interval'], 'r-', label='interval', drawstyle='steps-post')
				property_styles['interval'] = r
			if 'delay' in forward:
				r, = ax.plot(forward['t'], forward['delay'], 'r-', label='delay', drawstyle='steps-post')
				property_styles['delay'] = r
			
			ax.set_title(name + ' fwd from ' + str(from_))
			#pos = list(ax.get_position().bounds)
			#fig.text(pos[0] - 0.01, pos[1], name, fontsize = 8,
					#horizontalalignment = 'right')
			i += 1
			
	#last_ax.set_xticks(range(0, tmax, 500))
	last_ax.spines['bottom'].set_visible(True)
	last_ax.set_xlim((-1, tmax))
	setp(last_ax.get_xticklabels(), visible = True)
	
	kv = list(property_styles.items())
	fig.legend(tuple(x[1] for x in kv), tuple(x[0] for x in kv))
	
	fig.savefig('forward_timings.pdf') #, bbox_inches='tight', pad_inches=.1)
	#plt.show()
	
	
def fig_duty_cycle(namepattern = '.*'):
	global fig
	nodes = getnodes(namepattern)
	
	fig = plt.figure(figsize=(16, 0.3 * len(nodes)))
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
		if ':' not in a: return cmp(int(a), int(b))
		ta = (a.split(':')[1:], int(a.split(':')[0]))
		tb = (b.split(':')[1:], int(b.split(':')[0]))
		return cmp(ta, tb)
		
		#return cmp(tuple(reversed(a.split(':'))), tuple(reversed(b.split(':'))))
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
		#ax.get_xaxis().set_tick_params(size=0)
		last_ax = ax

		if 'on' in node:
			r, = ax.plot(node['on']['t'], node['on']['v'], 'b-', label='on', drawstyle='steps-post')
			property_styles['on'] = r
			
		if 'beacon' in node:
			ax.vlines(node['beacon']['t'], [0], node['beacon']['v'], colors=['r'], label='beacon')
			
		if 'awake' in node:
			r, = ax.plot(node['awake']['t'], node['awake']['v'], 'r-', label='awake', drawstyle='steps-post')
			property_styles['awake'] = r
		if 'forwarding' in node:
			r, = ax.plot(node['forwarding']['t'], node['forwarding']['v'], 'g-', label='forwarding', drawstyle='steps-post')
			property_styles['forwarding'] = r
		if 'active' in node:
			r, = ax.plot(node['active']['t'], node['active']['v'], 'k-', label='active', drawstyle='steps-post')
			property_styles['active'] = r
		if 'waiting_for_broadcast' in node:
			r, = ax.plot(node['waiting_for_broadcast']['t'], node['waiting_for_broadcast']['v'], 'y-', label='waiting for broadcast', drawstyle='steps-post')
			property_styles['waiting for broadcast'] = r
		
		#ax.set_title(name)
		pos = list(ax.get_position().bounds)
		fig.text(pos[0] - 0.01, pos[1], name, fontsize = 8,
				horizontalalignment = 'right')
		i += 1
			
	#last_ax.set_xticks(range(0, tmax, 500))
	#last_ax.get_xaxis().set_tick_params(size=1)
	#last_ax.spines['bottom'].set_visible(True)
	#last_ax.set_xlim((-1, 1801))
	#last_ax.set_xlim((-1, tmax))
	#last_ax.set_xlim((1200, 1205))
	#last_ax.set_xlim((23000, 30000))
	
	setp(last_ax.get_xticklabels(), visible = True)
	
	kv = list(property_styles.items())
	fig.legend(tuple(x[1] for x in kv), tuple(x[0] for x in kv))
	
	fig.savefig('duty_cycle.pdf', bbox_inches='tight') #, pad_inches=.1)
	#plt.show()


def fig_tree():
	global parents
	graph = pydot.Dot(graph_type='digraph')
	for k, v in parents.items():
		e = pydot.Edge(k, v)
		graph.add_edge(e)
	graph.write_pdf('tree.pdf')

print("parsing data...")
#parse(open('/home/henning/repos/wiselib/apps/generic_apps/token_construction_test/log_office.txt', 'r'))
parse(open('log.txt', 'r'))

print("adjusting time shift... t0={}".format(t0))

def shift(d):
	ks = list(d.keys())
	for k in ks:
		if k == 't': d[k] = map(lambda x: float(x) - t0, d[k])
		elif type(d[k]) == dict: shift(d[k])

shift(gnodes)

print("tree graph...")
fig_tree()

print("rtts graph...")
fig_rtts()

print("phases graph...")
#interpolate_phases()
fig_phases()

print("duty cycle graph...")
fig_duty_cycle() #r'.*:1\.2')

print("link metric graph...")
fig_link_metric()

#print("timings graph...")
#fig_timings()
#print("fwd timings graph...")
#fig_forward_timings()
#print("counts graph...")
#fig_count_onegraph(r'.*:1\.2')

