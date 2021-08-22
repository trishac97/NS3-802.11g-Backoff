README document
August 20, 2021

This MATLAB code is to create ALO-plot. 

Input folder contains raw data from NS3.25 experiments for
- N=no. of clients=10..150 each 30 trials. log1..log30 for BEB, LOG, LOGLOG, STB. 
- Input --> [log,H,RD,T,M,m,CM,total_occurrences,a,m1,m2,m3,m4]= importDataLoop("C:\Users\Trisha\Desktop\Data-to-Publish\CW-Time-Slot-Time\Slot-20-SIFS-10\")
- Output --> All variables and ALO plot in .fig and .pdf format

To run, type [log,H,RD,T,M,m,CM,total_occurrences,a,m1,m2,m3,m4] = importDataLoop(filename).