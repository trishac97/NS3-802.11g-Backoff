Author : Trisha Chakraborty
Date: November 12, 2020

This README file includes information on:

(1) produce plots from allTrials.txt (Visit README-NS3.txt to produce this allTrials.txt)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

(1) Run callPlot.m

 This function takes as input:
 -- packet --> takes packet size as input {'P64','P256','P1024'}
 -- xFont --> font for x-axis title
 -- yFont --> font for y-axis title
 -- lFont --> font for legend
 -- gFont --> font for x & y axis scale

 This function calls InputData, excludeOutliners, makePlot, pruneNAN internally and creates plots for following metrices: slotTime, packetTime, missedACKtime, totalACKtime.

 (a) Create Directory named 'P[size]' and keep alltrials-[alg name]-P[size]-D[meter].txt in it. Ex: P64 directory contains all alltrials-BEB-P64-D1.txt, alltrials-LOG-P64-D1.txt,...,alltrials-BEB-P64-D2.txt, alltrials-LOG-P64-D2.txt...

 (b) Line 29 InputData.m, change path to access the correct directory.
 
 Run this command: callPlot('P1024',10,10,10,5) 


Note: All the functionalities for each file is described inside each .m file
