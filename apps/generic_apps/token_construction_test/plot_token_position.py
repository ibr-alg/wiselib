#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from pylab import setp
import re
import io
from matplotlib import rc
rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)


re_begin_iteration = re.compile('-+ BEGIN ITERATION (\d+)')
re_tok = re.compile('@([0-9]+) tok S([0-9a-f]+.[0-9a-f]+) w([0-9]+) i([0-9]+) t([0-9]+) tr([0-9]+) d([0-9]+) e([0-9]+) c([0-9]+),([0-9]+) r([0-9]+)')

def parse(f):
	global ts
	global token_positions
	
	token_positions = []
	ts = []
	
	t = 0
	for line in f:
		m = re.match(re_begin_iteration, line)
		if m is not None:
			t = int(m.group(1))
			continue
		
		m = re.match(re_tok, line)
		if m is not None:
			node_id, se_id, w, i, t_, tr, d, e, c0, c1, r = m.groups()
			
			node_id = int(node_id)
			t_ = int(t_)
			c0 = int(c0)
			c1 = int(c1)
			
			if c0 != c1:
				ts.append(t_ / 1000.0)
				token_positions.append(node_id)
			
		

def fig_token_positions(ts, token_positions):
	fig = plt.figure()
	ax = plt.subplot(111)
	ax.set_xlim((10000, 20000))
	ax.plot(ts, token_positions, drawstyle='steps-post')
	fig.savefig('token_positions.pdf')

parse(open('log.txt', 'r'))
fig_token_positions(ts, token_positions)

