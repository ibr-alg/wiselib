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
tmax = None

def parse(f):
	global nodes
	
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
			continue
		if tmax is not None and t > tmax: break
		# which node is this line about?
		if 'node' not in line: continue
		kv = dict(re.findall(re_kv, line))
		name = nodename = get_value('node')
		if name is None:
			#print ("[!!!] nodename is none in line: " + origline)
			continue
		nodename = int(nodename)
		try: t = int(get_value('t')) / 1000
		except: pass
		
		parent = get_value('parent')
		root = get_value('root')
		distance = get_value('distance')
		filter_ = get_value('filter')
		if parent is None or root is None or distance is None or filter_ is None: continue
		
		if not len(nodes):
			latest = { '_t': t }
			nodes.append(latest)
		else:
			latest = nodes[-1]
		
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
	
	print("compress filter: {} -> {}".format(s_orig, s))
	return s

def print_dot(d, out):
	out.write('digraph G {\n')
	for name, v in d.items():
		if name == '_t': continue
		
		out.write('  {} [label="{}\\nroot: {}\\n{}"] ;\n'.format(
			name, name, v['root'], compress_filter(v['filter'])
		))
		out.write('  {} -> {} ;\n'.format(name, v['parent']))
	out.write('}\n')

def generate_all(directory):
	global nodes
	last_t = None
	count = 0
	for d in nodes:
		t = int(d['_t'])
		if t == last_t:
			count += 1
		else:
			count = 0
		last_t = t
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


print("parsing...")
parse(open('log.txt', 'r'))
print("found {} tree states".format(len(nodes)))
generate_all('dot')
print_dot(nodes[-1], sys.stdout)
			
