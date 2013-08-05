#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt
import json

data = {}

def parse(f):
	global data
	data = json.loads(f.read())

def plot(dataset):
	fig, ax = plt.subplots()
	ax2 = ax.twinx()
	ax2.set_yscale('log')
	#ax2.set_ylim(0.0001, 100000)
	
	#del dataset['hashes']['firstchar']
	#dataset['hashes']['fnv64']
	
	for k in dataset['hashes'].keys():
		if not k.endswith('32'):
			del dataset['hashes'][k]
	
	hash_collisions = sorted(dataset['hashes'].items())
	
	index = np.arange(len(hash_collisions))
	time_real = [c[1]['hash_time_real'] for c in hash_collisions]
	time_sys = [c[1]['hash_time_sys'] for c in hash_collisions]
	time_user = [c[1]['hash_time_sys'] for c in hash_collisions]
	collisions = [c[1]['collisions'] for c in hash_collisions]
	hashes = [c[1]['hashes'] for c in hash_collisions]
	names = [c[0] for c in hash_collisions]
	
	bar_width = 0.25
	
	#b1 = ax.bar(index, time_real, bar_width, color='#a0a0a0', label='computation time')
	b1 = ax.bar(index, time_sys, bar_width, label='sys', color='b')
	b2 = ax.bar(index, time_user, bar_width, bottom=time_sys, label='user', color='g')
	#b3 = ax.bar(index + bar_width, time_real, bar_width, label='real', color='r')
	
	b3 = ax2.bar(index + bar_width, collisions, bar_width, color='r', label='collisions',
			log=True, bottom=1e3)
	plt.xticks(index + 1.0 * bar_width, names)
	
	ax.set_ylabel('computation time (real, s)')
	ax2.set_ylabel('# collisions')
	#plt.legend((b1[0], b2[0], b3[0]), ('sys', 'user', 'collisions'))
	
	plt.show()
			

parse(open('./run_experiments.json', 'r'))


plot(data['__all__'])
#plot(data['data-3.nq'])

