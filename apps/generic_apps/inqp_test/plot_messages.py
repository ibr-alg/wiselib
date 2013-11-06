#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
#from pylab import setup
from mpl_toolkits.axes_grid1 import make_axes_locatable
import re
import io
import sys
from matplotlib import rc
import glob
import pylab

#DATA_PATH='/home/henning/annexe/experiments/2013-09-inqp-shawn'
DATA_PATH='/home/henning/annexe/experiments/2013-10-inqp-shawn/output_data'


rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)

MSGS, BYTES, TIME, NODES, MSGS_T, MSGS_AGGR_T, MSGS_QUERY_T = range(7)

MSG_PACKING = 'c8'
MSG_FLOODING = 'c9'
MSG_FORWARD = 'ca'

def parse(f):
	
	sum_msgs = [0]
	sum_bytes = [0]
	ts = [0]
	
	for line in f:
		m = re.match(r'SEND from (\d+) to (\d+) len (\d+) time (\d+)', line)
		if m is not None:
			from_ = int(m.groups()[0])
			to = int(m.groups()[1])
			len_ = int(m.groups()[2])
			t = int(m.groups()[3])
			
			ts.append(t)
			sum_msgs.append(sum_msgs[-1] + 1)
			sum_bytes.append(sum_bytes[-1] + len_)
			
			
	return ts, sum_msgs, sum_bytes


def parse_all(dir, fnametest = lambda sz, cnt, seed, add: True):
	r = {}
	for fn in glob.glob(dir + '/size*.log'):
		m = re.search(r'size(\d+)_count(\d+)_rseed(\d+)(.*).log', fn)
		if m is None: continue
		
		
		nodes_seen = set()
		size = int(m.groups()[0])
		count = int(m.groups()[1])
		seed = int(m.groups()[2])
		add_info = m.groups()[3]
		
		if not fnametest(size, count, seed, add_info): continue
		
		try:
			f = open(fn, 'r')
		except FileNotFoundError:
			print("not found: ", fn)
			continue
		
		print(fn)
		msgs = 0
		msgs_aggr = 0
		msgs_query = 0
		ts = []
		msgs_v = []
		msgs_t = []
		msgs_aggr_v = []
		msgs_aggr_t = []
		msgs_query_v = []
		msgs_query_t = []
		bytes = 0
		time = 0
		for line in f:
			m = re.match(r'-+ BEGIN ITERATION (\d+)', line)
			if m is not None:
				t = int(m.groups()[0]) * 1000
				t -= 10000
				
			#if 'aggr srow' in line:
				#aggr_srows += 1
				#aggr_srows_t.append(t)
				#aggr_srows_v.append(aggr_srows)
				
			#m = re.match(r'SEND from (\d+) to (\d+) len (\d+) time (\d+)', line)
			m = re.match(r'@(\d+) => (\d+) l(\d+) m([0-9a-f]+)', line)
			if m is not None:
				from_ = int(m.groups()[0])
				nodes_seen.add(from_)
				to = int(m.groups()[1])
				len_ = int(m.groups()[2])
				m = m.groups()[3]
				#t = int(m.groups()[3])
				#t -= 10000
				msgs += 1
				msgs_t.append(t)
				msgs_v.append(msgs)
				
				#if len_ == 66:
				if m == MSG_FLOODING:
					msgs_query +=1
					msgs_query_t.append(t)
					msgs_query_v.append(msgs_query)
				elif m == MSG_FORWARD:
					msgs_aggr +=1
					msgs_aggr_t.append(t)
					msgs_aggr_v.append(msgs_aggr)
				
				bytes += len_
				time = t
		
		
		k = (size, count, add_info)
		
		if k not in r:
			print(size, count, add_info)
			# msgs, bytes, time, connected nodes
			r[k] = ([], [], [], [], [], [], [])
				
		if len(nodes_seen) < count:
			print("NOT CONNECTED! count={} seen={} seed={} sz={}".format(count, len(nodes_seen),
				seed, size))
			
		#else:
		r[k][0].append(msgs)
		r[k][1].append(bytes)
		r[k][2].append(time)
		r[k][3].append(len(nodes_seen))
		r[k][4].append((msgs_t, msgs_v))
		r[k][5].append((msgs_aggr_t, msgs_aggr_v))
		r[k][6].append((msgs_query_t, msgs_query_v))
		
	return r

def boxes(r, keys, c, p, lblfmt):
	l = []
	for k in keys:
		l.append(r[k][c])
	
	p.set_xticks(range(1, 1 + len(l)))
	p.set_xticklabels([lblfmt.format(k[0], k[1]) for k in keys])
	return p.boxplot(l)

def scatter(r, keys, c, p):
	x = []
	y = []
	for k in keys:
		x.extend(r[k][3])
		y.extend(r[k][c])
		
	#print(x, y)
	p.set_xlim((0, max(x)))
	p.set_ylim((0, max(y)))
	p.scatter(x, y, marker='x', color='k')


