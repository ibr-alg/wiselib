#!/usr/bin/env python3

import re

re_time_sN = re.compile(r'^T([0-9]+)\.([0-9]+)\|(.*)$')

def reltime(f):
	def extract_time_sN(line):
		m = re.match(re_time_sN, line)
		if m is None:
			return None, line
		else:
			s, N, line = m.groups()
			return float(s) + 10**-9 * float(N), line

	t0 = None

	for line in f:
		t_, line = extract_time_sN(line)
		if t_ is not None: t = float(t_)
		if t0 is None: t0 = t

		print("T{:.3f}|{}".format(t - t0, line))


reltime(open('log.txt', 'r'))


