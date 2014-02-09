#!/usr/bin/env python3

import re
import matplotlib.pyplot as plt
import csv
from matplotlib import rc
import sys
import os
import os.path
import gzip
rc('font',**{'family':'serif','serif':['Palatino'], 'size': 6})
rc('text', usetex=True)

EXP_DIR='experiments/'

# avg over 64 measurements,
# 3000 measurements per sec.
# measurement interval is in seconds

MEASUREMENT_INTERVAL = 64.0 / 3000.0

# mA
CURRENT_FACTOR = 70.0 / 4095.0
BASELINE_ENERGY = 0.568206918430233


# Gateway hostname -> DB hostname
gateway_to_db = {
    'inode015': 'inode016',
    'inode007': 'inode010',
    'inode011': 'inode014',
    'inode009': 'inode008'
}

blacklist = [
    { 'job': '24814', 'mode': 'find', 'database': 'antelope' },
    { 'job': '24814', 'inode_db': 'inode014', '_tmax': 3000 },
    { 'job': '24815', 'mode': 'find', 'database': 'antelope' },
    { 'job': '24815', 'inode_db': 'inode010' },
    { 'job': '24815', 'inode_db': 'inode016', '_tmax': 3000 },
    { 'job': '24815', 'inode_db': 'inode014', '_valid': [(4000, 8000), (10000, 16000)] },
    { 'job': '24817', 'mode': 'find', 'database': 'antelope' },
    { 'job': '24817', 'inode_db': 'inode008', '_valid': [(0, 3000)] },
    { 'job': '24817', 'inode_db': 'inode014', '_valid': [(0, 1000), (4000, 8000)] },
    { 'job': '24817', 'inode_db': 'inode016', '_valid': [(0, 3000), (6000, 8000)] },
    { 'job': '24818', 'mode': 'find', 'database': 'antelope' },
    { 'job': '24818', 'inode_db': 'inode014', '_tmin': 2000 },
    { 'job': '24818', 'inode_db': 'inode008', '_tmax': 5000 },
    { 'job': '24819', 'inode_db': 'inode010' },
    { 'job': '24819', 'inode_db': 'inode016' },
    { 'job': '24819', 'inode_db': 'inode008', '_tmin': 2000 },
    { 'job': '24819', 'inode_db': 'inode014', '_tmax': 3000 },
    { 'job': '24821', 'inode_db': 'inode010' }, # empty
    { 'job': '24821', 'inode_db': 'inode016' }, # some sub-runs are actually ok
    { 'job': '24821', 'inode_db': 'inode008' }, # some sub-runs are actually ok
    { 'job': '24822', 'inode_db': 'inode014' }, # empty
    { 'job': '24822', 'inode_db': 'inode016' }, # empty
    { 'job': '24822', 'inode_db': 'inode008' }, # 1 run only
    { 'job': '24822', 'inode_db': 'inode010', '_tmax': 600 }, # 1 run only
    { 'job': '24823' },
    { 'job': '24824' },
    { 'job': '24825', 'inode_db': 'inode016' },
    #{ 'job': '24824', 'inode_db': 'inode008' },
    { 'job': '24825', 'inode_db': 'inode010' },

    { 'job': '24857', '_tmax': 15.0 },
]

teenylime_runs = set(['24857'])

# Experiment class => { 'ts': [...], 'vs': [...], 'cls': cls }
experiments = {}

style = {
    'tuplestore': {
        'ls': 'k-',
        'boxcolor': 'black',
    },
    'antelope': {
        'ls': 'grey',
        'boxcolor': 'grey',
    },
    'teeny': {
        'ls': 'g-',
        'boxcolor': 'green',
    }
}


def has_valid(b):
    """
    >>> has_valid({ '_valid': [] })
    False
    >>> has_valid({ })
    False
    >>> has_valid({ '_tmax': 0 })
    False

    >>> has_valid({ '_valid': [(100, 200), (350, 400)] })
    True
    >>> has_valid({ '_tmax': 100 })
    True
    >>> has_valid({ '_tmin': 100 })
    True
    >>> has_valid({ '_tmax': 0, '_tmin': 50 })
    True
    """

    if '_valid' in b:
        return len(b['_valid']) > 0
    elif '_tmin' in b: return True
    elif '_tmax' in b: return b['_tmax'] != 0
    return False

