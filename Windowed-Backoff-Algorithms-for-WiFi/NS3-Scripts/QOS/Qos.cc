/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/dcf-manager.h"
#include "ns3/mac-low.h"
#include "ns3/dca-txop.h"
#include "ns3/dcf.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/wifi-phy-state-helper.h"
#include "ns3/wifi-phy.h"
#include "ns3/random-variable-stream.h"

#include "ns3/flow-monitor-helper.h"
#include <string>


// Default Network Topology
// None Right Now                            

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BackoffExample");

int 
main (int argc, char *argv[])
{


// ***********************************************
// * * * *  Logging, Variables, and Setup  * * * *
// **************************************************************************************

// Variables for logging and simulation

  bool clientServer = false; // Enable logging for the Clients/Server
  bool dcfDca = true; // Enable logging for DcfManager/DcfState and DcaTxop
  bool nodeInfo = false; // Enable displaying info about Nodes
  bool tracing = true; // Enable packet tracing


// Variables for Config::Default calls

  uint32_t myRtsThreshold = 60000; // Threshold of bytes at which RTS/CTS will be used
  uint32_t myFragThreshold = 9999; // Threshold of bytes at which fragmenttion will occur

  uint32_t myMaxRtsRetry = 10000; // Maximum number of retries for an RTS packet ??
  uint32_t myMaxDataRetry = 10000; // Maximum number of retries for a data packet

  double myLogDistanceRefLoss = 46.667; // Reference Loss for the LogLossModel (dB?)
  double myLogDistanceRefDist = 1.0; // Reference Distance for the LogLossModel (Miles?)
  double myLogDistanceExponent = 2.0; // Exponent for the LogLossModel


// Variables for Infastructure setups

  uint32_t numAp = 1; // Number of APs

  uint32_t numSta = 20; // Number of STAs -- -- later is rewritten by command line input
  int numClients = 20; // Number of Clients -- later is rewritten by command line input

  std::string myStationManager = "ns3::AarfWifiManager"; // RemoteStationManager for WifiHelper

// Mobility Variables

  float myApX = 20.0; // X distance of the AP (originally 7.0)
  float myApY = 20.0; // Y distance of the AP (originally 7.0)
  float myApZ = 8.0; // Height of the AP (originally 8.0)

  float myStaZ = 2.0; // Height of the STAs (originally 2.0)

  float myRoomX = 40.0; // X distance of the Room (originally 14.0)
  float myRoomY = 40.0; // Y distance of the Room (originally 14.0)

  float myRoomStartX = 2.0; // X of the first Node (originally 2.0)
  float myRoomStartY = 2.0; // Y of the first Node (originally 2.0)
  float myIncX = 3.0; // How far each new Node is placed in the X (originally 2.0)
  float myIncY = 3.0; // How far each new Node is placed in the Y (originally 2.0)

// Client/Server variables

  uint16_t myPortNumber = 9; // Port number used by the Clients and PacketSink

  float myServerStartTime = 1.5; // Determines when the Server will start
  float myServerStopTime = 100.0; // Determines when the Server will stop
  float myClientStartTime = 30.0; // Determines when the Clients will start
  float myClientStopTime = 100.0; // Determines when the Clients will stop

  uint32_t myClientMaxPackets = 2; // Number of packets for applications to send
  float myClientInterval = 30.0; // Delay between sending packets
  uint32_t myClientPacketSize = 64; // Size of the packets Clients send (12)

// Final variables

  // !!!!
  
  uint32_t myMinCw = 1; // The minimum Contention Window for the DcfStates (default 13)
  float mySimStop = 100.5; // Determines when the simulation will end

  int myRngRun = 1; // Number for NS3s Random Number Generator

  std::string myXmlFile = "backoffFlowX.xml"; // For the XML file created

// For parsing command line arguments

  CommandLine cmd;

  cmd.AddValue ("numSta", "Number of wifi STA devices", numSta);
  cmd.AddValue ("numClients", "Number of Clients", numClients);
  
  cmd.AddValue ("myClientPacketSize", "Size of packet that Clients will send", myClientPacketSize);
  cmd.AddValue ("myClientMaxPackets", "Total number of packets Clients will send", myClientMaxPackets);

  cmd.AddValue ("clientServer", "Tell Client/Server applications to log if true", clientServer);
  cmd.AddValue ("dcfDca", "Tell DcfManager/DcaTxop to log if true", dcfDca);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("myRngRun", "Run number for the RNG", myRngRun);

  cmd.Parse (argc,argv); 

// Naming file, G++ cant to std::to_string for some reason

  if(myRngRun==1){myXmlFile="bFlow1.xml";}
  if(myRngRun==2){myXmlFile="bFlow2.xml";}
  if(myRngRun==3){myXmlFile="bFlow3.xml";}
  if(myRngRun==4){myXmlFile="bFlow4.xml";}
  if(myRngRun==5){myXmlFile="bFlow5.xml";}
  if(myRngRun==6){myXmlFile="bFlow6.xml";}
  if(myRngRun==7){myXmlFile="bFlow7.xml";}
  if(myRngRun==8){myXmlFile="bFlow8.xml";}
  if(myRngRun==9){myXmlFile="bFlow9.xml";}
  if(myRngRun==10){myXmlFile="bFlow10.xml";}

// 1000 is way too many, but we're setting it as a huge upper bound
  if (numSta > 1000)
    {
      std::cout << "ERROR: Too many wifi nodes." << std::endl;
      return 1;
    }

// Enable logging for the Clients/Server
  if (clientServer)
    {
      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
    }

// Enable logging for the DcfManagers and DcaTxaop
  if (dcfDca)
    {
      LogComponentEnable("DcaTxop", LOG_LEVEL_ALL);
      LogComponentEnable("DcaTxop", LOG_PREFIX_TIME);
      LogComponentEnable("DcaTxop", LOG_PREFIX_NODE);	

      LogComponentEnable("DcfManager", LOG_LEVEL_ALL);
      LogComponentEnable("DcfManager", LOG_PREFIX_TIME);
      LogComponentEnable("DcfManager", LOG_PREFIX_NODE);
    }
  
  ns3::RngSeedManager::SetRun(myRngRun);

  // For changing the run seed manually
  //ns3::RngSeedManager::SetRun(11);

  std::cout << std::endl << "NUMCLIENTS:" << numClients << std::endl << std::endl; // *** for logParser.cpp ***

// ************************************************
// * * * *  Creating Simulation Components  * * * *
// ****************************************************************************

// ************************************
// * *  Configuring Default Values  * * 
// ************************************

  Config::SetDefault ("ns3::ArpCache::PendingQueueSize", UintegerValue(4294967293));

  // "Creating Channel and PHY Layer"
  // Setting perameters for the Loss Model to be used
  Config::SetDefault ("ns3::LogDistancePropagationLossModel::ReferenceLoss", DoubleValue(myLogDistanceRefLoss));
  Config::SetDefault ("ns3::LogDistancePropagationLossModel::ReferenceDistance", DoubleValue(myLogDistanceRefDist));
  Config::SetDefault ("ns3::LogDistancePropagationLossModel::Exponent", DoubleValue(myLogDistanceExponent));

  // "Creating WifiHelper"
  // Turn off RTS/CTS for frames below a certain ammount of bytes in the WifiRemoteStationManager
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue(myRtsThreshold));
  // Turn off fragmentation for frames below a certain number of bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", UintegerValue(myFragThreshold));
  // Maximum number of retransmission attempts for RTS packets
  Config::SetDefault ("ns3::WifiRemoteStationManager::MaxSsrc", UintegerValue(myMaxRtsRetry));
  // Maximum number of retransmission attempts for DATA packets
  Config::SetDefault ("ns3::WifiRemoteStationManager::MaxSlrc", UintegerValue(myMaxDataRetry));


