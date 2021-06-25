/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
 *
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
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "dca-txop.h"
#include "dcf-manager.h"
#include "mac-low.h"
#include "wifi-mac-queue.h"
#include "mac-tx-middle.h"
#include "wifi-mac-trailer.h"
#include "wifi-mac.h"
#include "random-stream.h"

#include <cmath> // !!!

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT if (m_low != 0) { std::clog << "[mac=" << m_low->GetAddress () << "] "; }

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DcaTxop");

class DcaTxop::Dcf : public DcfState
{
public:
  Dcf (DcaTxop * txop)
    : m_txop (txop)
  {
  }
  virtual bool IsEdca (void) const
  { 
    // This is DCA, not EDCA
    return false;
  }
private:
  // Invoked from DcfState::NotifyAccessGranted
  // (Called from: DcfManager::DoGrantAccess)
  virtual void DoNotifyAccessGranted (void)
  {
    m_txop->NotifyAccessGranted ();
  }

  // Invoked from DcfState::NotifyInternalCollision
  // (Called from: DcfManager::DoGrantAccess)
  virtual void DoNotifyInternalCollision (void)
  {
    m_txop->NotifyInternalCollision ();
  }

  // Invoked from DcfState::NotifyCollision
  // (Called from: DcfManager::RequestAccess)
  virtual void DoNotifyCollision (void)
  {
    m_txop->NotifyCollision ();
  }

  // Invoked from DcfState::NotifyChannelSwitching
  // (Called from: DcfManager::NotifySwitchingStartNow)
  virtual void DoNotifyChannelSwitching (void)
  {
    m_txop->NotifyChannelSwitching ();
  }

  // Invoked from DcfState::NotifySleep
  // (Called from: DcfManager::NotifySleepNow)
  virtual void DoNotifySleep (void)
  {
    m_txop->NotifySleep ();
  }

  // Invoked from DcfState::NotifyWakeUp
  // (Called from: NotifyWakeUpNow)
  virtual void DoNotifyWakeUp (void)
  {
    m_txop->NotifyWakeUp ();
  }


  // Variable for this DcaTxop
  DcaTxop *m_txop;
};


/**
 * Listener for MacLow events. Forwards to DcaTxop.
 */
class DcaTxop::TransmissionListener : public MacLowTransmissionListener
{
public:
  /**
   * Create a TransmissionListener for the given DcaTxop.
   *
   * \param txop
   */
  TransmissionListener (DcaTxop * txop)
    : MacLowTransmissionListener (),
      m_txop (txop)
  {
  }

  virtual ~TransmissionListener ()
  {
  }

  virtual void GotCts (double snr, WifiMode txMode)
  {
    m_txop->GotCts (snr, txMode);
  }
  virtual void MissedCts (void)
  {
    m_txop->MissedCts ();
  }
  virtual void GotAck (double snr, WifiMode txMode)
  {
    m_txop->GotAck (snr, txMode);
  }
  virtual void MissedAck (void)
  {
    m_txop->MissedAck ();
  }
  virtual void StartNext (void)
  {
    m_txop->StartNext ();
  }
  virtual void Cancel (void)
  {
    m_txop->Cancel ();
  }
  virtual void EndTxNoAck (void)
  {
    m_txop->EndTxNoAck ();
  }

private:
  DcaTxop *m_txop;
};

NS_OBJECT_ENSURE_REGISTERED (DcaTxop);

TypeId
DcaTxop::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DcaTxop")
    .SetParent<ns3::Dcf> ()
    .SetGroupName ("Wifi")
    .AddConstructor<DcaTxop> ()
    .AddAttribute ("Queue", "The WifiMacQueue object",
                   PointerValue (),
                   MakePointerAccessor (&DcaTxop::GetQueue),
                   MakePointerChecker<WifiMacQueue> ())
  ;
  return tid;
}

DcaTxop::DcaTxop ()
  : m_manager (0),
    m_currentPacket (0),

    m_ackPackets (0), // ***
    m_assocReqPackets (0), // ***
    m_assocRespPackets (0), // ***
    m_ctsPackets (0), // ***
    m_rtsPackets (0), // ***

    m_dataPackets (0), // ***

    m_dcaGotCtsCount (0), // ***
    m_dcaMissedCtsCount (0), // ***
    m_dcaGotAckCount (0), // ***
    m_dcaMissedAckFailCount (0), // ***
    m_dcaMissedAckRetransmitCount (0), // ***
    
    // Logging for Queuing packets into the WifiMacQueue ***
    m_myQueueLogging (false), // ***
    // Logging for Dequeueing packets from the WifiMacQueue ***
    m_myDequeueLogging (false), // ***
    // Logging for sending packets to the MacLow Layer ***
    m_myLowLogging (false),
    // Logging for seeing when ACKs are recieved/missed ***
    m_myAckLogging (false),

    m_curIndex (0), // ***

    m_sendPacketTime (0.0), // ***
    m_missedAckTime (0.0), // ***
    m_totalAckTime (0.0), // ***

    m_finalAckTime (0.0), // ***

    m_dequeuePackets (0), // ***
    m_dequeueTime (0.0), // ***

    m_numPackets (2), // ***

    // LogLevelAll Logging
    m_myLogging (false), // ***

    m_beaconStopping (false), // !!!
    m_beaconStopCount (10), // !!!
    m_beaconFlag (0), // !!!
    m_countStartFlag (0), // !!!
    m_countClientsI (0.0), // !!!
    m_heardNoiseFlag (1), // !!!

    m_heardOne (1), // !!!
    m_heardTwo(1), // !!!
    m_heardThree(1), // !!!
    m_heardFour(1), // ???
    m_heardFive(1), // ???
    m_heardSix(1), // ???
    m_heardSeven(1), // ???

    m_outOfThreeCount(0) // !!!

{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // myLogging only ***

  m_transmissionListener = new DcaTxop::TransmissionListener (this);
  m_dcf = new DcaTxop::Dcf (this);
  m_queue = CreateObject<WifiMacQueue> ();
  m_rng = new RealRandomStream ();
}

