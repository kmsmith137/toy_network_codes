#!/usr/bin/env python

import os
import sys
import time

# print sys.argv
# print os.environ

fd = int(sys.argv[1])

# To test what happens if L1b is slow to start
# (Answer: L1a's write is nonblocking)
# time.sleep(5)

# raise RuntimeError('xx')

while True:
    maxsize = 10000
    x = os.read(fd, maxsize)
    print 'l1b: read %d bytes' % len(x)
    if len(x) == 0:
        sys.exit(0)
    

