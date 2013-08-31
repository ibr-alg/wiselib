#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from pylab import setp

import re
import io

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
	
	for line in f:
		if "messduino" in line: continue
		try:
			t, c, v = line.split()
		except ValueError:
			continue;
		ts.append(float(t))
		ps.append(float(c) * float(v))

def fig_p():
	global fig
	global ts
	global ps
	
	fig = plt.figure()
	ax = plt.subplot(111)
	#ax.set_xlim((0, 60000))
	ax.plot(ts, ps, 'b-', )
	
	fig.savefig('p.pdf') #, bbox_inches='tight', pad_inches=.1)
	#plt.show()
	
	

print("parsing data...")
#parse(open('acm5_usb.log', 'r'))
parse(open('acm5_hibernate.log', 'r'))
fig_p()