DcaTxop::~DcaTxop ()
{
  NS_LOG_INFO (" "); // ***

  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("-----------------------------------"); // ***

  NS_LOG_INFO ("WifiMacQueue" << m_queue); // *****
  
  NS_LOG_INFO ("Number of ACK Packets queued: " << m_ackPackets); // ***
  NS_LOG_INFO ("Number of Association Request Packets queued: " << m_assocReqPackets); // ***
  NS_LOG_INFO ("Number of Association Response Packets queued: " << m_assocRespPackets); // ***
  NS_LOG_INFO ("Number of CTS Packets queued: " << m_ctsPackets); // ***
  NS_LOG_INFO ("Number of RTS Packets queued: " << m_rtsPackets); // ***
  NS_LOG_INFO ("Number of DATA Packets queued: " << m_dataPackets); // ***  

  NS_LOG_INFO (" "); // ***

  NS_LOG_INFO ("Times device got a CTS: " << m_dcaGotCtsCount); // ***
  NS_LOG_INFO ("Times device missed a CTS: " << m_dcaMissedCtsCount); // ***

  NS_LOG_INFO (" "); // ***

  NS_LOG_INFO ("Times device got an ACK: " << m_dcaGotAckCount); // ***
  NS_LOG_INFO ("Times device missed an ACK & Failed: " << m_dcaMissedAckFailCount); // ***
  NS_LOG_INFO ("Times device missed an ACK & Retransmitted (Last Data Packet): " << m_dcaMissedAckRetransmitCount); // ***
  NS_LOG_INFO ("Missed ACK time: " << m_totalAckTime); // ***

  NS_LOG_INFO (" "); // ***

  NS_LOG_INFO ("Dequeued Packet at: " << m_dequeueTime); // ***
  NS_LOG_INFO ("Got ACK for Packet at: " << m_finalAckTime); // ***
  NS_LOG_INFO ("Packet time: " << m_finalAckTime-m_dequeueTime); // ***

  NS_LOG_INFO (" "); // ***

  NS_LOG_INFO ("-----------------------------------"); // ***
}

void
DcaTxop::DoDispose (void)
{
  
  NS_LOG_FUNCTION (this);

  m_queue = 0;
  m_low = 0;
  m_stationManager = 0;
  delete m_transmissionListener;
  delete m_dcf;
  delete m_rng;
  m_transmissionListener = 0;
  m_dcf = 0;
  m_rng = 0;
  m_txMiddle = 0;
}

// Sets the DcfManager used by this DcaTxop.
void
DcaTxop::SetManager (DcfManager *manager)
{
  //if(m_myLogging){NS_LOG_FUNCTION (this << manager);} // myLogging only ***

  // Sets the internal manager variable
  m_manager = manager;

  // The m_dcf represents this DcaTxop (which is the "state")
  m_manager->Add (m_dcf);
}

// Sets the MacTxMiddle used by this DcaTxop.
// MacTxMiddle handles sequence numbering for 802.11 data frames.
void DcaTxop::SetTxMiddle (MacTxMiddle *txMiddle)
{
  //if(m_myLogging){NS_LOG_FUNCTION (this << txMiddle);} // ***

  m_txMiddle = txMiddle;
}

// Sets the MacLow layer used by this DcaTxop.
// MacLow handles RTS/CTS/DATA/ACK transactions.
void
DcaTxop::SetLow (Ptr<MacLow> low)
{
  //if(m_myLogging){NS_LOG_FUNCTION (this << low);} // myLogging only ***

  m_low = low;
}

// Sets the WifiRemoteStationManager used by this DcaTxop.
// This holds a list of "per-remote-station state"
void
DcaTxop::SetWifiRemoteStationManager (Ptr<WifiRemoteStationManager> remoteManager)
{
  //if(m_myLogging){NS_LOG_FUNCTION (this << remoteManager);} // myLogging only ***

  m_stationManager = remoteManager;
}

// Sets the callback to invoke when a packet 
// transmission is completed successfully.
void
DcaTxop::SetTxOkCallback (TxOk callback)
{
  //if(m_myLogging){NS_LOG_FUNCTION (this << &callback);} // myLogging only ***

  m_txOkCallback = callback;
}

// Sets the callback to invoke when a packet transimission fails.
void
DcaTxop::SetTxFailedCallback (TxFailed callback)
{
  //if(m_myLogging){NS_LOG_FUNCTION (this << &callback);} // myLogging only

  m_txFailedCallback = callback;
}

// Returns the WifiMacQueue of this DcaTxop
Ptr<WifiMacQueue >
DcaTxop::GetQueue () const
{
  //if(m_myLogging){NS_LOG_FUNCTION (this);} // myLogging only ***

  return m_queue;
}

// Sets the Min CW for the state
void
DcaTxop::SetMinCw (uint32_t minCw)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << minCw);} // myLogging only ***

  if(m_beaconStopping && m_countStartFlag == 0) // !!!
  { // !!!

    m_countStartFlag = 1;

    Time startTime = Time("10us");

    m_countClientStart = Simulator::Schedule (startTime, &DcaTxop::CountClients, this);
  } // !!!

  // Calls DcfState::SetCwMin
  m_dcf->SetCwMin (minCw);
}

// Sets the Max CW for the state
void
DcaTxop::SetMaxCw (uint32_t maxCw)
{
  //if(m_myLogging){NS_LOG_FUNCTION (this << maxCw);} // myLogging only ***

  // Calls DcfState::SetCwMax
  m_dcf->SetCwMax (maxCw);
}

// Sets the AIFSN for the state
void
DcaTxop::SetAifsn (uint32_t aifsn)
{
  //if(m_myLogging){NS_LOG_FUNCTION (this << aifsn);} // myLogging only ***

  // Calls DcfSTate::SetAifsn
  m_dcf->SetAifsn (aifsn);
}

// Returns the states Min CW
uint32_t
DcaTxop::GetMinCw (void) const
{
  //if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  return m_dcf->GetCwMin ();
}

// Returns the states Max CW
uint32_t
DcaTxop::GetMaxCw (void) const
{
  //if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  return m_dcf->GetCwMax ();
}

// Returns the states AIFSN
uint32_t
DcaTxop::GetAifsn (void) const
{
  //if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  return m_dcf->GetAifsn ();
}

