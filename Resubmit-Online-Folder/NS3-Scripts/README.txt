Author: Maxwell Young
Date: January 12, 2019

Edited by: Trisha Chakraborty
Date: November 12, 2020

This README file includes information on:

(1) which files need to be modified to execute the algorithms implemented for this manuscript

(2) the commands used to execute an experiment

(3) how to run NS3 experiment - steps

(4) preparing output file from trials for plotting using our MATLAB code


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

(1) Files that are modified are located in

~/ns-allinone-3.25/ns-3.25/src/wifi/model

and are the following:

dca-txop.cc  -- this is modified to log certain metrics we're interested in. By processing the log data produced by this file using  .bash we are able to obtain our data on CW slots, ACKs, total time.

dcf-manager.cc -- this is where the algorithms BEB, LOG, LOGLOG, STB, and TSTB are implemented. Inside the constructor DcfState::DcfState, you'll find 
 
    // *** This variable chooses the type of backoff to be used ***
        // 0 = BE Backoff
        // 1 = Log Backoff
        // 2 = LogLog Backoff
        // 3 = Sawtooth Backoff
        etc.

which allows you to select the contention resolution algorithm that you wish to execute. 

Inside the function DcfState::UpdateFailedCw, you will find the rules for how the contention window is modified for each algorthm.

Add these files at location: ~/ns-allinone-3.25/ns-3.25/scratch

nonQoS.cc -- so titled because RTS/CTS is turned off. Inside you will find the code for how clients are placed, their number (taken from the command line which is called inside simScript.bash described below), the packet size, and other parameters that you may wish to set.

edca-txop-n.cc -- only needed so we can know which DcfStates are for Dca and not Edca; minor modifications for logging.

QoS.cc -- use this file for RTS/CTS is turned on. Use either of the one, QoS.cc or nonQoS.cc.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

(2) The following scripts and processing files are used:

simScript.bash -- executes the algorithm (set in dcf-manager.cc) for a number of packets n=10, 20, ..., 150, and uses 30 trials for each value of n. The resulting log files are written to directory N[n] as log[trial number].txt; the directories N[n] are created by the script.

topParser.cpp -- produces an executable topP that processes the log[trial number].txt file to obtain the total statistics for the execution.

slotParser.cpp -- produces an executable slotP that processes the log[trial number].txt file to obtain the median statistics for the execution.

processData.bash -- copies topP into each N[n] directory, then executes topP to process the log files and concatenate the resulting output to a single file allTrials-[algorithm nam]-P[packet size]-D[distance value].txt in the parent directory.

processMedianData.bash -- same as the above, but for processing the data to obtain median statistics.

resetDirs.bash -- deletes all data in the N[n] directories and recompiles/recopies topP and slotP into each empty directory.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


(3) We provide an example of how to execute an experiment using the directories in my system. Your directory setup may differ slightly, but this should convey the main points.

(a) Select your algorithm from inside dcf-manager.cc, say you use BEB. If you're running TSTB, then remember you will need to modify the actual code to select a coefficient for 'constant_trunc'.


	 firstlog = (log(m_cw)/log(2.0));   
         secondlog = (1+ log(firstlog)/log(2.0)); // add 1 to avoid having a zero
         m_sawtoothCount = std::min( (uint32_t)m_sawtoothRound ,  ((uint32_t)(floor(constant_trunc*secondlog) + 1) )); 

If you want to perform special case TSTB,
	 m_sawtoothCount = std::min( (uint32_t)m_sawtoothRound ,  ((uint32_t)X));
	 Change coefficient X to 100 for TSTB acting STB, change X to 1 for TSTB acting like BEB.

(b) Select your packet size (myClientPacketSize) from line 111 and distance between the clients (myRoomStartX, myRoomStartY, myIncX, myIncY) from line 95 in nonQoS.cc


(c) From within the directory:

/home/young/ns-allinone-3.25/ns-3.25/Data

run the command:

bash resetDirs.bash

which will create new directories N10 to N150 with topP and slotP executables in each one.

(d) From within the directory

/home/young/ns-allinone-3.25/ns-3.25

run the command: 

bash simScript.bash
 
After executing, this produces the log files in the directories N[n] within the subdirectory

/home/young/ns-allinone-3.25/ns-3.25/Data

for n=10 to 150 populated with the log files for each trial. The execution/simulation may take a while, of course.

(e) From within:

/home/young/ns-allinone-3.25/ns-3.25/Data

run the command:

bash processData.bash 

to grab the total statistics. This produces the file allTrials at location (/home/young/ns-allinone-3.25/ns-3.25/Data/).

Similarly, using;

processMedianData.bash.

will produce the the allTrials-...-median file.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

(4) We used MATLAB code to plot allTrials.txt

Rename allTrials.txt to alltrials-[alg name]-P[size]-D[meter].txt
Example: If you're using BEB for 64 Byte packets at distance 2 metres, rename allTrials to alltrials-BEB-P64-D2.txt

Find MATLAB-README.txt to find the instructions for executing our MATLAB code.


