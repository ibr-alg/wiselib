
import numpy as np

import matplotlib.pyplot as plt
from matplotlib import rc

rc('font',**{'family':'serif','serif':['Palatino'], 'size': 10})
rc('text', usetex=True)


def plot():
    fig = plt.figure()
    ax = fig.add_subplot(111)

    for fname in ('incontextsensing', 'ssp', 'btcsample0'):
        r = np.loadtxt(fname + '.rdf.element_counts')
        print(r.shape)
        r = np.cumsum(r)
        ax.plot(
                np.linspace(0.0, 1.0, len(r)),
                r / r[-1],
                label=fname,
                drawstyle='steps-post'
        )

    ax.legend(loc='lower right')
    fig.savefig('element_commonalities.pdf', bbox_inches='tight', pad_inches=0)

plot()

