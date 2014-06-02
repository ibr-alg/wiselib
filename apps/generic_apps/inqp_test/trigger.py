#!/usr/bin/env python

import time
import datetime
import math
import sys
#import os

INTERVAL=300.0

#sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

while True:
    now = time.time()
    nxt = INTERVAL * math.ceil(now / INTERVAL)
    time.sleep(nxt - time.time())
    sys.stdout.write('X\n')
    sys.stdout.flush()
    #print('X')
    #print (time.time())


