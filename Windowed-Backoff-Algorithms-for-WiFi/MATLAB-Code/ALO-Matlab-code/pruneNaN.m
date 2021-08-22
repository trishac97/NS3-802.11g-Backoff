function Y=pruneNaN(X)
L = length(X);
Y=[];
y=1;
for i=1:1:L
    if( isnan( X(i) ) == 0 )
       Y(y) = X(i);
       y=y+1;
    end
end