def valid(b, t):
    """
    >>> valid({ '_valid': [] }, 123)
    False
    >>> valid({ }, 123)
    False
    >>> valid({ '_tmax': 0 }, 123)
    False

    >>> valid({ '_valid': [(100, 200), (350, 400)] }, 100)
    True
    >>> valid({ '_valid': [(100, 200), (350, 400)] }, 101)
    True
    >>> valid({ '_valid': [(100, 200), (350, 400)] }, 357.2)
    True
    >>> valid({ '_valid': [(100, 200), (350, 400)] }, 200.01)
    False
    >>> valid({ '_tmax': 100 }, 99)
    True
    >>> valid({ '_tmin': 100 }, 100.123)
    True
    >>> valid({ '_tmax': 0, '_tmin': 50 }, 55.678)
    False
    """
    if '_valid' in b:
        for a, b in b['_valid']:
            if t >= a and t <= b: return True
        return False
    elif '_tmin' in b:
        if '_tmax' in b and b['_tmax'] is not None:
            return t >= b['_tmin'] and t <= b['_tmax']
        return t >= b['_tmin']
    elif '_tmax' in b:
        return b['_tmax'] is None or b['_tmax'] >= t
    return False


def median(l):
    if len(l) == 0: return None
    #print("{} -> {}".format(l, sorted(l)[int(len(l) / 2)]))
    sl = sorted(l)
    if len(l) % 2:
        return sorted(l)[int(len(l) / 2)]
    return (sl[int(len(l) / 2)] + sl[int(len(l) / 2 - 1)]) / 2.0

