#!/usr/ibn/env python 3

import sys
import re

properties = ('on', 'awake', 'active', 'window', 'interval', 'caffeine', 'count', 'node',
		'SE', 'parent', 'neighbor', 't', 'fwd_window', 'fwd_interval', 'fwd_from',
		'forwarding', 'waiting_for_broadcast')

def getnodes(namepattern):
	global gnodes
	nodes = dict(
			(k, v) for (k, v) in gnodes.items() if re.match(namepattern, k)
	)
	return nodes
	

re_begin_iteration = re.compile('-+ BEGIN ITERATION (\d+)')
re_properties = {}
for p in properties:
	re_properties[p] = re.compile(r'\b' + p + r'\s*(=|\s)\s*([^= ]+)')

re_kv = re.compile(r'([A-Za-z_]+) *[= ] *([0-9a-fA-Fx.-]+)')
tmax = None

def parse(f):
	global gnodes, parents
	global se_counts
	gnodes = {}
	nodes = gnodes
	parents = {}
	t = 0
	
	#se_counts = {
	#	se-id: [
	#		 {
	#			 t: 12000,
	#			 node-id: { 'is_active': 1, 'is_root': 1, 'count': 123 }
	#			 node-id: { 'is_active': 1, 'is_root': 1, 'count': 123 }
	#			 node-id: { 'is_active': 1, 'is_root': 1, 'count': 123 }
	#		 },
	#		 {
	#			 t: 12000,
	#			 node-id: { 'is_active': 1, 'is_root': 1, 'count': 123 }
	#			 node-id: { 'is_active': 1, 'is_root': 1, 'count': 123 }
	#			 node-id: { 'is_active': 1, 'is_root': 1, 'count': 123 }
	#		 },
	#	],
	#	...
	#}
	se_counts = {}
	
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
			print ("[!!!] nodename is none in line: " + origline)
			continue
		nodename = int(nodename)
		
		try: t = int(get_value('t')) / 1000
		except: pass
		
		# is it also about a SE?
		
		se = get_value('SE')
		is_active = get_value('is_active')
		is_root = get_value('is_root')
		count = get_value('count')
		
		if se is None or count is None or is_active is None or is_root is None: continue
		if se not in se_counts: se_counts[se] = []
		if len(se_counts[se]) < 1: se_counts[se].append({'t': t})
		latest = se_counts[se][-1]
		
		if nodename not in latest:
			latest[nodename] = { 'count': count, 'is_root': is_root, 'is_active': is_active }
		
		else:
			if (
					(int(count) != int(latest[nodename]['count'])) or
					#(int(is_active) and not int(latest[nodename]['is_active'])) or
					(int(is_active) != int(latest[nodename]['is_active'])) or
					(int(is_root) != int(latest[nodename]['is_root'])) #or
		#			(int(is_root) and any((type(x) == dict and int(x['is_root']) for x in latest.values()))) or
			#		(int(is_active) and any((type(x) == dict and int(x['is_active']) for x in latest.values())))
			):
				#print("count {}->{} is_active {}->{} is_root {}->{} t {}".format(
					#int(latest[nodename]['count']), int(count),
					#int(latest[nodename]['is_active']), int(is_active),
					#int(latest[nodename]['is_root']), int(is_root), t))
				#oldlatest = latest
				latest = latest.copy()
				#for v in latest.values():
					#if type(v) is dict:
						#v['is_active'] = '0'
				latest['t'] = t
				latest[nodename] = { 'count': count, 'is_root': is_root, 'is_active': is_active }
				#print(latest[nodename])
				#assert(latest[nodename] != oldlatest[nodename])
				se_counts[se].append(latest)
				
		
def print_se(se_id, data, out):
	node_ids = set()
	for v in data: node_ids.update(v.keys())
	node_ids.remove('t')
	node_ids = sorted(node_ids)
	
	out.write('      ')
	for node_id in node_ids:
		out.write('{:5d} '.format(node_id))
	out.write('\n')
	
	for d in data:
		out.write('{:5d} '.format(int(d['t'])))
		for node_id in node_ids:
			if node_id in d:
				#print(d[node_id])
				out.write('{:3d}{:1s}{:1s} '.format(
					int(d[node_id]['count']), int(d[node_id]['is_root']) and 'R' or ' ',
					int(d[node_id]['is_active']) and '*' or ' ', 
				))
			else:
				out.write('  -   ')
		out.write('\n')

def print_ses(out):
	global se_counts
	
	for k, v in se_counts.items():
		out.write(k + ':\n')
		print_se(k, v, out)
		out.write('\n')

parse(open('log.txt', 'r'))

print_ses(sys.stdout)

