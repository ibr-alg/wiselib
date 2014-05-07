#!/usr/bin/env python3

import re

re_time_sN = re.compile(r'^T([0-9]+)\.([0-9]+)\|(.*)$')
re_topo_start = re.compile(r'@([0-9]+) topo<([0-9]+):')
re_topo_end = re.compile(r'@([0-9]+) topo>([0-9]+):')
re_topo_child = re.compile(r'@([0-9]+) ch([0-9]+) ([0-9]+):')

def parse(f):
	def extract_time_sN(line):
		m = re.match(re_time_sN, line)
		if m is None:
			return None, line
		else:
			s, N, line = m.groups()
			return float(s) + 10**-9 * float(N), line

	t0 = None

	topomode_nodes = dict()
	
	current_topology = {}
	last_add = 0
	last_change = 0

	def in_topo_mode(n):
		return n in topomode_nodes

	def add_child(n, c):
		topomode_nodes[n]['childs'].append(c)

	def start_topo_mode(n, p):
		topomode_nodes[n] = { 'childs': [], 'parent': p }

	def end_topo_mode(n):
		del topomode_nodes[n]

	def check_topology(n):
		tx = t - t0

		if n in current_topology:
			if topomode_nodes[n] == current_topology[n]: return

			print("{} C {}: {} -> {}".format(tx, n, current_topology[n], topomode_nodes[n]))

		else:
			print("{} + {}: {}".format(tx, n, topomode_nodes[n]))

		current_topology[n] = topomode_nodes[n]


		# Complete topology soundness check

		ok = True

		for n in current_topology.keys():
			# check parent
			parent =  current_topology[n]['parent']
			if parent not in current_topology:
				#print("{} !P {}".format(tx, n))
				#ok = False
				pass
			else:
				if n not in current_topology[parent]['childs']:
					# parent is there but doesnt recognize us as child
					print("{} !PC {}".format(tx, n))
					ok = False

			# check childs
			#for c in current_topology[n]['childs']
		if ok:
			print("{} OK".format(tx))



	t = t_ = 0

	for line in f:
		t_, line = extract_time_sN(line)
		if t_ is not None: t = t_
		if t is not None and t0 is None or t < t0:
			t0 = t
		#if t > t_report:
			#print(t)
			#t_report = t + 1000
		#if (t - t0) > tmax: break

		#if ' ch' in line or 'topo' in line:
			#print(line)


		m = re.search(re_topo_start, line)
		if m is not None:
			n, p = m.groups()

			if in_topo_mode(n):
				end_topo_mode(n)
			#check_topology()
			start_topo_mode(n, p)
			continue

		m = re.search(re_topo_child, line)
		if m is not None:
			n, i, c = m.groups()

			if not in_topo_mode(n):
				continue
			if int(i) != len(topomode_nodes[n]['childs']):
				end_topo_mode(n)
				continue

			add_child(n, c)
			continue

		m = re.search(re_topo_end, line)
		if m is not None:
			n, i = m.groups()
			if not in_topo_mode(n):
				#print("ign1 " + n + " " + str(t));
				continue
			if int(i) != len(topomode_nodes[n]['childs']):
				#print("ign2 " + n +" " + str(t));
				end_topo_mode(n)
				continue
			check_topology(n)
			end_topo_mode(n)
			continue
		
parse(open('log.txt', 'r'))

