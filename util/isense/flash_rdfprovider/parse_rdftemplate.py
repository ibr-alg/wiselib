#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import sys

def parse_rdftemplate(filename, ns):
	foi = ''
	
	def parse_element(s, l):
		return {
			'<': parse_uri,
			'"': parse_literal,
		}.get(s[0], parse_blank)(s, l)

	def parse_uri(s, l):
		#write_uri(s[:s.find('>')+1])
		l.append(transform_uri(s[:s.find('>')+1]))
		return s[s.find('>')+1:]

	def parse_literal(s, l):
		s = s[1:]
		p = s.find('"')
		while s[p-1] == '\\':
			p += s[p+1:].find('"') + 1
		
		end = s[p].find(' ')
		if end != -1:
			p = end
		#write_literal(s[:p])
		l.append(transform_literal("\"" + s[:p+1]))
		return s[p:]

	def parse_blank(s, l):
		end = s.find(' ')
		#write_blank(s[:end])
		l.append(transform_blank(s[:end]))
		return s[end:]

	def parse_line(s, l):
		# XXX
		#f_out.write('{ ')
		s = parse_element(s.strip(), l)
		s = parse_element(s.strip(), l)
		s = parse_element(s.strip(), l)
		# XXX
		#f_out.write('},\n')
		
	def transform_uri(s):
		
		if s.startswith('<$') and s.endswith('$>'):
		
			u = {
				'PLATFORM': node_base + '/rdf',
				'TEMPSENSOR': node_base + '/temp',
				'LIGHTSENSOR': node_base + '/light',
				'PIRSENSOR': node_base + '/pir',
				'ROOM': room_base + '/' + ns['room'],
				'FOI': foi,
			}.get(s[2:-2])
			
			return '<%s>\0' % u
		
		else:
			return s + '\0'
	
	def transform_blank(s): return s + '\0'
	def transform_literal(s): return s + '\0'
	
	#if len(sys.argv) < 5:
		#print "syntax: $0 /dev/ttyUSBx [rdf file] [nodename] [room]"
		#return
	
	#filename = sys.argv[2]
	#nodename = sys.argv[3]
	#room = sys.argv[4]
	
	ssp_address = 'http://smart-service-proxy.cti.gr'
	backend_address = ssp_address + '/be-0002'
	#node_base = backend_address + '/' + nodename
	
	if not ns['addr']:
		print >>sys.stderr, "Need address for template expansion!"
		sys.exit(1)
	
	node_base = 'coap://[' + ns['addr'] + ']'
	room_base = 'http://spitfire-project.eu/foi'
	foi_base = 'http://spitfire-project.eu/foi'
	if ns['foi']:
		fois = (foi_base + '/' + f for f in ns['foi'])
	else:
		fois = ()
	
	# parse normal lines
	
	lines = open(filename, 'r').readlines()
	l = []
	docid = 0b0001
	for line in lines:
		line = line.strip()
		if not line or line.startswith('//'): continue
		
		if '<$FOI$>' in line: continue
		
		print "processing: ", line
		
		if line.startswith('.doc'):
			docid = int(line[4:].strip(), 2)
			continue
		
		l.append(chr(docid >> 8) + chr(docid & 0xff))
		parse_line(line, l)

	# parse foi lines
	
	docid = 0b0001
	for line in lines:
		line = line.strip()
		if not line or line.startswith('//'): continue
		if line.startswith('.doc'):
			docid = int(line[4:].strip(), 2)
			continue
		if not '<$FOI$>' in line: continue
		
		for f in fois:
			foi = f
			l.append(chr(docid >> 8) + chr(docid & 0xff))
			parse_line(line, l)
	
	return l

if __name__ == '__main__':
	print parse_rdftemplate(*sys.argv[1:])
	
