#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
import matplotlib.cbook as cbook
import matplotlib.ticker as ticker
import sys
import scipy
from scipy.optimize import curve_fit

skip_uris = 1
plot_uris = 50

def format_uri(x, pos=None):
	idx = np.clip(int(x+0.5), 0, len(uris)-1)
	return uris[idx]

def parse(f):
	global g_uris
	global g_occ
	uris = []
	occ = []
	g_uris = uris
	g_occ = occ
	for line in f:
		try:
			n, s = line.split(maxsplit = 1)
		except ValueError:
			print(line)
			continue
		occ.append(int(n))
		uris.append(s.strip())
	return uris, occ
		
		
def plot_uri_occ(ax, uris, occ):
	#ax.plot(uris, occ, 'o-')
	plt.bar(range(plot_uris), occ, label='occurences (of {})'.format(sum(g_occ)))
	
def fit(ax, uris, occ):
	def func(x, m, b, c):
		return np.exp(c * (x **2) + m * x + b)
	
	x = np.linspace(0, plot_uris)
	#y = func(x, 60000, 0.1, 10000)
	#print(x)
	#print(y)
	#popt, pcov = curve_fit(func, x, y) # occ)
	c, m, b = np.polyfit(x, np.log(occ), 2)
	lbl = "exp( {:.5f}x^2 + {:.2}x + {:4.2f} )".format(c, m, b)
	
	ax.plot(range(plot_uris), func(x, m, b, c), 'r-', label=lbl)

uris, occ = parse(open(sys.argv[1], 'r'))

print("sum {}".format(sum(g_occ)))

fig, ax = plt.subplots()
fig.set_size_inches(8, 10)

plot_uri_occ(ax, range(plot_uris), occ[skip_uris:skip_uris + plot_uris])

fit(ax, range(plot_uris), occ[skip_uris:skip_uris + plot_uris])

#ax.xaxis.set_major_formatter(ticker.FuncFormatter(format_uri))
#fig.autofmt_xdate()
ax.set_xticks([x + 0.5 for x in range(plot_uris)])
ax.grid()
#ax.set_yscale('log')
x_tick_names = plt.setp(ax, xticklabels = uris)

plt.subplots_adjust(bottom = 0.5, top = 0.95)
plt.setp(x_tick_names, rotation = 90, fontsize = 8)

print(uris[:plot_uris + skip_uris])
#ax.legend()
fig.savefig('plot_elements.pdf')
#plt.show()

