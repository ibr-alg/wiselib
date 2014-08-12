#!/usr/bin/env python3


import sys
import math
import re
import matplotlib.pyplot as plt
from matplotlib import rc
import csv
import numpy as np
import os
import os.path
import gzip
import gc
import scipy.signal
import itertools

sys.path.append('/home/henning/bin')
from cache import cached, cache_hash
from experiment_utils import t_average, band_stop, free_ram, np_moving_average
from experiment_model_np import ExperimentModel, Situation, Repeat
import policies

MAX_EXPERIMENT_RUNS = 7

MEASUREMENT_INTERVAL = 64.0 / 3000.0
MEASUREMENT_INTERVAL_SUBSAMPLE = 1.0 / 3000.0

PLOT_ENERGY = False
UPDATE_PLOT = False


# mA = 
CURRENT_FACTOR = 70.0 / 4095.0
# mA * 3300mV = microJoule (uJ)
CURRENT_DISPLAY_FACTOR = 3300.0
# s -> ms
TIME_DISPLAY_FACTOR = 1000.0
TIME_DISPLAY_FACTOR_FIND = 1000.0
EXP_DIR = 'experiments/'
TEENYLIME_INSERT_AT_ONCE = 4
# There should actually 3, but it seems the 3 doest cost any measurable energy
# for some reason (although according to the one-at-a-time experiments 12
# tuples should fit)
TEENYLIME_INSERT_CALLS = 2
FINDS_AT_ONCE = 10
TEENYLIME_FINDS_AT_ONCE = 1
TEENYLIME_ERASES_AT_ONCE = 1

# Prescilla-only mode, also make sure to edit policies.py accordingly
PRESCILLA = False

gateway_to_db = {
    # Convention: gateway nodes are odd, db nodes are even
    'inode015': 'inode016',
    'inode007': 'inode010',
    'inode011': 'inode014',
    'inode009': 'inode008',
    'inode019': 'inode018',
    'inode017': 'inode020',
    'inode023': 'inode022',
    'inode025': 'inode024',
    'inode027': 'inode026',
    'inode029': 'inode028',
    'inode031': 'inode030',
}

rc('font', family='serif',serif=['Palatino'], size=12)
rc('text', usetex=True)

# ( Experiment class , { 'ts': [...], 'vs': [...], 'cls': cls } )
experiments = []

class ExperimentClass:
    def __init__(self, d):
        self.ts_container = 'vector_static'
        self.ts_dict = 'chopper'
        self.__dict__.update(d.__dict__)

    def __eq__(self, other):
        return (
            self.dataset == other.dataset and
            self.mode == other.mode and
            self.debug == other.debug and
            self.database == other.database and (
                self.database != 'tuplestore' or (
                    self.ts_dict == other.ts_dict and
                    self.ts_container == other.ts_container
                )
            )
        )

    def reprname(self):
        return self.database + '_' + self.dataset.replace('.rdf', '') + '_' + self.mode + (' DBG' if self.debug else '')

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


        if len(self.time[i]) >= MAX_EXPERIMENT_RUNS: return
        if len(self.energy[i]) >= MAX_EXPERIMENT_RUNS: return

        if self.key.mode == 'find':
            #assert e >= 0
            self.time[i].append(t)
            self.energy[i].append(e)

        else:
            while len(self.tuplecounts) < i + 1:
                self.tuplecounts.append(100)
            #print(i, len(self.time), len(self.tuplecounts))
            diff = (self.tuplecounts[i] - (self.tuplecounts[i - 1] if i > 0 else 0))
            if diff <= 0:
                #print("  (!) warning: tc[{}]={} tc[{}]={} setting diff to 1".format(i, self.tuplecounts[i], (i-1), self.tuplecounts[i - 1] if i > 0 else 0))
                diff = 1
            self.time[i].append(t / diff)
            #assert e >= 0
            self.energy[i].append(e / diff)

    def set_tuplecounts(self, tcs):
        if len(self.tuplecounts) > len(tcs):
            return
        self.tuplecounts = np.cumsum(tcs).tolist()

class DirInfo:
    pass

class Area:
    def __init__(self):
        self.filename_energy = None
        self.filename_gw_out = None
        self.inode_db = None

    def __repr__(self):
        return 'Area({},{})'.format(self.filename_energy, self.inode_db)

    def __eq__(self, other):
        return self.__dict__ == other.__dict__

    def cache_hash(self):
        return cache_hash((self.filename_energy, self.inode_db))

class Policy:
    def __init__(self, **kws):
        self.process = True
        self.subsample = False
        self.tmin = 0
        self.tmax = None
        self.matches = []
        self.__dict__.update(kws)

    def __eq__(self, other):
        return (
                self.process == other.process and
                self.subsample == other.subsample and
                self.tmin == other.tmin and
                self.tmax == other.tmax
        )

    def __repr__(self):
        return 'Policy(process={},subsample={},tmin={},tmax={},matches={})'.format(self.process, self.subsample, self.tmin,self.tmax,self.matches)

    def cache_hash(self):
        return cache_hash((self.process, self.tmin, self.tmax, self.subsample))

