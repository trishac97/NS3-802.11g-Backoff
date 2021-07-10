function total_occurrences=cellArray(CM)
m = cellstr(num2str(CM(1,:)));
for(i=1:1:length(m))
     
       m{i,1} = str2num(m{i,1}); 
     
end
nr_in_cell = cellfun(@(x) find(x==10 | x==7), m, 'Uni',0);                 % Find OccurrencesIn Each Cell
total_occurrences = numel([nr_in_cell{:}]); 
disp(total_occurrences);
end
