#!/bin/bash
# usage: makelibraries cfg
# where cfg is one of Debug, DebugDynamic, Release, ReleaseDynamic

make CFG=$1 -f makegraphics.gte
make CFG=$1 -f makemathematicsgpu.gte
make CFG=$1 -f makeapplications.gte

