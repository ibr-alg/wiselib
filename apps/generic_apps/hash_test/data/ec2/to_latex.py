
import json
import re

name_translate = {
		'fnv1': 'FNV1',
		'fnv1a': 'FNV1a',
		'sdbm': 'SDBM',
		'kr': 'Additive (K\&R)',
		'bernstein': 'Bernstein (sum)',
		'bernstein 2': 'Bernstein (xor)',
		'elf': 'ELF',
		'oneatatime': 'Jenkins One-At-A-Time',
		'lookup 2': 'Jenkins Lookup 2',
		'lookup 3': 'Jenkins Lookup 3',
}


d = json.load(open('./run_experiments4.json', 'r'))
d = d['__all__']



for k, v in d['hashes'].items():
	#print(k)
	m = re.match('([a-z0-9]+[^23_])([2-3]?)(_?)(8|16|32|64)', k)
	#print(m.groups())
	
	n = m.groups()[0] + ' ' + m.groups()[1]
	#v['hash_time_user'] /= 3320193.0
	#v['hash_time_sys'] /= 3320193.0
	v['name'] = name_translate.get(n.strip(), n.capitalize())
	#v['bits'] = int(m.groups()[3])

hashes = sorted(d['hashes'].items(), key=lambda x: (x[1]['bits'], x[1]['name']))

print(r'\hline \hline')
print(r'Hash function & Bits & Time (user) & Collisions & \multicolumn{2}{c}{Values / Hash} \\');
print(r' & & & & $\mu$ & $\sigma^2$ \\')
oldbits = 0
for k, v in hashes:
	if v['bits'] != oldbits:
		print(r'\hline')
		oldbits = v['bits']
		
	v['hash_time_us'] = v['hash_time_user'] #+ v['hash_time_sys']
	print(r'{name} & {bits:d} & {hash_time_us:f} & {collisions:d} & {values_per_hash_mean:f} & {values_per_hash_variance:f} \\'.format(**v))
print(r'\hline \hline')