def add_experiment(cls):
    global experiments

    for k, v in experiments:
        if k == cls:
            return v

    v = Experiment()
    v.key = cls

    experiments.append((cls, v));
    return v


def get_style(k):
    l = '-'
    if k.database == 'tuplestore':
        c = dict(tree = '#88bbbb', avl = '#bbaa88', prescilla = '#808080').get(k.ts_dict, 'black')
    elif k.database == 'teeny':
        c = '#dd7777'
    return {
        'plot': { 'linestyle': l, 'color': c, },
        'box': { 'linestyle': l, 'color': c, }
    }

def style_box(bp, k):
    s = get_style(k)['box']
    for key in ('boxes', 'whiskers', 'fliers', 'caps', 'medians'):
        plt.setp(bp[key], **s)
    plt.setp(bp['fliers'], marker='+')

@cached(filename_kws=('filename',))
def read_energy(filename, subsample):
    #if filename.endswith('.gz'):
        #f = gzip.open(filename, 'rt')
    #else:
        #f = open(filename, 'rb')

    print("read_energy({}, {})".format(filename, subsample))
    sys.stdout.flush()

    try:
        if subsample:

            #dt = 'f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,f4,i4'
            dt = ','.join(['f4'] * 65) + ',i4'
            #a = np.genfromtxt(filename, names=True, usecols=list(range(20,84)) + [85], delimiter='\t', dtype=dt)
            a = np.loadtxt(filename, skiprows=1, usecols=[2] + list(range(20,84)) + [85], delimiter='\t', dtype=dt)
            #print(a)
            print("putting into sequential order...")
            sys.stdout.flush()

            r = np.zeros([a.size * 64], dtype=[('avg', np.float32), ('motelabMoteID', np.int32)])
            for i,row in enumerate(a):
                #print("i=", i, len(r[i*64:(i+1)*64]), len(row.tolist()))
                #sys.stdout.flush()

                #print(row.tolist())
                #avg = row.tolist()[0]
                #assert abs(np.average(row.tolist()[1:65]) - avg) < 1.0
                
                r['avg'][i * 64:(i+1) * 64] = row.tolist()[1:65]
                r['motelabMoteID'][i * 64:(i+1) * 64] = row.tolist()[65]

            r['avg'] *= CURRENT_FACTOR

            del a

        else:
            #np.genfromtxt(f, names=True, usecols=(1,2,3,85))
            r = np.loadtxt(filename, skiprows=1, usecols=(2,85), delimiter='\t', dtype=[('avg', np.float32), ('motelabMoteID', np.int32)])
            r['avg'] *= CURRENT_FACTOR
            #print("-------", r, r.dtype)
    except Exception as e:
        print(type(e), e)
        return np.array([], dtype=[('avg', np.float32), ('motelabMoteID', np.int32)])

    print("read_energy({}, {}) done".format(filename, subsample))
    sys.stdout.flush()
    return r

def postprocess_energy(energy, subsample):
    print("splitting by mote id...")
    sys.stdout.flush()

    mote_ids = np.unique(energy['motelabMoteID'])
    energy_maps = {}
    for mote_id in mote_ids:
        mask = (energy['motelabMoteID'] == mote_id)
        energy_maps[mote_id] = (energy[mask])['avg']

        if subsample:
            alpha = .05
            print("applying IIR filter ({}, alpha={})...".format(mote_id, alpha))
            energy_maps[mote_id] = iir(energy_maps[mote_id], alpha=alpha)
            sys.stdout.flush()

    return energy_maps

@cached(
    ignore_kws = ['energy'],
    add_filenames = lambda kws: [kws['area'].filename_energy]
)
def process_energy(energy, area, policy):
    print("process_energy {} {} {} {}".format(area.filename_energy, area.inode_db, area.database, area.mode))

    if area.database == 'tuplestore' and area.mode == 'erase':
        runs_t, runs_e, runs_ot = process_energy_ts_erase(energy, area, policy)

    elif area.database == 'tuplestore':
        runs_t, runs_e, runs_ot = process_energy_ts(energy, area, policy)

    elif area.database == 'teeny':
        runs_t, runs_e, runs_ot = process_energy_teenylime(energy, area, policy)

        if area.mode == 'insert':
            runs_t = [r[:TEENYLIME_INSERT_CALLS] for r in runs_t]
            runs_e = [r[:TEENYLIME_INSERT_CALLS] for r in runs_e]

    else:
        print("process_energy WTF db={} mode={}".format(area.database, area.mode))

    print('/process_energy')
    return runs_t, runs_e, runs_ot


