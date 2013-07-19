#!/usr/bin/python2
# -*- coding: utf-8 -*-

import argparse
import sys
import transfer_templated_strings as trans
import subprocess
import re
import os
import time
from parse_rdftemplate import parse_rdftemplate
import threading

javaprog_path = os.path.dirname(sys.argv[0])
#java_flags = ['-Djava.library.path=' + javaprog_path]
java_flags = ['-Djava.library.path=/usr/lib/jni:java.library.path=/usr/lib:java.library.path=/Volumes/HDD/Users/ctemail/Desktop/flash_rdfprovider:' + javaprog_path]
jar = javaprog_path + '/rsc.apps.flashloader-0.3-SNAPSHOT.one-jar.jar'
javacall = ['java'] + java_flags + ['-jar', jar]


def parse_arguments():
	parser = argparse.ArgumentParser(description='Flash RDF provider nodes.')
	parser.add_argument('actions', nargs='+',
			help='actions to execute, might be one or multiple of "flash", "send", "print-rdf" (see source code if this script for a full list)')
	parser.add_argument('--firmware', help='flash firmware image')
	parser.add_argument('--template', help='flash given rdf template. if "auto", infer template name from firmware name',
			default='auto')
	parser.add_argument('--room', help='set nodes room', default='Room1')
	parser.add_argument('--foi', help='add a feature of interest', action='append')
	parser.add_argument('--addr',
			default='auto',
			help='nodes ipv6 address, use \'auto\' for infering from mac addr',
		)
	parser.add_argument('--write-mac',
			default='none',
			help='MAC address to write to the device when using "write-mac"',
		)
	parser.add_argument('--addr-prefix',
			default='fd00:db08:0000:c0a1:',
			help='address prefix used when addr=auto'
		)
	parser.add_argument('--port', default='/dev/ttyUSB0', help='serial port')
	#parser.add_argument('--noencap', default=False, action='store_true', help='if set, do not expect serial port data to be encapsulated in DLE/STX/ETX markers')
	parser.add_argument('--type', default='isense', help='can be isense, arduino')
	parser.add_argument('--baud', default='115200', help='baudrate, for isense usually 115200, for arduino usually 9600')
	
	return parser.parse_args()
	
def read_ipv6_addr(port, prefix):
	"""
	Read ipv6 addr via usb/ftdi
	"""
	#print "--- reading mac address"
	#print ' '.join( javacall + ['readmac', '-device', 'jennic', '-port', port])
	output = subprocess.Popen(javacall + ['readmac', '-device', 'jennic', '-port', port],
			stdout=subprocess.PIPE).communicate()[0]
	m = re.search(r'mac:\s+(0x[0-9a-f]+)\s+', output)
	if m is None:
		print >>sys.stderr, "Could not read mac address, aborting!"
		sys.exit(1)
	
	def ipv6groups(n):
		r = '%04x' % ((n >> 6*8) & 0xffff)
		r += ':%04x' % ((n >> 4*8) & 0xffff)
		r += ':%04x' % ((n >> 2*8) & 0xffff)
		r += ':%04x' % ((n >> 0*8) & 0xffff)
		return r
	
	addr = int(m.groups()[0], 16)
	addr ^= 0x0200000000000000
	ipv6 ='fd00:db08:0000:c0a1:' + ipv6groups(addr)
	print "ipv6 address of node on %s is: %s" % (port, ipv6)
	return ipv6
	
def parse_template_filename(image):
	"""
	get rdf template filename from firmware image filename
	"""
	m = re.match(r'.*sensor_([a-z]+)\.bin', image.lower())
	if m is None: return None
	else: return m.groups()[0] + '.rdf.template'
	
def flash_node(port, image):
	if not image:
		print >>sys.stderr, "Can't flash without --firmware!"
		sys.exit(1)
	
	print "Flashing: %s -> %s" % (image, port)
	call = javacall + ['flash', '-device', 'jennic', '-port', port, '-file', image]
	subprocess.Popen(call).communicate()
	time.sleep(3)
	
def write_mac(port, mac):
	if not mac or mac == 'none':
		print >>sys.stderr, "Can't flash mac without --write-mac!"
		sys.exit(1)
	print "Writing mac: %s -> %s" % (mac, port)
	call = javacall + ['writemac', '-device', 'jennic', '-port', port, '-macAddress', mac]
	subprocess.Popen(call).communicate()
	time.sleep(3)
	
def receive(port, nodetype, baud):
	trans.connect_serial(port, nodetype, baud)
	trans.read_serial_forever()
	
def send(port, rdf, recv, nodetype, baud):
	trans.connect_serial(port, nodetype, baud)
	
	t = threading.Thread(target=trans.transfer_rdf, args=(port, rdf))
	t.start()
	
	if recv:
		trans.read_serial_forever()
		
	t.join()

def test(port, nodetype, baud):
	trans.connect_serial(port, nodetype, baud)
	t = threading.Thread(target=trans.send_loop, args=(port, 'Hello, world!', 1.0, 10))
	t.start()
	trans.read_serial_forever()
	t.join()

def main():
	args = parse_arguments()
	
	addr = args.addr
	addr_prefix = args.addr_prefix
	port = args.port
	firmware = args.firmware
	template = args.template
	nodetype = args.type
	baud = int(args.baud)
	mac = args.write_mac
	
	if addr == 'auto' and ('send' in args.actions or 'send-receive' in args.actions):
		addr = read_ipv6_addr(port, addr_prefix)
		
	if template == 'auto' and firmware:
		template = parse_template_filename(firmware)

	print "------------------"
	print "port:", port, "@", baud
	print "address:", addr
	print "template:", template
	print "firmware:", firmware
	print "nodetype:", nodetype
	print "------------------"
	
	rdf = None
	if template and template != 'auto':
		ns = vars(args)
		ns.update({'addr': addr})
		rdf = parse_rdftemplate(template, ns)

	for action in args.actions:
		if action == 'flash':
			flash_node(port, firmware)
		elif action == 'write-mac':
			write_mac(port, mac)
		elif action == 'send':
			send(port, rdf, False, nodetype, baud)
		elif action == 'send-receive':
			send(port, rdf, True, nodetype, baud)
		elif action == 'print-rdf':
			for elem in rdf:
				print elem
		elif action == 'receive':
			receive(port, nodetype, baud)
		elif action == 'test':
			test(port, nodetype, baud)
			

if __name__ == '__main__':
	main()

# vim: set ts=4 sw=4 tw=78 noexpandtab :
