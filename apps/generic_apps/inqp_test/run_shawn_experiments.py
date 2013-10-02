#!/usr/bin/env python

import random
import math
import subprocess
import threading
from multiprocessing.pool import Pool
#from multiprocessing.pool import ThreadPool as Pool

BASE_DATA_PATH = '/home/henning/annexe/experiments/2013-10-inqp-shawn'
INPUT_DATA_PATH = BASE_DATA_PATH + '/input_data'
OUTPUT_DATA_PATH = BASE_DATA_PATH + '/output_data'
BINARY='./example_app'
POOL_SIZE=5

def main():
	nseeds = 100
	
	print("== generating seeds...")
	generate_seeds(nseeds)
	
	#print("running const density experiments...")
	#run_constant_density(0.1, range(100, 1000, 100), nseeds)
	
	#print("running const size experiments...")
	#run_constant_size(50, range(100, 1000, 100), nseeds)
	
	print("== running aggregate interval experiments (const density)...")
	run_aggregate_interval_constant_density(0.1, range(100, 1000, 100),
			nseeds, [100, 500] + list(range(1000, 11000, 1000)))

# --- Experiments

def run_constant_density(density, nodes, nseeds):
	for n in nodes:
		sz = round(math.sqrt(n / density))
		run_shawn(sz, n, nseeds)
		
def run_constant_size(sz, nodes, nseeds):
	for n in nodes:
		run_shawn(sz, n, nseeds)

def run_aggregate_interval_constant_density(density, nodes, nseeds, intervals):
	for interval in intervals:
		compile('-DINQP_AGGREGATE_CHECK_INTERVAL=' + str(interval))
		
		pool = Pool(POOL_SIZE)
		for n in nodes:
			sz = round(math.sqrt(n / density))
			pool.apply_async(run_shawn, args=(sz, n, nseeds, '_aggrint{}'.format(interval)))
		pool.close()
		pool.join()

# --- General functions

def generate_seeds(n):
	for i in range(n):
		open(INPUT_DATA_PATH + '/rseed{}'.format(i), 'w').write(str(random.randint(0, 2 ** 32)))
		

def run_shawn(sz, n, nseeds, add_info = ''):
	print("-- size {} count {} {}".format(sz, n, add_info))
	for i in range(nseeds):
		p = subprocess.Popen([BINARY], stdin=subprocess.PIPE,
				stdout=open(OUTPUT_DATA_PATH + '/size{}_count{}_rseed{}{}.log'.format(sz, n, i, add_info), 'w')
				)
		p.stdin.write(generate_shawn_config(sz, n, i).encode('ascii'))
		p.stdin.close()
		p.wait()
		assert p.returncode == 0
		#open('size{}_count{}_rseed{}.log', 'w').write(out)

def compile(flags):
	print("-- flags {}".format(flags))
	subprocess.Popen(['make', 'clean']).wait()
	p = subprocess.Popen(['make', 'shawn', 'ADD_CXXFLAGS=\"' + flags + '\"'])
	stdout, stderr = p.communicate()


def generate_shawn_config(sz, n, seed):
	return """
random_seed action=load filename={IDP}/rseed{seed:d}
prepare_world edge_model=grid comm_model=disk_graph range=4
rect_world width={sz:d} height={sz:d} processors=wiselib_shawn_standalone count={count:d}
simulation max_iterations=1000
	""".format(sz=sz, count=n, seed=seed, IDP=INPUT_DATA_PATH)

if __name__ == '__main__':
	main()