def process_energy_ts_erase(energy, area, policy):
    mi = MEASUREMENT_INTERVAL_SUBSAMPLE if policy.subsample else MEASUREMENT_INTERVAL

    # fixd-width moving average for smoothness,
    # also serves as a band stop filter for timer ticks
    energy = np_moving_average(energy, int(0.0136 / mi))

    debug_figure_energy(energy, area)

    em = ExperimentModel(
            Situation('start', max = 1.2),
            t0 = policy.tmin,
            tmax = policy.tmax,
            delta_t = mi,
    )
    baseline = 0
    for m in em.match(energy):
        baseline = m.v_average
    print("baseline=", baseline)

    em = ExperimentModel(
            Situation('start', max = baseline + .2),
            Repeat(
                Situation('prep', min=baseline + .05, d_min=0.01, d_max=0.1),
                Situation('between', max=baseline + .1, d_min=0.05, d_max=0.2),
                Situation('exp', min=baseline + .05, d_min=0.01, d_max=0.1),
                Situation('after', max=baseline + .1, d_min=0.4, d_max=0.6),
            ),
            t0 = policy.tmin, delta_t = mi,
            tmax = policy.tmax,
        )

    tsums = []
    esums = []
    ots = []
    for m in em.match(energy):
        #print('-- {} {}-{} -> {} - {}'.format(m.situation.name, m.t0, m.t0+m.d, m.v_sum, baseline))
        if m.situation.name == 'exp':
            ots.append(m.t0)
            tsums.append(m.d)
            esums.append(m.v_sum * mi - baseline * m.d)
        elif m.situation.name in ('idle', 'start', 'between', 'after'):
            #print("---- setting baseline: ", m.situation.name, m.v_average)
            baseline = m.v_average

    print('/process_energy_ts_erase')

    return [tsums], [esums], [ots]

def process_energy_teenylime(energy, area, policy):
    mi = MEASUREMENT_INTERVAL_SUBSAMPLE if policy.subsample else MEASUREMENT_INTERVAL
    debug_figure_energy(energy, area)

    em = ExperimentModel(
            Situation('power-on', d_max=.02),
            Situation('start', max=.25, d_max=.05),
            t0 = policy.tmin, delta_t = mi,
            tmax = policy.tmax,
            )

    baseline = 0
    for m in em.match(energy):
        baseline = m.v_average
    print("baseline=", baseline)

    b = baseline
    em = ExperimentModel(
            Situation('power-on', d_max=.02),
            Situation('start', max=0.25, d_max=.05),
            Repeat(
                Situation('between', max = b + .1),
                Situation('exp', min = b + .05),
            ),
            t0 = policy.tmin, delta_t = mi,
            tmax = policy.tmax,
        )

    tsums = []
    esums = []
    ots = []
    for m in em.match(energy):
        print("---- found: {} {}...{}".format(m.situation.name, m.t0, m.t0 + m.d))
        if m.situation.name == 'exp':
            print('-- {} {}-{} -> {} - {}'.format(m.situation.name, m.t0, m.t0+m.d, m.v_sum * mi, m.d*baseline))
            ots.append(m.t0)
            tsums.append(m.d)
            esums.append(m.v_sum * mi - baseline * m.d)
        elif m.situation.name == 'between':
            print("---- correcting baseline: ", m.situation.name, m.v_average)
            baseline = m.v_average
    return [tsums], [esums], [ots]

def process_energy_ts(energy, area, policy):
    mi = MEASUREMENT_INTERVAL_SUBSAMPLE if policy.subsample else MEASUREMENT_INTERVAL
    debug_figure_energy(energy, area)

    if area.mode == 'find':
        em = ExperimentModel(
                Repeat(
                    Situation('high', min=1.4, d_min=.1),
                    Situation('before', max=1.5, d_min=.1),
                    Situation('insert', min=1.4, max=5, d_min=.01, d_max=2.0),
                    Situation('between', max=1.2, d_min=2.0, d_max=10.0),
                    Situation('find', min=1.1, max=5, d_max=2.0),
                    Situation('after_find', max=7.0, d_min=0.1, d_max=10.0),
                ),
                t0 = policy.tmin, delta_t = mi,
            tmax = policy.tmax,
        )
    else:
        em = ExperimentModel(
                Repeat(
                    Situation('high', min=1.4, d_min=.1),
                    Situation('before', max=1.5, d_min=.1),
                    Situation('insert', min=1.4, max=5, d_min=.01, d_max=2.0),
                    Situation('between', max=1.2, d_min=2.0, d_max=20.0),
                ),
                t0 = policy.tmin, delta_t = mi,
            tmax = policy.tmax,
        )

    tsums = []
    esums = []
    ots = []
    baseline = 0

    for m in em.match(energy):
        #print("xxx {}".format(m.situation.name))
        if m.situation.name in ('before', 'between', 'after_find'):
            baseline = m.v_average

        elif m.situation.name == 'insert' and area.mode == 'insert':
            ots.append(m.t0)
            tsums.append(m.d)
            esums.append(m.v_sum * mi - baseline * m.d)

        elif m.situation.name == 'find' and area.mode == 'find':
            #print("find t0={} d={} v_sum={} baseline={}".format(m.t0, m.d, m.v_sum, baseline))
            ots.append(m.t0)
            tsums.append(m.d / FINDS_AT_ONCE)
            esums.append((m.v_sum * mi - baseline * m.d) / FINDS_AT_ONCE)

    return [tsums], [esums], [ots]






