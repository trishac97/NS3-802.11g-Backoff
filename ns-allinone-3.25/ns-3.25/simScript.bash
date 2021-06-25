#!/bin/bash
NUMCLIENTS=10
RNGRUN=10
PATHARRAY=(N10 N20 N30 N40 N50 N60 N70 N80 N90 N100 N110 N120 N130 N140 N150)
PATHPOS=0

for loopme in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14
do
	echo $NUMCLIENTS Clients:
    
	for filename in log1.txt log2.txt log3.txt log4.txt log5.txt log6.txt log7.txt log8.txt log9.txt log10.txt log11.txt log12.txt log13.txt log14.txt log15.txt log16.txt log17.txt log18.txt log19.txt log20.txt log21.txt log22.txt log23.txt log24.txt log25.txt log26.txt log27.txt log28.txt log29.txt log30.txt

	do
		echo -- Run $RNGRUN ...

		./waf --run "scratch/nonQos --numSta=$NUMCLIENTS --numClients=$NUMCLIENTS --myRngRun=$RNGRUN" > "$filename" 2>&1

    		cp "$filename" /home/trishachakraborty/ns3/ns-allinone-3.25/ns-3.25/Data/${PATHARRAY[$loopme]}

    	let "RNGRUN++"
     
	done

	let "NUMCLIENTS+=10"
	#RNGRUN=1
	rm *.pcap

done

