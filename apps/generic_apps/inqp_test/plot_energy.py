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

t0 = 0
tdelta = None

def correct_arduino_time(t):
	return t # * 1.795 # nullrdc, contikimac8_nooff
	#return t * 2.2 # contikimac8

def gliding_mean(l, n = 100):
	l_new = []
	for i, x in enumerate(l):
		values = min(n, i+1)
		l_new.append(sum(l[i - values + 1:i+1]) / float(values))
	return l_new


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
	#p1s = gliding_mean(p1s, 100)
	#print (p1s)
	
	return (ts, vs, p1s, p2s, c1s, c2s)

def sum_peaks(ts, vs, t0, tmax, v_thres, v_base):
	rt = []
	r = []
	s = 0
	tprev = ts[0]
	tstart = None
	for t, v in zip(ts[1:], vs[1:]):
		if t < t0:
			tprev = t
			continue
		if t >= tmax: break
		
		if v < v_thres:
			if tstart is not None:
				print("tdelta: {}".format(t-tstart))
				if t - tstart > 100:
					rt.append(tstart)
					r.append(s)
				tstart = None
		else:
			if tstart is None:
				s = 0
				tstart = t
			a = (t - tprev) * (v - v_base) / 1000.0 # mA * V * mS / 1000 = mJ
			s += a
			
		tprev = t
		
	return rt, r


	

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

def plot_energy(d, p, **kws):
	p.plot(d['x'], d['y'], *d.get('args', []), **kws)

def boxplots(vs, labels, p):
	bp = p.boxplot(vs)
	p.set_xticks(range(1, len(labels) + 1))
	p.set_xticklabels(labels)
	return bp


		
fig = plt.figure(figsize=(10, 4))
penergy = fig.add_subplot(111)
#div.append_axes("bottom", size="150%", pad = 0.05)
#penergy.set_ylim((0, 150))
#penergy.set_xlim((0, 5000))
penergy.set_ylabel('Energy consumption / query (mJ)')


#ts, vs, p1s, p2s, c1s, c2s = parse_energy(open(sys.argv[1], 'r'))

#ttemp, _, ptemp, _, _, _ = parse_energy(open('./inqp_isense_standalone_temp.log', 'r'))
#_, vtemp = sum_peaks(ttemp, ptemp, 3500, 180, 130)


BASEDIR = '/home/henning/annexe/experiments/2013-09-inqp-energy-local/'

ttemp_gps1, _, ptemp_gps1, _, _, _ = parse_energy(open(BASEDIR + '/isense_standalone_temp_gps1.log', 'r'))
_, vtemp_gps1 = sum_peaks(ttemp_gps1, ptemp_gps1, 3000, 9000, 150, 90)

ttemp2, _, ptemp2, _, _, _ = parse_energy(open(BASEDIR + '/isense_standalone_temp3.log', 'r'))
_, vtemp2 = sum_peaks(ttemp2, ptemp2, 2000, 10000, 150, 90)

tall, _, pall, _, _, _ = parse_energy(open(BASEDIR + '/isense_standalone_all.log', 'r'))
#_, vall = sum_peaks(tall, pall, 3000, 49000, 180, 130)
_, vall = sum_peaks(tall, pall, 3000, 12000, 150, 90)


# for cross join its one run/file per experiment
# t0/tmax for cross join datasets
cross_limits = [(3000, 6000), (2000, 4000), (2000, 5000), (3000, 5000), (3000, 6000), (3000, 6000), (2500, 5000), (2000, 4500), (2200, 5000), (2000, 4000)]

vcross = []
for i in range(10):
	s = '' if i == 0 else str(i + 1)
	print("i=" + s)
	tcross, _, pcross, _, _, _ = parse_energy(open(BASEDIR + '/isense_standalone_cross{}.log'.format(s), 'r'))
	_, vc = sum_peaks(tcross, pcross, cross_limits[i][0], cross_limits[i][1], 150, 90)
	vcross.extend(vc)


#penergy.set_xlim((40000, 55000))
#penergy.set_xlim((0, 30000))
#plot_energy(dict(x=ttemp_gps1, y=ptemp_gps1, args=('k-',)), penergy)
#plot_energy(dict(x=ttemp2, y=ptemp2, args=('k-',)), penergy)
#plot_energy(dict(x=tall, y=pall, args=('k-',)), penergy)
bp = boxplots([vtemp_gps1, vtemp2, vall, vcross], ['GPS 1', 'Temperature', 'All', 'Cross-Join'], penergy)
#print (vtemp2)

try:
	plt.setp(bp['boxes'], color='black')
	plt.setp(bp['medians'], color='black')
	plt.setp(bp['whiskers'], color='black')
	plt.setp(bp['fliers'], color='black')
except Exception:
	pass

fig.savefig('p.pdf')