// Store the given packet in the internal queue until it can be sent safely.
void
DcaTxop::Queue (Ptr<const Packet> packet, const WifiMacHeader &hdr)
{
  //if(m_myLogging){NS_LOG_FUNCTION (this << packet << &hdr);} // only myLogging ***


  // The WifiMacHeader contains most of the information about the packet being queued.
  // This code is used to see what kind of packets are being queued.

  // Header Packet Logging

  if(hdr.IsAck()){m_ackPackets++;} // ***
  if(hdr.IsAssocReq()){m_assocReqPackets++;} // ***
  if(hdr.IsAssocResp()){m_assocRespPackets++;} // ***
  if(hdr.IsCts()){m_ctsPackets++;} // ***
  if(hdr.IsRts()){m_rtsPackets++;} // ***

  if(hdr.IsData()){m_dataPackets++;} // ***

  if(m_myQueueLogging){ // ***

    std::cout << Simulator::Now().GetMicroSeconds() << "us [DcaTxop-Queue] (" << this << ")  "; // ***     
    
    // Type

    std::cout << "Type:" << hdr.GetTypeString() << "  "; // ***

    // QoS
    if(hdr.IsQosData()){
        if(hdr.IsQosAck()){std::cout << "QoS:NormalAck  ";} // ***
        if(hdr.IsQosBlockAck()){std::cout << "QoS:BlockAck  ";} // ***
        if(hdr.IsQosNoAck()){std::cout << "QoS:NoAck  ";} // ***
        if(hdr.IsQosAmsdu()){std::cout << "A-MSDU:1  ";} // ***
        else{std::cout << "A-MSDU:0  ";} // ***
        if(hdr.IsQosEosp()){std::cout << "EOSP:1  ";} // ***
        else{std::cout << "EOSP:0  ";} // ***
    }
    else{std::cout << "QoS:None  ";} // ***

    // Bits

    int addrId = 0;

    if(hdr.IsToDs()){ // ***
        std::cout << "ToDS:1  "; // ***
        if(hdr.IsFromDs()){ // ***
            std::cout <<"FromDs:1  "; // ***
            addrId = 1; // *** [[ToDS:1  FromDS:1 - 1]] ***
        }
        else{std::cout << "FromDS:0  "; addrId = 2;} // *** [[ToDs:1  FromDs:0 - 2]] ***
    } // ***

    if(!hdr.IsToDs()){  // ***
        std::cout << "ToDS:0  "; // ***
        if(hdr.IsFromDs()){ // ***
            std::cout <<"FromDs:1  "; // ***
            addrId = 3; // *** [[ToDS:0  FromDs:1 - 3]] ***
        }
        else{std::cout << "FromDS:0  "; addrId = 4;} // *** [[ToDs:0  FromDS:0 - 4]] ***
    } // ***

    if(hdr.IsMoreFragments()){std::cout << "MoreFrags:1  ";} // ***
    else{std::cout << "MoreFrags:0  ";} // ***
    if(hdr.IsRetry()){std::cout << "Retry:1  ";} // ***
    else{std::cout << "Retry:0  ";} // ***

    // Size & Seq Num

    std::cout << "PcktSize:" << packet->GetSize() << "  SeqNum:" << hdr.GetSequenceNumber() << "  Uid:" << packet->GetUid(); // ***
    std::cout << std::endl; // ***
  
    if(addrId==1){std::cout << "               --  Destination:" << hdr.GetAddr3() << "  Source:" << hdr.GetAddr4();  // ***
                  std::cout << "  BSSID:" << hdr.GetAddr1() << std::endl;} // ***
    if(addrId==2){std::cout << "               --  Destination:" << hdr.GetAddr3() << "  Source:" << hdr.GetAddr2();  // ***
                  std::cout << "  BSSID:" << hdr.GetAddr3() << std::endl;} // ***
    if(addrId==3){std::cout << "               --  Destination:" << hdr.GetAddr1() << "  Source:" << hdr.GetAddr3();  // ***
                  std::cout << "  BSSID:" << hdr.GetAddr2() << std::endl;} // ***
    if(addrId==4){std::cout << "               --  Destination:" << hdr.GetAddr1() << "  Source:" << hdr.GetAddr2();  // ***
                  std::cout << "  BSSID:" << hdr.GetAddr3() << std::endl;} // ***

  } // *** End of m_myQueueLogging;

  // For m_beaconStopCount number of Beacon frames, don't add them to the queue. !!!
  // Instead, 'discard' them until the count has reached zero, this will allow the !!!
  // clients to run through their counting algorithm during this time without any !!!
  // packets being sent accross the channel. !!!

  if(hdr.IsBeacon() && m_beaconStopping && m_beaconStopCount != 0) // !!!
  { // !!!

    m_beaconFlag = 1;
    m_beaconStopCount-=1; // !!!

    // Not counting number of clients sending if we're an AP.
    m_countClientStart.Cancel(); // !!!

    if(m_myQueueLogging){std::cout << "STOPPING BEACON: Remaining that will be stopped = " << m_beaconStopCount << std::endl;} // !!!
  } // !!!

  if(!m_beaconStopping || m_beaconStopCount==0 || m_beaconFlag == 0){ // !!!

  WifiMacTrailer fcs;

  // Preparing to queue the packet (Addr1 is the remote address, where it came from)
  m_stationManager->PrepareForQueue (hdr.GetAddr1 (), &hdr, packet);

  //if(m_myLogging){NS_LOG_INFO ("Enqueueing packet and header to WifiMacQueue");} // ***

  m_queue->Enqueue (packet, hdr);

  // This function requests access from the DcfManager if it is needed.
  StartAccessIfNeeded ();
 
  } // end of if(!m_beaconStopping ... )!!!
  
}

// Assign a fixed random variable stream number to the random variables used by 
// this model.  Return the number of streams (possibly zero) that have been assigned.
int64_t
DcaTxop::AssignStreams (int64_t stream)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << stream);} // only myLogging

  m_rng->AssignStreams (stream);

  return 1;
}

// Restart access request if needed.
void
DcaTxop::RestartAccessIfNeeded (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging

  // If there is currently a packet OR the WifiMacQueue isn't empty 
  // AND the DcaTxop hasn't requested access to the channel
  if ((m_currentPacket != 0
       || !m_queue->IsEmpty ())
      && !m_dcf->IsAccessRequested ())
    {
      if(m_myLogging){NS_LOG_INFO ("Manager is requesting access...");} // ***

      // Have the manager request access for the state.
  
      m_manager->RequestAccess (m_dcf);
    }
}

