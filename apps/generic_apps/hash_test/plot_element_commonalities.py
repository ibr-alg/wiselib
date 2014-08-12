
import numpy as np

import matplotlib.pyplot as plt
from matplotlib import rc

rc('font',**{'family':'serif','serif':['Palatino'], 'size': 12})
rc('text', usetex=True)
fs = (4, 3)

def get_style(ds):
    l = '-'
    lw = 1

    c = {
            'incontextsensing': '#dd7777',
            'ssp': '#88bbbb',
            '100': '#bbaa88',
            'btcsample0': 'black',
    }.get(ds, 'black')

   # u) not in (20, 40, 100, 200): return
    return {
            'plot': {
                'linestyle': l,
                'linewidth': lw,
                'marker': ' ',
                'color': c,
            }
        }



def plot():
    fig = plt.figure(figsize=fs)
    ax = fig.add_subplot(111)
    ax.set_xlabel(r'rel. \# of distinct elements')
    ax.set_ylabel(r'rel. coverage of data set')

    for fname in ('incontextsensing', 'ssp', 'btcsample0'):
        r = np.loadtxt(fname + '.rdf.element_counts')
        print(r.shape)
        r = np.cumsum(r)
        ax.plot(
                np.linspace(0.0, 1.0, len(r)),
                r / r[-1],
                label=fname,
                drawstyle='steps-post',
                **get_style(fname)['plot']
        )

    #ax.legend(loc='lower right')
    fig.savefig('element_commonalities.pdf', bbox_inches='tight', pad_inches=0.1)

plot()

