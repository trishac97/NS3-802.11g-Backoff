README document
June 20, 2021

This MATLAB code is used to create our transmission map and condensed map plots. This was written in June 2021, and we are using MATLAB version R2020a, Update 5 (9.8.0.1451342).

These are the steps to create the plot:

1. Import the data, such as "log-BEB-new.txt", using the "Home" tab in MATLAB. 

- Make sure that you set the delimiter to  "space", pull the selected columns across to the first 4,

- capture only the rows down to the last hex identifier (after that point you see summary information in the data file),

- Set the "Output Type" to be "Cell Array",

- Set the "Text Option" to be "Cell Array of Character Vectors".

Go ahead and click the Import Selection (green checkmark).

2. Call:

[H,RD]=ModifyClientName([X]);

where X is the imported data in your workspace from Step 1.

3. Call:

[T]= MapClientName(RD,H);

4. Call:

[M]=transmitMap(T);

which will draw the plot. 


 
