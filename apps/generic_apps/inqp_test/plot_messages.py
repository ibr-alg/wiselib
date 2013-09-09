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

rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)

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


def parse_all(dir):
	r = {}
	for fn in glob.glob(dir + '/size*.log'):
		m = re.match('size(\d+)_count(\d+)_rseed(\d+).log', fn)
		if m is None: continue
		
		size = int(m.groups()[0])
		count = int(m.groups()[1])
		seed = int(m.groups()[2])
		
		f = open(fn, 'r')
		msgs = 0
		bytes = 0
		time = 0
		for line in f:
			m = re.match(r'SEND from (\d+) to (\d+) len (\d+) time (\d+)', line)
			if m is not None:
				from_ = int(m.groups()[0])
				to = int(m.groups()[1])
				len_ = int(m.groups()[2])
				t = int(m.groups()[3])
				msgs += 1
				bytes += len_
				time = t
		
		if (size, count) not in r:
			r[(size, count)] = ([], [], [])
			
		r[(size, count)][0].append(msgs)
		r[(size, count)][1].append(bytes)
		r[(size, count)][2].append(time)
		
	return r

def boxes(r, keys, p):
	for k in keys:
		

def plot_packets(d, p, **kws):
	p.plot(d['x'], d['y'], *d.get('args', []), **kws) #, drawstyle='steps-post')


fig = plt.figure()
p = fig.add_subplot(111)

# By density
#fns_ = [
	#'size30_count100.log',
	#'size30_count500.log',
	#'size30_count1000.log'
#]

# By size
#fns_ = [
	#'size30_count100.log',
	#'size52_count300.log',
	#'size67_count500.log',
	#'size79_count700.log',
	#'size85_count800.log',
	#'size90_count900.log',
	##'size95_count1000.log',
#]

#fns = [ 'size32_count100_rseed{}.log'.format(i) for i in range(100) ]

def fn_to_label(fn):
	m = re.match('size(\d+)_count(\d+)_rseed(\d+).log', fn)
	# yeah, i know string formatting, but this way is "portable" across py2 vs py3!
	return m.groups()[0] + ' nodes, ' + m.groups()[1] + 'x' + m.groups()[1] + ' area'

#p.set_xscale('log')
#p.set_yscale('log')

#for fn in fns:
	#ts, msgs, bytes = parse(open('shawn_experiments/' + fn, 'r'));
	#plot_packets(dict(x=ts, y=msgs), p, label=fn_to_label(fn))

#ts, msgs, bytes = parse(open('size30_count500.log', 'r'));
#plot_packets(dict(x=ts, y=msgs), p)

#ts, msgs, bytes = parse(open('size30_count1000.log', 'r'));
#plot_packets(dict(x=ts, y=msgs), p)

#plot_packets(dict(x=ts, y=bytes), p)

p.legend()

fig.savefig('msgs.pdf')