def main():
    process_directories(
        sys.argv[1:],
        lambda k: k.mode == 'insert' #and k.database != 'antelope'
    )
    
    fs = (10, 5)
    
    fig_i_e = plt.figure(figsize=fs)
    ax_i_e = plt.subplot(111)

    fig_i_t = plt.figure(figsize=fs)
    ax_i_t = plt.subplot(111)

    fig_f_e = plt.figure(figsize=fs)
    ax_f_e = plt.subplot(111)

    fig_f_t = plt.figure(figsize=fs)
    ax_f_t = plt.subplot(111)

    #ax_i_e.set_yscale('log')

    shift_i = 1
    shift_f = 1
    l_f = 1
    l_e = 1
    for k, exp in experiments.items():
        #plot_energies([v], k.reprname() + '.pdf')
        if k.mode == 'insert':
            pos_e = [x + shift_i for x in exp.tuplecounts[:len(exp.energy)]]
            pos_t = [x + shift_i for x in exp.tuplecounts[:len(exp.time)]]

            # If for some weird reason we have more data points than
            # sent out packets, put the rest at pos 100 so we notice something
            # is up (but still can see most of the values)
            pos_e += [100] * (len(exp.energy) - len(pos_e))
            pos_t += [100] * (len(exp.time) - len(pos_t))

            pos_e, es = cleanse(pos_e, exp.energy)
            pos_t, ts = cleanse(pos_t, exp.time)

            print(k.mode, k.database, [len(x) for x in es])

            if len(es):
                bp = ax_i_e.boxplot(es, positions=pos_e)
                plt.setp(bp['boxes'], color=style[k.database]['boxcolor'])
                plt.setp(bp['whiskers'], color=style[k.database]['boxcolor'])
                plt.setp(bp['fliers'], color=style[k.database]['boxcolor'], marker='+')

                ax_i_e.plot(pos_e, [median(x) for x in es], style[k.database]['ls'], label=k.database)

            if len(ts):
                bp = ax_i_t.boxplot(ts, positions=pos_t)
                plt.setp(bp['boxes'], color=style[k.database]['boxcolor'])
                plt.setp(bp['whiskers'], color=style[k.database]['boxcolor'])
                plt.setp(bp['fliers'], color=style[k.database]['boxcolor'], marker='+')

                ax_i_t.plot(pos_t, [median(x) for x in ts],style[k.database]['ls'], label=k.database)

            shift_i -= 1

        elif k.mode == 'find':
            #if k.database == 'antelope': continue

            pos_e = [x + shift_f for x in exp.tuplecounts[:len(exp.energy)]]
            pos_t = [x + shift_f for x in exp.tuplecounts[:len(exp.time)]]

            # If for some weird reason we have more data points than
            # sent out packets, put the rest at pos 100 so we notice something
            # is up (but still can see most of the values)
            pos_e += [100] * (len(exp.energy) - len(pos_e))
            pos_t += [100] * (len(exp.time) - len(pos_t))

            pos_e, es = cleanse(pos_e, exp.energy)
            pos_t, ts = cleanse(pos_t, exp.time)

            print(k.mode, k.database, [len(x) for x in es])
            #print(pos_e, [median(x) for x in es])

            if len(es):
                bp = ax_f_e.boxplot(es, positions=pos_e)
                plt.setp(bp['boxes'], color=style[k.database]['boxcolor'])
                plt.setp(bp['whiskers'], color=style[k.database]['boxcolor'])
                plt.setp(bp['fliers'], color=style[k.database]['boxcolor'], marker='+')

                ax_f_e.plot(pos_e, [median(x) for x in es], style[k.database]['ls'], label=k.database)

            if len(ts):
                bp = ax_f_t.boxplot(ts, positions=pos_t)
                plt.setp(bp['boxes'], color=style[k.database]['boxcolor'])
                plt.setp(bp['whiskers'], color=style[k.database]['boxcolor'])
                plt.setp(bp['fliers'], color=style[k.database]['boxcolor'], marker='+')

                ax_f_t.plot(pos_t, [median(x) for x in ts], style[k.database]['ls'], label=k.database)

            shift_f -= 1

    ax_i_e.set_xlim((0, 80))
    ax_i_e.set_ylim((0, .1))
    ax_i_t.set_xlim((0, 80))

    ax_i_e.legend()
    ax_i_t.legend()
    ax_f_e.legend()
    ax_f_t.legend()

    fig_i_e.savefig('pdf_out/energies_insert.pdf', bbox_inches='tight', pad_inches=0.1)
    fig_i_t.savefig('pdf_out/times_insert.pdf', bbox_inches='tight',pad_inches=0.1)
    fig_f_e.savefig('pdf_out/energies_find.pdf', bbox_inches='tight',pad_inches=0.1)
    fig_f_t.savefig('pdf_out/times_find.pdf', bbox_inches='tight',pad_inches=0.1)
            

def cleanse(l1, l2):
    """
    Given two lists l1 and l2 return two sublists l1' and l2' such that
    whenever an element at position i in either l1 or l2 is None or the empty
    list, data from that
    position at l1 or l2 will not be present in the returned lists.

    Note that specifically considers only None and [], not other values that
    evaluate to false like 0, False or the empty string.

    As a side effect it ensures the returned lists are cut to the same length

    >>> cleanse([1, 2, None, 4, [], 6], [None, 200, 300, 400, 500, 600, 70, 88])
    ([2, 4, 6], [200, 400, 600])

    >>> cleanse([1, [None, None, 2, None, 3]], [[None, 100], [200, 300]])
    ([1, [2, 3]], [[100], [200, 300]])

    """

    r1 = []
    r2 = []
    for x, y in zip(l1, l2):
        if isinstance(x, list):
            x = list(filter(lambda x: x is not None and x != [], x))
        if isinstance(y, list):
            y = list(filter(lambda z: z is not None and z != [], y))

        if x is not None and x != [] and y is not None and y != []:
            r1.append(x)
            r2.append(y)
    return r1, r2

#def plot_energies(experiments, fname='energies.pdf'):
    #fig = plt.figure()
    #ax = plt.subplot(111)
    ##ax.set_xticklabels([str(x) for x in experiments[0].tuplecounts])
    #for exp in experiments:
        #if exp.energy:
            ##print(len(exp.energy), len(exp.tuplecounts))
            #ax.boxplot(exp.energy, positions=exp.tuplecounts[:len(exp.energy)])
    #print("writing {}.".format(fname))
    #fig.savefig(fname)

