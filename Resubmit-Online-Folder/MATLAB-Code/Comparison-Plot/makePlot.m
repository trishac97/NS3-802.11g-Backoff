% Author: Maxwell Young & Trisha Chakraborty
% Created Date: February 18, 2018
% Modified Date: January 20, 2020
% This function takes as input:
%
% -- a string for the metric of interest {'slotTime','packetTime','ackMissed','ackTime'}
% -- a string for packet size {'P64','P256','P1024'}
% -- a string for distance {'D1', 'D2', 'D3'}
%
% and returns a plot of all algorithms along this metric for this distance

function makePlot(Metric, Psize, Dist, xFont, yFont, lFont, gFont)


% Figure out the matrix names
m1 = strcat('BEB',Metric,'_',Psize,'_',Dist);
m2 = strcat('LOG',Metric,'_',Psize,'_',Dist);
m3 = strcat('LOGLOG',Metric,'_',Psize,'_',Dist);
m4 = strcat('STB',Metric,'_',Psize,'_',Dist);
m5 = strcat('TSTB',Metric,'_',Psize,'_',Dist);

% we are using evalin to match the matrix in our workspace to the
% variable m_i
m1 = evalin('base', m1);
m2 = evalin('base', m2);
m3 = evalin('base', m3);
m4 = evalin('base', m4);
m5 = evalin('base', m5);

% Declare variables
stationChanges = 15; % we go from 10 to 150 stations in increments of 10
numtrials = 30; % number of trials for each N value

% will store the median value
Med1 = zeros(1,stationChanges); 
Med2 = zeros(1,stationChanges); 
Med3 = zeros(1,stationChanges); 
Med4 = zeros(1,stationChanges);  
Med5 = zeros(1,stationChanges);  

% will store the standard deviation
Std1 = zeros(1,stationChanges); 
Std2 = zeros(1,stationChanges);  
Std3 = zeros(1,stationChanges);  
Std4 = zeros(1,stationChanges);  
Std5 = zeros(1,stationChanges);  

% will store error-bar values
E1 = zeros(1,stationChanges);  
E2 = zeros(1,stationChanges);
E3 = zeros(1,stationChanges);
E4 = zeros(1,stationChanges);
E5 = zeros(1,stationChanges);

for i=1:1:stationChanges
   
   % BEB 
   Nincrement = m1( (((i-1)*numtrials)+1):(i*numtrials) ); % get next bunch of 30 trials 
   prunedNincrement = pruneNaN(excludeOutliers(Nincrement)); % prune outliers
   Med1(i) = median(prunedNincrement); % get median 
   disp(Med1(i));
   Std1(i) = std(prunedNincrement); % get standard deviation
   E1(i) = 1.96*Std1(i)/sqrt(numtrials); 
   
   % LOG  
   Nincrement = m2( (((i-1)*numtrials)+1):(i*numtrials) ); % get next bunch of 30 trials 
   prunedNincrement = pruneNaN(excludeOutliers(Nincrement)); % prune outliers
   Med2(i) = median(prunedNincrement); % get median 
   Std2(i) = std(prunedNincrement); % get standard deviation
   E2(i) = 1.96*Std2(i)/sqrt(numtrials);
   
   % LOGLOG 
   Nincrement = m3( (((i-1)*numtrials)+1):(i*numtrials) ); % get next bunch of 30 trials 
   prunedNincrement = pruneNaN(excludeOutliers(Nincrement)); % prune outliers
   Med3(i) = median(prunedNincrement); % get median 
   Std3(i) = std(prunedNincrement); % get standard deviation
   E3(i) = 1.96*Std3(i)/sqrt(numtrials);
   
   % STB  
   Nincrement = m4( (((i-1)*numtrials)+1):(i*numtrials) ); % get next bunch of 30 trials 
   prunedNincrement = pruneNaN(excludeOutliers(Nincrement)); % prune outliers
   Med4(i) = median(prunedNincrement); % get median 
   Std4(i) = std(prunedNincrement); % get standard deviation
   E4(i) = 1.96*Std4(i)/sqrt(numtrials);
   
   % TSTB  
   Nincrement = m5( (((i-1)*numtrials)+1):(i*numtrials) ); % get next bunch of 30 trials 
   prunedNincrement = pruneNaN(excludeOutliers(Nincrement)); % prune outliers
   Med5(i) = median(prunedNincrement); % get median 
   Std5(i) = std(prunedNincrement); % get standard deviation
   E5(i) = 1.96*Std5(i)/sqrt(numtrials);
   
  
