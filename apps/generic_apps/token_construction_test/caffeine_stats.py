#!/usr/bin/env python2

import re
import sys

# d = {
# 	node => { name => (push, push) }
# }

d = {}
caff = {}
tmax = None #285000

if len(sys.argv) > 1:
	tmax = int(sys.argv[1])
	print "cropping at tmax=" + str(tmax)

def parse(f):
	global d
	global tmax
	global caff
	
	caff_re = r'.*node (\d+).*//\s*(push|pop)\s+(.*)$'
	t = 0
	for line in f:
		origline = line
		comment_idx = line.find('//')
		if comment_idx != -1: line = line[:comment_idx]
		
		
		
		def get_value(s):
			m = re.search(r'\b' + s + r'\s*(=|\s)\s*([^= ]+)', line)
			if m is None: return None
			return m.group(2).strip()
		
		t_ = get_value('t')
		if t_ is not None:
			t = int(t_)
			
		c = get_value('caffeine')
		n = get_value('node')
		if c is not None and n is not None:
			caff[int(n)] = c
			
		if tmax is not None and t >= tmax:
			print 'tmax=' + str(tmax) + ' reached. t=' + str(t)
			break
	
	
	
		
		m = re.match(caff_re, origline)
		if m is None: continue
		
		node = int(m.group(1).strip())
		action = m.group(2).strip()
		name = m.group(3).strip()
		
		if node not in d: d[node] = {}
		if name not in d[node]: d[node][name] = { 'pushes': 0, 'pops': 0, 'level': 0,
				'earliest_unresolved_push': -1 }
		if action == 'push':
			d[node][name]['pushes'] += 1
			d[node][name]['level'] += 1
			if d[node][name]['level'] == 1:
				d[node][name]['earliest_unresolved_push'] = t
		else:
			d[node][name]['pops'] += 1
			d[node][name]['level'] -= 1
			if d[node][name]['level'] == 0:
				d[node][name]['earliest_unresolved_push'] = -1

def output():
	global d
	
	for node, d2 in d.items():
		
		def fnnamecmp(a, b):
			return cmp(tuple(reversed(a[0].split('_'))), tuple(reversed(b[0].split('_'))))
		
		print '\n%d: caffeine = %s' % (node, caff[node])
		for name, pp in sorted(d2.items(), cmp = fnnamecmp):
			print '   %-40s %5d %5d     lvl %4d   t_push %7d' % (name, pp['pushes'], pp['pops'],
					pp['level'], pp['earliest_unresolved_push'])
			
parse(open('log_office.txt', 'r'))
output()