#def plot_times(experiments, fname='times.pdf'):
    #fig = plt.figure()
    #ax = plt.subplot(111)
    ##ax.set_xticklabels([str(x) for x in experiments[0].tuplecounts])
    #for exp in experiments:
        ##print(exp.energy)
        #if exp.energy:
            #ax.boxplot(exp.energy, positions=exp.tuplecounts)
    #print("writing {}.".format(fname))
    #fig.savefig(fname)

class ExperimentClass:
    def __init__(self, d):
        self.__dict__.update(d)

    def __eq__(self, other):
        return (
            self.dataset == other.dataset and
            self.mode == other.mode and
            self.debug == other.debug and
            self.database == other.database
        )

    def reprname(self):
        return self.database + '_' + self.dataset.replace('.rdf', '') + '_' + self.mode + (' DBG' if
self.debug else '')

    def __hash__(self):
        return hash(self.dataset) ^ hash(self.mode) ^ hash(self.debug) ^ hash(self.database)

class Experiment:
    def __init__(self):
        self.time = []
        self.energy = []
        self.tuplecounts = []
        self.key = None

    def add_measurement(self, i, t, e):
        while len(self.time) < i + 1:
            self.energy.append([])
            self.time.append([])
        if self.key.mode == 'find':
            print("addm find");
            self.time[i].append(t)
            self.energy[i].append(e)
        else:
            print(i, len(self.time), len(self.tuplecounts))
            self.time[i].append(t / (self.tuplecounts[i] - (self.tuplecounts[i - 1] if i > 0 else 0)))
            self.energy[i].append(e / (self.tuplecounts[i] - (self.tuplecounts[i - 1] if i > 0 else 0)))

    def set_tuplecounts(self, tcs):
        self.tuplecounts = cum(tcs)

def add_experiment(cls):
    global experiments
    if cls not in experiments:
        experiments[cls] = Experiment()
        experiments[cls].key = cls
    return experiments[cls]

def inode_to_mote_id(s):
    """
    >>> inode_to_mote_id('inode123')
    '10123'
    >>> inode_to_mote_id('inode001')
    '10001'
    """
    return '10' + s[5:]

def mote_id_to_inode_id(s):
    """
    >>> mote_id_to_inode_id('10123')
    'inode123'
    >>> mote_id_to_inode_id('10001')
    'inode001'
    """
    return 'inode' + s[2:]

def process_directories(dirs,f=lambda x: True):
    for d in dirs:
        process_directory(d, f)

