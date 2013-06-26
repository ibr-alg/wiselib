#!/usr/bin/env python2

# Usage:
# python2 codesize.py out/isense/Map.txt

import sys
import string
import re

def read_map(fn):
	symbols = []
	
	r = re.compile(r'\s*(\.[a-z]+|\*[a-z]+\*|[A-Z]+|)\s+(0x[0-9a-f]+)\s+(.*)')
	
	f = open(fn)
	
	it = iter(f.readlines())
	while it.next() != "Linker script and memory map\n":
		pass
	
	#it.next()
	
	#prev_offset = 0x04000000
	for i in it:
		m = re.match(r, i)
		
		if m is None: continue
		
		#print m.groups()
		
		# ignore contained object files (their functions are listed
		# seperately)
		if '/home/henning' in i: continue
		if 'out/isense' in i: continue
		if '/home/wiselib' in i: continue
		
		section = m.groups()[0]
		offset = string.atoi(m.groups()[1], 16)
		name = m.groups()[2]
		
		#l = i.split(None, 1)
		#if len(l) < 2: continue
		#try:
			#offset = string.atoi(l[0], 16)
		#except ValueError, e:
			#continue
		
		##if offset < 0x04000000: continue
			
		symbols.append((offset, name))
		
	symbols.sort() #cmp=lambda a, b: -cmp(a[1], b[1]))
	
	sizes = []
	prev = 0
	for (start, name), (end, _) in zip(symbols, symbols[1:]):
		sizes.append(((end - start), name, start, end))
	
	sizes.sort(cmp = lambda a, b: cmp(b, a))
	
	for size, symbol, start, end in sizes:
		print "%10d | %8x - %8x | %s" % (size, start, end, symbol.strip())


if __name__ == '__main__':
	read_map(sys.argv[1])
	
