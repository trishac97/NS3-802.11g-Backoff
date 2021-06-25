README document
June 20, 2021

This code will yield plots for CW slots, total time, missed acks, and ack time. Your data needs to be in the same folder. You can generate the plots for 64 byte payloads using the MATLAB command:

callPlot('P64', 12, 12, 12, 12);

and 

callPlot('P1024', 12, 12, 12, 12);

for 1024 byte packets.

The plots are being saved as .fig files to the folder:

/Users/myoung/Dropbox/Cooper-Max-Backoff/Journal-Version-2020/Journal-DC-Version/Data-NewSlotDuration/Plots-June-2021

and you will need to change this (inside of makePlot.m) in order to save your plots.