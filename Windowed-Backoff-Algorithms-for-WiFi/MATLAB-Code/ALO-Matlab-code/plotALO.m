
%Author: Trisha Chakraborty 07/25/2020
%Basic plot code generates manual plotting, from journal, java plot and
%median-cumm-transmission plot is produced using this function.

function plotALO(Metric, Psize, xFont, yFont, lFont, gFont,m1,m2,m3,m4)

% Declare variables
stationChanges = 15; % we go from 400 to 10000 stations in increments of 400
numtrials = 30; % number of trials for each N value


% will store the median value
Med1 = zeros(1,stationChanges); 
Med2 = zeros(1,stationChanges); 
Med3 = zeros(1,stationChanges); 
Med4 = zeros(1,stationChanges);  
%Med5 = zeros(1,stationChanges);  

% will store the standard deviation
Std1 = zeros(1,stationChanges); 
Std2 = zeros(1,stationChanges);  
Std3 = zeros(1,stationChanges);  
Std4 = zeros(1,stationChanges);  
%Std5 = zeros(1,stationChanges);  

% will store error-bar values
E1 = zeros(1,stationChanges);  
E2 = zeros(1,stationChanges);
E3 = zeros(1,stationChanges);
E4 = zeros(1,stationChanges);
%E5 = zeros(1,stationChanges);

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
  
end

%%%%%%%%%%%%%%% Plotting %%%%%%%%%%%%%%%

X = [10:10:150];  

% plotting BEB 
e1=errorbar(X(1:stationChanges),Med1(1:stationChanges),E1(1:stationChanges),'.b','DisplayName','BEB');
xlim([8 152]);
grid on;
hold on;

e1.LineStyle = ':';
e1.LineWidth = 1;
e1.Marker = 'o';
e1.MarkerSize = 3;
e1.MarkerFaceColor = 'r';
e1.MarkerEdgeColor = 'r';
e1.Color = 'red';

%%%%%%%%%%%%%%%

% plotting LOG
e2=errorbar(X(1:stationChanges),Med2(1:stationChanges),E2(1:stationChanges),'.b','DisplayName','LB');

e2.LineStyle = ':';
e2.LineWidth = 1;
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
e3.LineWidth = 1;

%%%%%%%%%%%%%%%

% Plotting STB
e4=errorbar(X(1:stationChanges),Med4(1:stationChanges),E4(1:stationChanges),'.b','DisplayName','STB');

e4.LineStyle = ':';
e4.LineWidth = 1;
e4.Marker = 'square';
e4.MarkerSize = 3;
e4.MarkerFaceColor = 'g';
e4.MarkerEdgeColor = 'g';
e4.Color = 'g';


%%%%%%%%%%%%%%% Plotting Options %%%%%%%%%%%%%%%

lgd = legend('Location', 'NorthWest');
lgd.FontSize = lFont;

set(gca, 'XTick', [10:10:150]);
set(gca,'fontsize',gFont);

xTitle = strcat('Number of Packets (','64 Bytes',')');
xlabel(xTitle,'Fontsize',xFont);

yTitle = 'Median Total Number of ALO Instances';

ylabel(yTitle,'Fontsize',yFont);

%saving the file in pdf
saveas(gcf,'alo-plot.fig');
saveas(gcf,'alo-plot.pdf');

hold off;

end