// ************************
// * *  Creating Nodes  * *
// ************************

// Creates a container for the Wifi station nodes
  NodeContainer wifiStaNodes;
  // Creates 'numSta' ammount of Wifi nodes in the container
  wifiStaNodes.Create (numSta);

// Creates an a new node container for the AP
  NodeContainer wifiApNode;
  // Creates 'numAp' ammount of APs in the container
  wifiApNode.Create (numAp);


// ****************************************
// * *  Creating Channel and PHY Layer  * * 
// ****************************************

// Creates a Wifi channel in the YANS model in the default working state:
  
  // ** YansWifiChannelHelper channel = YansWifiChannelHelper::Default (); **

  // Originally created in the default working state, but we want to create our own, 
  // so just define a ChannelHelper for use.
  YansWifiChannelHelper channel;

    // !!! SetDefault (LogDistancePropagationLossModel) !!!

  // Adding the PrpagationLoss and PropagationDelay to the channel
  channel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
  channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");

// Creates a physical helper in the YANS model with the default settings
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();

  // Creates a channel object and associates it with the PHY layer to make sure objects share 
  // the same underlying channel, that is, they share the same wireless medium and can communicate.
  phy.SetChannel (channel.Create ());


// *****************************
// * *  Creating WifiHelper  * *
// *****************************
  
