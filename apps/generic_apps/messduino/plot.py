#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from pylab import setp

import re
import io


# Measurement with 221 Ohm resistor as consumer.
# U = 4.8V, I ~= 200mA
# measured values: u ~= 261, i1 ~= 18

F_U = (4.8 / 261.0)
F_I = (200.0 / 18.0)


from matplotlib import rc
#rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
## for Palatino and other serif fonts use:


tmax = 10000




rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)


fig = None
ts = []
ps = []

def parse(f):
	global ts
	global ps
	
	#np.loadtxt(fname, delimiter=' ', dtype={
		#'names': ('t', 'I', 'U'),
		#'formats': ('i4', 'f4', 'f4')
	#})
		
	
	n = 1
	start = False
	for line in f:
		if "messduino" in line:
			start = True
			continue
		try:
			t, c, v = line.split()
		except ValueError:
			continue;
		
		
		try:
			t = int(t)
			c = int(c)
			v = int(v)
		except Exception:
			continue
			
		if t < 10: start = True
		if not start: continue
		
		#if c == 0: continue
		
		ts.append((t))
		ps.append(((c) * F_I)) # * float(v))
		n += 1
		
	ps = gliding_mean(ps)


running_avg_prev = 0.0
def ema(x):
	global running_avg_prev
	running_avg_prev = running_avg_prev * 0.2 + x * 0.8
	return running_avg_prev

def mma(x, n):
	global running_avg_prev
	running_avg_prev = (float(n - 1) * running_avg_prev + x) / float(n)
	return running_avg_prev


def gliding_mean(l, n = 20):
	l_new = []
	for i, x in enumerate(l):
		values = min(n, i+1)
		l_new.append(sum(l[i - values + 1:i+1]) / float(values))
	return l_new



def fig_p():
	global fig
	global ts
	global ps
	
	fig = plt.figure()
	ax = plt.subplot(111)
	#ax.set_xlim((19000, 20000))
	ax.set_ylim((0, 500))
	ax.plot(ts, ps, 'b-', )
	
	fig.savefig('p.pdf') #, bbox_inches='tight', pad_inches=.1)
	#plt.show()
	
	

print("parsing data...")
#parse(open('acm5_usb.log', 'r'))
#parse(open('acm5_hibernate.log', 'r'))
parse(open('acm5_hibernate_sleep.log', 'r'))
fig_p()

