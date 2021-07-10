#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>


// This function calculates the maximum slot, packet, ack, and ack misses (see slotparser for median calculations)

int main() 
{

	// Strings for parsing
    std::string fileLine;
	std::string numString;	

	int numClients = 0;

	int slotTotal = 0;
	double packetTimeTotal = 0.0;
	double ackTimeTotal = 0.0;
	int missedAckTotal = 0;
	double dTemp = 0.0;
	int iTemp = 0;

	int slotCount = 0;
	int packetTimeCount = 0;

	int missedAckCount = 0;
	int ackTimeCount = 0;

	int estimatedCw = 0;

	int firstPass = true;

//std::cout << std::endl << "Clients: " << numClients << std::endl;
//std::cout << "------------------------------------------" << std::endl;
//std::cout << "Slots -- Packet Send Time -- Missed ACKs -- Missed ACK time -- Estimated CW" <<  std::endl;

for (int i = 0; i < 30; i++){

    // Creating and opening the log file
	std::ifstream infile;

    if(i==0){infile.open ("log1.txt");}
	if(i==1){infile.open ("log2.txt");}
	if(i==2){infile.open ("log3.txt");}
	if(i==3){infile.open ("log4.txt");}
	if(i==4){infile.open ("log5.txt");}
	if(i==5){infile.open ("log6.txt");}
	if(i==6){infile.open ("log7.txt");}
	if(i==7){infile.open ("log8.txt");}
	if(i==8){infile.open ("log9.txt");}
	if(i==9){infile.open ("log10.txt");}
        
	if(i==10){infile.open ("log11.txt");}
	if(i==11){infile.open ("log12.txt");}
	if(i==12){infile.open ("log13.txt");}
	if(i==13){infile.open ("log14.txt");}
	if(i==14){infile.open ("log15.txt");}
	if(i==15){infile.open ("log16.txt");}
	if(i==16){infile.open ("log17.txt");}
	if(i==17){infile.open ("log18.txt");}
	if(i==18){infile.open ("log19.txt");}
        if(i==19){infile.open ("log20.txt");}
        
        if(i==20){infile.open ("log21.txt");}
        if(i==21){infile.open ("log22.txt");}
	if(i==22){infile.open ("log23.txt");}
	if(i==23){infile.open ("log24.txt");}
	if(i==24){infile.open ("log25.txt");}
	if(i==25){infile.open ("log26.txt");}
	if(i==26){infile.open ("log27.txt");}
	if(i==27){infile.open ("log28.txt");}
	if(i==28){infile.open ("log29.txt");}
        if(i==29){infile.open ("log30.txt");}

    // Iterating through the lines
    while(!infile.eof())
	{
	    // Get the next line 
		std::getline (infile, fileLine);

		// Parsing number of clients
        if(fileLine.substr(0,10)=="NUMCLIENTS")
		{
			std::size_t pos = fileLine.find(":");
			numString = fileLine.substr(pos+1);

            numClients = std::atoi (numString.c_str());

			slotCount = numClients;
			packetTimeCount = numClients;

			missedAckCount = numClients;
			ackTimeCount = numClients;
			
		}

		// Predicted CW
        if(fileLine.substr(0,9)=="Predicted")
		{
			std::size_t pos = fileLine.find(":");
			numString = fileLine.substr(pos+1); // grab from beginning of number to end

            estimatedCw = std::atoi (numString.c_str());
			
		}

		// How many slots to finish?
		if(fileLine.substr(0,9)=="No more s" && slotCount != 0) 
		{

        	std::size_t	pos = fileLine.find(": ");
			numString = fileLine.substr(pos+2); // grab from beginning of number to end
			
			iTemp = std::atoi (numString.c_str());

			if(iTemp>slotTotal){slotTotal=iTemp;}

			slotCount -= 1;
		}

		// How long to send packet?
		if(fileLine.substr(0,11)=="Packet time" && packetTimeCount != 0)
		{

        	std::size_t	pos = fileLine.find(": ");
			numString = fileLine.substr(pos+2); // grab from beginning of number to end
			
			dTemp = std::atof (numString.c_str());

			if(dTemp>packetTimeTotal){packetTimeTotal=dTemp;}

			packetTimeCount -= 1;
		}

		// How many ACKs missed 
		if(fileLine.substr(0,30)=="Times device missed an ACK & R" && missedAckCount != 0)
		{

        	std::size_t	pos = fileLine.find(": ");
			numString = fileLine.substr(pos+2);
			
			iTemp = std::atoi (numString.c_str());

			if(iTemp>missedAckTotal){missedAckTotal=iTemp;}

			missedAckCount -= 1;
		}

		// Time of ACKs Missed
		if(fileLine.substr(0,10)=="Missed ACK" && ackTimeCount != 0)
		{

        	std::size_t	pos = fileLine.find(": ");
			numString = fileLine.substr(pos+2);
			
			dTemp = std::atof (numString.c_str());

			if(dTemp>ackTimeTotal){ackTimeTotal=dTemp;}

			ackTimeCount -= 1;
		}

	} // End of while(!eof)


	if(firstPass){
		//std::cout << std::endl << "Clients: " << numClients << std::endl;
		//std::cout << "------------------------------------------" << std::endl;
		//std::cout << "Slots -- Packet Send Time -- Missed ACKs -- Missed ACK time -- Estimated CW" <<  std::endl;
		firstPass=false;
	}

	
	std::cout << slotTotal << "    " << packetTimeTotal << "    " << missedAckTotal << "    " << ackTimeTotal << "    " << estimatedCw << std::endl;

	slotTotal = 0;
	packetTimeTotal = 0.0;
	ackTimeTotal = 0.0;
	missedAckTotal = 0;
	dTemp = 0.0;
	iTemp = 0;

	estimatedCw = 0;

}



	//std::cout << "------------------------------------------" << std::endl << std::endl;

	return 0;

} // End main()

