#!/usr/bin/env python2

import ConfigParser
import sys

class FakeSecHead(object):
	def __init__(self, fp):
		self.fp = fp
		self.sechead = '[dummy]\n'
		
	def readline(self):
		if self.sechead:
			try: return self.sechead
			finally: self.sechead = None
		else: return self.fp.readline()


def parse_boards(filename, board, tag):
	config = ConfigParser.ConfigParser()
	config.readfp(FakeSecHead(open(filename, 'r')))
	return config.get('dummy', board + '.' + tag)

if __name__ == '__main__':
	print parse_boards(sys.argv[1], sys.argv[2], sys.argv[3])

