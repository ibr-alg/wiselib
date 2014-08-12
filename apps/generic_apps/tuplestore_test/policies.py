
def let(**kws):
    d = { '_action': 'let' }
    d.update(kws)
    return d

def node(e, i=None, **kws):
    d = dict(experiment = str(e))
    if i is not None:
        d['inode_db'] = 'inode{:03d}'.format(i)
    d.update(kws)
    return d
def ignore(): return dict(_action = 'ignore')
def mode(m): return dict(mode = m)
def db(m): return dict(database = m)
def default(): return dict()
def continue_(): return dict(_action = 'continue')

def not_all(**kws):
    return dict(
            _match = lambda a: any(getattr(a, k, None) != v for k, v in kws.items()),
    )

def experiment_interval(a, b, **kws):
    #d = dict(_match = lambda a: a <= int(a.experiment) <= b)
    # TODO XXX
    d = dict(_match = lambda x: a <= int(x.experiment) <= b)
    d.update(kws)
    return d

teenylime_runs = set(
    [str(x) for x in range(24857, 24871)]
    + [
        '26392',
        '26393'
    ]
)

def broken_teenylime_run():
    global teenylime_runs
    #print("XXXX", '26392' not in teenylime_runs)
    return dict(
            _match = lambda a: (a.experiment not in teenylime_runs),
            database = 'teeny'
    )


# blacklist TS experiments before 24883, as they
# had the broken LE mode enabled
#policies += [ (node(i, database='tuplestore'), ignore()) for i in range(24814, 24833) ]

