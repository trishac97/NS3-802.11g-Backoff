% Input -->
% -- Takes T matrix as input.

% Functionality -->
% -- Creates a plot for device 1-20 corresponding to transmission time and
% ack timeout.
% -- BLUE lines: Start of transmission  
% -- RED: Ack timeout
% -- Matrix M contains colorcode values

% Output -->
% -- Produces Tranmission Map
function [M,m,nr_in_cell,total_occurrences]=countALOInstance(T,RD,upperBoundTime,n)

[r,c]=size(T);
N=n;
gran = 1;% each step is worth gran mu secs
startTime = 60000000/gran;

%GetUpperBoundTime(T,RD)

%upperBoundTime = upperBoundTime - startTime; % note that you must MODIFY this, 5500 for BEB, 8000 for STB looks good

M(1:1,1:ceil((upperBoundTime-startTime)/gran))=100;  % will store the color map %change M(1:N) for number of clients
colormap(colorcube(65)); % set colormap

for(i = 1:1:r)
if(isempty(T{i,1}))
        break;
end
    %i
if( isnan( T{i,2} ) == 0 && length(T{i,1}) < 4) % there is a number here, so transmission start time
   %client = T{i,1};
    xstep = ceil((T{i,2}/gran) - startTime + 1);
    M(1, [xstep:1:xstep]) = 7;  % color this yellow %change xstep+6
end


end

m = num2cell(M(1,:));
nr_in_cell = cellfun(@(x) find(x==7), m, 'Uni',0);                 % Find OccurrencesIn Each Cell
total_occurrences = numel([nr_in_cell{:}]); 
colorbar;
image((M(1:1, 1:1:length(M))));

%grid on;
%set(gca,'LineWidth',1.5)
%set(gca,'XTick',[0:500:5500]);
%set(gca,'XTickLabel',[0:500:5500]);
%set(gca,'YTick',0.5+1:20.5);
%set(gca,'YTickLabel',[]);
%xlabel('Time (\mus) ','Fontsize',20);
%ylabel('Stations 1-20','Fontsize',20);

end
        
    
