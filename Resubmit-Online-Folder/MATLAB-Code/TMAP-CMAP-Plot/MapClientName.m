% Input --> Function takes RD as input

% Functionality -->
% -- Maps the hex addresses which are associated with 1-20 clients.
% -- Change client names from string to integer type.

% Output -->
% -- T matrix containing the changed names which will be an input to next
% function transmitMap
function [T]= MapClientName(RawData,H)
[r,c] = size(RawData);
N = 10; % put the number of clients here

% k goes from 1-20 corresponding to device name inside the loop
k=1;

% Ex: Hex address for 21,22 row gets mapped to device 1.
for(i = N+1:2:30)
pattern = H(i:i+1,1:1);
H =  strrep(H(:,1), pattern(1), int2str(k));
H =  strrep(H(:,1), pattern(2), int2str(k));
k = k+1;
end

% Change client names from string to integer type.
for(i=1:1:length(H))
     if(length(H{i,1})<3)
       H{i,1} = str2num(H{i,1}); 
     end
end

RawData(:,1) = H;
T=RawData;
end