README document
June 20, 2021

This code is unchanged from the code in the folder for plotting the original data under the grid model CW-Time. We are including it again, just to keep things separate.

Since this data is only involving packets with a 64 byte payload, the plots can be generated with:

callPlot('P64', 12, 12, 12, 12);

The plots are being saved as .fig files to the folder:

/Users/myoung/Dropbox/Cooper-Max-Backoff/Journal-Version-2020/Journal-DC-Version/Data-NewSlotDuration/Plots-June-2021

and you will need to change this (inside of makePlot.m) in order to save your plots.