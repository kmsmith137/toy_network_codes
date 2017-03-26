#!/usr/bin/env python

import re

first_line = True
ncpus = 0

for line in open('/proc/interrupts'):
    tokens = line.split()

    if first_line:
        ncpus = len(tokens)
        assert tokens == [ ('CPU%d' % i) for i in xrange(ncpus) ]
        first_line = False
        continue

    if tokens[0] in [ 'ERR:', 'MIS:' ]:
        continue

    assert (len(tokens) >= ncpus+2) and (tokens[0][-1] == ':')
    assert all(re.match(r'\d+$',t) for t in tokens[1:(ncpus+1)])

    k = ' '.join([tokens[0]] + tokens[(ncpus+1):])
    print k

    for (i,t) in enumerate(tokens[1:(ncpus+1)]):
        n = int(t)
        if n > 0:
            print '   CPU%d %d' % (i,n)
