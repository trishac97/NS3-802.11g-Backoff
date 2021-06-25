#!/bin/bash


g++ topParser.cpp -o topP
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

rm allTrials
touch allTrials

cd N10
./topP >> ../allTrials
cd ../N20
./topP >> ../allTrials
cd ../N30
./topP >> ../allTrials
cd ../N40
./topP >> ../allTrials
cd ../N50
./topP >> ../allTrials
cd ../N60
./topP >> ../allTrials
cd ../N70
./topP >> ../allTrials
cd ../N80
./topP >> ../allTrials
cd ../N90
./topP >> ../allTrials
cd ../N100
./topP >> ../allTrials
cd ../N110
./topP >> ../allTrials
cd ../N120
./topP >> ../allTrials
cd ../N130
./topP >> ../allTrials
cd ../N140
./topP >> ../allTrials
cd ../N150
./topP >> ../allTrials