// Request access to the DCF from DCF manager if needed.
void
DcaTxop::StartAccessIfNeeded (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  // If there is not a current packet
  // AND the WifiMacQueue isn't empty
  // AND the DcaTxop hasn't requested access to the channel
  if (m_currentPacket == 0
      && !m_queue->IsEmpty ()
      && !m_dcf->IsAccessRequested ())
    {
      if(m_myLogging){NS_LOG_INFO ("Manager is requesting access...");} // ***

      // Have the manager request access for the state.
      m_manager->RequestAccess (m_dcf);
    }
}

// Return the MacLow layer associated with this DcaTxop
Ptr<MacLow>
DcaTxop::Low (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  return m_low;
}

// Initialize the DcaTxop
void
DcaTxop::DoInitialize ()
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // myLogging only ***
 
  if(m_myLogging){std::cout << "DcaTxop:(" << this << ")  MacLow:(" << m_low << ")" << std::endl;} // ***

  // Resets the CW back to the Min CW
  m_dcf->ResetCw (1); // "1" added for logging (m_cwResetCountDoInitialize) ***

  // Starts backoff with a random number of slots between 0 and the current CW.
  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()), 1); // "1" added for logging (m_startBackoffCountDcaDoInitialize) ***

  ns3::Dcf::DoInitialize ();
}

// Check if RTS should be re-transmitted if CTS was missed. 
// Return true if RTS should be re-transmitted, false otherwise.
bool
DcaTxop::NeedRtsRetransmission (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***
  
  // Returns true if we want to restart a failed RTS/CTS handshake, false otherwise.
  return m_stationManager->NeedRtsRetransmission (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                                  m_currentPacket);
}

// Check if DATA should be re-transmitted if ACK was missed. 
// Return true if DATA should be re-transmitted, false otherwise.
bool
DcaTxop::NeedDataRetransmission (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  // Returns true if we want to resend a packet after a failed transmission attempt, false otherwise.
  return m_stationManager->NeedDataRetransmission (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                                   m_currentPacket);
}

// Check if the current packet should be fragmented.
// Return true if the current packet should be fragmented, false otherwise.
bool
DcaTxop::NeedFragmentation (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  // Returns true if this packet should be fragmented, false otherwise.
  return m_stationManager->NeedFragmentation (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                              m_currentPacket);
}

// Continue to the next fragment. This method simply increments the 
// internal variable that keep trackof the current fragment number.
void
DcaTxop::NextFragment (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  m_fragmentNumber++;
}

// Get the size of the fragment.
uint32_t
DcaTxop::GetFragmentSize (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  // Returns the size of the corresponding fragment.
  return m_stationManager->GetFragmentSize (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                            m_currentPacket, m_fragmentNumber);

}

// Check if the current fragment is the last fragment.
bool
DcaTxop::IsLastFragment (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging *** 

  // Returns true if this is the last fragment, false otherwise.
  return m_stationManager->IsLastFragment (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                           m_currentPacket, m_fragmentNumber);
}

// Get the size of the next fragment.
uint32_t
DcaTxop::GetNextFragmentSize (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***
  
  // Returns the size of the corresponding fragment (by incrementing m_fragmentNumber). 
  return m_stationManager->GetFragmentSize (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                            m_currentPacket, m_fragmentNumber + 1);
}

// Get the offset of this fragment.
uint32_t
DcaTxop::GetFragmentOffset (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  // Returns the offset within the original packet where this fragment starts.
  return m_stationManager->GetFragmentOffset (m_currentHdr.GetAddr1 (), &m_currentHdr,
                                              m_currentPacket, m_fragmentNumber);
}

// Get the next fragment from the packet with appropriate Wifi header for the fragment.
Ptr<Packet>
DcaTxop::GetFragmentPacket (WifiMacHeader *hdr)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << hdr);} // only myLogging ***

  *hdr = m_currentHdr;

  // Set the current Fragment Number as the current Headers fragment number
  hdr->SetFragmentNumber (m_fragmentNumber);

  uint32_t startOffset = GetFragmentOffset ();

  // Creates a packet for the fragment
  Ptr<Packet> fragment;

  // If it's the last fragment, there are no more
  if (IsLastFragment ())
    {
      hdr->SetNoMoreFragments ();
    }
  // Else, there are more fragments
  else
    {
      hdr->SetMoreFragments ();
    }

  // Using the Offset and the fragment size, create a fragment of the packet
  fragment = m_currentPacket->CreateFragment (startOffset,
                                              GetFragmentSize ());

  // Return the fragment packet
  return fragment;
}

// Check if the DCF requires access.
// Return true if the DCF requires access, false otherwise
bool
DcaTxop::NeedsAccess (void) const
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  return !m_queue->IsEmpty () || m_currentPacket != 0;
}