def process_directory(d, f=lambda x: True):
    d = str(d).strip('/')
    teenylime = (d in teenylime_runs)

    print()
    print("*** iMinds exp #{} {} ***".format(d, '(teenylime mode)' if teenylime else ''))


    # 1st pass: Blacklisting & Filtering of experiments

    # blacklist by db inode id
    bl = {}
    for gw, db in gateway_to_db.items():
        fn_gwinfo = EXP_DIR + d + '/' + gw + '/' + gw + '.vars'
        fn_gwout = EXP_DIR + d + '/' + gw + '/output.txt'
        if not os.path.exists(fn_gwinfo):
            print("{} not found, ignoring that area.".format(fn_gwinfo))
            continue

        v = read_vars(fn_gwinfo)
        cls = ExperimentClass(v)
        cls.job = d

        for b in blacklist:
            for k, x in b.items():
                if k.startswith('_'): continue
                if getattr(cls, k) != x:
                    # does not match this blacklist entry
                    break
            else:
                bl[db] = b
                break

    # Now actually read energy values
    # we only need subsamples for teenylime as they only insert one value at a
    # time
    energy = read_energy(EXP_DIR + d, bl, use_subsamples=teenylime, alpha=(.05 if teenylime else 1.0))

    for gw, db in gateway_to_db.items():
        if db in bl and not has_valid(bl[db]): continue
        #if tmaxs.get(db, None) == 0: continue
        #tmax = tmaxs.get(db, None)

        fn_gwinfo = EXP_DIR + d + '/' + gw + '/' + gw + '.vars'
        #fn_dbout = d + '/' + db + '/output.txt'
        fn_gwout = EXP_DIR + d + '/' + gw + '/output.txt'
        if not os.path.exists(fn_gwinfo):
            print("{} not found, ignoring that area.".format(fn_gwinfo))
            continue

        print()
        print("processing: {} -> {}".format(gw, db))

        v = read_vars(fn_gwinfo)
        cls = ExperimentClass(v)
        cls.job = d
        if not f(cls):
            print("({} ignored by filter)".format(db))
            continue

        print("  {}/{} {}".format(cls.database, cls.mode, cls.dataset))

        exp = add_experiment(cls)
        if teenylime:
            tc = [1] * 50 # range(20)
        else:
            tc = parse_tuple_counts(open(fn_gwout, 'r', encoding='latin1'), d)
            if not tc:
                print("  (!) no tuplecounts found in {}, using default".format(fn_gwout))
                tc = [7, 6, 8, 11, 11, 10, 11, 9]
        exp.set_tuplecounts(tc)
        #print(energy[inode_to_mote_id(db)])
        mid = inode_to_mote_id(db)
        if mid not in energy:
            print("  (!) no energy values for {}. got values for {}".format(mid, str(energy.keys())))
            continue

        #print("energy", type(energy))
        #print("v", type(v))
        if teenylime:
            runs_t, runs_e = process_energy_teenylime(energy[mid], v['mode'], lbl=db, tmin=bl.get(db, {}).get('_tmin', 0))
        else:
            runs_t, runs_e = process_energy(energy[mid], v['mode'], lbl=db, tmin=bl.get(db, {}).get('_tmin', 0))

        runs_count = 0
        for j, (ts, es) in enumerate(zip(runs_t, runs_e)):
            runs_count += 1
            print("  adding run {} of {} with {} entries".format(j, len(runs_e), len(es)))
            for i, (t, e) in enumerate(zip(ts, es)):
                if t is not None and e is not None: #t != 0 or e != 0:
                    exp.add_measurement(i, t, e)
                else:
                    print("  skipping i={} t={} e={}".format(i, t, e))
                    #pass
        print("  processed {} experiment runs.".format(runs_count))

def read_vars(fn):
    r = {}
    for line in open(fn, 'r'):
        k, v = line.split('=')
        k = k.strip()
        v = v.strip()
        k = k.lower()
        if v.startswith('"'):
            r[k] = v[1:-1]
        else:
            r[k] = int(v)
    if 'database' not in r:
        r['database'] = 'db'

    return r

def read_energy(fn, bl, use_subsamples=False, alpha=0.05):
    r = {}

    f = None
    if os.path.exists(fn + '.csv.gz'):
        f = gzip.open(fn + '.csv.gz', 'rt', encoding='latin1')
    else:
        f = open(fn + '.csv', 'r')

    #reader = csv.DictReader(f, delimiter=';', quotechar='"')
    reader = csv.DictReader(f, delimiter='\t', quotechar='"')
    #t = {}
    t = {}
    for row in reader:
        mote_id = row['motelabMoteID']
        if mote_id not in r: r[mote_id] = {'ts':[], 'vs':[]}
        b = bl.get(mote_id_to_inode_id(mote_id))
        if b is not None and not valid(b, t.get(mote_id, 0)):
            continue

        def add_sample(v, delta):
            t[mote_id] = t.get(mote_id, -delta) + delta
    #r[mote_id]['ts'][-1] + MEASUREMENT_INTERVAL if len(r[mote_id]['ts']) else 0
            r[mote_id]['ts'].append(t[mote_id])
            r[mote_id]['vs'].append(alpha * v + (1.0 - alpha) * (r[mote_id]['vs'][-1]
if len(r[mote_id]['vs']) else 0.0))

        if use_subsamples:
            for i in range(64):
                s = 'sample_{}'.format(i)
                add_sample(float(row[s]) * CURRENT_FACTOR, MEASUREMENT_INTERVAL / 64.0)
        else:
            add_sample(float(row['avg']) * CURRENT_FACTOR, MEASUREMENT_INTERVAL)

    return r

