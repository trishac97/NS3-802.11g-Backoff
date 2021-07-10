% Author: Maxwell Young
% Date: June 28, 2021
function plotTimeJava(BEBtime, LBtime, LLBtime, STBtime)

% import the files (which you need to make sure match)
BEBtime = importdata('BEBtime.txt')
BEBcolls = importdata('BEBcolls.txt')

LBtime = importdata('LBtime.txt');
LBcolls = importdata('LBcolls.txt');

LLBtime = importdata('LLBtime.txt');
LLBcolls = importdata('LLBcolls.txt');

STBtime = importdata('STBtime.txt');
STBcolls = importdata('STBcolls.txt');

% remember to set this correctly depending on nunmber of packets
%X = [10000:10000:1000000];  
X = [5:5:150]; 


% plotting BEB 
e1=plot(X,BEBtime');
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
e2=plot(X,LBtime');
e2.LineStyle = ':';
e2.LineWidth = 1.2;
e2.Marker = 'x';
e2.MarkerSize = 5;
e2.MarkerFaceColor = 'b';
e2.MarkerEdgeColor = 'b';
e2.Color = 'blue';

%%%%%%%%%%%%%%%

% plotting LOGLOG 
e3=plot(X,LLBtime');
e3.LineStyle = ':';
e3.Marker = '^';
e3.MarkerSize = 5;
e3.MarkerFaceColor = [0.9,0.5,0];
e3.MarkerEdgeColor = [0.9,0.5,0];
e3.Color = [0.9,0.5,0];
e3.LineWidth = 1.2;

%%%%%%%%%%%%%%%

% Plotting STB
e4=plot(X,STBtime');
e4.LineStyle = ':';
e4.LineWidth = 1.2;
e4.Marker = 'square';
e4.MarkerSize = 5;
e4.MarkerFaceColor = 'g';
e4.MarkerEdgeColor = 'g';
e4.Color = 'g';

%%%%%%%%%%%%%%%%%%
xlim([5,150]);
% Plotting settings
lgd = legend('Location', 'NorthWest');
lgd.FontSize = 20;
legend('BEB','LB','LLB','STB');

xticks([10:10:150])

%ax = ancestor(e1, 'axes')
%ax.XAxis.Exponent = 0
%xtickformat('%.0f')

yTitle = 'Number of Slots';
xTitle = 'Number of Packets';


xlabel(xTitle,'Fontsize',20);
ylabel(yTitle,'Fontsize',20);