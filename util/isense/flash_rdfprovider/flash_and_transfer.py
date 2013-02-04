#!/usr/bin/python2

import sys
import re
import transfer_templated_strings as trans
import subprocess
import time

javaprog_path = '/home/henning/repos/wisebed_ibr_svn/scripts/isenseprog-1.0/java-prog'
#java_flags = ['-Djava.library.path=' + javaprog_path]
java_flags = ['-Djava.library.path=/usr/lib']
jar = javaprog_path + '/rsc.apps.flashloader-0.3-SNAPSHOT.one-jar.jar'
javacall = ['java'] + java_flags + ['-jar', jar]

def parse_template_filename(image):
	m = re.match(r'.*sensor_([a-z]+)\.bin', image.lower())
	if m is None: return None
	else: return m.groups()[0] + '.rdf.template'

def read_ipv6(port):
	print "--- reading mac address"
	print ' '.join( javacall + ['readmac', '-device', 'jennic', '-port', port])
	output = subprocess.Popen(javacall + ['readmac', '-device', 'jennic', '-port', port],
			stdout=subprocess.PIPE).communicate()[0]
	m = re.search(r'mac:\s+(0x[0-9a-f]+)\s+', output)
	if m is None: return None
	
	def ipv6groups(n):
		r = '%04x' % ((n >> 6*8) & 0xffff)
		r += ':%04x' % ((n >> 4*8) & 0xffff)
		r += ':%04x' % ((n >> 2*8) & 0xffff)
		r += ':%04x' % ((n >> 0*8) & 0xffff)
		return r
	
	addr = int(m.groups()[0], 16)
	addr ^= 0x0200000000000000
	ipv6 ='fd00:db08:0000:c0a1:' + ipv6groups(addr)
	return ipv6

def flash_node(port, image):
	print "--- flashing node"
	subprocess.Popen(javacall + ['flash', '-device', 'jennic', '-port', port,
		'-file', image]).communicate()
	time.sleep(3)

if __name__ == '__main__':
	
	if len(sys.argv) < 3:
		print 'syntax: $0 [image] [room] (port)'
		sys.exit(1)
	
	image = sys.argv[1] 
	room = sys.argv[2]
	port = sys.argv[3] if len(sys.argv) >= 4 else '/dev/ttyUSB0'
	template_filename = parse_template_filename(image)
	nodename = read_ipv6(port)
	
	print template_filename
	print nodename
	
	flash_node(port, image)
	
	trans.connect_serial(port)
	print "--- sending rdf"
	trans.transfer_templated_rdf(trans.slave, template_filename, nodename, room)
	

