#!/usr/bin/env python2
import graphgen

graphgen.srand(1)

# testing LinearSample
LS = graphgen.LinearSample(20, 100)
for i in LS:
    print i,
print

# testing DSU
sets = graphgen.DSU(100)
last = 0
for i in LS:
    sets.merge(last, i)
    last = i
print " ".join(map(str, [sets.find(i) for i in xrange(100)]))
