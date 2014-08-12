#!/usr/bin/env python3

import os
import os.path
import sys
import pickle
import time
import datetime

MEASUREMENT_INTERVAL = 64.0 / 3000.0
MEASUREMENT_INTERVAL_SUBSAMPLE = 1.0 / 3000.0

def show_cache(fn):
    print("{}: {:.2f}M".format(fn, os.stat(fn).st_size / (1024 ** 2)))
    d = pickle.load(open(fn, 'rb'))
    print("  function: {}".format(d['function_name']))
    print("  kws: {}".format(d['kws']))
    print("  timestamp: {} ({})".format(d['timestamp'],
        time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(d['timestamp']))))

    r = d['return_value']
    print("  return: {} measurements ({:.2f} node-minutes)".format(
            len(r),
            (1/60.0) * len(r) * (MEASUREMENT_INTERVAL_SUBSAMPLE if d['kws']['subsample'] else MEASUREMENT_INTERVAL)))
    #for k,v in sorted(d['return_value'].items()):
        #print("    mote {}: {} entries (={:.2f}min)".format(k, len(v),
            #(1/60.0) * len(v) * (MEASUREMENT_INTERVAL_SUBSAMPLE if d['kws']['subsample'] else MEASUREMENT_INTERVAL)))
    print()


def list_subsample(fn):
    d = pickle.load(open(fn, 'rb'))
    if d['kws']['subsample']:
        print(fn)

if __name__ == '__main__':
    for arg in sys.argv[1:]:
        show_cache(arg)
        #list_subsample(arg)


