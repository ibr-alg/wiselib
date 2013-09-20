#!/usr/bin/env python3

import sys
import re
import copy

# [
# 	{
# 		'_t': time,
# 		id => { 'parent': id, 'filter': num, 'root': id, 'childs': [...] },
# 		id => { 'parent': id, 'filter': num, 'root': id, 'childs': [...] },
# 		id => { 'parent': id, 'filter': num, 'root': id, 'childs': [...] },
# 	}
# ]
nodes = []

re_begin_iteration = re.compile('-+ BEGIN ITERATION (\d+)')
re_kv = re.compile(r'([A-Za-z_]+) *[= ] *([0-9a-fA-Fx.-]+)')

tmin = None
tmax = None

def parse(f):
	global nodes
	global tmin
	global tmax
	
	t = 0
	for line in f:
		kv = {}
		def get_value(s):
			return kv.get(s, None)
		# strip off comments
		origline = line
		comment_idx = line.find('//')
		if comment_idx != -1: line = line[:comment_idx]
		# is this a begin iteration line?
		m = re.match(re_begin_iteration, line)
		if m is not None:
			t = int(m.group(1))
			
			if t % 10000 == 0:
				print("{}".format(t))
			continue
		
		# always start from t=0, else we will miss the tree state updates
		# which usually happen and the beginning
		# (and other state changes)
		#if tmin is not None and t < tmin: break
		if tmax is not None and t > tmax: break
		
		# which node is this line about?
		if 'node' not in line and not line.startswith('@'): continue
		
		kv = dict(re.findall(re_kv, line))
		name = nodename = get_value('node')
		if name is None:
			m = re.match(r'@(\d+) .*', line)
			if m is not None:
				name = nodename = m.groups()[0]
			
		if name is None:
			#print ("[!!!] nodename is none in line: " + origline)
			continue
		nodename = int(nodename)
		#try: t = int(get_value('t')) / 1000
		#except: pass
		
		if not len(nodes):
			latest = { '_t': t }
			nodes.append(latest)
		else:
			latest = nodes[-1]
		
		if ' on ' in line or ' off ' in line:
			if nodename in latest:
				latest = copy.deepcopy(latest)
				latest['_t'] = t
				nodes.append(latest)
				latest[nodename]['on'] = (' on ' in line)
				continue
				
		if ' ACT ' in line or ' /ACT ' in line:
			if nodename in latest:
				latest = copy.deepcopy(latest)
				latest['_t'] = t
				nodes.append(latest)
				latest[nodename]['active'] = (' ACT ' in line)
				continue
		
		parent = get_value('parent')
		root = get_value('root')
		distance = get_value('distance')
		filter_ = get_value('filter')
		if parent is None or root is None or distance is None or filter_ is None: continue
		
		if 	nodename in latest and (
				parent != latest[nodename]['parent'] or
				root != latest[nodename]['root'] or
				filter_ != latest[nodename]['filter']
		):
			latest = copy.deepcopy(latest)
			latest['_t'] = t
			nodes.append(latest)
			latest[nodename] = {
					'parent': parent, 'root': root, 'distance': distance,
					'filter': filter_
			}
		elif nodename not in latest:
			latest[nodename] = {
					'parent': parent, 'root': root, 'distance': distance,
					'filter': filter_
			}
		
def compress_filter(s):
	#return s.replace('00000000', ':').replace('0000', '.')
	s_orig = s
	
	k = 4
	s_new = ''
	for offs in range(0, len(s), k):
		if s[offs:offs + k] == '0' * k:
			s_new += '.'
		else:
			s_new += s[offs:offs + k]
	s = s_new
			
	s = s.replace('..', ':')
	#s = s.replace('::', '#')
	
	#print("compress filter: {} -> {}".format(s_orig, s))
	return s

def print_dot(d, out):
	out.write('digraph G {\n')
	for name, v in d.items():
		if name == '_t': continue
		
		
		#print("------------ " + str(v))
		
		color = 'white'
		if v.get('active', False): color = 'green'
		elif v.get('on', True): color = 'red'
		
		out.write('  {} [label="{}\\nroot: {}\\n{}",style="filled",fillcolor={}] ;\n'.format(
			name, name, v['root'], compress_filter(v['filter']), color
		))
		out.write('  {} -> {} ;\n'.format(name, v['parent']))
	out.write('}\n')

def generate_all(directory):
	global nodes
	global tmin
	global tmax
	last_t = None
	count = 0
	for d in nodes:
		t = int(d['_t'])
		if t == last_t:
			count += 1
		else:
			count = 0
		last_t = t
		
		if tmin is not None and t < tmin: continue
		if tmax is not None and t > tmax: continue
		
		fn = '{}/t{:08d}_{:04d}.dot'.format(directory, t, count)
		print(fn)
		f = open(fn, 'w')
		print_dot(d, f)
		f.close()


def test_compress():
	l = """
	0000000002040000401000000000444200000000000002000000004002000020
	0000000000000000000000000000040000000000000000000000000002000000
	0000000002000000000000000000440000000000000000000000000000000000
	0000000000000000001000000000040000000000000000000000000000000000
	""".split()

	for x in l:
		compress_filter(x)


if len(sys.argv) > 1:
	tmin = int(sys.argv[1])
	
if len(sys.argv) > 2:
	tmax = int(sys.argv[2])


print("parsing... tmin={} tmax={}".format(tmin, tmax))
parse(open('log.txt', 'r'))
print("found {} tree states".format(len(nodes)))
generate_all('dot')
print_dot(nodes[-1], sys.stdout)
			
