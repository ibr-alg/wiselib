#!/usr/bin/env python2

# Usage:
# python2 codesize.py out/isense/Map.txt

import sys
import string

def read_map(fn):
	symbols = []
	
	
	f = open(fn)
	
	it = iter(f.readlines())
	while it.next() != "Linker script and memory map\n":
		pass
	
	it.next()
	
	#prev_offset = 0x04000000
	for i in it:
		if '/home/henning' in i: continue
		if 'out/isense' in i: continue
		
		l = i.split(None, 1)
		if len(l) < 2: continue
		try:
			offset = string.atoi(l[0], 16)
		except ValueError, e:
			continue
		
		#if offset < 0x04000000: continue
			
		symbols.append((offset, l[1]))
		
	symbols.sort() #cmp=lambda a, b: -cmp(a[1], b[1]))
	
	sizes = []
	prev = 0
	for (start, name), (end, _) in zip(symbols, symbols[1:]):
		sizes.append(((end - start), name))
	
	sizes.sort(cmp = lambda a, b: cmp(b, a))
	
	for size, symbol in sizes:
		print "%10d %s" % (size, symbol.strip())


if __name__ == '__main__':
	read_map(sys.argv[1])
	
