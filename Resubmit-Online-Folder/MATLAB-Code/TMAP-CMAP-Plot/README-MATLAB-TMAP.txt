Author : Trisha Chakraborty
Date: November 12, 2020

This README file includes information on:

(1) how produce trasmission map for fixed N = no. of clients

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

(1) importDataLoop.m -- the starter code that would call importFile, MapClientName, ModifyClientName, condensedTmap, transmitMap files internally.

 Run Command: [log,H,RD,T,M,m,CM,total_occurrences,a] = importDataLoop(filename) 
	
 where filename = "..\N[N]\" from NS3 Data folder containing log1.txt ... log30.txt {[N] --> No. of client}

 Go to all matlab files to change N = no. of clients:
	(a) Line 12 in MapClientName.m
	(b) Line 16 in ModifyClientName.m
	(c) Line 16 in TransmitMap.m

Note: Description about functionality of each file is defined inside .m files.