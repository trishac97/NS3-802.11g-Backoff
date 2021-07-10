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
function [M]=trasmitMap(T)

[r,c]=size(T);
N=10;
gran = 20;% each step is worth gran mu secs
%packetTxTime = 10/gran; % mu secs to transmit packet
startTime = 60000000/gran;

M(1:N,1:ceil(10000/gran))=100;  % will store the color map %change M(1:N) for number of clients
colormap(colorcube(65)); % set colormap

for(i = 1:1:r)
if(isempty(T{i,1}))
        break;
end
    %i
if( isnan( T{i,2} ) == 0 && length(T{i,1}) < 3) % there is a number here, so transmission start time
    client = T{i,1};
    xstep = ceil((T{i,2}/gran) - startTime + 1);
    M(client, [xstep:1:xstep]) = 7;  % color this yellow %change xstep+6
end
if(isnan( T{i,4} ) == 0 && length(T{i,1})<3) %T{i,1} < 21) % this is an ACK timeout
    client = T{i,1};
    xstep = ceil((T{i,4}/gran) - startTime + 1);
    M(client, xstep) = 10; % color this something else
end

end


colorbar;
image((M(1:N, 1:500)));

grid on;
set(gca,'LineWidth',1.5)
%set(gca, 'XTickLabel', [0:gran*5:500*gran]);
set(gca,'YTick',0.5+1:20.5);
set(gca,'YTickLabel',[]);
set(gca,'XTickLabel',[0:1000:5000]);
%set(gca, 'YTick', [.5:1:20.5]);
%set(gca,'fontsize',14);
%xlabel('Number of Packets (64-byte payload)','Fontsize',20);
xlabel('Time (\mus) ','Fontsize',20);
ylabel('Stations 1-20','Fontsize',20);
xlim([-1 1000]);

end
        
    
