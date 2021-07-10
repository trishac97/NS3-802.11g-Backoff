// Author: Maxwell Young
// Date: March 28, 2016 Updated: Dec 16, 2016  Updated Jan 2016

// This code was used to run experiments for our SPAA 2017 paper. 
// To whoever uses this: I make no guarantees on the correctness of this code. 

import java.io.*;
import java.net.*;
import java.util.Random;   
import java.util.Collections;   
import java.util.Vector; 

public class backoff{
    
	// record variables
	public static int slotCount;
	public static int numProcesses;
	public static int numFails;
	public static int ackFails;
	public static int numColls;
	
    // random number generator
    Random rnd = new Random();
	
	// for writing plot data to file
    public static FileWriter time_fw;
    public static BufferedWriter time_bw;
	
    public static FileWriter fail_fw;
    public static BufferedWriter fail_bw;
	
	
	// constructor
	public backoff(int n){
		numProcesses = n;
		slotCount = 0;
		numFails = 0;
		ackFails = 0;
		numColls = 0;
	}
	

	public void reset(int n){
		//System.out.println("Called reset.");
		numProcesses = n;
		slotCount = 0;
		numFails = 0;
		ackFails = 0;
		numColls = 0;
	}
	
	
	public void simWindow(int windowSize){		
		int[] bins = new int[windowSize];	
		int index = -1;
		
		// for selecting the slot to send in
		for(int i=0; i<numProcesses; i++){
			index = rnd.nextInt(windowSize);
			bins[index] = bins[index] + 1;
		}	
		
		// for testing only
		// ---------------------------------		
		/*for(int k=0; k<windowSize; k++){
			System.out.print(bins[k] + " ");
		}*/
		//System.out.println(); 
		//System.out.println(); 
		// end of for testing only
		// ---------------------------------	


		// go through the window and count collisions
		for(int j=0; j<windowSize-1; j++){
			if( (bins[j] > 1) ){ // ack failure
				numColls = numColls + 1;
			}
		}
		//System.out.println("    AckFails up to this point: " + ackFails + " SlotCount up to this point: " + slotCount);

		int flag = 0;
		for(int j=0; j<windowSize; j++){
			if( bins[j] == 1 ){ // success
				numProcesses--;
				if(numProcesses==0){
					slotCount = slotCount + j + 1;
					return;
				}
			}
		}
		slotCount = slotCount + windowSize; // note the return statement above	which means we don't add this if we finish in this current window (doing otherwise would cause us to overcount.)		
	}
	
	public void exponentialBackoff(){
		int wSize = 1;
		int windowIndex = 1;
		
		while( numProcesses > 0 ){			
			wSize = ((int)Math.pow(2, windowIndex));
			//System.out.println(wSize + " and the number of clients left: " + numProcesses );
			simWindow(wSize);
			//System.out.println(numFails);
			//numFails = 0;
			windowIndex++;
		}
		
	}
	
	
	public void SawToothBackoff(){
		int wSize = 1;
		int windowIndex = 1;		
		while( numProcesses > 0 ){	
			wSize = ((int)Math.pow(2, windowIndex));		
			for(int i = windowIndex; i>=1; i--){			
				if(numProcesses > 0){
					wSize = ((int)Math.pow(2, i));
					simWindow(wSize);
				}
				else{
					//System.out.println("slotcount: " + slotCount);
					return;
				} 
				//System.out.println("Window has size: " + wSize + " and the number of clients left: " + numProcesses );	
				//System.out.println(numFails);
				//numFails = 0;
			}
			windowIndex++;
			//System.out.println("slotcount: " + slotCount);
			//System.out.println();
		}
	}
	
	
	// N is the number of devices
	public void linearBackoff(){
		int wSize = 1;
		int windowIndex = 1;
		
		while( numProcesses > 0 ){			
			wSize++;
			//System.out.println("Window size: " + wSize);
			simWindow(wSize);
			windowIndex++;
		}	
	}
	
	
	// N is the number of devices
	public void logBackoff(){
		int wSize = 2;
		
		double ratio = 0.0;
		double raw = 0.0;
		while( numProcesses > 0 ){	
			simWindow(wSize);
			ratio = 1.0/(Math.log(wSize)/Math.log(2.0));
			//System.out.println("ratio: " + ratio);
					
			raw = (1.0 + ratio)*((double)wSize);
			//System.out.println("raw window: " + raw );
			wSize =(int)Math.floor(raw);
			
			//System.out.println(wSize + " and the number of clients left: " + numProcesses);
			
			//System.out.println(numFails);
			//numFails = 0;
		}		
	}
	