policies = (
        (broken_teenylime_run(), ignore()),

        # From select_exp, plot.py line 515
        (db('antelope'), ignore()),
        (experiment_interval(24814, 24832, database = 'tuplestore'), ignore()),
        (experiment_interval(0, 25084, database = 'tuplestore'), ignore()),
        (experiment_interval(0, 25143, database = 'tuplestore', ts_dict = 'tree'), ignore()),


        (mode('erase'), let(subsample = True), continue_()),
        (db('teeny'), let(subsample = True), continue_()),

        (dict(database='tuplestore', ts_dict='prescilla'), ignore()),
        #(not_all(database='tuplestore', ts_dict='prescilla'), ignore()),


        # manual twearks for erase

        (node(24818,  14), continue_(), let(tmin = 2000 )),
        (node(24819,   8), continue_(), let(tmin = 2010 )),
        (node(24819,  14), continue_(), let(tmin = 240, tmax = 3000 )),
        (node(24862), continue_(), let(tmax = 10.0)),
        (node(24862,   10), ignore()),
        (node(24862,   14), ignore()),
        (node(24862,   16), ignore()),

        (node(24864), continue_(), let(tmax = 20.0)),
        #(node(24864,    8), ignore()),
        (node(24864,   10), ignore()),
        (node(24864,   14), ignore()),
        (node(24864,   16), ignore()),

        (node(24865), continue_(), let(tmax = 20.0)),
        (node(24877,  14), continue_(), let(tmin = 780, tmax = 900 )),
        (node(24877,   8), continue_(), let(tmin = 790, tmax = 900 )),
        (node(24879),      continue_(), let(tmin = 780, tmax = 900)),
        (node(24880,  10), continue_(), let(tmin = 760, tmax = 1000, )),
        (node(24880,   8), continue_(), let(tmin = 786, tmax = 1000, )),
        (node(24885,  14), continue_(), let(tmin = 545, threshold = 2.0, alpha = .04)),
        (node(24885,  16), continue_(), let(tmin = 520, threshold = 1.5, alpha = .04)),
        (node(24886,   8), continue_(), let(tmin = 655, threshold = 2.0, alpha = .04)),
        (node(24887,   8), continue_(), let(tmin = 740, threshold = 2.0, alpha = .04)),
        (node(24887,  14), continue_(), let(tmin = 740, threshold = 2.0, alpha = .04)),
        (node(24887,  10), continue_(), let(tmin = 740, threshold = 2.0, alpha = .04)),
        (node(24990,   8), continue_(), let(tmin = 610, tmax = 665,   threshold = 1.5, alpha = .04 )),
        (node(24990,  10), continue_(), let(tmin = 610, tmax = 658,   threshold = 1.5, alpha = .04 )),
        (node(24990,  14), continue_(), let(tmin = 610, tmax = 662,   threshold = 1.5, alpha = .04 )),
        (node(24990,  16), continue_(), let(tmin = 600, tmax = 652.4, threshold = 1.5, alpha = .04 )),
        (node(24991,   8), continue_(), let(tmin = 590, tmax = 645,   threshold = 1.5, alpha = .04 )),
        (node(24991,  10), continue_(), let(tmin = 590, tmax = 644.2, threshold = 1.6, alpha = .04 )),
        (node(24991,  14), continue_(), let(tmin = 595, tmax = 644,   threshold = 1.75, alpha = .04 )),
        (node(24991,  16), continue_(), let(tmin = 585, tmax = 635.5, threshold = 1.2, alpha = .04 )),
        (node(25064,  10), continue_(), let(tmin = 735)),
        (node(25064,  14), continue_(), let(tmin = 740)),
        (node(25064,   8), continue_(), let(tmin = 725)),
        (node(25066,  10), continue_(), let(tmin = 520)),
        (node(25066,  14), continue_(), let(tmin = 520)),
        (node(25066,   8), continue_(), let(tmin = 520)),
        (node(25103,  10), continue_(), let(tmin = 590)),
        (node(25104,  10), continue_(), let(tmin = 496, tmax = 541.5 , threshold = 1.25 , alpha = 1.0 )),
        (node(25104,  14), continue_(), let(tmin = 495, tmax = 541   , threshold = 1.5  , alpha = 1.0 )),
        (node(25104,   8), continue_(), let(tmin = 490, tmax = 540   , threshold = 1.4  , alpha = 1.0 )),
        (node(25143,   8), continue_(), let(tmin = 435, tmax = 445   , threshold = 1.1  , alpha = 1.0 )),
        (node(25143,  14), continue_(), let(tmin = 435, tmax = 444   , threshold = 1.1  , alpha = 1.0 )),
        (node(25143,  10), continue_(), let(tmin = 440, tmax = 452   , threshold = 1.1  , alpha = 1.0 )),
        (node(25164,   8), continue_(), let(tmin = 465, tmax = 487.5, alpha = 1.0 )),
        (node(25164,  14), continue_(), let(tmin = 465, tmax = 486, alpha = 1.0 )),
        (node(25197,   8), continue_(), let(tmin = 330, tmax = 350, alpha = 1.0 )),
        (node(25197,  14), continue_(), let(tmin = 340, tmax = 360, alpha = 1.0 )),
        (node(25197,  16), continue_(), let(tmin = 320, tmax = 340, alpha = 1.0 )),
        (node(25209,   8), continue_(), let(tmin = 320, alpha = 1.0 )),
        (node(25210,  10), continue_(), let(tmin = 315, alpha = 1.0 )),
        (node(25210,   8), continue_(), let(tmin = 315, alpha = 1.0 )),
        (node(25215,   8), continue_(), let(tmin = 410)),
        (node(25215,  10), continue_(), let(tmin = 410)),
        (node(25215,  20), continue_(), let(tmin = 410)),
        (node(25215,  30), continue_(), let(tmin = 410)),
        (node(26364,   8), continue_(), let(tmin = 700)),
        (node(26388,   8), continue_(), let(tmin = 345)),
        (node(26388,  14), continue_(), let(tmin = 340)),
        (node(26388,  18), continue_(), let(tmin = 352)),
        (node(26388,  20), continue_(), let(tmin = 351)),
        (node(26389,   8), continue_(), let(tmin = 370)),
        (node(26389,  14), continue_(), let(tmin = 360)),
        (node(26374,   8), continue_(), let(tmin = 310)),
        (node(26374,  14), continue_(), let(tmin = 330)),
        (node(26375,   8), continue_(), let(tmin = 485)),
        (node(26375,  18), continue_(), let(tmin = 495)),
        (node(26376,   8), continue_(), let(tmin = 450)),
        (node(26376,  14), continue_(), let(tmin = 460)),
        (node(26376,  18), continue_(), let(tmin = 460)),
        (node(26376,  20), continue_(), let(tmin = 460)),
        (node(26403,   8), continue_(), let(tmin = 755)),
        (node(26403,  14), continue_(), let(tmin = 761)),
        (node(26403,  20), continue_(), let(tmin = 760)),
        (node(26405,   8), continue_(), let(tmin = 750)),
        (node(26405,  14), continue_(), let(tmin = 760)),
        (node(26405,  18), continue_(), let(tmin = 750.5)),
        (node(26415,   8), continue_(), let(tmin = 0)),
        (node(26405,   8), continue_(), let(tmin = 750)),

        (node(26422,   8), continue_(), let(tmin = 380)),
        (node(26422,  14), continue_(), let(tmin = 380)),
        (node(26422,  10), continue_(), let(tmin = 380)),
        (node(26422,  18), continue_(), let(tmin = 380)),
        (node(26424,   8), ignore()),
        (node(26424,  10), ignore()),
        (node(26424,  16), ignore()),
        (node(26424,  18), ignore()),
        (node(26424,  14), continue_(), let(tmin = 420)),
        (node(26425,   8), continue_(), let(tmin = 450)),
        (node(26425,14), continue_(), let(tmin = 455)),
        (node(26425,10), continue_(), let(tmin = 450)),
        (node(26425,18), continue_(), let(tmin = 440)),



        # SUBSAMPLE RUNS

        (node(24877), let(subsample = True), continue_()),
        (node(24879), let(subsample = True), continue_()),
        (node(24880), let(subsample = True), continue_()),
        (node(24882), let(subsample = True), continue_()),
        (node(24885), let(subsample = True), continue_()),
        (node(24886), let(subsample = True), continue_()),
        (node(24887), let(subsample = True), continue_()),
        (node(24990), let(subsample = True), continue_()),
        (node(24991), let(subsample = True), continue_()),
        (node(25064), let(subsample = True), continue_()),
        (node(25066), let(subsample = True), continue_()),
        (node(25103), let(subsample = True), continue_()),
        (node(25104), let(subsample = True), continue_()),
        (node(25143), let(subsample = True), continue_()),
        (node(25144), let(subsample = True), continue_()),
        (node(25145), let(subsample = True), continue_()),
        (node(25164), let(subsample = True), continue_()),
        (node(25197), let(subsample = True), continue_()),
        (node(25209), let(subsample = True), continue_()),
        (node(25210), let(subsample = True), continue_()),
        (node(25215), let(subsample = True), continue_()),
        (node(26355), let(subsample = True), continue_()),
        (node(26361), let(subsample = True), continue_()),
        (node(26364), let(subsample = True), continue_()),
        (node(26388), let(subsample = True), continue_()),
        (node(26389), let(subsample = True), continue_()),
        (node(26374), let(subsample = True), continue_()),
        (node(26375), let(subsample = True), continue_()),
        (node(26376), let(subsample = True), continue_()),
        (node(26403), let(subsample = True), continue_()),
        (node(26405), let(subsample = True), continue_()),
        (node(26408), let(subsample = True), continue_()),

        # Blacklist from plot.py

        (node(24768), ignore()),
        (node(24775), ignore()),
        (node(24776), ignore()),
        (node(24823), ignore()),
        (node(24824), ignore()),
        (node(24824,   8), ignore()),
        (node(24825,  10), ignore()),
        (node(24825,  16), ignore()),
        (node(24826), ignore()),
        (node(24827), ignore()),
        (node(24828), ignore()),
        (node(24829), ignore()),
        (node(24832), ignore()),
        (node(24833), ignore()),
        (node(24834), ignore()),
        (node(24835), ignore()),
        (node(24836), ignore()),
        (node(24838), ignore()),
        (node(24839), ignore()),
        (node(24840), ignore()),
        (node(24841), ignore()),
        (node(24842), ignore()),
        (node(24843), ignore()),
        (node(24844), ignore()),
        (node(24849), ignore()),
        (node(24850), ignore()),
        (node(24851), ignore()),
        (node(24851,  10), ignore()),
        (node(24851,  16), ignore()),
        (node(24854), ignore()),
        (node(24854), ignore()),
        (node(24857), ignore()),
        (node(24858), ignore()),
        (node(24859), ignore()),
        (node(24860), ignore()),
        (node(24861), ignore()),
        (node(24867,  10), ignore()),
        (node(24867,  14), ignore()),
        (node(24867,  16), ignore()),
        (node(24868,  10), ignore()),
        (node(24868,  14), ignore()),
        (node(24868,  16), ignore()),
        (node(24868), let(tmax = 20.0), continue_()),
        (node(24871), ignore()),
        (node(24872), ignore()),
        (node(24873), ignore()),
        (node(24874), ignore()),
        (node(24875), ignore()),
        (node(24877,  10), ignore()),
        (node(24877,  16), ignore()),
        (node(24877,  14), let(tmin = 780, tmax = 900)),
        (node(24877,   8), let(tmin = 790, tmax = 900)),
        (node(24879), let(tmin = 780, tmax=900)),
        (node(24880,  10), let(tmin=760, tmax=1000)),
        (node(24880,   8), let(tmin=786, tmax=1000)),
        (node(24880,  14), ignore()),
        (node(24880,  16), ignore()),
        (node(24882), ignore()),
        (node(24882,  10), ignore()),
        (node(24882,  14), ignore()),
        (node(24883,   8), ignore()),
        (node(24884,   8), ignore()),
        (node(24885,   8), ignore()),
        (node(24885,  10), ignore()),
        #(node(24885,  14), let(tmin=545, threshold = 2.0, alpha=.04)),
        #(node(24885,  16), let(tmin=520, threshold = 1.5, alpha=.04)),
        (node(24886,  10), ignore()),
        (node(24886,  14), ignore()),
        (node(24886,  16), ignore()),
        (node(24887,  16), ignore()),
        (node(24898), ignore()),
        (node(24899), ignore()),
        (node(24900), ignore()),
        (node(24901), ignore()),
        (node(24902), ignore()),
        (node(24903), ignore()),
        (node(24904), ignore()),
        (node(24905), ignore()),
        (node(24906), ignore()),
        (node(24907), ignore()),
        (node(24909,  16), ignore()),
        (node(24911), ignore()),

        (node(24990,  8), let(tmin = 610, tmax = 665,   mode = 'state', threshold = 1.5, alpha = .04)),
        (node(24990, 10), let(tmin = 610, tmax = 658,   mode = 'state', threshold = 1.5, alpha = .04)),
        (node(24990, 14), let(tmin = 610, tmax = 662,   mode = 'state', threshold = 1.5, alpha = .04)),
        (node(24990, 16), let(tmin = 600, tmax = 652.4, mode = 'state', threshold = 1.5, alpha = .04)),

        (node(24991,  8), let(tmin = 590, tmax = 645,   mode = 'state', threshold = 1.5, alpha = .04)),
        (node(24991, 10), let(tmin = 590, tmax = 644.2, mode = 'state', threshold = 1.6, alpha = .04)),
        (node(24991, 14), let(tmin = 595, tmax = 644,   threshold = 1.75, alpha = .04)),
        (node(24991, 16), let(tmin = 585, tmax = 635.5, mode = 'state', threshold = 1.2, alpha = .04)),

        (node(24994), ignore()),
        (node(25012), ignore()),
        (node(25064,  16), ignore()),
        (node(25066,  16), ignore()),
        (node(25085,  16), ignore()),
        (node(25086,   8), ignore()),
        (node(25103,   8), ignore()),
        (node(25103,  10), ignore()),
        (node(25103,  14), ignore()),
        (node(25103,  16), ignore()),
        (node(25104,  16), ignore()),
        (node(25123 ), ignore()),
        (node(25125 ), ignore()),
        (node(25126 ), ignore()),
        (node(25144,   8), ignore()),
        (node(25144,  10), ignore()),
        (node(25144,  14), ignore()),
        (node(25145), ignore()),
        (node(25146), ignore()),
        (node(25193), ignore()),
        (node(25196), ignore()),
        (node(25197,  10), ignore()),
        (node(25209,  10), ignore()),
        (node(25209,  20), ignore()),
        (node(25209,  30), ignore()),
        (node(25210,  20), ignore()),
        (node(25210,  30), ignore()),
        (node(26349,  18), ignore()),
        (node(26351,  18), ignore()),
        (node(26353,  20), ignore()),
        (node(26355), ignore()),
        (node(26357,  18), ignore()),
        (node(26359,  18), ignore()),
        (node(26360), ignore()),
        (node(26361), ignore()),
        (node(26364,  18), ignore()),
        (node(26364,  20), ignore()),
        (node(26365), ignore()),
        (node(26373), ignore()),
        (node(26374,  10), ignore()),
        (node(26375,  10), ignore()),
        (node(26376,  10), ignore()),
        (node(26377), ignore()),
        (node(26388,  10), ignore()),
        (node(26389,  10), ignore()),
        (node(26389,  16), ignore()),
        (node(26389,  18), ignore()),
        (node(26389,  20), ignore()),
        (node(26389,  22), ignore()),
        (node(26389,  30), ignore()),
        (node(26391), ignore()),
        (node(26392,   8), ignore()),
        (node(26392,  10), ignore()),
        (node(26392,  16), ignore()),
        (node(26392,  22), ignore()),
        (node(26392,  24), ignore()),
        (node(26392,  26), ignore()),
        (node(26392,  28), ignore()),
        (node(26392,  14), let(tmax = 15)),
        (node(26392,  18), let(tmax = 15)),
        (node(26392,  20), let(tmax = 15)),
        (node(26393), let(tmax = 20)),
        (node(26393,  10), ignore()),
        (node(26393,  16), ignore()),
        (node(26394), ignore()),
        (node(26395), ignore()),
        (node(26396), ignore()),
        (node(26397), ignore()),
        (node(26398), ignore()),
        (node(26399), ignore()),
        (node(26400), ignore()),
        (node(26401), ignore()),
        (node(26402), ignore()),
        (node(26403,  10), ignore()),
        (node(26403,  18), ignore()),
        (node(26405,  10), ignore()),
        (node(26415), ignore()),

        ({}, let(process = True))
)


