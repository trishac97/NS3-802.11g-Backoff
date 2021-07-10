#!/bin/bash


g++ slotParser.cpp -o slotP
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

rm allTrials
touch allTrials

cd N10
./slotP >> ../allTrials
cd ../N20
./slotP >> ../allTrials
cd ../N30
./slotP >> ../allTrials
cd ../N40
./slotP >> ../allTrials
cd ../N50
./slotP >> ../allTrials
cd ../N60
./slotP >> ../allTrials
cd ../N70
./slotP >> ../allTrials
cd ../N80
./slotP >> ../allTrials
cd ../N90
./slotP >> ../allTrials
cd ../N100
./slotP >> ../allTrials
cd ../N110
./slotP >> ../allTrials
cd ../N120
./slotP >> ../allTrials
cd ../N130
./slotP >> ../allTrials
cd ../N140
./slotP >> ../allTrials
cd ../N150
./slotP >> ../allTrials

