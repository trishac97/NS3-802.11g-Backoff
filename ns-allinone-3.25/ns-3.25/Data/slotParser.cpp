#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>

double getMedian(std::vector<float> vect)
{
  	double median;
  	size_t size = vect.size();

  	sort(vect.begin(), vect.end());

  	if (size  % 2 == 0)
  	{
    	median = (vect[size / 2 - 1] + vect[size / 2]) / 2;
  	}
  	else 
  	{
    	median = vect[size / 2];
  	}

  	return median;
}

int main() 
{

	// Strings for parsing
    std::string fileLine;
	std::string numString;	

	int numClients;
	int numLogs=30; // added by MY

	double Overall_slotTotal[numLogs];
	int slotTotal = 0;
	int slotCount;

	double Overall_packetTimeTotal[numLogs];
    double packetTimeTotal = 0.0;
	int packetTimeCount;

	double Overall_missedAckTotal[numLogs];
	int missedAckTotal = 0;
	int missedAckCount;

	double Overall_ackTimeTotal[numLogs];
	double ackTimeTotal = 0.0;
	int ackTimeCount;

	double Overall_estimatedCw[numLogs];

	std::vector<float> medPacket;
	std::vector<float> medSlot;
	std::vector<float> medAck;
	std::vector<float> medTimeout;

for (int i = 0; i < numLogs; i++){

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

    // Itterating through the lines
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
			numString = fileLine.substr(pos+1);

            Overall_estimatedCw[i] = std::atoi (numString.c_str());
			
		}

		// How many slots to finish?
		if(fileLine.substr(0,9)=="No more s" && slotCount != 0)
		{

        	std::size_t	pos = fileLine.find(": ");
			numString = fileLine.substr(pos+2);
			
			slotTotal = std::atoi (numString.c_str());

			medSlot.push_back(slotTotal);

			slotCount -= 1;
		}

		// How long to send packet?
		if(fileLine.substr(0,11)=="Packet time" && packetTimeCount != 0)
		{

        	std::size_t	pos = fileLine.find(": ");
			numString = fileLine.substr(pos+2);
			
			packetTimeTotal = std::atof (numString.c_str());

			medPacket.push_back(packetTimeTotal);

			packetTimeCount -= 1;
		}

		// How many ACKs missed 
		if(fileLine.substr(0,30)=="Times device missed an ACK & R" && missedAckCount != 0)
		{

        	std::size_t	pos = fileLine.find(": ");
			numString = fileLine.substr(pos+2);
			
			missedAckTotal = std::atof (numString.c_str());

			medAck.push_back(missedAckTotal);

			missedAckCount -= 1;
		}

		// Time of ACKs Missed
		if(fileLine.substr(0,10)=="Missed ACK" && ackTimeCount != 0)
		{

        	std::size_t	pos = fileLine.find(": ");
			numString = fileLine.substr(pos+2);
			
			ackTimeTotal = std::atof (numString.c_str());

			medTimeout.push_back(ackTimeTotal);

			ackTimeCount -= 1;
		}

	} // End of while(!eof)

	Overall_slotTotal[i] = getMedian(medSlot);

	Overall_packetTimeTotal[i] = getMedian(medPacket);

	Overall_missedAckTotal[i] = getMedian(medAck);

	Overall_ackTimeTotal[i] = getMedian(medTimeout);
	
	slotTotal = 0;
	packetTimeTotal = 0.0;
	missedAckTotal = 0;
	ackTimeTotal = 0.0;

	medPacket.clear();
	medSlot.clear();
	medAck.clear();
	medTimeout.clear();

}

	//std::cout << std::endl << "Clients: " << numClients << std::endl;
	//std::cout << "------------------------------------------" << std::endl;

	//std::cout << "Slots -- Packet Send Time -- Missed Acks -- MissedAckTime -- CW Estimate" <<  std::endl;




	for (int c = 0; c < numLogs; c++)
	{
		std::cout << Overall_slotTotal[c] << "    " << Overall_packetTimeTotal[c] << "    " << Overall_missedAckTotal[c] << "    " << Overall_ackTimeTotal[c] << "    " << Overall_estimatedCw[c] << std::endl;
		//std::cout << Overall_slotTotal[c] << "  --  ";
		//std::cout << Overall_packetTimeTotal[c] << "  --  ";
		//std::cout << Overall_missedAckTotal[c] << " -- ";
		//std::cout << Overall_ackTimeTotal[c] << " -- ";
		//std::cout << Overall_estimatedCw[c] << std::endl;
	}

//	std::cout << "------------------------------------------" << std::endl << std::endl;

	return 0;

} // End main()