void // !!!
DcaTxop::CountClients (void) // !!!
{

   // m_outOfThreeCount

   if(m_heardNoiseFlag == 0)
   {
        if(m_outOfThreeCount==1){m_heardOne = 0;}
        if(m_outOfThreeCount==2){m_heardTwo = 0;}
        if(m_outOfThreeCount==3){m_heardThree = 0;}
        if(m_outOfThreeCount==4){m_heardFour = 0;}
        if(m_outOfThreeCount==5){m_heardFive = 0;}
   } 

   // m_outOfThreeCount==3
   if(m_outOfThreeCount==3)
   {
        // one + two + three < 2
        if((m_heardOne+m_heardTwo+m_heardThree+m_heardFour+m_heardFive)<3)
        {
            int estClients = pow(2.0, m_countClientsI);
            uint32_t cwEst = static_cast<uint32_t>(estClients); 

            m_dcf->ResetCw(100+cwEst);
            m_manager->NotCountingClients();

            return;
        }

        m_countClientsI+=1.0;
   }

   switch(m_outOfThreeCount) {
        case 0: m_outOfThreeCount = 1;
                break;
        case 1: m_outOfThreeCount = 2;
                break;
        case 2: m_outOfThreeCount = 3;
                break;
        case 3: m_outOfThreeCount = 4; // back to 1 originally
                break;
        case 4: m_outOfThreeCount = 5;
                break;
        case 5: m_outOfThreeCount = 1;

   }


   // Randomly choose a number in 2^i which will be used to determine if a packet will be sent
   int sendProb = m_rng->GetNext (1, int(pow(2.0, m_countClientsI)));

   if(sendProb == 1){ // !!!
      const Packet noise = Packet(); // !!!
      Ptr<const Packet> noisePtr = &noise; // !!!

      WifiMacHeader noiseHdr; // !!!

      noiseHdr.SetAction(); // !!!

      MacLowTransmissionParameters noiseParams; // !!!
      noiseParams.DisableAck(); // !!!
      noiseParams.DisableNextData(); // !!!
      noiseParams.DisableRts(); // !!!

      Low ()->StartTransmission (noisePtr, &noiseHdr, noiseParams, m_transmissionListener); // !!!

      m_heardNoiseFlag = 1;

      if(m_outOfThreeCount==1){m_heardOne = 1;}
      if(m_outOfThreeCount==2){m_heardTwo = 1;}
      if(m_outOfThreeCount==3){m_heardThree = 1;}  
      if(m_outOfThreeCount==4){m_heardFour = 1;}
      if(m_outOfThreeCount==5){m_heardFive = 1;}    
   } 

   else{ 
      m_heardNoiseFlag = 0;
    
      if(m_outOfThreeCount==1){m_heardOne = 0;}
      if(m_outOfThreeCount==2){m_heardTwo = 0;}
      if(m_outOfThreeCount==3){m_heardThree = 0;}  
      if(m_outOfThreeCount==4){m_heardFour = 0;}
      if(m_outOfThreeCount==5){m_heardFive = 0;} 
   }

   Time startTime = Time("35us"); 

   m_countClientStart = Simulator::Schedule (startTime, &DcaTxop::CountClients, this); 

} // !!!

