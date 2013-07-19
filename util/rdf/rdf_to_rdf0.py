#!/usr/bin/python

import sys

# -> source array
#def write_uri(s): f_out.write('"' + s + '", ')
#def write_literal(s): f_out.write('"' + s + '", ')
#def write_blank(s): f_out.write('"' + s + '", ')


# -> 0-separated strings


def parse_element(s):
	return {
		'<': parse_uri,
		'"': parse_literal,
	}.get(s[0], parse_blank)(s)

def parse_uri(s):
	write_uri(s[:s.find('>')+1])
	return s[s.find('>')+1:]

def parse_literal(s):
	s = s[1:]
	p = s.find('"')
	while s[p-1] == '\\':
		p += s[p+1:].find('"') + 1
	
	write_literal(s[:p])
	end = s[p:].find(' ')
	#print "s=%s pspace=%d s[p:]=%s" % (s, end, s[end:])
	if end != -1:
		p += end
	return s[p:]

def parse_blank(s):
	end = s.find(' ')
	write_blank(s[:end])
	return s[end:]

def parse_line(s):
	# XXX
	#f_out.write('{ ')
	write_statement_start()
	s = parse_element(s.strip())
	write_element_delimiter()
	s = parse_element(s.strip())
	write_element_delimiter()
	s = parse_element(s.strip())
	write_statement_end()
	# XXX
	#f_out.write('},\n')
	

if __name__ == '__main__':
#	global write_uri
#	global write_literal
#	global write_blank
#	global f_in
#	global f_out
	
	def write_statement_start(): pass
	def write_statement_end(): pass
	def write_element_delimiter(): pass
	
	def write_uri(s):
		if len(s) == 0: print "warning: subject length 0!"
		f_out.write(s + '\0')
	def write_literal(s):
		if len(s) == 0: print "warning: literal length 0!"
		f_out.write(s + '\0')
	def write_blank(s):
		if len(s) == 0: print "warning: blank length 0!"
		f_out.write(s + '\0')
	
	if len(sys.argv) < 3:
		print "syntax: %s [-c|-0] [filename]"
		sys.exit(1)
		
	if sys.argv[1] == '-c':
		def write_uri(s): f_out.write('"' + s + '"')
		def write_literal(s): f_out.write('"' + s + '"')
		def write_blank(s): f_out.write('"' + s + '"')
		def write_statement_start(): f_out.write('{ ')
		def write_statement_end(): f_out.write(' },\n')
		def write_element_delimiter(): f_out.write(', ')

	f_in = open(sys.argv[2], 'r')
	f_out = sys.stdout

	for line in f_in:
		parse_line(line)
	