def fn_to_label(fn):
	m = re.match('size(\d+)_count(\d+)_rseed(\d+).log', fn)
	# yeah, i know string formatting, but this way is "portable" across py2 vs py3!
	return m.groups()[0] + ' nodes, ' + m.groups()[1] + 'x' + m.groups()[1] + ' area'

def plot_packets(d, p, **kws):
	p.plot(d['x'], d['y'], *d.get('args', []), **kws) #, drawstyle='steps-post')


def eval_experiment(r, exp):
	print('->', exp['pdf'])
	fig = plt.figure()
	p = fig.add_subplot(111)
	#bp = boxes(r, exp['key'], exp['c'], p, '{1:d}')
	bp = scatter(r, exp['key'], exp['c'], p)
	#plt.setp(color='k')
	p.set_xlabel('\\# nodes connected to base station')
	
	p.set_ylabel(['Messages (\\#)', 'Bytes transmitted', 'Answer time (ms)'][exp['c']])
	
	try:
		plt.setp(bp['boxes'], color='black')
		plt.setp(bp['medians'], color='black')
		plt.setp(bp['whiskers'], color='black')
		plt.setp(bp['fliers'], color='black')
	except Exception as e:
		print(e)
	fig.savefig(exp['pdf'], bbox_inches='tight', pad_inches=0)
	
def num(s):
	m = re.search(r'(\d+)', s)
	if m is not None:
		return m.groups()[0]
	print("num not found in " + s)
	return None


def eval_aggrint(r, exp):
	print('->', exp['pdf'])
	fig = plt.figure()
	p = fig.add_subplot(111)
	
	l = []
	for k in exp['key']:
		if k not in r: continue
		
		# strip 0 values (=0 messages sent/everything answered immediaetly),
		# they can only result from a disconnected sink
		ys = [y for y in r[k][exp['c']] if y > 0 or True]
		l.append(ys)
	
	p.set_xticks(range(1, 1 + len(l)))
	p.set_xticklabels([num(k[2]) for k in exp['key']])
	#bp = boxes(r, exp['key'], exp['c'], p, '{1:d}')
	#bp = scatter(r, exp['key'], exp['c'], p)
	#plt.setp(color='k')
	bp = p.boxplot(l)
	p.set_xlabel('aggr interval')
	
	p.set_ylabel(['Messages (\\#)', 'Bytes transmitted', 'Answer time (ms)'][exp['c']])
	
	try:
		plt.setp(bp['boxes'], color='black')
		plt.setp(bp['medians'], color='black')
		plt.setp(bp['whiskers'], color='black')
		plt.setp(bp['fliers'], color='black')
	except Exception as e:
		print(e)
	fig.savefig(exp['pdf'], bbox_inches='tight', pad_inches=0)
	

#fix_density = [(32, 100), (45, 200), (55, 300), (63, 400), (71, 500), (77, 600), (84, 700), (89, 800), (95, 900)]
#fix_size = [(50, 100), (50, 200), (50, 300), (50, 400), (50, 500), (50, 600), (50, 700), (50, 800), (50, 900)]
#experiments = [
		#{ 'key': fix_density, 'c': 0, 'pdf': 'fix_density_messages.pdf', },
		#{ 'key': fix_density, 'c': 1, 'pdf': 'fix_density_bytes.pdf', },
		#{ 'key': fix_density, 'c': 2, 'pdf': 'fix_density_time.pdf', },
		#{ 'key': fix_size, 'c': 0, 'pdf': 'fix_size_messages.pdf', },
		#{ 'key': fix_size, 'c': 1, 'pdf': 'fix_size_bytes.pdf', },
		#{ 'key': fix_size, 'c': 2, 'pdf': 'fix_size_time.pdf', },
#]

aggrint32 = [(71, 500, '_aggrint{}'.format(i)) for i in [100] + list(range(1000, 11000, 1000))]
experiments = [
		{ 'key': aggrint32, 'c': TIME, 'pdf': 'fix_density_aggrint71_messages.pdf', }
]

r = parse_all(DATA_PATH, lambda s, n, sd, a: s == 71 and n == 500)

for exp in experiments:
	#eval_experiment(r, exp)
	eval_aggrint(r, exp)

#fig = plt.figure()
#p = fig.add_subplot(111)

#p.scatter(x, y, marker='x', color='k')
#fig.savefig('fix_density_aggrint32_messages.pdf', bbox_inches='tight', pad_inches=0)

#fig = plt.figure()
#p = fig.add_subplot(111)
			

#p.plot(r[(50, 500)][4][0][0], r[(50, 500)][4][0][1], 'k-', label='Total msgs') #, drawstyle='steps-post')
#p.plot(r[(50, 500)][5][0][0], r[(50, 500)][5][0][1], 'r-', label='Aggregate msgs') #, drawstyle='steps-post')

#p.set_xlim((0, max(r[(50, 500)][4][0][0][-1],r[(50, 500)][5][0][0][-1])))
#p.set_ylim((0, r[(50, 500)][4][0][1][-1]));
#p.set_xlabel('t (ms)')
#p.set_ylabel('messages sent (\\#)')
#p.legend()
#fig.savefig('msgs.pdf', bbox_inches='tight', pad_inches=0)


