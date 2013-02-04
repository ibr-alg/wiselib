#!/usr/bin/env python

from optparse import OptionParser
import sys, math, re, random
import cairo

pi = math.pi
VERBOSE=False
YELLOW_HIGHWAYS=False

class visor:
	def __init__(self):
		#Default topology sizes
		self.width = 1024
		self.height = 1024

		self.drawCircle = {0 : self.leaderCircle, 1 : self.portCircle, 2 : self.standardCircle}
		self.edgeColor = {'c': self.setBlueViolet, 'h': self.setAppleGreen, 't': self.setWhite}
		self.edgeWidth = {'c': self.setThin, 'h': self.setThick, 't': self.setMedium}
		self.colorpick = [self.setAirForceBlue, self.setAlizarin, self.setAmber, self.setAppleGreen, self.setArmyGreen, self.setAsparagus, self.setBanana, self.setBlueViolet, self.setBurgundy, self.setBubblegum, self.setByzantine, self.setCamel, self.setCarrotOrange, self.setInchWorm, self.setOlive]

	def surfaceCreate(self, png):
		if png == True:
			self.surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, self.width, self.height)
		else:
			self.surface = cairo.SVGSurface(self.parser.options.outfile, self.width, self.height)
		self.c = cairo.Context (self.surface)
		self.c.scale (self.width, self.height) # Normalizing the canvas
		self.drawBg(0, 0, 0)

	def setThin(self):
		self.c.set_line_width(0.001)

	def setThick(self):
		self.c.set_line_width(0.005)

	def setMedium(self):
		self.c.set_line_width(0.0025)

	def setAirForceBlue(self):
		self.c.set_source_rgb(0.36, 0.54, 0.66)

	def setAlizarin(self):
		self.c.set_source_rgb(0.89, 0.15, 0.21)

	def setAmber(self):
		self.c.set_source_rgb(1.0, 0.75, 0.00)

	def setAppleGreen(self):
		self.c.set_source_rgb(0.55, 0.71, 0.00)

	def setArmyGreen(self):
		self.c.set_source_rgb(0.29, 0.33, 0.13)

	def setAsparagus(self):
		self.c.set_source_rgb(0.53, 0.66, 0.42)

	def setBanana(self):
		self.c.set_source_rgb(1.00, 0.82, 0.16)

	def setBlueViolet(self):
		self.c.set_source_rgb(0.54, 0.17, 0.89)

	def setBurgundy(self):
		self.c.set_source_rgb(0.50, 0.00, 0.13)

	def setBubblegum(self):
		self.c.set_source_rgb(0.99, 0.76, 0.80)

	def setByzantine(self):
		self.c.set_source_rgb(0.74, 0.20, 0.64)

	def setCamel(self):
		self.c.set_source_rgb(0.76, 0.40, 0.62)

	def setCarrotOrange(self):
		self.c.set_source_rgb(0.93, 0.57, 0.13)

	def setInchWorm(self):
		self.c.set_source_rgb(0.70, 0.93, 0.36)

	def setOlive(self):
		self.c.set_source_rgb(0.50, 0.50, 0.00)

	def setCircleWidth(self):
		self.c.set_line_width(0.006)

	def setWhite(self):
		self.c.set_source_rgb(1.00, 1.00, 1.00)

	def drawNodeId(self, x, y, rad, id):
		c = self.c
		c.select_font_face('Georgia', cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_BOLD)
		c.set_font_size(0.02)
		c.move_to(x+2*rad, y)
		c.show_text(str(id))
		c.stroke()

	#Empty circle for leaders (Sepia Color)
	def leaderCircle(self, x, y, rad):
		c = self.c
		c.set_source_rgb(0.44, 0.26, 0.08)
		self.setCircleWidth()
		c.arc(x, y, rad/2.0, 0, 2*pi)
		c.stroke()
		if VERBOSE:
			print 'Leader'

	#Empty circle for ports (Tangerine yellow)
	def portCircle(self, x, y, rad):
		c = self.c
		c.set_source_rgb(1.00, 0.80, 0.00)
		self.setCircleWidth()
		c.arc(x, y, rad/2.0, 0, 2*pi)
		c.stroke()
		if VERBOSE:
			print 'Port'

	#Empty circle for regular ports (White)
	def standardCircle(self, x, y, rad):
		c = self.c
		c.set_source_rgb(1, 1, 1)
		self.setCircleWidth()
		c.arc(x, y, rad/2.0, 0, 2*pi)
		c.stroke()
		if VERBOSE:
			print 'Standard'

	def drawNode(self, x, y, rad, k, col, id, show, bigger):
		if bigger and k == 0:
			rad*=3
		self.drawCircle[k](x, y, rad)
		col()
		if show:
			self.drawNodeId(x, y, rad, id)

		#Filling circle
		c = self.c
		c.set_line_width(0.003)
		c.arc(x, y, rad/1.2, 0, 2*pi)
		c.fill()

	def drawBg(self, r, g, b):
		c = self.c
		c.set_source_rgb(r, g, b)
		c.rectangle (0, 0, self.width, self.height) # Rectangle(x0, y0, x1, y1)
		c.fill ()

	def drawEdge(self, o, t, k):
		c = self.c
		self.edgeColor[k]()
		self.edgeWidth[k]()
		c.move_to(o.x, o.y)
		c.line_to(t.x, t.y)
		c.stroke()

	def testDraw(self):
		#Draw a node
		self.drawNode(0.2, 0.7, .005, 0, 0, 0, 1)

	def testParseAndDraw(self):
		self.parser = wsnParser()
		parser = self.parser
		parser.parse()
		parser.normalize()
		
		if parser.options.yellow:
			self.edgeColor['h'] = self.setBanana

		nodesDict = parser.nodesDict
		edgesDict = parser.edgesDict
		self.surfaceCreate(parser.options.save_to_png)
		
		#After this step draw lines between nodes and parent id in a new iteration
		#First we draw the non highways and then the highways
		for k in edgesDict.keys():
			try:
				e = edgesDict[k]
				origin = nodesDict[e.originId]
				target = nodesDict[e.targetId]
				if e.kind != 'h':
					self.drawEdge(origin, target, e.kind)
			except:
				if VERBOSE:
					print 'One of the edge components is unexistant'

		for k in edgesDict.keys():
			try:
				e = edgesDict[k]
				origin = nodesDict[e.originId]
				target = nodesDict[e.targetId]
				if e.kind == 'h':
					self.drawEdge(origin, target, e.kind)
			except:
				if VERBOSE:
					print 'One of the edge components is unexistant'


		# After drawing the line we draw the nodes themselves
		# Set initial color in a random way (We are tired of the same colors all the time)
		colors = self.colorpick[:]
		if parser.options.random_colors:
			random.shuffle(colors)
		colorAssign = dict()
		for k in nodesDict.keys():
			n = nodesDict[k]
			if n.isLeader:
				k = 0
			elif n.isPort:
				k = 1
			else:
				k = 2
			try:
				self.drawNode(n.x, n.y, 0.005, k, colorAssign[n.sid], n.id, parser.options.show_id, parser.options.bigger_leaders)
			except:
				colorAssign[n.sid] = colors.pop()
				self.drawNode(n.x, n.y, 0.005, k, colorAssign[n.sid], n.id, parser.options.show_id, parser.options.bigger_leaders)
		if parser.options.save_to_png == True:
			self.c.translate (0.1, 0.1) # Changing the current transformation matrix
			self.surface.write_to_png(self.parser.options.outfile) # Output to PNG

