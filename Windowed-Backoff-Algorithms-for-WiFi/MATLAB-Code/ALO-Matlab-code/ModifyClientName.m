% Input --> 
% -- Create the log20 variable by import data tool in Matlab on log20.txt.
% --This function takes log20 variable as input and produces H and RD matrix which
% --is used in next function (MapClientNames).

% Functionality --> 
% -- truncates log20 from 60,000,000 timestamp.
% -- replace 20 Device name(hex addresses) to 1-20 numerical value.

% Output-->
% --output RD containing the modified client names.

function [H,RD]=ModifyClientName(RawData,n)

[r,c] = size(RawData);
N = n; % put the number of clients here
% find first cell with 60,000,000
j=1;
while(RawData{j,2}~=60000000)
    j=j+1;
end

truncRD = RawData(j:r, 1:c);

% reset the client names
H = truncRD(:,1); % grab the first column with names in it
for(i = 1:1:N)
    H =  strrep(H(:,1), H(i), int2str(i)); 
end

truncRD(:,1) = H;
RD=truncRD;
end