// Notify the DCF that access has been granted.
void
DcaTxop::NotifyAccessGranted (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  // If there is no current packet
  if (m_currentPacket == 0)
    {

      if (m_queue->IsEmpty ())
        {
          // If the queue is empty then there is nothing to send. ***

          if(m_myLogging){NS_LOG_DEBUG ("queue empty");} // only myLogging ***

          return;
        }

      // Dequeues the packet at the front of the queue as the Current Packet
      m_currentPacket = m_queue->Dequeue (&m_currentHdr);

      m_dequeuePackets += 1; // ***

      // 3rd Packet is when we start dealing with actual "Data" packets
      // 4th Packet is for "single" packet; after ARP request is finsihed
      if(Simulator::Now().GetMicroSeconds() == 60000000){m_dequeueTime=Simulator::Now().GetMicroSeconds();} // ????
          
      NS_ASSERT (m_currentPacket != 0);

      // Using the MacTxMiddle, get the sequence number using the Current Header
      uint16_t sequence = m_txMiddle->GetNextSequenceNumberfor (&m_currentHdr);

      // Set the Headers Sequence Number
      m_currentHdr.SetSequenceNumber (sequence);
      m_stationManager->UpdateFragmentationThreshold ();

      // Set the Headers fragment number to 0
      m_currentHdr.SetFragmentNumber (0);

      // Unset the More Fragment bit in the Frame Control Field
      m_currentHdr.SetNoMoreFragments ();
      
      // Unset the Retry bit in the Frame Control Field
      m_currentHdr.SetNoRetry ();

      // Set the fragment number back to 0
      m_fragmentNumber = 0;

      if(m_myDequeueLogging){ // ***
        
        std::cout << Simulator::Now().GetMicroSeconds() << "us [DcaTxop-Dequeue] (" << this << ")"; // ***
        std::cout << "  Type:" << m_currentHdr.GetTypeString(); // ***
        std::cout << "  PckSize:" << m_currentPacket->GetSize(); // ***
        std::cout << "  SeqNum:" << m_currentHdr.GetSequenceNumber(); // ***
        std::cout << "  Uid:" << m_currentPacket->GetUid(); // *** 
        std::cout << std::endl; // ***
 
      } // End of m_myDequeueLogging ***

      if(m_myLogging){NS_LOG_DEBUG ("dequeued size=" << m_currentPacket->GetSize () <<
                    ", to=" << m_currentHdr.GetAddr1 () <<
                    ", seq=" << m_currentHdr.GetSequenceNumber ());} // only myLogging ***
    }

  // This params variable controls how a pacet is transmitted
  MacLowTransmissionParameters params;
  
  // Do not force the duration/id field of the packet: its value is 
  // automatically calculated by the MacLow before calling WifiPhy::Send.
  params.DisableOverrideDurationId ();

  // If the remote address' Group bit is set
  if (m_currentHdr.GetAddr1 ().IsGroup ())
    {
      // Don't send RTS and wait for CTS before sending data
      params.DisableRts ();
      // Do not wait for ACK after data transmission
      params.DisableAck ();
      // Do not attempt to send data burst after current transmission.
      params.DisableNextData ();

      if(m_myLogging){NS_LOG_INFO ("Starting packet transmission to MacLow layer");} // ***

      if(m_myLowLogging){ // ***

        std::cout << Simulator::Now().GetMicroSeconds() << "us [DcaTxop-LOW Tx Broadcast] (" << this << ")"; // ***
        std::cout << "  Type:" << m_currentHdr.GetTypeString(); // ***

        if(m_currentHdr.IsRetry()){std::cout << "  Retry:1 ";} // ***
        else{std::cout << "  Retry:0";} // ***

        std::cout << "  PcktSize:" << m_currentPacket->GetSize(); // ***
        std::cout << "  SeqNum:" << m_currentHdr.GetSequenceNumber(); // ***
        std::cout << "  Uid:" << m_currentPacket->GetUid(); // *** 

        std::cout << "  -- "; // ***

        if(params.MustSendRts()){std::cout << "Must send RTS/CTS;";} // ***
        if(params.MustWaitAck()){std::cout << "Must wait for ACK;";} // ***
        if(params.MustWaitBasicBlockAck()){std::cout << "Block ACK mechanism used;";} // ***
        if(params.MustWaitCompressedBlockAck()){std::cout << "Compressed block ACK mechanism used;";} // ***
        if(params.MustWaitFastAck()){std::cout << "Fast ACK protocol;";} // ***
        if(params.MustWaitNormalAck()){std::cout << "Normal ACK protocol;";} // ***
        if(params.MustWaitSuperFastAck()){std::cout << "Super fast ACK protocol;";} // ***
    
        std::cout << std::endl; // ***

      } // End of myLowLogging ***

      Low ()->StartTransmission (m_currentPacket,
                                 &m_currentHdr,
                                 params,
                                 m_transmissionListener);

      if(m_myLogging){NS_LOG_DEBUG ("tx broadcast");} // only myLogging ***
    }

  else
    {
      // Wait ACKtimeout for an ACK
      params.EnableAck ();

      // If fragmentation is needed for the packet
      if (NeedFragmentation ())
        {
          // Creates a header and gets a fragment of the packet
          WifiMacHeader hdr;
          Ptr<Packet> fragment = GetFragmentPacket (&hdr);

          // If it is the last fragment
          if (IsLastFragment ())
            {
              if(m_myLogging){NS_LOG_DEBUG ("fragmenting last fragment size=" << fragment->GetSize ());} // only myLogging ***
              params.DisableNextData ();
            }
          // Else (if it is NOT the last fragment)
          else
            {
              if(m_myLogging){NS_LOG_DEBUG ("fragmenting size=" << fragment->GetSize ());} // only myLogging ***
              params.EnableNextData (GetNextFragmentSize ());
            }
    
          // Start the transmission of the fragment to the MacLow layer
          if(m_myLogging){NS_LOG_INFO ("Starting fragment transmission to MacLow layer");} // ***

          if(m_myLowLogging){ // ***

            std::cout << Simulator::Now().GetMicroSeconds() << "us [DcaTxop-LOW Fragment] (" << this << ")"; // ***
            std::cout << "  Type:" << hdr.GetTypeString(); // ***

            if(hdr.IsRetry()){std::cout << "  Retry:1";} // ***
            else{std::cout << "  Retry:0";} // ***

            std::cout << "  PcktSize:" << fragment->GetSize(); // ***
            std::cout << "  SeqNum:" << hdr.GetSequenceNumber(); // ***
            std::cout << "  Uid:" << fragment->GetUid(); // ***

            std::cout << "  -- "; // ***

            if(params.MustSendRts()){std::cout << "Must send RTS/CTS;";} // ***
            if(params.MustWaitAck()){std::cout << "Must wait for ACK;";} // ***
            if(params.MustWaitBasicBlockAck()){std::cout << "Block ACK mechanism used;";} // ***
            if(params.MustWaitCompressedBlockAck()){std::cout << "Compressed block ACK mechanism used;";} // ***
            if(params.MustWaitFastAck()){std::cout << "Fast ACK protocol;";} // ***
            if(params.MustWaitNormalAck()){std::cout << "Normal ACK protocol;";} // ***
            if(params.MustWaitSuperFastAck()){std::cout << "Super fast ACK protocol;";} // ***

            std::cout << std::endl; // ***

          } // End of myLowLogging ***
   
          Low ()->StartTransmission (fragment, &hdr, params,
                                     m_transmissionListener);


        }

      // Else (If fragmentation is NOT needed)
      else
        {
          params.DisableNextData ();

          // Start the transmission of the packet to the MacLow layer
          if(m_myLogging){NS_LOG_INFO ("Starting packet transmission to MacLow layer");} // ***

          if(m_myLowLogging){ // ***

            std::cout << Simulator::Now().GetMicroSeconds() << "us [DcaTxop-LOW] (" << this << ")"; // ***
            std::cout << "  Type:" << m_currentHdr.GetTypeString(); // ***

            if(m_currentHdr.IsRetry()){std::cout << "  Retry:1";} // ***
            else{std::cout << "  Retry:0";} // ***

            std::cout << "  PcktSize:" << m_currentPacket->GetSize(); // ***
            std::cout << "  SeqNum:" << m_currentHdr.GetSequenceNumber(); // ***
            std::cout << "  Uid:" << m_currentPacket->GetUid(); // ***

            std::cout << "  -- "; // ***

            if(params.MustSendRts()){std::cout << "Must send RTS/CTS;";} // ***
            if(params.MustWaitAck()){std::cout << "Must wait for ACK;";} // ***
            if(params.MustWaitBasicBlockAck()){std::cout << "Block ACK mechanism used;";} // ***
            if(params.MustWaitCompressedBlockAck()){std::cout << "Compressed block ACK mechanism used;";} // ***
            if(params.MustWaitFastAck()){std::cout << "Fast ACK protocol;";} // ***
            if(params.MustWaitNormalAck()){std::cout << "Normal ACK protocol;";} // ***
            if(params.MustWaitSuperFastAck()){std::cout << "Super fast ACK protocol;";} // ***

            std::cout << std::endl; // ***

          } // *** End of myLowLogging ***

          // *** We know explictely the packet will be dequeued at this time ***
          if(Simulator::Now().GetMicroSeconds() >= 60000000) // ***
          { 
            m_missedAckTime = Simulator::Now().GetMicroSeconds(); // ***   
          }

          Low ()->StartTransmission (m_currentPacket, &m_currentHdr,
                                     params, m_transmissionListener);

        }
    }
}

// Notify the DCF that an internal collision has occurred.
void
DcaTxop::NotifyInternalCollision (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  NotifyCollision ();
}

// Notify the DCF that a collision has occurred.
void
DcaTxop::NotifyCollision (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  if(m_myLogging){NS_LOG_DEBUG ("collision");} // only myLogging ***
  
  // Starts backoff with a random number of slots between 0 and the current CW.
  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()), 2); // "2" added for logging (m_startBackoffCountDcaDoInitialize) ***

  // Request access to the channel if needed
  RestartAccessIfNeeded ();
}

// When channel switching occurs, enqueued packets are removed.
void
DcaTxop::NotifyChannelSwitching (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  // Flush the packets from the queue
  m_queue->Flush ();

  // Unset the current packet
  m_currentPacket = 0;
}