def parse_tuple_counts(f, expid):
    r = []
    for line in f:
        m = re.search(r'sent ([0-9]+) tuples chk', line)
        if m is not None:
            if int(expid) >= 24638:
                # starting from exp 24638 tuple counts are reported
                # incrementally
                r.append(int(m.groups()[0]) - (sum(r) if len(r) else 0))
            else:
                r.append(int(m.groups()[0]))
    return r


def process_energy_teenylime(d, mode, lbl='', tmin=0, tmax=11.0):
    ts = d['ts']
    vs = d['vs']
    fig_energy(ts, vs, lbl)

    MEASUREMENT_UP = 0.6
    MEASUREMENT_DOWN = 0.3
    delta_t = 0.8

    class State:
        def __init__(self, s): self.s = s
        def __str__(self): return self.s
    idle_low = State('low')
    measurement = State('measurement')

    sums_e = []
    sums_t = []
    runs_e = []
    runs_t = []

    thigh = 0
    t0 = 0
    tprev = 0
    esum = 0
    t = 0
    v = 0

    #esums_i = []
    #esums_f = []
    #tsums_i = []
    #tsums_f = []

    BASELINE_ENERGY_TEENYLIME = 0
    baseline_estimate = 0
    baseline_estimate_n = 0

    state = idle_low

    def change_state(s):
        nonlocal state
        nonlocal thigh
        if s is not state:
            print("{}: {} -> {} v={}".format(t, state, s, v))
            state = s

    for t, v in zip(ts, vs):
        if t < tmin: continue
        if t > tmax:
            print("{} > tmax".format(t))
            break

        if state is idle_low:
            if v > MEASUREMENT_UP:

                change_state(measurement)
                #if mode == 'insert':
                t0 = t
                esum = (t - tprev) * (v - BASELINE_ENERGY_TEENYLIME)
                #tprev = t
            else:
                baseline_estimate *= baseline_estimate_n / (baseline_estimate_n + 1.0)
                baseline_estimate_n += 1.0
                baseline_estimate += v / baseline_estimate_n 

        elif state is measurement:
            if v < MEASUREMENT_DOWN:
                change_state(idle_low)
                #if mode == 'insert':
                esum += (t - tprev) * (v - BASELINE_ENERGY_TEENYLIME)
                sums_t.append(t - t0)
                sums_e.append(esum)
            else:
                esum += (t - tprev) * (v - BASELINE_ENERGY_TEENYLIME)
        tprev = t


    print("  baseline estimate (teenylime): {}".format(baseline_estimate))
    runs_t.append(sums_t)
    runs_e.append(sums_e)
    return runs_t, runs_e


