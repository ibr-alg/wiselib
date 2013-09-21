#!/usr/bin/env python2

import re
import sys

# d = {
# 	node => { name => (push, push) }
# }

d = {}
caff = {}
tmax = None #285000
tmax = 147002

# after tmax continue until caffeine is == cafstop
cafstop = None
cafstop = 1
cafstop_node = 1

if len(sys.argv) > 1:
	tmax = int(sys.argv[1])
	print("cropping at tmax=" + str(tmax))

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
			
		if tmax is not None and cafstop is None and t >= tmax:
			print('tmax=' + str(tmax) + ' reached. t=' + str(t))
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

def parse2(f):
	global d
	global tmax
	global caff
	
	time_re = r'-+ BEGIN ITERATION (\d+)'
	push_re = r'@(\d+) caf(\d+) ([^/ ]+)'
	pop_re = r'@(\d+) caf(\d+) /([^/ ]+)'
	
	lastcaf = None
	lastnode = None
	
	def register(node, name, action, c):
		nonlocal lastcaf
		nonlocal lastnode
		#print("{} {} {}".format(node, name, action))
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
		
		lastcaf = c
		lastnode = int(node)
		caff[int(node)] = c
	
	t = 0
	for line in f:
		m = re.match(time_re, line)
		if m is not None:
			t = int(m.groups()[0])
			if t % 10000 == 0: print(t)
			
		m = re.match(push_re, line)
		if m is not None:
			register(int(m.groups()[0]), m.groups()[2].strip(), 'push', int(m.groups()[1]))
			
		m = re.match(pop_re, line)
		if m is not None:
			register(int(m.groups()[0]), m.groups()[2].strip(), 'pop', int(m.groups()[1]))
			
		#print("tmax {} t {} cafstop {} lastcaf {} cafstopnode {} lastnode {}" .format(tmax, t, cafstop, lastcaf, cafstop_node, lastnode))
		
		if ((tmax is not None) and (t >= tmax)) and ((cafstop is not None) <=
				(lastcaf == cafstop and lastnode == cafstop_node)):
			print("break!")
			break


def output():
	global d
	
	for node, d2 in d.items():
		
		def fnnamecmp(a, b):
			return cmp(tuple(reversed(a[0].split('_'))), tuple(reversed(b[0].split('_'))))
		
		def fnnamekey(a):
			return tuple((a[0].split('_')))
		
		print('\n{}: caffeine = {}'.format(node, caff[node]))
		for name, pp in sorted(d2.items(), key = fnnamekey): #cmp = fnnamecmp):
			#print '   %-40s %5d %5d     lvl %4d   t_push %7d' % (name, pp['pushes'], pp['pops'], pp['level'], pp['earliest_unresolved_push'])
			print('   {:<40} {:5d} {:5d}     lvl {:4d}   t_push {:7d}'.format(name, pp['pushes'], pp['pops'], pp['level'], pp['earliest_unresolved_push']))
			
#parse2(open('log_office.txt', 'r'))
parse2(open('log.txt', 'r'))
output()

