% goes through array and measures difference D between 1st and 3rd quartiles
% renames any value >= 1.5*D to NaN
function Y=excludeOutliers(X)

S = sort(X);

m = median(X);
first = S(ceil(length(X)/4));
third = S(3*ceil(length(X)/4));

dist = third - first;
error = 1.5*dist;

count = 0;
for i=1:1:length(X)
    if( X(i)> third + error | X(i)<first-error)
        X(i)=NaN;
        count = count + 1;
    end
end

Y=X;

fprintf('NUMBER OF POINTS EXCLUDED: -------------------------------> %i\n', count); 
%fid = fopen('number-excluded.txt', 'a+');
%fprintf(fid, '%i\n', count);
%fclose(fid);