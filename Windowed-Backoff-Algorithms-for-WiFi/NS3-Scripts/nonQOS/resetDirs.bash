#!/bin/bash

rm -rf N*
mkdir N10 N20 N30 N40 N50 N60 N70 N80 N90 N100 N110 N120 N130 N140 N150

g++ topParser.cpp -o topP
g++ slotParser.cpp -o slotP

cp topP N10
cp topP N20
cp topP N30
cp topP N40
cp topP N50
cp topP N60
cp topP N70
cp topP N80
cp topP N90
cp topP N100
cp topP N110
cp topP N120
cp topP N130
cp topP N140
cp topP N150

cp slotP N10
cp slotP N20
cp slotP N30
cp slotP N40
cp slotP N50
cp slotP N60
cp slotP N70
cp slotP N80
cp slotP N90
cp slotP N100
cp slotP N110
cp slotP N120
cp slotP N130
cp slotP N140
cp slotP N150