	// N is the number of devices
	public void loglogBackoff(){
		int wSize = 4;
		double ratio = 0.0;
		double raw = 0.0;
		double placeholder = 0.0;
		while( numProcesses > 0 ){	
			simWindow(wSize);
			
			placeholder = (Math.log(wSize)/Math.log(2.0)); // do one logarithm evaluation		
			ratio = 1.0/(Math.log(placeholder)/Math.log(2.0));
				
			raw = (1.0 + ratio)*((double)wSize);
			wSize =(int)Math.floor(raw);
			//System.out.println("ratio: " + ratio + "    raw window: " + raw );
			//System.out.println(wSize + " and the number of clients left: " + numProcesses);
			
			//System.out.println(numFails);
			//numFails = 0;
		}		
	}
	
	
	
    public static void main(String [] args){// main function    
      	System.out.println("Welcome to the Backoff Simulator. How can I help you?\n");
		
		Integer x,y,z, xx;
		String t,c,col, tt;
		
		int l, m, u = 0;
	
		
		Vector<Integer> vector = new Vector<>(); 
		Vector<Integer> vectorAckFails = new Vector<>(); 
		try{
            time_fw = new FileWriter("time.txt");
            time_bw = new BufferedWriter(time_fw);  
				
            fail_fw = new FileWriter("colls.txt");
            fail_bw = new BufferedWriter(fail_fw);	
				
			backoff myBackoff = new backoff(100); // note that this is reset inside the loop   
					
			// Code for testing 
			int numTrials = 100;
			int sumTime = 0;
			int sumFails = 0;
			int sumAckFails = 0;
			int sumColls = 0;
			
			int maxSize = 100000;
			for(int k=400; k<=maxSize; k=k+400){
				System.out.println("N=" + k);
				for(int i=0; i<numTrials; i++){
					myBackoff.reset(k);
									
					myBackoff.exponentialBackoff();   // run binary exponential backoff
	                //myBackoff.linearBackoff();      // run linear backoff
					//myBackoff.logBackoff();         // run linear backoff
					//myBackoff.loglogBackoff();
					//myBackoff.SawToothBackoff();
				
				
					sumTime = sumTime + slotCount;
					sumColls = sumColls + numColls;
					//sumFails = sumFails + numFails;
					//sumAckFails = sumAckFails + ackFails;
					//System.out.println("Ack fails total: " + sumAckFails);
					//System.out.println("Slots total: " + sumTime);
					//vector.add(slotCount);	
					//vectorAckFails.add(ackFails);
				}
				
				// record data on total slots
				x = new Integer( (int)(((double)sumTime)/((double)numTrials)) );
				t = x.toString();
				time_bw.write(t + "\n");
				sumTime = 0;
				
				// record data on ack failures
				xx = new Integer( (int)(((double)sumColls)/((double)numTrials)) );
				tt = xx.toString();
				fail_bw.write(tt + "\n");
				sumColls = 0;
				
				
			}
			time_bw.close();
			time_fw.close();
			
			fail_bw.close();
			fail_fw.close();
		} // end try
	    catch(IOException e){
	            System.out.println("I/O error2");  
				System.exit(0);
	    }
	}	
}
