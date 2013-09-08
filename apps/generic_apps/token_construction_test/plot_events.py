#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
#from pylab import setup
from mpl_toolkits.axes_grid1 import make_axes_locatable
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

t0 =     600000
tdelta = 600000 #2400000

#t0 = 0
#tdelta = None

def correct_arduino_time(t):
	return t * 1.795 # nullrdc, contikimac8_nooff
	#return t * 2.2 # contikimac8

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
	window = dict(t=[], v=[])
	interval = dict(t=[], v=[])
	
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
			
		if t < t0: continue
		
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
			
		m = re.match(r'@[0-9]+ tok SE .* win ([-0-9]+) int ([-0-9]+).*', line)
		if 'tok ' in line:
			window['t'].append(t)
			w = int(m.groups()[0])
			if w < 0: w += 65536
			window['v'].append(w)
			
			interval['t'].append(t)
			i = int(m.groups()[1])
			if i < 0: i += 65536
			interval['v'].append(i)
			
		if 'snd' in line:
			send['t'].append(t)
			send['v'].append(1 if len(send['v']) == 0 else send['v'][-1] + 1)
			
		if 'loss' in line and not 'noloss' in line:
			loss['t'].append(t)
			loss['v'].append(1 if len(loss['v']) == 0 else loss['v'][-1] + 1)

	return dict(
			activity = activity,
			on = on,
			window = window,
			interval = interval,
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
		
		t = correct_arduino_time(t)
		
		if t < t0: continue
		#if c == 0: continue
		
		ts.append(t)
		vs.append(v)
		c1s.append(c1)
		c2s.append(c2)
		#ps.append(((c) * F_I) * (v * F_U)) # * float(v))
		p1s.append(((c1) * F_I) * (v * F_U)) # * float(v))
		#p1s_mean.append(ema((((c1) * F_I) * (v * F_U))))
		p2s.append(((c2) * F_I) ) #* (v * F_U)) # * float(v))
		n += 1
		
	#p1s_mean = gliding_mean(p1s, 100)
	p1s = gliding_mean(p1s, 100)
	
	return (ts, vs, p1s, p2s, c1s, c2s)


def plot_onoff(ts, ys, p, **kws):
	left = None
	l = []
	for t, y in zip(ts, ys):
		if y and left is None:
			left = t
		elif left is not None:
			l.append((left, t - left))
			left = None
	if left is not None:
		l.append((left, t - left))
		left = None
	
	print( l)
	p.set_ylim((0, 1))
	p.set_yticks([])
	p.set_xticks([])
	p.broken_barh(l, (0, 1), **kws) #facecolors='green')


def plot_events(d, pon, pact, pwin, pev):
	#penergy = fig.add_subplot(313)
	
	#pon.set_xlim((0,  600000))
	#pact.set_xlim((0, 600000))
	#pev.set_xlim((0,  600000))
	
	pon.set_ylim((0, 1.1))
	pact.set_ylim((0, 1.1))
	#pev.set_ylim((0, 1.1))
	
	#pon.plot(d['on']['t'], d['on']['v'], 'k-', drawstyle='steps-post')
	plot_onoff(d['on']['t'], d['on']['v'], pon, facecolor='grey', linewidth=0,
			edgecolor='none')
	#pon.set_ylabel('Radio on')
	pon.set_yticks([0.5])
	pon.set_yticklabels(['Radio on'])
	
	#pact.plot(d['activity']['t'], d['activity']['v'], 'k-', drawstyle='steps-post')
	plot_onoff(d['activity']['t'], d['activity']['v'], pact, facecolor='grey',
			linewidth=0,
			edgecolor='none')
	#pact.set_ylabel('Token')
	pact.set_yticks([0.5])
	pact.set_yticklabels(['Token'])
	
	pwin.plot(d['interval']['t'], d['interval']['v'], 'k-', drawstyle='steps-post')
	pwin.plot(d['window']['t'], d['window']['v'], 'b-', drawstyle='steps-post')
	
	pev.plot(d['send']['t'], d['send']['v'], 'k-', drawstyle='steps-post')
	#print(d['loss'])
	#pev.vlines(d['loss']['t'], [d['send']['v'][-1]], d['loss']['v'], 'r')
	pev.plot(d['loss']['t'], d['loss']['v'], 'r-', drawstyle='steps-post')
	#pev.plot(d['loss']['t'], d['loss']['v'], 'r^-')
	

def plot_energy(d, p, **kws):
	p.plot(d['x'], d['y'], *d.get('args', []), **kws)
	
	

def cum(t, y):
	rt = t[1:]
	ry = []

	cy = y[0]
	ct = t[1] - t[0]
	
	t_prev = t[0]
	pv = 0
	#ry.append(0)
	for ct, cy in zip(t[1:], y[1:]):
		#print(cy, (ct - t_prev), cy * (ct - t_prev))
		pv += cy * (ct - t_prev)
		ry.append(pv / 1000000.0) # mA * V * mS / 10^6 = Joule
		t_prev = ct
	print("Joule:", ry[-1])
	return rt, ry
		
fig = plt.figure(figsize=(10, 4))
pwin = fig.add_subplot(111)
pwin.set_ylabel('Inteval/Window size (ms)')
pwin.set_xticks([])

div = make_axes_locatable(pwin)

pact = div.append_axes("top", size="30%", pad = 0.05)
pev = div.append_axes("bottom", size="150%", pad = 0.05)
pev.set_ylabel('Packets sent/lost (\\#)')
pev.set_xticks([])

pon = div.append_axes("bottom", size="30%", pad = 0.05)
penergy = div.append_axes("bottom", size="150%", pad = 0.05)
penergy.set_ylim((0, 150))
penergy.set_ylabel('Energy consumption ($\\mu$W)')


#pact = fig.add_subplot(812)
#pwin = fig.add_subplot(412)
#pev = fig.add_subplot(413)
#penergy = fig.add_subplot(414)

#pwin.set_yscale('log')

if tdelta is not None:
	for p in (pon, pact, pwin, pev, penergy):
		p.set_xlim((t0,t0 + tdelta))
	

d = parse_events(open(sys.argv[1], 'r', encoding='latin1'))
plot_events(d, pon, pact, pwin, pev)

ts, vs, p1s, p2s, c1s, c2s = parse_energy(open(sys.argv[2], 'r'))
plot_energy(dict(x=ts, y=p1s, args=('k-',)), penergy)

#cx, cy = cum(ts, p1s)
#plot_energy(dict(x=cx, y=cy, args=('g-',)), penergy)

fig.savefig('p.pdf')

