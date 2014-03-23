#!/usr/bin/env python3

import os
import os.path
import glob
import sys

def merge(filenames):

    def add_name(line, name):
        t, l = line.split('|', 1)
        return t + '|@' + name+ ' ' + l

    l = []
    for fn in filenames:
        fn2 = fn.split('/')[1]
        with open(fn, 'r') as f:
            l.extend((
                add_name(x, fn2) for x in f.readlines()
            ))

    print(''.join(l))


merge(glob.glob(sys.argv[1] + '/*/output.txt'))


