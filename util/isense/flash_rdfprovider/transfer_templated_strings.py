#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import serial
import sys
import time
import datetime
import struct
import threading
import os
from parse_rdftemplate import parse_rdftemplate

DLE, STX, ETX = 0x10, 0x02, 0x03
ECHO_REQUEST = 60

#port = '/dev/ttyUSB0'
#baudrate = 115200
baudrate = None

sent_crcs = dict()
rcvd_crcs = dict()

ISENSE, ARDUINO = ('isense', 'arduino')
	
class State:
	IDLE, RECEIVE, ESCAPE = range(3)

class UARTSlave:
	
	def __init__(self, nodetype):
		self.receive_state = State.IDLE
		self.receiving = ''
		self.nodetype = nodetype

	def reset_node(self):
		
		if self.nodetype == ISENSE:
			time.sleep(0.5)
			self.tty.setRTS(True)
			self.tty.setDTR(True)
			
			time.sleep(0.5)
			self.tty.setRTS(False)
			self.tty.setDTR(False)

			time.sleep(0.5)
		
		
	#def send_echo_request(self):
		#print "sending echo request"
		#address = "456"
		#message = "huhu isense"
		
		#packet = struct.pack('!BBBBBQQx', DLE, STX, 10, ECHO_REQUEST, len(message), int(address), 0)
		#packet += message
		#packet += struct.pack('!BB', DLE, ETX)
		#self.tty.write(packet)
	
	def connect(self, port, baudrate):
		self.tty = serial.Serial(port=port, baudrate=baudrate,
					 bytesize=8, parity='N', stopbits=1,
					 timeout=0.01)
		
		if self.nodetype == ISENSE:
			# reboot iSense Node
			time.sleep(0.5)
			self.tty.setRTS(True)
			self.tty.setDTR(True)
			
			time.sleep(0.5)
			self.tty.setRTS(False)
			self.tty.setDTR(False)

			time.sleep(0.5)
		
	def receive_message(self, body, source):
		print "recv message: ", source, body, len(body)
		#for m in body:
		#	print "%s (%d)" % (m, ord(m))
		
		body = body.replace('\x00', '')
		
		textbuffer = self.textview.get_buffer()
		end_iter = textbuffer.get_end_iter()
		mark = textbuffer.create_mark(None, end_iter, False)
		textbuffer.insert_with_tags(end_iter, '[%s] %s: ' % (datetime.datetime.now().strftime('%H:%M:%S'), source), self.tag_source)
		
		end_iter = textbuffer.get_end_iter()
		textbuffer.insert_with_tags(end_iter, '%s\n' % body, self.tag_message)
		
		self.textview.scroll_to_mark(mark, 0, True, 0.0, 1.0)
		textbuffer.delete_mark(mark)
		textbuffer.set_modified(True)
		
		
	def receive_debug(self, body):
		#print "recv debug: ", body
		print body
		
	def receive_fatal(self, body):
		print '\x1b[31m' + body + '\x1b[m'

	def receive_transfer(self, packet):
		if len(packet) < 8:
			print "WRONG Transfer Reply"
		else:
			chunk = int(ord(packet[0])) + (int(ord(packet[1])) << 8) + (int(ord(packet[2])) << 16) + (int(ord(packet[3])) << 24)
			checksum = int(ord(packet[4])) + (int(ord(packet[5])) << 8) + (int(ord(packet[6])) << 16) + (int(ord(packet[7])) << 24)
			print "Transfer reply. Chunk id:", chunk, " Checksum:", checksum
			rcvd_crcs[chunk] = checksum
		
	def receive_packet(self, data):
		if self.nodetype == ISENSE:
			isense_type = ord(data[0])
			
			if isense_type == 105:
				cmd_type = ord(data[1])
				if cmd_type == 60:
					assert len(data) >= 19
					length, destination, source = struct.unpack('!BQQ', data[2:19])
					print "length=", length
					self.receive_message(data[20:], source=source)
					
			elif isense_type == 104:
				if ord(data[1]) == 1:
					self.receive_fatal(data[2:])
				else:
					self.receive_debug(data[2:])
			elif isense_type == 106:
				packet_type = ord(data[1])
				if packet_type == 84:
					self.receive_transfer(data[2:])
				else:
					print "Unknown Packet", packet_type
		else:
			self.receive_debug(data)
		
	def receive_byte(self, c):
		if self.nodetype == ISENSE:
			#print "recv_byte %s (%d)" % (c, ord(c))
			if self.receive_state == State.ESCAPE:
				if ord(c) == ETX:
					self.receive_packet(self.receiving)
					self.receive_state = State.IDLE
					
				elif ord(c) == STX:
					self.receiving = ''
					self.receive_state = State.RECEIVE
					
				else:
					self.receiving += c
					
			elif ord(c) == DLE:
				self.receive_state = State.ESCAPE
				
			elif self.receive_state == State.RECEIVE:
				self.receiving += c
				
		else:
			self.receiving += c # hex(ord(c))
			if c == '\n':
				self.receive_packet(self.receiving.strip())
				self.receiving = ''

def w(x): slave.tty.write(chr(x))
def send_string(s, pkttype=0x0a):
	w(DLE); w(STX); w(pkttype)
	slave.tty.write(s);
	#w(0x00)
	w(DLE); w(ETX);

def transfer_rdf(port, rdf): #@nodename, room):
	global slave
	#connect_serial(port)
	
	
	print "erasing..."
	# erase
	w(DLE); w(STX); w(0x0c); w(DLE); w(ETX)
	time.sleep(15)
	
	for line in rdf:
		print line
		send_string(line, pkttype=0x0b)
		time.sleep(.2)
	
def send_loop(port, s, interval, n):
	global slave
	print("send_loop()")

	for i in range(n):
		print("now sleeping for ", interval)
		time.sleep(interval)
		print "sending '" + s + "'..."
		send_string(s + '\0')

	print("send_loop() done.")

def connect_serial(port, nodetype, baud):
	global slave
	global baudrate
	baudrate = baud
	slave = UARTSlave(nodetype)
	slave.connect(port, baudrate)
	
def read_serial_forever():
	#print "reading from ", port, " with ", baudrate
	
	while True:
		char = slave.tty.read()
		if (char):
			slave.receive_byte(char)

if __name__ == '__main__':
	
	port = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyUSB0"
	
	filename = sys.argv[2]
	nodename = sys.argv[3]
	room = sys.argv[4]
	
	connect_serial(port)
	
	print "[transfer_isense] connected to ", port, " with ", baudrate
	
	t = threading.Thread(target=transfer_templated_rdf, args=(slave, filename,
		nodename, room))
	t.start()
	
	read_serial_forever()
	
