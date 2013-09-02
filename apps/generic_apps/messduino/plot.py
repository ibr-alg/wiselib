#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from pylab import setp

import re
import io


# Measurement with 221 Ohm resistor as consumer.
# U = 4.8V, I ~= 200mA
# measured values: u ~= 261, i1 ~= 18

#F_U = (4.8 / 261.0)
#F_I = (200.0 / 18.0)

# Measurement: 1st sep 2013, 100 Ohm resistor
# U = 5.05V
# I = 50.5 mA
# c = 23.1383370125
# v = 1.5556

F_U = (5.5 / 270.271936186)
F_I = (50.5 / 44.7316896302)

from matplotlib import rc
#rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})
## for Palatino and other serif fonts use:


tmax = 10000




rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)


fig = None
ts = []
p1s = []
p1s_mean = []
p2s = []
c1s = []
c2s = []
vs = []

def parse(f):
	global ts
	global c1s
	global c2s
	global vs
	global p1s
	global p2s
	global p1s_mean
	
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
			t, c2, c1, v = line.split()
		except ValueError:
			continue;
		
		
		try:
			t = int(t)
			c1 = int(c1)
			c2 = int(c2)
			v = int(v)
		except Exception:
			continue
			
		if t < 10: start = True
		if not start: continue
		
		#if c == 0: continue
		
		ts.append((t))
		vs.append(v)
		c1s.append(c1)
		c2s.append(c2)
		#ps.append(((c) * F_I) * (v * F_U)) # * float(v))
		p1s.append(((c1) * F_I)) # * (v * F_U)) # * float(v))
		#p1s_mean.append(ema((((c1) * F_I) * (v * F_U))))
		p2s.append(((c2) * F_I)) # * (v * F_U)) # * float(v))
		n += 1
		
	#p1s_mean = gliding_mean(p1s, 1000)
	p1s = gliding_mean(p1s, 100)


running_avg_prev = 0.0
def ema(x):
	global running_avg_prev
	running_avg_prev = running_avg_prev * 0.995 + x * 0.005
	return running_avg_prev

def mma(x, n):
	global running_avg_prev
	running_avg_prev = (float(n - 1) * running_avg_prev + x) / float(n)
	return running_avg_prev


def gliding_mean(l, n = 100):
	l_new = []
	for i, x in enumerate(l):
		values = min(n, i+1)
		l_new.append(sum(l[i - values + 1:i+1]) / float(values))
	return l_new



def fig_p():
	global fig
	global ts
	global p1s
	global p2s
	
	fig = plt.figure()
	ax = plt.subplot(111)
	ax.set_xlim((600000, 900000))
	#ax.set_ylim((0, 150))
	ax.plot(ts, p1s, 'k-', )
	#ax.plot(ts, p1s_mean, 'k-', )
	
	fig.savefig('p.pdf') #, bbox_inches='tight', pad_inches=.1)
	#plt.show()
	

def means():
	global ts
	global c1s
	global c2s
	global vs
	
	print "c1: ", float(sum(c1s)) / len(c1s)
	print "c2: ", float(sum(c2s)) / len(c2s)
	print "v: ", float(sum(vs)) / len(vs)


print("parsing data...")
#parse(open('acm5_usb.log', 'r'))
#parse(open('acm5_hibernate.log', 'r'))
parse(open('acm5_hibernate_sleep.log', 'r'))
fig_p()

#parse(open('eich.log', 'r'))
#means()

