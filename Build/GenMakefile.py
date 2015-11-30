#!/usr/bin/env python

import os

os.system("./premake5 gmake")

files = (
    ("VdkControls.make", "wxUtil.cpp"),
    ("M4Player.make", "OSD.cpp"),
)

for f in files:
    content = open(f[0]).read()
    pos = content.index(f[1])
    pos = content.index("-o", pos)

    content = content[:pos] + "`pkg-config --cflags gtk+-2.0` " + content[pos:]

    with open(f[0], "w") as outf:
        outf.write(content)

content = open("M4Player.make").read()
content = content.replace("$(ALL_LDFLAGS) $(LIBS)", "$(LIBS) $(ALL_LDFLAGS)")

with open("M4Player.make", "w") as outf:
    outf.write(content)