// When sleep operation occurs, if there is a pending packet transmission,
// it will be reinserted to the front of the queue.
void
DcaTxop::NotifySleep (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  // If there is a pending packet
  if (m_currentPacket != 0)
    {
      // Push the packet and it's header to the front of the queue
      m_queue->PushFront (m_currentPacket, m_currentHdr);

      // Unset the current packet
      m_currentPacket = 0;
    }
}

// When the device has woken up
// !!!  
// !!!  MODIFIED FOR COUNTING CLIENTS
void
DcaTxop::NotifyWakeUp (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  m_heardNoiseFlag = 1; // !!!

  if(m_outOfThreeCount==1){m_heardOne = 1;}
  if(m_outOfThreeCount==2){m_heardTwo = 1;}
  if(m_outOfThreeCount==3){m_heardThree = 1;} 
  if(m_outOfThreeCount==4){m_heardFour = 1;}
  if(m_outOfThreeCount==5){m_heardFive = 1;}     

  // Request access to the channel if needed

  // *** DO NOT REMOVE BELLOW ***
  // *** JUST REMOVE COMMENT  ***

  //RestartAccessIfNeeded (); !!!
}

// Event handler when a CTS is received.
void
DcaTxop::GotCts (double snr, WifiMode txMode)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << snr << txMode);} // only myLogging ***

  m_dcaGotCtsCount++; // ***
}

// Event handler when a CTS timeout has occurred.
void
DcaTxop::MissedCts (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***
  
  m_dcaMissedCtsCount++; // ***

  // This returns true if we want to restart a failed RTS/CTS handshake, false otherwise
  // So, if we DONT want to restart a RTS/CTS handshake (i.e. timed out/failed)
  if (!NeedRtsRetransmission ())
    {
      if(m_myLogging){NS_LOG_DEBUG ("Cts Fail");} // only myLogging ***

      // Tell the station manager, the RTS failed for this packet
      m_stationManager->ReportFinalRtsFailed (m_currentHdr.GetAddr1 (), &m_currentHdr);

      if (!m_txFailedCallback.IsNull ())
        {
          m_txFailedCallback (m_currentHdr);
        }

      // Unset the current packet.
      m_currentPacket = 0;
      
      // Set the Current CW back to the Min CW
      m_dcf->ResetCw (2); // "2" added for logging (m_cwResetCountMissedCts) ***
    }
  
  // Else (We DO want to restart a RTS/CTS handshake)
  else
    {
      // Update the CW (Run backoff)
      m_dcf->UpdateFailedCw (1); // "1" added for logging (m_cwFailedCountDcaMissedCts) ***
    }
  
  // Start backoff at a random number of slots between 0 and the Current CW
  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()), 3); // "3" added for logging (m_startBackoffCountDcaMissedCts) ***
  
  // Request access to the channel if it is needed
  RestartAccessIfNeeded ();
}

// Event handler when an ACK is received.
void
DcaTxop::GotAck (double snr, WifiMode txMode)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << snr << txMode);} // only myLogging ***
  
  m_dcaGotAckCount++; // ***

  // For NonQos.cc - ACK time of packet
  
  // *** We know explictely the packet will be dequeued at this time ***
  if(Simulator::Now().GetMicroSeconds() > 60000000) // ***
  {
    m_finalAckTime = Simulator::Now().GetMicroSeconds(); // ***
  }

  // If we don't need fragmentation
  // OR the previous fragment was the last one.
  if (!NeedFragmentation ()
      || IsLastFragment ())
    {
      if(m_myLogging){NS_LOG_DEBUG ("got ack. tx done.");} // only myLogging ***

      if(m_myAckLogging){ // ***

        std::cout << Simulator::Now().GetMicroSeconds() << "us [DcaTxop-GOT ACK: Finished] (" << this << ")  for packet: "; // ***
        std::cout << "  Type:" << m_currentHdr.GetTypeString(); // ***

        if(m_currentHdr.IsRetry()){std::cout << "  Retry:1";} // ***
        else{std::cout << "  Retry:0";} // ***

        std::cout << "  PcktSize:" << m_currentPacket->GetSize(); // ***
        std::cout << "  SeqNum:" << m_currentHdr.GetSequenceNumber(); // ***
        std::cout << "  Uid:" << m_currentPacket->GetUid(); // ***

        std::cout << std::endl; // ***

      } // End of if myAckLogging ***

      if (!m_txOkCallback.IsNull ())
        {
          m_txOkCallback (m_currentHdr);
        }

      /* we are not fragmenting or we are done fragmenting
       * so we can get rid of that packet now.
       */
      m_currentPacket = 0;

      // Transmitted successfully, reset the CW back to the Min CW
      m_dcf->ResetCw (3); // "3" added for logging (m_cwResetCountGotAck) ***

      // Start backoff at a random number of slots between 0 and the Current CW
      m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()), 4); // "4" added for logging (m_startBackoffCountDcaGotAck) ***

      // Request access to the channel if needed
      RestartAccessIfNeeded ();
    }
  // Else (We DO need fragmentation or there are MORE fragments)
  else
    {
      if(m_myLogging){NS_LOG_DEBUG ("got ack. tx not done, size=" << m_currentPacket->GetSize ());} // only myLogging ***
    }
}

