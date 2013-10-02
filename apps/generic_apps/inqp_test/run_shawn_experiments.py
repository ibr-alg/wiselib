#!/usr/bin/env python

import random
import math
import subprocess
import threading
import shutil
from multiprocessing.pool import Pool
#from multiprocessing.pool import ThreadPool as Pool

BASE_DATA_PATH = '/home/henning/annexe/experiments/2013-10-inqp-shawn'
BINARY='example_app'
SOURCE_PATH = '/home/henning/repos/wiselib'

INPUT_DATA_PATH = BASE_DATA_PATH + '/input_data'
OUTPUT_DATA_PATH = BASE_DATA_PATH + '/output_data'
BINARY_PATH = BASE_DATA_PATH + '/binaries'
LOG_PATH = BASE_DATA_PATH + '/logs'
SOURCECODE_PATH = BASE_DATA_PATH + '/src'

POOL_SIZE=4

def main():
	global pool
	pool = Pool(POOL_SIZE)
	
	
	nseeds = 100
	
	print("== generating seeds...")
	generate_seeds(nseeds)
	
	#print("running const density experiments...")
	#run_constant_density(0.1, range(100, 1000, 100), nseeds)
	
	#print("running const size experiments...")
	#run_constant_size(50, range(100, 1000, 100), nseeds)
	
	print("== running aggregate interval experiments (const density)...")
	run_aggregate_interval_constant_density(0.1, range(100, 1000, 100),
			nseeds, [100, 500] + list(range(1000, 4000, 1000)))

	pool.close()
	pool.join()

# --- Experiments

def snapshot_sourcecode():
	excludes = """
	out/ dot/ *.pdf *.log *.o *.orig *.d *.swp *.pyc
	build/ bin/ *.so doxygen_output/ data/ *~
	""".split()
	
	cmd = ['tar', '-cpzf', SOURCECODE_PATH + '/wiselib.tar.gz']
	cmd += ['--exclude=' + x for x in excludes]
	cmd.append(SOURCECODE_PATH)

def run_constant_density(density, nodes, nseeds):
	for n in nodes:
		sz = round(math.sqrt(n / density))
		run_shawn(sz, n, nseeds)
		
def run_constant_size(sz, nodes, nseeds):
	for n in nodes:
		run_shawn(sz, n, nseeds)

def run_aggregate_interval_constant_density(density, nodes, nseeds, intervals):
	for interval in intervals:
		addinfo = '_aggrint{}'.format(interval)
		compile('-DINQP_AGGREGATE_CHECK_INTERVAL=' + str(interval), addinfo)
		#for n in nodes:
		
		n = 500
		sz = round(math.sqrt(n / density))
		run_shawn(sz, n, nseeds, addinfo)
	
# --- General functions

def generate_seeds(n):
	for i in range(n):
		open(INPUT_DATA_PATH + '/rseed{}'.format(i), 'w').write(str(random.randint(0, 2 ** 32)))
		

def run_shawn(sz, n, nseeds, add_info = ''):
	print("-- schedule size {} count {} {}".format(sz, n, add_info))
	for i in range(nseeds):
		pool.apply_async(run_shawn_with_seed, args=(sz, n, i, add_info))
		
		
def run_shawn_with_seed(sz, n, i, add_info):
	print("-- run size {} count {} seed {} {} ".format(sz, n, i, add_info))
	p = subprocess.Popen([BINARY_PATH + '/' + BINARY + add_info], stdin=subprocess.PIPE,
			stdout=open(OUTPUT_DATA_PATH + '/size{}_count{}_rseed{}{}.log'.format(sz, n, i, add_info), 'w')
			)
	p.stdin.write(generate_shawn_config(sz, n, i).encode('ascii'))
	p.stdin.close()
	p.wait()
		#assert p.returncode == 0

def compile(flags, add_info = ''):
	print("-- compile {} flags {}".format(add_info, flags))
	subprocess.Popen(['make', 'clean'],
			stdout=open(LOG_PATH + '/make_clean_' + add_info + '_out.log', 'w'),
			stderr=open(LOG_PATH + '/make_clean_' + add_info + '_err.log', 'w'),
			).wait()
	subprocess.Popen(['make', 'shawn', 'ADD_CXXFLAGS=\"' + flags + '\"'],
			stdout=open(LOG_PATH + '/make_' + add_info + '_out.log', 'w'),
			stderr=open(LOG_PATH + '/make_' + add_info + '_err.log', 'w'),
			).wait()
	shutil.copy(BINARY, BINARY_PATH + '/' + BINARY + add_info)


def generate_shawn_config(sz, n, seed):
	return """
random_seed action=load filename={IDP}/rseed{seed:d}
prepare_world edge_model=grid comm_model=disk_graph range=4
rect_world width={sz:d} height={sz:d} processors=wiselib_shawn_standalone count={count:d}
simulation max_iterations=1000
	""".format(sz=sz, count=n, seed=seed, IDP=INPUT_DATA_PATH)

if __name__ == '__main__':
	main()

