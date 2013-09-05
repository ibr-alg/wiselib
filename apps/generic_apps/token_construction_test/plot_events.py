#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
#from pylab import setup
import re
import io
import sys
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
rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)

def gliding_mean(l, n = 100):
	l_new = []
	for i, x in enumerate(l):
		values = min(n, i+1)
		l_new.append(sum(l[i - values + 1:i+1]) / float(values))
	return l_new


def parse_events(f):
	
	activity = dict(t=[], v=[])
	on = dict(t=[], v=[])
	send = dict(t=[], v=[])
	loss = dict(t=[], v=[])
	
	activity['t'].append(0)
	activity['v'].append(0)
	
	on['t'].append(0)
	on['v'].append(1)
	
	
	t = 0
	toffs = 0
	
	for line in f:
		
		t_new = None
		m = re.search(r'\bt([-0-9]+)\b', line)
		if m is not None:
			#print("---- MATCH", m.groups())
			t_new = int(m.groups()[0])
			
		if t_new is not None:
			if t_new + toffs < t: toffs += 65536
			t = t_new + toffs
		
		#print(line)
		
		if '/ACT' in line:
			#print(line)
			#print ("/ACT", t, toffs)
			activity['t'].append(t)
			activity['v'].append(0)
		elif 'ACT' in line:
			#print(line)
			#print("ACT", t, toffs)
			activity['t'].append(t)
			activity['v'].append(1)
		else:
			#print(repr(line))
			pass
			
		if re.search(r'\bon\b', line):
			on['t'].append(t)
			on['v'].append(1)
		elif re.search(r'\boff\b', line):
			on['t'].append(t)
			on['v'].append(0)
			
		if 'snd' in line:
			send['t'].append(t)
			send['v'].append(1 if len(send['v']) == 0 else send['v'][-1] + 1)
			
		if 'loss' in line:
			loss['t'].append(t)
			loss['v'].append(1 if len(loss['v']) == 0 else loss['v'][-1] + 1)

	return dict(
			activity = activity,
			on = on,
			send = send,
			loss = loss)
def parse_energy(f):
	ts = []
	vs = []
	p1s = []
	p2s = []
	c1s = []
	c2s = []
	
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



def plot_events(d, pon, pact, pev):
	#penergy = fig.add_subplot(313)
	
	#pon.set_xlim((0, 700000))
	#pact.set_xlim((0, 700000))
	#pev.set_xlim((0, 1000000))
	
	pon.set_ylim((0, 1.1))
	pact.set_ylim((0, 1.1))
	#pev.set_ylim((0, 1.1))
	
	pon.plot(d['on']['t'], d['on']['v'], drawstyle='steps-post')
	pact.plot(d['activity']['t'], d['activity']['v'], drawstyle='steps-post')
	pev.plot(d['send']['t'], d['send']['v'], 'k-')
	#print(d['loss'])
	pev.vlines(d['loss']['t'], [d['send']['v'][-1]], d['loss']['v'], 'r')
	#pev.plot(d['loss']['t'], d['loss']['v'], 'r^-')
	

def plot_energy(d, p):
	p.plot(d['x'], d['y'], *d.get('args', []))

fig = plt.figure()
pon = fig.add_subplot(411)
pact = fig.add_subplot(412)
pev = fig.add_subplot(413)
penergy = fig.add_subplot(414)

for p in (pon, pact, pev, penergy):
	p.set_xlim((0,240000))

d = parse_events(open(sys.argv[1], 'r'))
plot_events(d, pon, pact, pev)

ts, vs, p1s, p2s, c1s, c2s = parse_energy(open(sys.argv[2], 'r'))
plot_energy(dict(x=ts, y=p1s), penergy)

fig.savefig('p.pdf')