class wsnParser:
	def __init__(self):
		usage = 'usage: visor.py [options] inputFile'
		aparser = OptionParser(usage, version="visor 0.9.6")
		aparser.add_option('-s', '--show_id', action='store_true', default=False, dest='show_id', help='displays the nodes with their id.')
		aparser.add_option('-r', '--random_colors', action='store_true', default=False, dest='random_colors', help='displays the cluster picking the colors randomly.')
		aparser.add_option('-b', '--bigger_leaders', action='store_true', default=False, dest='bigger_leaders', help='displays the cluster leaders in a bigger size.')
		aparser.add_option('-y', '--yellow_highway', action='store_true', default=False, dest='yellow', help='changes the default green color of the highways to yellow.')
		aparser.add_option('-o', '--output', default='example.png', dest='outfile', help='defines the name of the output picture.')
		aparser.add_option('-p', '--png', action='store_true', default='false', dest='save_to_png', help='saves the file in png format, if not set in svg.')
		aparser.add_option('-v', '--verbose', action='store_true', default=False, dest='verbose')


		(self.options, args) = aparser.parse_args()
		if len(args) != 1:
			aparser.error('incorrect usage')
			sys.exit(0)
		global VERBOSE
		VERBOSE = self.options.verbose
		self.f = open(args[0], 'r')
		self.nodesDict = dict()
		self.edgesDict = dict()
		self.maxX = 0
		self.maxY = 0

	def printOut(self):
		for line in self.f:
			print line,
	
	def normalize(self):
		nodesDict = self.nodesDict

		for k in nodesDict.keys():
			n = nodesDict[k]
			n.x = n.x / self.maxX
			n.y = n.y / self.maxY

	def parse(self):
		maxX = self.maxX;
		maxY = self.maxY;

		for line in self.f:
			nodePattern = re.compile('^@([0-9]*\\.?[0-9]+)#([0-9]*\\.?[0-9]+)#([0-9]+)#([0-9]+)#(0|1)#(0|1)')
			edgePattern = re.compile('^\\$([0-9]+)->([0-9]+)\\$(c|h|t)')
			nodeModPattern = re.compile('^\\+([0-9]+)#([0-9]+)#(0|1)#(0|1)')
			nodeSearch = nodePattern.search(line)
			#is the line a node declaration 
			if nodeSearch:
				x = float(nodeSearch.group(1))
				y = float(nodeSearch.group(2))
				if x > maxX:
					maxX = x
				if y > maxY:
					maxY = y
				id = nodeSearch.group(3)
				sid = nodeSearch.group(4)
				isLeader = nodeSearch.group(5) == '1'
				isPort = nodeSearch.group(6) == '1'

				if self.nodesDict.has_key(id):
					self.nodesDict[id].sid = sid
					self.nodesDict[id].x = x
					self.nodesDict[id].y = y
				else:
					self.nodesDict[id] = node(x, y, id, sid, isLeader, isPort)
			else :
				edgeSearch = edgePattern.search(line)
				#is the line a edge declaration
				if edgeSearch:
					oid = edgeSearch.group(1)
					tid = edgeSearch.group(2)
					k = edgeSearch.group(3)
					e = edge(oid, tid, k)
					self.edgesDict[e.key()] = e
				else:
					nodeModSearch = nodeModPattern.search(line)
					#is the line a node information update.
					#Remove previous edges on update.
					if nodeModSearch:
						id = nodeModSearch.group(1)
						sid = nodeModSearch.group(2)
						isLeader = nodeModSearch.group(3) == '1'
						isPort = nodeModSearch.group(4) == '1'
						if self.nodesDict.has_key(id):
							oldsid = self.nodesDict[id].sid
							self.nodesDict[id].sid = sid
							self.nodesDict[id].isLeader = isLeader
							self.nodesDict[id].isPort = isPort
							
							if oldsid != sid:
								for k in self.edgesDict.keys():
									e = self.edgesDict[k]
									if e.contains(id):
										del self.edgesDict[k]
						else:
							#This shouldn't happen (redefining a node that didn't previously exist).
							self.nodesDict[id] = node(-1, -1, id, sid, isLeader, isPort)
		if(maxX >= maxY):
			self.maxX = self.maxY = math.ceil(maxX*1.1)
		else:
			self.maxX = self.maxY = math.ceil(maxY*1.1)

class node:
	def __init__(self):
		self.x = 0.0
		self.y = 0.0
		self.id = ''
		self.sid = ''
		self.isLeader = False
		self.isPort = False

	def __init__(self, x, y, i, s, l, p):
		self.x = x
		self.y = y
		self.id = i
		self.sid = s
		self.isLeader = l
		self.isPort = p


class edge:
	def __init__(self):
		self.originId = ''
		self.targetId = ''
		self.kind = ''

	def __init__(self, oid, tid, k):
		self.originId = oid
		self.targetId = tid
		self.kind = k

	def __str__(self):
		return self.originId+'-('+self.kind+')->'+self.targetId

	def key(self):
		return self.originId+'-->'+self.targetId
	
	def contains(self, nid):
		res = False
		if nid == self.originId or nid == self.targetId:
			res = True
		return res


def main():
	v = visor()
	v.testParseAndDraw()

if __name__ == "__main__":
	sys.exit(main())
