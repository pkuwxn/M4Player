#!/usr/bin/env python

import os

os.system("premake5 gmake")

content = open("VdkControls.make").read()
pos = content.index("wxUtil.cpp")
pos = content.index("-o", pos)

content = content[:pos] + "`pkg-config --cflags gtk+-2.0` " + content[pos:]

with open("VdkControls.make", "w") as outf:
    outf.write(content)