def process_energy(d, mode, lbl='', tmin=0):
    ts = d['ts']
    vs = d['vs']
    fig_energy(ts, vs, lbl)

    # setup as follows:
    #
    # (1) first, start in high-load mode ( >= HIGH ),
    # (2) then, itll drop down to idle ( <= IDLE )
    # (3) then itll go above MEASUREMENT
    # (4) then fall below IDLE
    # then back to 1

    HIGH = 7.0
    RADIO = 1.5
    MEASUREMENT = 1.5

    class State:
        def __init__(self, s): self.s = s
        def __str__(self): return self.s

    idle_high = State('high')

    idle_before = State('idle before')
    insert = State('insert')
    idle_between = State('idle between')
    find = State('find')
    idle_after = State('idle_after')

    state = idle_high

    # unit: mA * s = mC
    #energy_sums_insert = []
    #energy_sums_find = []
    #time_sums_insert = []
    #time_sums_find = []
    sums_e = []
    sums_t = []
    runs_e = []
    runs_t = []

    thigh = 0
    t0 = 0
    tprev = 0
    esum = 0
    t = 0

    #esums_i = []
    #esums_f = []
    #tsums_i = []
    #tsums_f = []

    baseline_estimate = 0
    baseline_estimate_n = 0

    def change_state(s):
        nonlocal state
        nonlocal thigh
        if s is not state:
            #print("{}: {} -> {}".format(t, state, s))
            if s is idle_high:
                thigh = t
            state = s

    for t, v in zip(ts, vs):
        if t < tmin: continue

        if state is idle_high:
            if v < RADIO:
                if t - thigh > 500:
                    #print("  reboot t={} thigh={}".format(t, thigh))
                    # reboot
                    runs_e.append(sums_e)
                    runs_t.append(sums_t)
                    sums_e = []
                    sums_t = []
                    
                    #esums_i.append(energy_sums_insert)
                    #esums_f.append(energy_sums_find)
                    #tsums_i.append(time_sums_insert)
                    #tsums_f.append(time_sums_find)
                    #energy_sums_insert = []
                    #energy_sums_find = []
                    #time_sums_insert = []
                    #time_sums_find = []
                #else:
                    #print("  t-tigh={} t={}".format(t-thigh, t))
                change_state(idle_before)

        elif state is idle_before:
            if v > MEASUREMENT:
                change_state(insert)
                if mode == 'insert':
                    t0 = t
                    esum = (t - tprev) * (v - BASELINE_ENERGY)
                    #tprev = t
            else:
                baseline_estimate *= baseline_estimate_n / (baseline_estimate_n + 1.0)
                baseline_estimate_n += 1.0
                baseline_estimate += v / baseline_estimate_n 

        elif state is insert:
            if v < MEASUREMENT:
                change_state(idle_between)
                if mode == 'insert':
                    esum += (t - tprev) * (v - BASELINE_ENERGY)
                    sums_t.append(t - t0)
                    sums_e.append(esum)
            else:
                #print(v, t)
                #assert v < HIGH
                if v >= HIGH:
                    print("    insert abort high t={}".format(t))
                    sums_e.append(None)
                    sums_t.append(None)
                    change_state(idle_high)
                    #thigh = t
                elif mode == 'insert':
                    esum += (t - tprev) * (v - BASELINE_ENERGY)
                    #tprev = t

        elif state is idle_between:
            if v > HIGH:
                #print("  find measurement skipped high at {}".format(t))
                if mode == 'find':
                    sums_e.append(None)
                    sums_t.append(None)
                change_state(idle_high)
                #thigh = t
            elif v > MEASUREMENT:
                change_state(find)
                if mode == 'find':
                    t0 = t
                    #tprev = t
                    #esum = 0
                    esum = (t - tprev) * (v - BASELINE_ENERGY)
            else:
                baseline_estimate *= baseline_estimate_n / (baseline_estimate_n + 1.0)
                baseline_estimate_n += 1.0
                baseline_estimate += v / baseline_estimate_n 

        elif state is find:
            if v < MEASUREMENT:
                #print("find measurement at {} e {}".format(t, esum))
                change_state(idle_after)
                if mode == 'find':
                    esum += (t - tprev) * (v - BASELINE_ENERGY)
                    sums_t.append(t - t0)
                    sums_e.append(esum)
            elif v > HIGH:
                #print("  find measurement aborted high at {} t0={} esum={}".format(t, t0, esum - BASELINE_ENERGY))
                # what we thought was a find measurement was actually a
                # rising edge for the high idle state,
                # seems there was no (measurable) find process, record a
                # 0-measurement
                change_state(idle_high)
                #thigh = t
                if mode == 'find':
                    sums_t.append(None)
                    sums_e.append(None)
            else:
                if mode == 'find':
                    esum += (t - tprev) * (v - BASELINE_ENERGY)
                    #tprev = t

        elif state is idle_after:
            if v > HIGH:
                change_state(idle_high)
                #thigh = t
            else:
                baseline_estimate *= baseline_estimate_n / (baseline_estimate_n + 1.0)
                baseline_estimate_n += 1.0
                baseline_estimate += v / baseline_estimate_n 

        tprev = t


    print("  baseline estimate: {}".format(baseline_estimate))

    #state = "H"
    #oldstate = "H"
    #for t, v in zip(ts, vs):
        #if state == "H" and v < IDLE:
            #state = "I0"
        #elif state == "I1" and v > HIGH:
            #state = "H"
        #elif state == "I0" and v > MEASUREMENT:
            #state = "M"
            #t0 = t
            #tprev = t
            #esum = 0
        #elif state == "M" and v < IDLE:
            #state = "I1"
            #time_sums.append(t - t0)
            #energy_sums.append(esum)
        #elif state == "M":
            ##assert v < HIGH
            #esum += (t - tprev) * v
            #tprev = t
    
        #if state != oldstate:
            #print("{}: {} ({} -> {})".format(t, v, oldstate, state))
            #oldstate = state

    #esums_i.append(energy_sums_insert)
    #esums_f.append(energy_sums_find)
    #tsums_i.append(time_sums_insert)
    #tsums_f.append(time_sums_find)
    runs_t.append(sums_t)
    runs_e.append(sums_e)
    return (runs_t, runs_e)

