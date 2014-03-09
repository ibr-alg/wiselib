#!/usr/bin/env python2

import sys
import re

properties = ('send_to', 'send_type', 'node', 't', 'resend')

re_begin_iteration = re.compile('-+ BEGIN ITERATION (\d+)')
re_kv = re.compile(r'([A-Za-z_]+) *[= ] *([0-9a-zA-Z._-]+)')

def parse(f):
	
	print "seqdiag {"
	print "activation=none;"
	print "bcast; 0; 1; 2;"
	
	t_shown = False
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
			t_shown = False
			continue
		
		# which node is this line about?
		
		if 'node' not in line: continue
		kv = dict(re.findall(re_kv, line))

		if 'send_to' in kv:
			if not t_shown:
				print "=== %d ===" % t
				t_shown = True
				
			print "%s -> %s [label=\"%s (%s)\"];" % (kv['node'], kv['send_to'], kv.get('send_type', '?'), kv.get('resend', '0'))
			
	print "}"
	
	
parse(open(sys.argv[1]))