//Creates a WifiHelper (used to create WifiNetDevices) in the default state; 
// Adhoc MAC layer w/ ARF algorithm, with both using their default attributes.
// By default, MAC and PHY are configured for 802.11a.
  // ** = WifiHelper::Default(); either is or will be depricated
  WifiHelper wifi; 

  // Sets the 802.11 Standard to be used.
  wifi.SetStandard (WIFI_PHY_STANDARD_80211g); 

    // !!! SetDefault (WifiRemoteStationManager) !!!

  // Sets the type of rate control algorithm to be used.

  // **** wifi.SetRemoteStationManager (myStationManager); ****

    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", 
                                 "DataMode", StringValue ("ErpOfdmRate54Mbps"), 
                                 "ControlMode", StringValue ("ErpOfdmRate54Mbps"));


// ****************************
// * *  Creating MAC Helper * *
// ****************************

// Configuring the SSID that will be used
  Ssid ssid = Ssid ("backoff-ssid");

// Creates a WifiMacHelper to implement the MAC layer for Wifi
// Non Qos
 // WifiMacHelper mac = NqosWifiMacHelper::Default (); 
// Qos
   WifiMacHelper mac = QosWifiMacHelper::Default ();


// **************************************************
// * *  Configuring MAC and NetDevices for Nodes  * *
// **************************************************

// * * * STA NODES * * *

// Configuring MAC Layer for the STA nodes
  mac.SetType ("ns3::StaWifiMac",
               // Associates with the SSID that was configured
               "Ssid", SsidValue (ssid),
               // Disabling active probing
               "ActiveProbing", BooleanValue (false)); 

// Creates a container for the STA devices
  NetDeviceContainer staDevices;
  // Using the WifiHelper, creates the WifiNetDevices using the given PHY and MAC on the given Nodes.
  staDevices = wifi.Install (phy, mac, wifiStaNodes);


// * * * AP NODES * * *

// Using the same WifiMachHelper, configure it for the AP(s)
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

// Creates a container for the AP devices
  NetDeviceContainer apDevices;
  // Creates the WifiNetDevices with the given PHY and (newly changed) MAC on the given Nodes.
  apDevices = wifi.Install (phy, mac, wifiApNode);


// ************************************
// * *  Configuring Mobility Model  * *
// ************************************

// * * * AP NODES * * *

// Creates a MobilityHelper to place the nodes
  MobilityHelper mobility;
  // The Nodes will be stationary, so we use the ConstantPositionModel
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

// Installing the MobilityModel on the AP and STAs
  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNodes);

// Now that we have the objects installed, we need to access and modify them
  // Creates a pointer to the model that we can modify
  Ptr<ConstantPositionMobilityModel> mobilityAp = wifiApNode.Get(0)->GetObject<ConstantPositionMobilityModel> ();

  // Sets the position of the AP (normally 7,7,8)
  mobilityAp->SetPosition (Vector (myApX, myApY, myApZ));


// * * * STA NODES * * *

// Creating a pointer to use when itterating through each STA
  Ptr<ConstantPositionMobilityModel> mobilitySta;

// Variables to keep track of where the Nodes will be places
  float currX = myRoomStartX;
  float currY = myRoomStartY;
  
// Itterating through each of the STAs to palce them.
  for(uint32_t r = 0; r < numSta; r++)
  {
     // Create a pointer and place the node at the current positions
     mobilitySta = wifiStaNodes.Get(r)->GetObject<ConstantPositionMobilityModel> ();
     mobilitySta->SetPosition (Vector (currX, currY, myStaZ));

     // Increment the current X value by the given 'Increment X' value
     currX += myIncX;
     // If the new X value is outside the room, reset it to the starting X and increment Y
     if(currX > myRoomX){currX = myRoomStartX; currY += myIncY;}
     // If the new Y value is outside the room, reset it to the starting Y and increment the STAs Z 
     if(currY > myRoomY){currY = myRoomStartY; myStaZ += 0.5;}
  }

// ******************************
// * *  IP Stack & Addresses  * *
// ******************************
 
// Installing the IP stack to the nodes
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

// Creating the IPv4 addresses to assign to the channel
  Ipv4AddressHelper address;
  // Setting the base for the addresses to be given
  address.SetBase ("172.16.0.0", "255.255.0.0");

// Creating an Ipv4InterfaceContainer for the APs
  Ipv4InterfaceContainer apInterface;
  // Assigning Addresses to the APs
  apInterface = address.Assign (apDevices);

// Creating an Ipv4InterfaceContainer for the STAs
  Ipv4InterfaceContainer staInterfaces;
  // Assigning Addresses to the STAs
  staInterfaces = address.Assign (staDevices);


// ****************************************
// * *  Creating Apps, Clients/Servers  * *
// ****************************************

