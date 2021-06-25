% Author: Maxwell Young
% Date: February 18, 2018
%
% This will read a file output from top-parser and create the 
% matrices (not tables) for: A = slotTime, B= packetTime, C=missedACKtime, D=totalACKtime 
%
% Data is assumed to be in at the same level (not a subfolder) in ../Data-Journal
% I'm assuming that for each algorithm the file name produced by top-parser
% is:
%
% alltrials-[alg name]-P[size]-D[meter].txt
%
% We are going to use this function for all algorithms, all packet sizes,
% and all distances for the journal version. 


function InputData(packet)

format long;

% set the correct distance for filename
Algs = {'BEB', 'LOG', 'LOGLOG', 'STB'}%, 'TSTB'};
Sizes = {packet}; % add this back in once data collected for P256: 'P256'};

for s=1:length(Sizes)

    for n=1:length(Algs)   

        %/Users/myoung/Dropbox/Cooper-Max-Backoff/Journal-Version-2020/Journal-DC-Version/Data-NewSlotDuration/P64
        
        %fname_prefix = strcat('\Users\myoung\Dropbox\Cooper-Max-Backoff\Journal-Version-2020\Journal-DC-Version\Data-NewSlotDuration\', 'allTrials-', Algs{n},'-',Sizes{s},'-D');
        %fname_prefix = strcat('C:\Users\Trisha\Dropbox\Journal-ToN-Version\Data-Journal\',Sizes{s}, '\alltrials-', Algs{n},'-',Sizes{s},'-D');

        %fname = strcat('allTrials-', Algs{n},'-',Sizes{s},'-D2.txt');

        fname = strcat('allTrials-', Algs{n},'-',Sizes{s},'-D2.txt')
        
        %for d=2:1:2   
            % set the correct distance for filename
            dist = int2str(2);
            %mysuffix = strcat(dist,'.txt');
            %fname = strcat('../Data-Journal/P64/alltrials-BEB-P64-D',mysuffix);
            %fname = strcat(fname_prefix, mysuffix);

            fileID = fopen(fname,'r'); % connect to file
            sizeA = [5 Inf]; % specify that we're reading in 5 columns
            A = fscanf(fileID, '%f %f %f %f %f', sizeA); % read in floats
            A = A'; % transpose so each column is configured same as the data file

            % define variable name
            myNames = {genvarname(strcat(Algs{n},'slotTime_', Sizes{s}, '_D',dist)), genvarname(strcat(Algs{n},'packetTime_',Sizes{s},'_D',dist)), genvarname(strcat(Algs{n},'ackMissed_',Sizes{s},'_D',dist)), genvarname(strcat(Algs{n},'ackTime_',Sizes{s},'_D',dist))  };

            assignin('base', myNames{1}, A(:, 1));
            assignin('base', myNames{2}, A(:, 2));
            assignin('base', myNames{3}, A(:, 3));
            assignin('base', myNames{4}, A(:, 4));

        %end
    end
end