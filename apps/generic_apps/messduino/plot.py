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

#F_U = (5.5 / 270.271936186)
#F_I = (50.5 / 44.7316896302)

# Measurement: 4th sep 2013, 220 Ohm resistor
# U = 5.04V
# I = 22.4 mA
# c = 42.6372445553
# v = 520.366288731
# Aref = 2,56V internal
 
F_U = (5.04 / 520.366288731)
F_I = (22.4 / 42.6372445553)

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
	#global ts
	#global c1s
	#global c2s
	#global vs
	#global p1s
	#global p2s
	#global p1s_mean
	
	ts = []
	vs = []
	p1s = []
	p2s = []
	c1s = []
	c2s = []
	
	#np.loadtxt(fname, delimiter=' ', dtype={
		#'names': ('t', 'I', 'U'),
		#'formats': ('i4', 'f4', 'f4')
	#})
		
	
	n = 1
	start = False
	for line in f:
		#if "messduino" in line:
			#start = True
			#continue
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
			
		if t < 1510 and t >= 1500: start = True
		if not start: continue
		
		#if c == 0: continue
		
		ts.append((t))
		vs.append(v)
		c1s.append(c1)
		c2s.append(c2)
		#ps.append(((c) * F_I) * (v * F_U)) # * float(v))
		p1s.append(((c1) * F_I) ) # * (v * F_U)) # * float(v))
		#p1s_mean.append(ema((((c1) * F_I) * (v * F_U))))
		p2s.append(((c2) * F_I) ) #* (v * F_U)) # * float(v))
		n += 1
		
	#p1s_mean = gliding_mean(p1s, 100)
	p1s = gliding_mean(p1s, 10)
	
	return (ts, vs, p1s, p2s, c1s, c2s)


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



def fig_p(*args):
	global fig
	global ts
	global p1s
	global p2s
	
	fig = plt.figure()
	ax = plt.subplot(111)
	#ax.set_xlim((0, 60000))
	#ax.set_xlim((200000, 600000))
	#ax.set_ylim((00, 3100))
	
	for d in args:
		ax.plot(d['x'], d['y'], *d['args'])
	
	#ax.plot(ts, p1s_mean, 'k-', )
	
	fig.savefig('p.pdf') #, bbox_inches='tight', pad_inches=.1)
	#plt.show()
	

def means(t, vs, cs):
	print "c: ", float(sum(cs)) / len(cs)
	print "v: ", float(sum(vs)) / len(vs)


print("parsing data...")
#parse(open('acm5_usb.log', 'r'))
#parse(open('acm5_hibernate.log', 'r'))
#parse(open('acm5_hibernate_sleep.log', 'r'))
#parse(open('/home/henning/repos/wiselib/util/isense/flash_rdfprovider/ts_ssp_insert_block_arduino.log', 'r'))
#parse(open('/home/henning/repos/wiselib/util/isense/flash_rdfprovider/antelope_ssp_insert_block_telosb.log', 'r'))

#t1_insert, _, p1_insert, _, _, _ = parse(open('./antelope_telosb_ssp_uart_insert.log', 'r'))
#t1_insert2, _, p1_insert2, _, _, _ = parse(open('./antelope_telosb_ssp_uart_insert_defaultrdc.log', 'r'))
#t1_idle, _, p1_idle, _, _, _ = parse(open('./antelope_telosb_ssp_uart_idle.log', 'r'))
#t1_idle2, _, p1_idle2, _, _, _ = parse(open('./antelope_telosb_ssp_uart_idle_defaultrdc.log', 'r'))
#fig_p(
		##{ 'x': t1_insert, 'y': p1_insert, 'args': ('k-',) },
		#{ 'x': t1_insert2, 'y': p1_insert2, 'args': ('r-',) },
		##{ 'x': t1_idle, 'y': p1_idle, 'args': ('b-',) },
		#{ 'x': t1_idle2, 'y': p1_idle2, 'args': ('g-',) }
	#)

#t, _, p, _, _, _ = parse(open('./inse_vector_tree_arduino_nouart.log', 'r'))
#fig_p( { 'x': t, 'y': p, 'args': ('k-',) })

#t, _, v, _, c, _ = parse(open('eich.log', 'r'))
#means(t, v, c)


t, _, p, _, _, _ = parse(open('./inse_telosb_nullrdc_act10k_energy.log', 'r'))
fig_p( { 'x': t, 'y': p, 'args': ('k-',) })



