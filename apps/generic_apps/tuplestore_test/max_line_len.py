#!/usr/bin/env python

import sys

s = ""

l = 0
for line in sys.stdin:
#open(sys.argv[1], 'r', encoding='latin1'):
    if len(line) > l:
        l = len(line)
        s = line

print(l)
print(s)