end

%%%%%%%%%%%%%%% Plotting %%%%%%%%%%%%%%%

X = [10:10:150];  

% plotting BEB 
e1=errorbar(X(1:stationChanges),Med1(1:stationChanges),E1(1:stationChanges),'.b','DisplayName','BEB');
xlim([8 152]);
grid on;
hold on;

e1.LineStyle = ':';
e1.LineWidth = 1.2;
e1.Marker = 'o';
e1.MarkerSize = 3;
e1.MarkerFaceColor = 'r';
e1.MarkerEdgeColor = 'r';
e1.Color = 'red';

%%%%%%%%%%%%%%%

% plotting LOG
e2=errorbar(X(1:stationChanges),Med2(1:stationChanges),E2(1:stationChanges),'.b','DisplayName','LB');

e2.LineStyle = ':';
e2.LineWidth = 1.2;
e2.Marker = 'x';
e2.MarkerSize = 3;
e2.MarkerFaceColor = 'b';
e2.MarkerEdgeColor = 'b';
e2.Color = 'blue';

%%%%%%%%%%%%%%%

% plotting LOGLOG 
e3=errorbar(X(1:stationChanges),Med3(1:stationChanges),E3(1:stationChanges),'.b','DisplayName','LLB');

e3.LineStyle = ':';
e3.Marker = '^';
e3.MarkerSize = 3;
e3.MarkerFaceColor = [0.9,0.5,0];
e3.MarkerEdgeColor = [0.9,0.5,0];
e3.Color = [0.9,0.5,0];
e3.LineWidth = 1.2;

%%%%%%%%%%%%%%%

% Plotting STB
e4=errorbar(X(1:stationChanges),Med4(1:stationChanges),E4(1:stationChanges),'.b','DisplayName','STB');

e4.LineStyle = ':';
e4.LineWidth = 1.2;
e4.Marker = 'square';
e4.MarkerSize = 3;
e4.MarkerFaceColor = 'g';
e4.MarkerEdgeColor = 'g';
e4.Color = 'g';


%%%%%%%%%%%%%%%

% Plotting TSTB
e4=errorbar(X(1:stationChanges),Med5(1:stationChanges),E5(1:stationChanges),'.b','DisplayName','TSTB');

e4.LineStyle = ':';
e4.LineWidth = 1.2;
e4.Marker = 'p';
e4.MarkerSize = 3;
e4.MarkerFaceColor = 'k';
e4.MarkerEdgeColor = 'k';
e4.Color = 'k';

%%%%%%%%%%%%%%% Plotting Options %%%%%%%%%%%%%%%

lgd = legend('Location', 'NorthWest');
lgd.FontSize = lFont;

set(gca, 'XTick', [10:10:150]);
set(gca,'fontsize',gFont);

% automate the x-axis title
if(strcmp(Psize,'P64'))
    xTitle = strcat('Number of Packets (','64 Bytes',')');
elseif(strcmp(Psize,'P256'))
    xTitle = strcat('Number of Packets (','256 Bytes',')');
elseif(strcmp(Psize,'P1024'))
    xTitle = strcat('Number of Packets (','1024 Bytes',')');
end
xlabel(xTitle,'Fontsize',xFont);

% automate the y-axis title
if(strcmp(Metric,'slotTime'))
    %yTitle = 'Contention-Window Slots';
    yTitle = 'Contention-Window Slots';

elseif(strcmp(Metric,'packetTime'))
    %yTitle = 'Total Time ';
    yTitle = 'Total Time';
elseif(strcmp(Metric,'ackMissed'))
    %yTitle = 'Max Number of ACK Timeouts';
     yTitle = 'Max Number of ACK Timeouts';
else
    %yTitle = 'Max Time for ACK Timeouts';
    yTitle = 'Max Time for ACK Timeouts';
end

ylabel(yTitle,'Fontsize',yFont);

%saving the file in pdf
saveas(gcf,strcat('C:\Users\Trisha\Desktop\', Psize,'-',Metric,'-',Dist), 'fig');


hold off;

end



