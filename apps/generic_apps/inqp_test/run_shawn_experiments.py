#!/usr/bin/env python

import random
import math
import subprocess

DATA_PATH='/home/henning/annexe/experiments/2013-09-inqp-shawn/'
BINARY='../example_app'

def main():
	nseeds = 100
	
	print("generating seeds...")
	generate_seeds(nseeds)
	
	print("running const density experiments...")
	run_constant_density(0.1, range(100, 1000, 100), nseeds)
	
	print("running const size experiments...")
	run_constant_size(50, range(100, 1000, 100), nseeds)
	
def generate_seeds(n):
	for i in range(n):
		open('rseed{}'.format(i), 'w').write(str(random.randint(0, 2 ** 32)))
		
def run_constant_density(density, nodes, nseeds):
	for n in nodes:
		sz = round(math.sqrt(n / density))
		run_shawn(sz, n, nseeds)
		
def run_constant_size(sz, nodes, nseeds):
	for n in nodes:
		run_shawn(sz, n, nseeds)
		
def run_shawn(sz, n, nseeds):
	print("size {} count {}".format(sz, n))
	for i in range(nseeds):
		p = subprocess.Popen([BINARY], stdin=subprocess.PIPE,
				stdout=open('size{}_count{}_rseed{}.log'.format(sz, n, i), 'w')
				)
		p.stdin.write(generate_shawn_config(sz, n, i).encode('ascii'))
		p.stdin.close()
		p.wait()
		#open('size{}_count{}_rseed{}.log', 'w').write(out)
		
def generate_shawn_config(sz, n, seed):
	return """
random_seed action=load filename=rseed{seed:d}
prepare_world edge_model=grid comm_model=disk_graph range=4
rect_world width={sz:d} height={sz:d} processors=wiselib_shawn_standalone count={count:d}
simulation max_iterations=1000
	""".format(sz=sz, count=n, seed=seed)

if __name__ == '__main__':
	main()

