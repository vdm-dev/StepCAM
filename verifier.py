import sys
import re
from decimal import *

if __name__ == '__main__':
	if len(sys.argv) < 3:
		print "Invalid arguments"
		sys.exit(0)
		
	file1 = open(sys.argv[1], "rt")
	file2 = open(sys.argv[2], "rt")
	
	while 1:
		line1 = file1.readline()
		if line1.startswith("G1 X") or line1.startswith("G0 X"):
			break
		if not line1:
			break
			
	while 1:
		line2 = file2.readline()
		if line2.startswith("G1 X") or line2.startswith("G0 X"):
			break
		if not line2:
			break
			
	maxX = 0
	maxY = 0
			
	while line1 and line2:
		line1 = re.search(r"G\d\s+X([\+\-]?\d*\.?\d*)\s+Y([\+\-]?\d*\.?\d*)", line1)
		line2 = re.search(r"G\d\s+X([\+\-]?\d*\.?\d*)\s+Y([\+\-]?\d*\.?\d*)", line2)
		
		if not line1 or not line2:
			line1 = file1.readline()
			line2 = file2.readline()
			continue
		
		x1 = Decimal(line1.group(1))
		y1 = Decimal(line1.group(2))
		x2 = Decimal(line2.group(1))
		y2 = Decimal(line2.group(2))
		
		dx = abs(x1 - x2)
		dy = abs(y1 - y2)
		
		if dx > maxX:
			maxX = dx
			
		if dy > maxY:
			maxY = dy
		
		print "dX: {}, dY: {}".format(dx, dy)

		line1 = file1.readline()
		line2 = file2.readline()

	print "maxX: {}, maxY: {}".format(maxX, maxY)
	
	file2.close()
	file1.close()