def get_policy(area):

    def matches_values(m, area):
        #print('teeny', type(area.experiment), area.database)
        for k, v in m.items():
            if k[0] != '_' and (not hasattr(area, k) or getattr(area, k) != v):
                return False
        return True

    p = Policy()
    for x in policies.policies:
        m = x[0]
        actions  = x[1:]

        if not matches_values(m, area):
            continue

        if '_match' in m and not m['_match'](area):
            continue

        p.matches.append(m)
        cont = False
        for action in actions:
            if action['_action'] == 'let':
                p.__dict__.update(action)
            elif action['_action'] == 'ignore':
                p.process = False
            elif action['_action'] == 'stop':
                return p
            elif action['_action'] == 'continue':
                cont = True
        if not cont:
            return p
    return p

def get_directory_info(d):
    teenylime_runs = set(
        [str(x) for x in range(24857, 24871)]
        + [
            '26392'
        ]
    )

    def read_vars(fn):
        r = {}
        with open(fn, 'r') as f:
            for line in f:
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

    dirinfo = DirInfo()
    dirname = EXP_DIR + d.strip('/')
    dirinfo.filename_energy = dirname + '.csv.gz'
    dirinfo.areas = []

    for gw, db in gateway_to_db.items():
        area = Area()
        fn_gwinfo = dirname + '/' + gw + '/' + gw + '.vars'
        fn_gwout = dirname + '/' + gw + '/output.txt'

        area.filename_gw_out = fn_gwout
        #print(fn_gwinfo)
        if not os.path.exists(fn_gwinfo):
            #print("{} not found, ignoring that area.".format(fn_gwinfo))
            continue

        #print("considering area {} (free ram: {})".format(fn_gwinfo, free_ram()))
        vs = read_vars(fn_gwinfo)
        #print("variables", vs)
        area.experiment = d.strip('/')

        # <HACK>
        if area.experiment in teenylime_runs:
            area.database = 'teeny'
        # </HACK>

        #area.dirinfo = dirinfo
        area.filename_energy = dirinfo.filename_energy
        area.__dict__.update(vs)
        dirinfo.areas.append(area)

    return dirinfo

def process_directory(d):

    dirinfo = get_directory_info(d)

    subsample = any(get_policy(a).subsample for a in dirinfo.areas)
    process = any(get_policy(a).process for a in dirinfo.areas)
    #print("subsample:", subsample)
    #print("process:", process)
    print("policies for {}:".format(d))
    for a in dirinfo.areas:
        print("  {} {}: {}".format(d, a, get_policy(a)))
    if not process:
        print("Not processing {} at all. Reason are the following policies:".format(d))
        for a in dirinfo.areas:
            print("  {}: {}".format(a, get_policy(a)))
        return False

    e = read_energy(filename=dirinfo.filename_energy, subsample=subsample)
    energy_maps = postprocess_energy(e, subsample=subsample)
    del e
    
    # <HACK>
    # 
    # Due to an earlier bug in IIR, subsampled energy values come out
    # with dtype [('avg', float32)] which leads to elements not directly comparable to floats.
    # Fix this here (so cached values still work)
    for k in energy_maps.keys():
        if energy_maps[k].dtype.type is np.void:
            energy_maps[k] = energy_maps[k]['avg']
    #
    # </HACK>


    gc.collect()

    for area in dirinfo.areas:
        policy = get_policy(area)
        cls = ExperimentClass(area)

        if policy.process:
            mid = inode_to_mote_id(area.inode_db)
            if mid not in energy_maps:
                print("warning: no energy present for {} in {} although logs say there should be, skipping.".format(
                    area.inode_db, dirinfo.filename_energy))
                continue
            energy = energy_maps[mid]
            print("before process_energy {} {} {} {}".format(area.filename_energy, area.inode_db, area.database, area.mode))
            runs_t, runs_e, runs_ot = process_energy(energy=energy, area=area, policy=policy)

            #
            # Fix observation times for plausibility
            #
            for i in range(len(runs_ot)):
                rt = runs_t[i]
                re = runs_e[i]
                rot = runs_ot[i]

                if area.database == 'teeny': delta = 1.0
                elif area.database == 'tuplestore' and area.mode == 'erase': delta = .6
                elif area.database == 'tuplestore': delta = 60.0

                runs_t[i], runs_e[i], _ = plausible_observation_times( rt, re, rot, delta)

            #
            # Create experiment object (which will hold our results)
            # and figure out the tuple counts
            #

            exp = add_experiment(ExperimentClass(area))
            if area.database == 'teeny':
                if area.mode == 'insert': tc = [TEENYLIME_INSERT_AT_ONCE] * 50 # range(20)
                elif area.mode == 'find': tc = [TEENYLIME_FINDS_AT_ONCE] * 50 # range(20)
                else: tc = [TEENYLIME_ERASES_AT_ONCE] * 50 # range(20)
                print("teenylime mode {} tuplecounts {}".format(area.mode, tc))
            else:
                if area.mode == 'erase' and area.database == 'tuplestore': tc = [1] * 100
                else:
                    tc = parse_tuple_counts(open(area.filename_gw_out, 'r', encoding='latin1'), d)
                    if not tc:
                        print("  (!) no tuplecounts found in {}, using default".format(fn_gwout))
                        tc = [7, 6, 8, 11, 11, 10, 11, 9]
            exp.set_tuplecounts(tc)

            #
            # Add the extracted measurements to the experiment object
            #

            runs_count = 0
            for j, (ts, es) in enumerate(zip(runs_t, runs_e)):
                runs_count += 1
                #print("  {}:{} adding run {} of {} with {} entries min={} max={} avg={}".format(d, area.inode_db, j, len(runs_e), len(es), np.min(es), np.max(es), np.average(es)))
                print("  {}:{} adding run {} of {} with {} entries".format(d, area.inode_db, j, len(runs_e), len(es)))
                if len(es) > len(exp.tuplecounts):
                    print("  warning: len(tuplecounts)={}".format(len(exp.tuplecounts)))
                #print("  ts={} es={}".format(ts, es))
                for i, (t, e) in enumerate(zip(ts, es)):
                    if t is not None and e is not None: #t != 0 or e != 0:
                        exp.add_measurement(i, t, e)
                    else:
                        print("  skipping i={} t={} e={}".format(i, t, e))
                        #pass
            print("  processed {} experiment runs.".format(runs_count))

    return True