// Creating the UPD  Server on port 9
  UdpServerHelper myServer (myPortNumber);

// Creating an ApplictionContainer for the server.
  ApplicationContainer serverApps = myServer.Install (wifiApNode.Get (0));

// When the Server(s) will start/stop
  serverApps.Start (Seconds (myServerStartTime));
  serverApps.Stop (Seconds (myServerStopTime));

// Creating the Client to communicate with the Server
  UdpClientHelper myClient (apInterface.GetAddress (0), myPortNumber);
  myClient.SetAttribute ("MaxPackets", UintegerValue (myClientMaxPackets));
  myClient.SetAttribute ("Interval", TimeValue (Seconds (myClientInterval)));
  myClient.SetAttribute ("PacketSize", UintegerValue (myClientPacketSize));

// Creating an ApplicationContainer for the Clients
  ApplicationContainer clientApps; 
  
// Installing and adding the clients the clientApps container
  for (int c = 0; c < numClients; c++){
    clientApps.Add(myClient.Install (wifiStaNodes.Get (c)));
  }
 
// When the Clients will start/stop
  clientApps.Start (Seconds (myClientStartTime));
  clientApps.Stop (Seconds (myClientStopTime));

// *******************************
// * * * *  Final Section  * * * *
// ******************************************************************************


// ******************************
// * *  Outputting Node Info  * *
// ******************************

  if(nodeInfo){

  std::cout << std::endl << "* * * *  Nodes  * * * *" << std::endl << std::endl;

  std::cout << "* *  APs  * *" << std::endl;
// Information for AP nodes
  for (uint32_t z = 0; z < numAp; z++)
  {
    std::cout << "AP " << z << ": ";
    std::cout << apInterface.GetAddress(z);
    std::cout << "  MAC: (" << apDevices.Get(z)->GetAddress() << ")";
    std::cout << std::endl;
  }

  std::cout << "* *  STAs  * *" << std::endl;
// Information for STA nodes (and Clients) 
  for (uint32_t i = 0; i < numSta; i++)
  {
    std::cout << "STA " << i << ": ";
    std::cout << staInterfaces.GetAddress(i);
    std::cout << "  MAC: (" << staDevices.Get(i)->GetAddress() << ") ";
    
    // Determines if the Node is a client
    if(i < uint32_t (numClients)){std::cout << " [CLIENT]";}

    std::cout << std::endl;
  }

  std::cout << std::endl << "* * * * * * * * * * * * * " << std::endl << std::endl;

  } // End of if(nodeInfo)

// *****************************************************
// * * * * Last Few Things & Running Simulation  * * * *
// *****************************************************

  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/Queue/MaxDelay", TimeValue (NanoSeconds (9193372036854775805)));

// Setting the Minimum CW for all the DcaTxops. (originally 15)
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/MinCw", UintegerValue (myMinCw));

  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/MaxCw", UintegerValue (10000));

// Setting the AIFSN for all the DcaTxops. (originally 2, default value conforms to simple DCA)
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/Aifsn", UintegerValue (2));

// Setting ACK timeout (originally 75000 NanoSeconds)
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/AckTimeout", TimeValue (NanoSeconds (75000))); // !!!!!

// Setting Slot time (originally 9000 NanoSeconds)
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/Slot", TimeValue (NanoSeconds (9000)));

// Setting Sifs (originally 16000 NanoSeconds) 
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/Sifs", TimeValue (NanoSeconds (16000))); // !!!!

// Setting EifsNoDifs (oringally 60000 NanoSeconds)
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/EifsNoDifs", TimeValue (NanoSeconds (60000)));

// Seting Rifs (originally 2000 NanoSeconds) 
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/Rifs", TimeValue (NanoSeconds (2000)));

// Setting Pifs (originally 25000 NanoSeconds) 
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/Rifs", TimeValue (NanoSeconds (25000)));

// Setting 

// There is an internetwork, so enable global internetwork routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

// Because the AP is generating beacons, it will never actually stop,
// so the simulator must explicitly be told to stop.
  Simulator::Stop (Seconds (mySimStop));

// Creating pcap tracing files to cover all three networks
  if (tracing)
    {
      // promiscuous mode trace on the Wifi network
      phy.EnablePcap ("backoff", apDevices.Get (0));
    }

  
// Enabling flow monitoring on the nodes
  FlowMonitorHelper flowMon;
  Ptr<FlowMonitor> flowNodes = flowMon.InstallAll();


// Run the Simulator
  Simulator::Run ();
 
// Create an XML file for the FlowMonitor

  flowNodes->SerializeToXmlFile(myXmlFile, true, true);

  Simulator::Destroy ();


  std::cout << " " << std::endl;

  return 0;

}

