#!/usr/bin/python

import sys

# -> source array
#def write_uri(s): f_out.write('"' + s + '", ')
#def write_literal(s): f_out.write('"' + s + '", ')
#def write_blank(s): f_out.write('"' + s + '", ')


# -> 0-separated strings
def write_uri(s):
	if len(s) == 0: print "warning: subject length 0!"
	f_out.write(s + '\0')
def write_literal(s):
	if len(s) == 0: print "warning: literal length 0!"
	f_out.write(s + '\0')
def write_blank(s):
	if len(s) == 0: print "warning: blank length 0!"
	f_out.write(s + '\0')


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
	
	end = s[p].find(' ')
	if end != -1:
		p = end
	write_literal(s[:p])
	return s[p:]

def parse_blank(s):
	end = s.find(' ')
	write_blank(s[:end])
	return s[end:]

def parse_line(s):
	# XXX
	#f_out.write('{ ')
	s = parse_element(s.strip())
	s = parse_element(s.strip())
	s = parse_element(s.strip())
	# XXX
	#f_out.write('},\n')
	

f_in = open(sys.argv[1], 'r')
f_out = sys.stdout

for line in f_in:
	parse_line(line)
	