def debug_figure_energy(energy, area):
    if not PLOT_ENERGY: return

    print("debug figure for {}".format(area.inode_db))

    print("  downsampling...")
    e = energy
    factor = 1.0
    #"""
    while e.size > 1000000:
        print("    {} (x{})".format(e.size, factor))
        if e.size % 2 != 0:
            np.append(e, [0.0])
        e = e.reshape(e.size / 2, -1).mean(axis = 1)
        factor *= 2
    #"""
    print("    {} (x{})".format(e.size, factor))
    

    fig = plt.figure(figsize=(12,5))
    ax = plt.subplot(111)
    
    #ax.set_xticks(range(250, 311, 2))
    #ax.set_yticks(frange(0, 3, 0.2))

    #ax.set_xlim((388.06, 388.1))
    ax.set_xlim((0, 50))
    #ax.set_xlim((750/64.0, 760/64.0))
    #ax.set_xlim((400, 500))
    #ax.set_ylim((0, 3.0))
    #ax.set_xlim((1210, 1220))
    #ax.set_ylim((0, 2.5))
    #ax.set_ylim((.5, 2.5))
    ax.grid()

    #e = np.where(energy['motelabMoteID'] == inode_to_mote_id(area.inode_db))
    #mask = (energy['motelabMoteID'] == inode_to_mote_id(area.inode_db))
    #e = np.where(mask, energy)['avg']
    #e = (energy['avg'])[mask]
    #e = energy
    #np.zeros(energy.shape, dtype=bool)

    #e = []
    #for row in energy:
        #if row['motelabMoteID'] == inode_to_mote_id(area.inode_db):
            #e.append(energy['avg'])


    mi = MEASUREMENT_INTERVAL / 64.0
    #print(e['avg'].shape, np.linspace(0.0, e.size * mi, num=e.size).shape )
    ax.plot(np.linspace(0.0, len(e) * mi * factor, num=len(e)), e, 'k-')
    #fig.show()
    fig.savefig('energy_{}.pdf'.format( area.inode_db), bbox_inches='tight', pad_inches=.1)
    plt.close(fig)
    print("debug figure for {} done".format(area.inode_db))

