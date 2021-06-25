% Author: Trisha Chakraborty
% Date: February 1, 2020
%
% This function takes as input:
% -- packet --> takes packet size as input {'P64','P256','P1024'}
% -- xFont --> font for x-axis title
% -- yFont --> font for y-axis title
% -- lFont --> font for legend
% -- gFont --> font for x & y axis scale

% function calls InputData, this will read a file output from top-parser and create the 
% matrices (not tables) for: A = slotTime, B= packetTime, C=missedACKtime, D=totalACKtime 
% and calls makePlot function with the arguments and create graph for each
% in one go.
% callPlot('P1024',10,10,10,5) --> Creates all the graph related to
% packet size 1024B



function callPlot(packet,xFont,yFont,lFont,gFont)

Metric = ["slotTime","packetTime","ackMissed","ackTime"];
InputData(packet);
k=2; 
for j = 1:length(Metric)
    %for k = 1:3
        distance = strcat('D',num2str(k));
        makePlot(Metric(j),packet,distance,xFont,yFont,lFont,gFont);
    %end
end
end