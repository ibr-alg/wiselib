#!/usr/bin/env python3

# use:
# cat 25968/*/output.txt|grep --color=never MBF|sort|cut -c 27- |./mktree.py > tree.dot
# input lines should thus look like:
# 56188 p0,48389 d255,4

import sys
import re

r = re.compile(r'(\d+) p\d+,(\d+) d\d+,\d+')

parents = {}
for line in sys.stdin.readlines():
    m = re.match(r, line)
    if m is not None:
        parents[m.groups()[0]] = m.groups()[1]

print("""
digraph G {
node [shape = none];
55778 [label="   1"];
49185 [label="   4"];
55323 [label="   5"];
53440 [label="   7"];
55546 [label="   9"];
56648 [label="  10"];
49765 [label="  11"];
48389 [label="  12"];
49279 [label="  13"];
49272 [label="  14"];
56188 [label="  15"];
49900 [label="  17"];
56265 [label="  18"];
50053 [label="  19"];
50502 [label="  23"];
49292 [label="  24"];
49268 [label="  25"];
51243 [label="  27"];
56345 [label="  28"];
53851 [label="  29"];
48445 [label="  30"];
55054 [label="  31"];
48570 [label="  33"];
49898 [label="  35"];
55421 [label="  36"];
56038 [label="  37"];
54286 [label="  38"];
52154 [label="  39"];
54493 [label="  40"];
56375 [label="  41"];
56458 [label="  42"];
54171 [label="  43"];
56445 [label="  44"];
52434 [label="  46"];
49819 [label="  47"];
52174 [label="  50"];
51765 [label="  51"];
48994 [label="  52"];
51419 [label="  54"];
52793 [label="  55"];
53431 [label="  56"];
47742 [label=" 199"];
50223 [label=" 200"];
""")
for k, v in parents.items():
    print("{} -> {}".format(k, v))
print("}")