def plot_figures():

    print("plotting figures...")

    fs = (8, 3)

    fig_i_e = plt.figure(figsize=fs)
    ax_i_e = plt.subplot(111)
    fig_i_t = plt.figure(figsize=fs)
    ax_i_t = plt.subplot(111)

    fig_f_e = plt.figure(figsize=fs)
    ax_f_e = plt.subplot(111)
    fig_f_t = plt.figure(figsize=fs)
    ax_f_t = plt.subplot(111)

    fig_e_e = plt.figure(figsize=fs)
    ax_e_e = plt.subplot(111)
    fig_e_t = plt.figure(figsize=fs)
    ax_e_t = plt.subplot(111)

    shift_i = 0
    shift_f = 0
    shift_e = 0

    for k, exp in experiments: #.items():
        print('plotting experiment:', k.__dict__)
        print("k.mode={}".format(k.mode))

        es = [[y * CURRENT_DISPLAY_FACTOR for y in x] for x in exp.energy]
        #es = [x * CURRENT_DISPLAY_FACTOR for x in exp.energy]
        print("len(es)={}".format(len(es)))

        if k.mode == 'find':
            ts = []
            for v in exp.time:
                t_ = []
                for w in v: t_.append(w * TIME_DISPLAY_FACTOR_FIND)
                ts.append(t_)
            #ts = [x * TIME_DISPLAY_FACTOR_FIND for x in exp.time]
        else:
            ts = [[y * TIME_DISPLAY_FACTOR for y in x] for x in exp.time]
            #ts = [x * TIME_DISPLAY_FACTOR for x in exp.time]

        if k.mode == 'insert':
            pos_e = exp.tuplecounts[:len(exp.energy)]
            pos_t = exp.tuplecounts[:len(exp.time)]

            # If for some weird reason we have more data points than
            # sent out packets, put the rest at pos 100 so we notice something
            # is up (but still can see most of the values)
            pos_e += [100] * (len(exp.energy) - len(pos_e))
            pos_t += [100] * (len(exp.time) - len(pos_t))

            pos_e, es = cleanse(pos_e, es)
            pos_t, ts = cleanse(pos_t, ts)

            print(" ntuples: {}".format(k.ntuples))
            pos_e = [x for x in pos_e if x <= k.ntuples]
            pos_t = [x for x in pos_t if x <= k.ntuples]
            es = es[:len(pos_e)]
            ts = ts[:len(pos_t)]

            #w = 2.5 if k.database == 'antelope' else 3.5
            w = 1
            if len(es):
                bp = ax_i_e.boxplot(es, positions=pos_e, widths=w)
                style_box(bp, k)
                ax_i_e.plot(pos_e, [median(x) for x in es], label=mklabel(k), **get_style(k)['plot'])
                #ax_i_e.plot(pos_e, [median(x) for x in es],  label=mklabel(k))

            if len(ts):
                bp = ax_i_t.boxplot(ts, positions=pos_t, widths=w)
                style_box(bp, k)

                #ax_i_t.plot(pos_t, [median(x) for x in ts],style[k.database]['ls'], label=mklabel(k))
                ax_i_t.plot(pos_t, [median(x) for x in ts], label=mklabel(k), **get_style(k)['plot'])

            shift_i -= 1

        elif k.mode == 'find':
            if k.database == 'teeny':
                pos_e = [TEENYLIME_INSERT_AT_ONCE * TEENYLIME_INSERT_CALLS] #* len(exp.energy)
                pos_t = [TEENYLIME_INSERT_AT_ONCE * TEENYLIME_INSERT_CALLS] #* len(exp.time)
                es = [flatten(es)]
                ts = [flatten(ts)]
            else:
                pos_e = exp.tuplecounts[:len(exp.energy)]
                pos_t = exp.tuplecounts[:len(exp.time)]

                # If for some weird reason we have more data points than
                # sent out packets, put the rest at pos 100 so we notice something
                # is up (but still can see most of the values)
                pos_e += [100] * (len(exp.energy) - len(pos_e))
                pos_t += [100] * (len(exp.time) - len(pos_t))

            pos_e, es = cleanse(pos_e, es)
            pos_t, ts = cleanse(pos_t, ts)

            pos_e = [x for x in pos_e if x <= k.ntuples]
            pos_t = [x for x in pos_t if x <= k.ntuples]
            es = es[:len(pos_e)]
            ts = ts[:len(pos_t)]

            #print(k.mode, k.database, [len(x) for x in es])
            #print(pos_e, [sum(x)/len(x) for x in es])

            #w = 3
            w = 1
            if len(es):
                bp = ax_f_e.boxplot(es, positions=pos_e, widths=w)
                style_box(bp, k)
                ax_f_e.plot(pos_e, [median(x) for x in es], label=mklabel(k), **get_style(k)['plot'])

            if len(ts):
                bp = ax_f_t.boxplot(ts, positions=pos_t, widths=w)
                style_box(bp, k)
                ax_f_t.plot(pos_t, [median(x) for x in ts], label=mklabel(k), **get_style(k)['plot'])

            shift_f -= 1

        elif k.mode == 'erase':
            #if k.database == 'antelope': continue
            if k.database == 'teeny':
                k.ntuples = TEENYLIME_INSERT_AT_ONCE * TEENYLIME_INSERT_CALLS

            pos_e = [x for x in exp.tuplecounts[:len(exp.energy)]]
            pos_t = [x for x in exp.tuplecounts[:len(exp.time)]]
            print("pos_e = {}".format(pos_e))

            # If for some weird reason we have more data points than
            # sent out packets, put the rest at pos 100 so we notice something
            # is up (but still can see most of the values)
            pos_e += [100] * (len(exp.energy) - len(pos_e))
            pos_t += [100] * (len(exp.time) - len(pos_t))
            print("pos_e = {}".format(pos_e))

            pos_e, es = cleanse(pos_e, es)
            pos_t, ts = cleanse(pos_t, ts)
            print("pos_e = {}".format(pos_e))

            # <HACK>
            # Some prescilla metadata contains wrong tuple counts
            if k.database == 'tuplestore' and k.ts_dict == 'prescilla':
                k.ntuples = 32
            # </HACK>

            # positions count from 0 upwards, actually what we want is count
            # downwards from k.ntuples, fix that here
            pos_e = [k.ntuples - x for x in pos_e]
            pos_t = [k.ntuples - x for x in pos_t]
            print("pos_e = {}".format(pos_e))

            #print("runs {} {} {}".format(k.mode, mklabel(k), [len(x) for x in es]))
            #print(pos_e, [median(x) for x in es])

            if len(es):
                bp = ax_e_e.boxplot(es, positions=pos_e)
                style_box(bp, k)

                ax_e_e.plot(pos_e, [median(x) for x in es], label=mklabel(k), **get_style(k)['plot'])

            if len(ts):
                bp = ax_e_t.boxplot(ts, positions=pos_t)
                style_box(bp, k)

                ax_e_t.plot(pos_t, [median(x) for x in ts], label=mklabel(k), **get_style(k)['plot'])

            shift_e -= 1
            
        print("runs {} {} {}".format(k.mode, mklabel(k), [len(x) for x in es]))
        #print(k.mode, k.database, k.ts_dict, [len(x) for x in es], 'at', pos_e)

    if PRESCILLA:
        xlim = 35
    else:
        xlim = 75

    ax_i_e.set_xticks(range(0,100,5))
    ax_i_e.set_xlim((0, xlim))
    ax_i_e.tick_params(axis = 'x', which = 'both', bottom = 'off', top = 'off', labelbottom = 'off')

    if PRESCILLA:
        ax_i_e.set_ylim((0, 310))
    else:
        ax_i_e.set_ylim((0, 90))


    #ax_i_e.set_xlabel(r"tuples in store")
    ax_i_e.set_ylabel(r"$\mu J$ / insert")

    ax_i_t.set_xticks(range(0,100,5))
    ax_i_t.set_xlim((0, xlim))
    #ax_i_t.set_ylim((0, 12))
    ax_i_t.set_xlabel(r"\#tuples inserted")
    ax_i_t.set_ylabel(r"ms / insert")

    ax_f_e.set_xticks(range(0,100,5))
    ax_f_e.set_xlim((0, xlim))
    ax_f_e.tick_params(axis = 'x', which = 'both', bottom = 'off', top = 'off', labelbottom = 'off')
    #ax_f_e.set_ylim((0, 25))

    # Prescilla mode
    if PRESCILLA:
        ax_f_e.set_ylim((0, 130))

    #ax_f_e.set_xlabel(r"tuples in store")
    ax_f_e.set_ylabel(r"$\mu J$ / find")

    ax_f_t.set_xticks(range(0,100,5))
    ax_f_t.set_xlim((0, xlim))
    ax_f_t.set_ylim((0, 18))
    ax_f_t.set_xlabel(r"tuples in store")
    ax_f_t.set_ylabel(r"ms / find")
    
    ax_e_e.set_xticks(range(0,100,5))
    ax_e_e.set_xlim((0, xlim))
    #ax_e_e.set_ylim((0, 250))
    #ax_e_e.set_ylim((0, 70))
    ax_e_e.set_xlabel(r"tuples in store")
    ax_e_e.set_ylabel(r"$\mu J$ / erase")

    ax_e_t.set_xticks(range(0,100,5))
    ax_e_t.set_xlim((0, xlim))
    #ax_e_t.set_ylim((0, 50))
    ax_e_t.set_xlabel(r"tuples in store")
    ax_e_t.set_ylabel(r"ms / erase")

    ax_i_e.grid()
    ax_i_t.grid()
    ax_f_e.grid()
    ax_f_t.grid()
    ax_e_e.grid()
    ax_e_t.grid()
    #ax_i_e.legend()
    #ax_i_t.legend()
    #ax_f_e.legend(loc='lower right')
    #ax_f_t.legend()
    #ax_e_e.legend()
    #ax_e_t.legend(loc='lower right')

    fig_i_e.savefig('pdf_out/energies_insert.pdf', bbox_inches='tight', pad_inches=0.1)
    fig_i_t.savefig('pdf_out/times_insert.pdf', bbox_inches='tight',pad_inches=0.1)
    fig_f_e.savefig('pdf_out/energies_find.pdf', bbox_inches='tight',pad_inches=0.1)
    fig_f_t.savefig('pdf_out/times_find.pdf', bbox_inches='tight',pad_inches=0.1)
    fig_e_e.savefig('pdf_out/energies_erase.pdf', bbox_inches='tight',pad_inches=0.1)
    fig_e_t.savefig('pdf_out/times_erase.pdf', bbox_inches='tight',pad_inches=0.1)

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

    >>> cleanse(np.array([1, 2, 3]), np.array([100, 200, 300]))
    ([1, 2, 3], [100, 200, 300])

    >>> cleanse([0, 1, float('nan'), None, []], [[1,2,3], [4,5,6], [7,8,9], [10,11,12], [13,14,15]])
    ([0, 1], [[1, 2, 3], [4, 5, 6]])

    """
    #print(" before cleanse: {}".format((type(l1), type(l2), type(l2[0]))))

    generator = type((x for x in []))

    def is_ok(x):
        return (x is not None and
                (not hasattr(x, '__len__') or len(x) > 0) and
                (type(x) is not float or not math.isnan(x))
                )

    r1 = []
    r2 = []
    for x, y in zip(l1, l2):
        if isinstance(x, list) or isinstance(x, generator) or hasattr(x, '__iter__'):
            x = list(filter(is_ok, x))
        if isinstance(y, list) or isinstance(y, generator) or hasattr(y, '__iter__'):
            y = list(filter(is_ok, y))

        if is_ok(x) and is_ok(y):
            r1.append(x)
            r2.append(y)
    #print(" after cleanse: {}".format((r1, r2)))
    return r1, r2


def inode_to_mote_id(s):
    """
    >>> inode_to_mote_id('inode123')
    10123
    >>> inode_to_mote_id('inode001')
    10001
    """
    return int('10' + s[5:])

def mote_id_to_inode_id(s):
    """
    >>> mote_id_to_inode_id('10123')
    'inode123'
    >>> mote_id_to_inode_id('10001')
    'inode001'
    >>> mote_id_to_inode_id(10001)
    'inode001'
    """
    return 'inode' + str(s)[2:]

def iir(a, alpha):
    return scipy.signal.lfilter(np.array([alpha]), np.array([1.0, alpha-1.0]), a)

#def iir_(a, alpha):
    #b = np.empty([a.size], dtype='f4')
    #b[0] = a[0]
    #for i in range(1, len(a)):
        #b[i] = alpha * a[i] + (1.0 - alpha) * b[i-1]
    #return b
def parse_tuple_counts(f, expid):
    r = []
    for line in f:
        m = re.search(r'sent ([0-9]+) tuples chk', line)
        if m is not None:
            if int(expid.strip('/')) >= 24638:
                # starting from exp 24638 tuple counts are reported
                # incrementally
                r.append(int(m.groups()[0]) - (sum(r) if len(r) else 0))
            else:
                r.append(int(m.groups()[0]))
    print(" found tuplecounts {}".format(r))
    return r

def plausible_observation_times(es, ts, ots, d):
    """
    >>> es = range(10)
    >>> ts = range(10)
    >>> ots = [ 1.0, 2.01, 2.98, 4.7, 5.7, 6.77, 7.89, 9.6, 10.01, 10.99 ]
    >>> e2, t2, ot2 = plausible_observation_times(es, ts, ots, 1.0)
    >>> ot2
    [1.0, 2.01, 2.98, 3.98, 4.7, 5.7, 6.77, 7.89, 8.89, 9.6, 10.99]
    >>> e2
    [0, 1, 2, None, 3, 4, 5, 6, None, 7, 9]
    >>> t2 == e2
    True

    >>> es = ts = (0, 1)
    >>> ots = [ 0.0, 5.0 ]
    >>> es2, ts2, ots2 = plausible_observation_times(es, ts, ots, 1.0)
    >>> es2
    [0, None, None, None, None, 1]
    >>> ts2 == es2
    True
    >>> ots2
    [0.0, 1.0, 2.0, 3.0, 4.0, 5.0]
    """

    dmin = d / 2.0
    dmax = d * 1.5

    es2 = []
    ts2 = []
    ots2 = []

    tprev = None
    for (e, t, ot) in zip(es, ts, ots):
        if tprev is not None and (ot - tprev) < dmin:
            # too close, skip this entry
            pass
        elif tprev is not None and (ot - tprev) > dmax:
            for f in frange(tprev + d, ot - d/2, d):
                # too far, insert a new entry
                ots2.append(f)
                ts2.append(None)
                es2.append(None)
            ots2.append(ot)
            ts2.append(t)
            es2.append(e)
            tprev = ot
        else:
            ots2.append(ot)
            ts2.append(t)
            es2.append(e)
            tprev = ot

    return es2, ts2, ots2

def frange(a, b, step):
    """
    >>> frange(0.1, 4.7, 1.0)
    [0.1, 1.1, 2.1, 3.1, 4.1]
    """
    return [a + x * step for x in range(int(math.ceil((b - a) / step)))]

def median(l):
    if len(l) == 0: return None
    elif len(l) == 1: return l[0]
    #print("{} -> {}".format(l, sorted(l)[int(len(l) / 2)]))
    sl = sorted(l)
    if len(l) % 2:
        return sorted(l)[int(len(l) / 2)]
    return (sl[int(len(l) / 2)] + sl[int(len(l) / 2 - 1)]) / 2.0

def mklabel(k):
    if k.database == 'tuplestore':
        return tex('ts-' + k.ts_container + '-' + k.ts_dict)
    return tex(k.database)

def flatten(l):
    return list(itertools.chain.from_iterable(l))
def tex(s):
    escape = ('_', '/')
    for e in escape:
        s = s.replace(e, '\\' + e)
    return s

def main():
    #a = read_energy(filename='experiments/' + sys.argv[1], subsample=True)
    #print(a)
    for d in sys.argv[1:]:
        r = process_directory(d)
        if UPDATE_PLOT and r:
            plot_figures()
        gc.collect()

    plot_figures()

    print("The end.")

if __name__ == '__main__':
    main()

