#!/usr/bin/env python3

import re
import matplotlib.pyplot as plt
import csv
from matplotlib import rc
rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)

# avg over 64 measurements,
# 3000 measurements per sec.
# measurement interval is in seconds

MEASUREMENT_INTERVAL = 64.0 / 3000.0

# mA
CURRENT_FACTOR = 70.0 / 4095.0

def parse_energy(f):
    ts = []
    vs = []

    #reader = csv.DictReader(f, delimiter=';', quotechar='"')
    reader = csv.DictReader(f, delimiter='\t', quotechar='"')
    t = 0
    for row in reader:
        v = float(row['avg']) * CURRENT_FACTOR
        t += MEASUREMENT_INTERVAL

        ts.append(t)
        vs.append(v)

    return ts, vs

def parse_tuple_counts(f, expid):
    r = []
    for line in f:
        m = re.search(r'sent ([0-9]+) tuples chk', line)
        if m is not None:
            if expid >= 24638:
                # starting from exp 24638 tuple counts are reported
                # incrementally
                r.append(int(m.groups()[0]) - (sum(r) if len(r) else 0))
            else:
                r.append(int(m.groups()[0]))
    return r

def find_tuple_spikes(ts, vs):
    # setup as follows:
    #
    # (1) first, start in high-load mode ( >= HIGH ),
    # (2) then, itll drop down to idle ( <= IDLE )
    # (3) then itll go above MEASUREMENT
    # (4) then fall below IDLE
    # then back to 1

    HIGH = 7.0
    IDLE = 1.5
    MEASUREMENT = 1.5

    # unit: mA * s = mC
    energy_sums = []
    time_sums = []

    state = "H"
    oldstate = "H"
    for t, v in zip(ts, vs):
        if state == "H" and v < IDLE:
            state = "I0"
        elif state == "I1" and v > HIGH:
            state = "H"
        elif state == "I0" and v > MEASUREMENT:
            state = "M"
            t0 = t
            tprev = t
            esum = 0
        elif state == "M" and v < IDLE:
            state = "I1"
            time_sums.append(t - t0)
            energy_sums.append(esum)
        elif state == "M":
            #assert v < HIGH
            esum += (t - tprev) * v
            tprev = t
    
        if state != oldstate:
            print("{}: {} ({} -> {})".format(t, v, oldstate, state))
            oldstate = state

    return energy_sums, time_sums

def frange(a, b, step):
    return [x * step for x in range(int(a / step), int(b / step))]

def fig_energy(ts, vs):
    fig = plt.figure()
    ax = plt.subplot(111)
    
    #ax.set_xticks(range(250, 311, 2))
    #ax.set_yticks(frange(0, 3, 0.2))

    ax.set_xlim((200, 500))
    #ax.set_ylim((0, 3))
    ax.grid()

    ax.plot(ts, vs)
    fig.savefig('energy.pdf')

def cum(l):
    s = 0
    r = []
    for v in l:
        s += v
        r.append(s)
    return r

def plot_experiment(n, ax, **kwargs):
    global fig
    ts, vs = parse_energy(open('{}.csv'.format(n), 'r'))
    fig_energy(ts, vs)
    tc = parse_tuple_counts(open('{}/inode001/output.txt'.format(n), 'r', encoding='latin1'), int(n))
    energy_sums, time_sums = find_tuple_spikes(ts, vs)

    # incontextsensing
    #tc = [7, 6, 8, 11, 11, 10, 11, 9]

    #ax = plt.subplot(111)
    print("XXX", energy_sums)
    print("YYY", tc)
    ax.plot(cum(tc[:len(energy_sums)]), ([x/y for x, y in zip(energy_sums, tc)]), 'o-',
            **kwargs) 
    ax.plot(cum(tc[:len(time_sums)]), ([x/y for x, y in zip(time_sums, tc)]), 'x-',
            **kwargs)
    ax.legend()

#ts, vs = parse_energy(open('tuplestore_24526_1242.csv', 'r'))
#ts, vs = parse_energy(open('tuplestore_24528_1242.csv', 'r'))
#fig_energy(ts, vs)
#print()

fig = plt.figure()
ax = plt.subplot(111)

#plot_experiment(24539, ax, label='antelope 39')

## unbaltreedict, list_dynamic container
##plot_experiment(24529, ax, label='ts')

## ts staticdict(100, 15), staticvector(76)
##plot_experiment(24578, ax, label='ts')

## antelope with more realistic config?
##plot_experiment(24579, ax, label='antelope2')

## ts staticdict(100, 15), staticvector(76), strncmp_etc_builtin
##plot_experiment(24580, ax, label='ts100')

## ts staticdict(200, 15), staticvector(76), strncmp_etc_builtin
##plot_experiment(24582, ax, label='ts200a')
## ts staticdict(200, 15), staticvector(76), strncmp_etc_builtin
##plot_experiment(24583, ax, label='ts200b')

# ts insert incontextsensing staticdict(200, 15), staticvector(76), strncmp_etc_builtin
#plot_experiment(24638, ax, label='ts200')
# ts insert incontextsensing staticdict(100, 15), staticvector(76), strncmp_etc_builtin
plot_experiment(24639, ax, label='ts100')

# ts insert incontextsensing staticdict-le(100, 15), staticvector(76), strncmp_etc_builtin
plot_experiment(24643, ax, label='ts100le 43')
plot_experiment(24644, ax, label='ts100le 44')
plot_experiment(24645, ax, label='ts100le 45')
#plot_experiment(24646, ax, label='ts100le 46')
plot_experiment(24648, ax, label='ts100le 48')
plot_experiment(24649, ax, label='ts100le 49')

plot_experiment(24641, ax, label='antelope 41')

plot_experiment(24650, ax, label='antelope 50')
plot_experiment(24651, ax, label='antelope 51')
plot_experiment(24652, ax, label='antelope 52')

fig.savefig('energy_inserts.pdf')

