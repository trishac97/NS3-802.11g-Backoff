% Author: Maxwell Young
% Date: June 28, 2021
function plotCollisionsJava(BEBcolls,LBcolls,LLBcolls,STBcolls)

% import the files (which you need to make sure match)
BEBtime = importdata('BEBtime.txt');
BEBcolls = importdata('BEBcolls.txt');

LBtime = importdata('LBtime.txt');
LBcolls = importdata('LBcolls.txt');

LLBtime = importdata('LLBtime.txt');
LLBcolls = importdata('LLBcolls.txt');

STBtime = importdata('STBtime.txt');
STBcolls = importdata('STBcolls.txt');

% Remember to set this to the correct length based on number of packets
%X = [10000:10000:1000000];  
X = [10:10:150]; 

% plotting BEB 
e1=plot(X,BEBcolls);
grid on;
hold on;

e1.LineStyle = ':';
e1.LineWidth = 1.2;
e1.Marker = 'o';
e1.MarkerSize = 5;
e1.MarkerFaceColor = 'r';
e1.MarkerEdgeColor = 'r';
e1.Color = 'red';

%%%%%%%%%%%%%%%

% plotting LOG
e2=plot(X,LBcolls');
e2.LineStyle = ':';
e2.LineWidth = 1.2;
e2.Marker = 'x';
e2.MarkerSize = 3;
e2.MarkerFaceColor = 'b';
e2.MarkerEdgeColor = 'b';
e2.Color = 'blue';

%%%%%%%%%%%%%%%

% plotting LOGLOG 
e3=plot(X,LLBcolls');
e3.LineStyle = ':';
e3.Marker = '^';
e3.MarkerSize = 5;
e3.MarkerFaceColor = [0.9,0.5,0];
e3.MarkerEdgeColor = [0.9,0.5,0];
e3.Color = [0.9,0.5,0];
e3.LineWidth = 1.2;

%%%%%%%%%%%%%%%



% Plotting STB
e4=plot(X,STBcolls);
e4.LineStyle = ':';
e4.LineWidth = 1.2;
e4.Marker = 'square';
e4.MarkerSize = 5;
e4.MarkerFaceColor = 'g';
e4.MarkerEdgeColor = 'g';
e4.Color = 'g';

%%%%%%%%%%%%%%%

% Plotting settings
lgd = legend('Location', 'NorthWest');
lgd.FontSize = 20;
legend('BEB','LB','LLB','STB');

yTitle = 'Number of Collisions';
xTitle = 'Number of Packets';


xlabel(xTitle,'Fontsize',20);
ylabel(yTitle,'Fontsize',20);