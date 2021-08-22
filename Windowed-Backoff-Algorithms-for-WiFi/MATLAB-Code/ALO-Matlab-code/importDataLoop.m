%Author : Trisha Chakraborty 08/20/2021
%For journal figure ALO-Plots
%Input folder contains raw data from NS3.25 experiments for
%N=no. of clients=10..150 each 30 trials. log1..log30 for BEB, LOG, LOGLOG, STB. 
%Input --> [log,H,RD,T,M,m,CM,total_occurrences,a,m1,m2,m3,m4]= importDataLoop("C:\Users\Trisha\Dropbox\Journal-ToN-Version\Revise-Resubmit-April-2021\Data-to-Publish\CW-Time-Slot-Time\Slot-20-SIFS-10\")
%Output --> All variables and ALO plot in .fig and .pdf format

function [log,H,RD,T,M,m,CM,total_occurrences,a,m1,m2,m3,m4] = importDataLoop(filename)

%n = number of clients
n = 10;
%Algorithms
Algs = {'BEB','LOG','LOGLOG', 'STB'};
%Variables to store Atleast one instance value for each algorithm. m1 =BEB, m2 = LOG, m3 = LOGLOG, m4= STB
m1 = zeros(450,1);
m2 = zeros(450,1);          
m3 = zeros(450,1);
m4 = zeros(450,1);
%Temp var for iterating inside each algorithm. 15*30 = 450 
k=1;
for j=1:length(Algs)   
a = zeros(450, 1);
n = 10;
while(n<151)
i=1;
    while(i<31)
        s = int2str(i);
        newfilename = strcat(filename,Algs(j),"-64B-RAW\N",int2str(n),"\log",s,".txt");
        disp(newfilename);
        %Reads each log[trialnumber].txt file using default function importfile
        log = importfile(newfilename);
        %Modifies hex client name to human readable names
        [H,RD]= ModifyClientName(log,n);
        %Maps remaining hex client names to human readable names
        [T]= MapClientName(RD,H,n);
        %Finds the last transmission time for every trial
        [upperBoundTime] = GetUpperBoundTime(T,RD);
        %Counts the ALO instances.
        [M,m,nr_in_cell,total_occurrences]=countALOInstance(T,RD,upperBoundTime,n);
        %Stores ALO values for each log.txt file
        a(k) = total_occurrences;
        %clearing variable for next round
        clear newfilename;
        clear log;
        clear H;
        clear RD;
        clear T;
        clear M;
        clear m;
        clear CM;
        clear upperBoundTime;
        clear total_occurrences;
        i = i+1;
        k=k+1;
    end
n = n+10;
disp('End of a folder');
end

%%BEB%%
if j==1
    disp('Assigned to m1');
    m1 = a;
    disp(m1);
%%LOG%%
elseif j==2
   disp('Assigned to m2');
    m2 = a;
    disp(m2);
%%LOGLOG%%
elseif j==3
   disp('Assigned to m3');
    m3 = a;
    disp(m3);
%%STB%%
else
    m4 = a;
    disp(m4);
end
k=1;
disp('End of an algo');
end
plotALO('P64', 12, 12, 12, 12, 12,m1,m2,m3,m4);
end
