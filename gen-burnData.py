# burnData header-file generator from xobjdump output binary file
# output can be used for dynamic loading of binaries via USB interface
#
# refer: http://macgyver.sh.cvut.cz/wiki/vybaveni/demo_boardy/xmos_startkit

import numpy as np

file = open("image_n0c0_2.bin","rb")

num_array = np.fromfile(file, dtype = np.uint32)

print "int burnData[]= {"
for i in num_array:
	print "0x%0.8x," % i
print "};"
