#!/usr/bin/python

import sys
import os

# Append current directory to sys.path to find TOSSIM.py when script is called
# from another directory
sys.path.append( os.getcwd() )
#print "Appended '" + os.getcwd() + "' to sys.path..."
#print "sys.path =", sys.path

from TOSSIM import *
from random import *

print "Preparing simulation."
t = Tossim([])
r = t.radio()

f = open(sys.path[0] + "/sparse-grid.txt", "r")
lines = f.readlines()
for line in lines:
  s = line.split()
  if (len(s) > 0):
    if s[0] == "gain":
      r.add(int(s[1]), int(s[2]), float(s[3]))

#noise = open(sys.path[0] + "/meyer-heavy.txt", "r")
#noise = open(sys.path[0] + "/meyer-short.txt", "r")
noise = open(sys.path[0] + "/casino-lab.txt", "r")
lines = noise.readlines()
for line in lines:
  str = line.strip()
  if (str != ""):
    val = int(str)
    for i in range(0, 10):
      m = t.getNode(i);
      m.addNoiseTraceReading(val)

for i in range(0, 10):
  m = t.getNode(i);
  m.createNoiseModel();
  time = randint(t.ticksPerSecond(), 10 * t.ticksPerSecond())
  m.bootAtTime(time)
  print "Booting ", i, " at time ", time

print "Starting simulation."

t.addChannel("Wiselib", sys.stdout);
t.addChannel("Boot", sys.stdout);
#t.addChannel("Timer", sys.stdout);
#t.addChannel("AMControl", sys.stdout);

while (t.time() < 1000 * t.ticksPerSecond()):
  t.runNextEvent()

print "Completed simulation."