// Event handler when an ACK is missed.
void
DcaTxop::MissedAck (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  std::cout << "(" << this << ") Missed ACK " << Simulator::Now().GetMicroSeconds() << std::endl; // ???

  if(Simulator::Now().GetMicroSeconds() > 60000000){  // ????
    m_missedAckTime = Simulator::Now().GetMicroSeconds() - m_missedAckTime; // ***  
    m_totalAckTime += m_missedAckTime; // ***
  }

  // If we DONT need data retransmission
  if (!NeedDataRetransmission ())
    {
      if(m_myLogging){NS_LOG_DEBUG ("Ack Fail");} // only myLogging ***

      std::cout << this << " - We don't want to retransmit packet" << std::endl;

      m_dcaMissedAckFailCount++; // ***

      // Have the station manager report an ACK faield
      m_stationManager->ReportFinalDataFailed (m_currentHdr.GetAddr1 (), &m_currentHdr);

      if (!m_txFailedCallback.IsNull ())
        {
          m_txFailedCallback (m_currentHdr);
        }

      //to reset the dcf.
      m_currentPacket = 0;
      m_dcf->ResetCw (4); // "4" added for logging (m_cwResetCountMissedAck) ***
    }
  
  // Else (We DO need data retransmission)
  else
    {
      if(m_myLogging){NS_LOG_DEBUG ("Retransmit");} // only myLogging ***

      if(Simulator::Now().GetMicroSeconds() >= 60000000){m_dcaMissedAckRetransmitCount++;} // ???
 
      // Set the Retry bit in the Frame Control Field
      m_currentHdr.SetRetry ();

      if(m_myAckLogging){ // ***

        std::cout << Simulator::Now().GetMicroSeconds() << "us [DcaTxop-MISSED ACK: Retransmit] (" << this << ")  for packet:"; // ***
        std::cout << "  Type:" << m_currentHdr.GetTypeString(); // ***

        if(m_currentHdr.IsRetry()){std::cout << "  Retry:1";} // ***
        else{std::cout << "  Retry:0";} // ***

        std::cout << "  PcktSize:" << m_currentPacket->GetSize(); // ***
        std::cout << "  SeqNum:" << m_currentHdr.GetSequenceNumber(); // ***
        std::cout << "  Uid:" << m_currentPacket->GetUid(); // ***

        std::cout << std::endl; // ***

      } // End of if myAckLogging ***

      // Update the CW (run Backoff)
      m_dcf->UpdateFailedCw (2); // "2" added for logging (m_cwFailedCountDcaMissedAck) ***
    }

  // Start backoff at a random number of slots between 0 and the Current CW
  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()), 5); // "5" added for logging (m_startBackoffCountDcaMissedAck) ***

  // Restart access to the channel if needed
  RestartAccessIfNeeded ();
}

// Start transmission for the next fragment. This is called for fragment only. ***
void
DcaTxop::StartNext (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  if(m_myLogging){NS_LOG_DEBUG ("start next packet fragment");} // only myLogging ***

  /* this callback is used only for fragments. */
  NextFragment ();
  WifiMacHeader hdr;
  Ptr<Packet> fragment = GetFragmentPacket (&hdr);
  MacLowTransmissionParameters params;
  params.EnableAck ();
  params.DisableRts ();
  params.DisableOverrideDurationId ();

  if (IsLastFragment ())
    {
      params.DisableNextData ();
    }
  else
    {
      params.EnableNextData (GetNextFragmentSize ());
    }
  
  // Start transmission of the fragment to the MacLow layer
  if(m_myLogging){NS_LOG_INFO ("Starting fragment transmission to MacLow layer");} // ***

  if(m_myLowLogging){

    std::cout << Simulator::Now().GetMicroSeconds() << "us [DcaTxop-LOW Fragment] (" << this << ")"; // ***
    std::cout << "  Type:" << hdr.GetTypeString(); // ***

    if(hdr.IsRetry()){std::cout << "  Retry:1";} // ***
    else{std::cout << "  Retry:0";} // ***

    std::cout << "  PckSize:" << fragment->GetSize(); // ***
    std::cout << "  SeqNum:" << hdr.GetSequenceNumber();  // ***
    std::cout << "  Uid:" << fragment->GetUid(); // ***

    std::cout << "  -- "; // ***

    if(params.MustSendRts()){std::cout << "Must send RTS/CTS;";} // ***
    if(params.MustWaitAck()){std::cout << "Must wait for ACK;";} // ***
    if(params.MustWaitBasicBlockAck()){std::cout << "Block ACK mechanism used;";} // ***
    if(params.MustWaitCompressedBlockAck()){std::cout << "Compressed block ACK mechanism used;";} // ***
    if(params.MustWaitFastAck()){std::cout << "Fast ACK protocol;";} // ***
    if(params.MustWaitNormalAck()){std::cout << "Normal ACK protocol;";} // ***
    if(params.MustWaitSuperFastAck()){std::cout << "Super fast ACK protocol;";} // ***

    std::cout << std::endl; // ***

  } // End of myLowLogging ***

  Low ()->StartTransmission (fragment, &hdr, params, m_transmissionListener);

}

// Cancel the transmission.
void
DcaTxop::Cancel (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  if(m_myLogging){NS_LOG_DEBUG ("transmission cancelled");} // only myLogging ***

  std::cout << this << " - Transmission Cancelled!!!" << std::endl;

  /**
   * This happens in only one case: in an AP, you have two DcaTxop:
   *   - one is used exclusively for beacons and has a high priority.
   *   - the other is used for everything else and has a normal
   *     priority.
   *
   * If the normal queue tries to send a unicast data frame, but
   * if the tx fails (ack timeout), it starts a backoff. If the beacon
   * queue gets a tx oportunity during this backoff, it will trigger
   * a call to this Cancel function.
   *
   * Since we are already doing a backoff, we will get access to
   * the medium when we can, we have nothing to do here. We just
   * ignore the cancel event and wait until we are given again a
   * tx oportunity.
   *
   * Note that this is really non-trivial because each of these
   * frames is assigned a sequence number from the same sequence
   * counter (because this is a non-802.11e device) so, the scheme
   * described here fails to ensure in-order delivery of frames
   * at the receiving side. This, however, does not matter in
   * this case because we assume that the receiving side does not
   * update its <seq,ad> tupple for packets whose destination
   * address is a broadcast address.
   */
}

// Event handler when a transmission that does not require an ACK has completed.
void
DcaTxop::EndTxNoAck (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // only myLogging ***

  if(m_myLogging){NS_LOG_DEBUG ("a transmission that did not require an ACK just finished");} // only myLogging

  // Successful transmission, so unset the current packet
  m_currentPacket = 0;
  
  // Reset the Current CW to the Min Cw
  m_dcf->ResetCw (5); // "5" added for logging (m_cwResetCountEndTxNoAck) ***

  // Start backoff at a random number of slots between 0 and the Current CW
  m_dcf->StartBackoffNow (m_rng->GetNext (0, m_dcf->GetCw ()), 6); // "6" added for logging (m_startBackoffCountDcaEndTxNoAck) ***
  
  // Request access to channel if it is needed
  StartAccessIfNeeded ();
}

} //namespace ns3