def frange(a, b, step):
    return [x * step for x in range(int(a / step), int(b / step))]


def fig_energy(ts, vs, n):
    return
    fig = plt.figure(figsize=(10,5))
    ax = plt.subplot(111)
    
    #ax.set_xticks(range(250, 311, 2))
    #ax.set_yticks(frange(0, 3, 0.2))

    ax.set_xlim((0, 50))
    #ax.set_ylim((0, 2))
    ax.grid()

    ax.plot(ts, vs, 'k-')
    fig.savefig('energy_{}.pdf'.format(n), bbox_inches='tight', pad_inches=.1)

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
    fig_energy(ts, vs, n)
    #tc = parse_tuple_counts(open('{}/inode001/output.txt'.format(n), 'r', encoding='latin1'), int(n))
    d = find_tuple_spikes(ts, vs)
    
    tc = [7, 6, 8, 11, 11, 10, 11, 9]
    #ax = plt.subplot(111)
    for e in range(len(d['e_insert'])):
        es = d['e_insert'][e]
        fs = d['e_find'][e]
        fs = d['e_find'][e]

        print(d['e_insert'][e])
        print(d['t_insert'][e])

        #ax.plot(cum(tc[:len(es)]), ([x/y for x, y in zip(es, tc)]), 'o-', **kwargs) 
        ax.plot(cum(tc[:len(fs)]), ([x/y for x, y in zip(fs, tc)]), 'x-', **kwargs) 
        ax.plot(cum(tc[:len(fs)]), ([x/y for x, y in zip(d['t_find'][e], tc)]), 'x-', **kwargs) 
            

    ## incontextsensing
    ##tc = [7, 6, 8, 11, 11, 10, 11, 9]

    ##ax = plt.subplot(111)
    #print("XXX", energy_sums)
    #print("YYY", tc)
    #ax.plot(cum(tc[:len(energy_sums)]), ([x/y for x, y in zip(energy_sums, tc)]), 'o-',
            #**kwargs) 
    #ax.plot(cum(tc[:len(time_sums)]), ([x/y for x, y in zip(time_sums, tc)]), 'x-',
            #**kwargs)
    #ax.legend()

#ts, vs = parse_energy(open('tuplestore_24526_1242.csv', 'r'))
#ts, vs = parse_energy(open('tuplestore_24528_1242.csv', 'r'))
#fig_energy(ts, vs)
#print()

#fig = plt.figure()
#ax = plt.subplot(111)

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
#plot_experiment(24639, ax, label='ts100')

## ts insert incontextsensing staticdict-le(100, 15), staticvector(76), strncmp_etc_builtin
#plot_experiment(24643, ax, label='ts100le 43')
#plot_experiment(24644, ax, label='ts100le 44')
#plot_experiment(24645, ax, label='ts100le 45')
##plot_experiment(24646, ax, label='ts100le 46')
#plot_experiment(24648, ax, label='ts100le 48')
#plot_experiment(24649, ax, label='ts100le 49')

#plot_experiment(24641, ax, label='antelope 41')

#plot_experiment(24650, ax, label='antelope 50')
#plot_experiment(24651, ax, label='antelope 51')
#plot_experiment(24652, ax, label='antelope 52')

#plot_experiment(24778, ax, label='antelope')

#fig.savefig('energy_inserts.pdf')

if __name__ == '__main__':
    main()


