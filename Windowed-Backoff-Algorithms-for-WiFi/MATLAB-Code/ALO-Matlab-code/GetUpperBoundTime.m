function [upperBoundTime]=GetUpperBoundTime(T,RD)

[r,c]=size(RD);

for(i=r:-1:1)
    
if( ( isnan(T{i,2}) == 0) && (length(T{i,1}) < 4)) % there is a number here, so transmission start time
   upperBoundTime = T{i,2};
   break;
end

end
end