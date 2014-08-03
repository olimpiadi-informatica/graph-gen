#!/usr/bin/env python2
import graphgen

graphgen.srand(1)

LS = graphgen.LinearSample(20, 100)
for i in LS:
    print i
