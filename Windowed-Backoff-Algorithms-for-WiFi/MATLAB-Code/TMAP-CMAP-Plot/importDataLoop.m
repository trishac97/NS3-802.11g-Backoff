%Author : Trisha Chakraborty 07/25/2020
%For journal figure total-median-cum-transmission
%If want to generate normal TMAP and CMAP, start from importing the data
%from "Home/Import Data" and go to ModifyClientName.m
%Looping on file name
%Change number of clients n in three files,
%MapClientName.m,ModifyClientName.m,transmitMap.m
%Input --> [log,H,RD,T,M,m,CM,total_occurrences,a] = importDataLoop("C:\Users\Trisha\Desktop\CMAP-DATA-LOGLOG\N140\")
%Output --> All variables
function [log,H,RD,T,M,m,CM,total_occurrences,a] = importDataLoop(filename)
%Looping on file name
i=1;
a = zeros(30, 1); 
%30 trials
while(i<31)
   
    s = int2str(i);
    % Looping through log1..log30
    newfilename = strcat(filename,"log",s,".txt");
    log = importfile(newfilename);
    [H,RD]= ModifyClientName(log);
    [T]= MapClientName(RD,H);
    [M]=trasmitMap(T);
    [m,CM]=condensedTmap(T,M);
    total_occurrences=cellArray(CM);
   
    a(i) = total_occurrences;
    %clearing variable for next round
    clear newfilename;
    clear log;
    clear H;
    clear RD;
    clear T;
    clear M;
    clear m;
    clear CM;
    clear total_occurrences;
    i = i+1;
end
disp(a);
end
