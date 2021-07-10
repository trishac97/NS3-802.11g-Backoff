function [m,CM]=condensedTmap(T,M)
[r,c]=size(M);

gran = 10;
startTime = 60000000/gran;

CM(1:1,1:c)=100;  % will store the color map
colormap(colorcube(65)); % set colormap

for(i = 1:1:c)
    %i
      m = cellstr(num2str(M(:,i)));
      for(j=1:1:length(m))
     
       m{j,1} = str2num(m{j,1}); 
     
end
    nr_in_cell = cellfun(@(x) find(x==7), m, 'Uni',0);                 % Find OccurrencesIn Each Cell
    ack_in_cell = cellfun(@(v) find(v==10), m, 'Uni',0);                 % Find OccurrencesIn Each Cell
    
    count_send = numel([nr_in_cell{:}]); 
    count_ack = numel([ack_in_cell{:}]); 
  
    if( count_send >1) %Collision
    CM(1, i) = 7;  % color this orange
    
    elseif(count_ack >1) %Keeping track of Acktimeout
    CM(1, i) = 7;
        
    
    elseif( count_send == 1 ) %Success
    CM(1, i) = 10;  % color this blue
    
    else %anything else ack-timeouts, etc
       CM(1, i) = 100;  % color this yellow
       end
end
%comment this
%start = 0
%for(j= 1:1:c)
%    if(CM(1,j) == 10)
%        for(k = start+1:1:j)
%            CM(1,k)=10;
%        end
%    elseif(CM(1,j) == 7)
%        start = j+1;
%    end
%end
colorbar;
image(CM);


grid on;
set(gca,'LineWidth',1.5)
%set(gca, 'XTickLabel', [0:gran*5:500*gran]);
set(gca,'YTick',0.5+1:20.5);
set(gca,'YTickLabel',[]);
set(gca,'XTickLabel',[0:1000:10000]);
%set(gca, 'YTick', [.5:1:20.5]);
set(gca,'fontsize',12);
%xlabel('Number of Packets (64-byte payload)','Fontsize',20);
xlabel('Time (\mus) ','Fontsize',20);
%ylabel('Stations 1-20','Fontsize',20);
xlim([-1 1000]);

end