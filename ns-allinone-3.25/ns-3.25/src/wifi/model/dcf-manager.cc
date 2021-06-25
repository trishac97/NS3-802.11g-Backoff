/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

// All code added for experiments are marked with '// ***'

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <cmath>
#include "dcf-manager.h"
#include "wifi-phy.h"
#include "wifi-mac.h"
#include "mac-low.h"

#include <math.h> // ***

#define MY_DEBUG(x) \
  NS_LOG_DEBUG (Simulator::Now () << " " << this << " " << x)

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DcfManager");

/****************************************************************
 *      Implement the DCF state holder
 ****************************************************************/

DcfState::DcfState ()
  : m_backoffSlots (0),
    m_backoffStart (Seconds (0.0)),
    m_cwMin (0),
    m_cwMax (0),
    m_cw (0),
    m_accessRequested (false),

    // EdcaTxopN::
    m_slotsCountEdca (0), // ***
    // Total
    m_slotsCountTotal (0), // ***

    m_accessRequestedCount (0), // ***
    m_accessGrantedCount (0), // ***
    m_collisionCount (0), // ***
    m_internalCollisionCount (0), // ***


    // *** This variable chooses the type of backoff to be used ***
        // 0 = BE Backoff
        // 1 = Log Backoff
        // 2 = LogLog Backoff
        // 3 = Sawtooth Backoff
        // 4 = TEB
        // 5 = QEB
        // 5 = FiveEB
    m_backoffType (2), // ***


    // Used for Sawtooth Backoff
    m_sawtoothRound (0), // ***
    m_sawtoothCount (0), // ***

    m_curSlots (0), // ***

    m_whatWasCw (0), // ***
    m_timesCwFailed (0), // ***

    m_noMoreSlots (0), // ***

    m_countingClients (false), // ***
    m_keepPredictedCw (false), // ***
    m_predictedCw (0), // ***

    m_myLogging (false) // ***
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // ***
}

DcfState::~DcfState ()
{
  if(m_slotsCountEdca==0) // *** Don't want EDCA data ***
  {

    NS_LOG_INFO (" "); // ***

    NS_LOG_FUNCTION (this); // ***

    NS_LOG_INFO ("-----------------------------------"); // ***

    NS_LOG_INFO ("Slots allocated from EdcaTxopN: " << m_slotsCountEdca); // ***
    NS_LOG_INFO ("-- TOTAL Slots allocated: " << m_slotsCountTotal); // ***

    NS_LOG_INFO (" "); // ***

    NS_LOG_INFO ("Times DcfState requested access to the channel: " << m_accessRequestedCount); // ***
    NS_LOG_INFO ("Times DcfState was granted access to the channel: " << m_accessGrantedCount); // ***

    NS_LOG_INFO (" "); // ***

    NS_LOG_INFO ("Number of collisions: " << m_collisionCount); // ***
    NS_LOG_INFO ("Number of internal collisions: " << m_internalCollisionCount); // ***

    NS_LOG_INFO (" "); // ***

    NS_LOG_INFO ("No more slots: " << m_noMoreSlots); // !!! 

    NS_LOG_INFO (" "); // ***

    NS_LOG_INFO ("-----------------------------------"); // ***

  }

}

void
DcfState::SetAifsn (uint32_t aifsn)
{  
  // Arbitration Inter Frame Spacing Number, used for EDCA
  m_aifsn = aifsn;
}


// *** Returns whether or not the State is finished counting clients, ***
// *** This is set to false after it receives it's client estimate. ***
bool 
DcfState::IsCountingClients (void) // ***
{ 
    return m_countingClients; // ***
}

void
DcfState::SetCwMin (uint32_t minCw)
{
  bool changed = (m_cwMin != minCw);
  
  // Set the Minimum CW
  m_cwMin = minCw;

  // Reset the CW if the Min CW has been changed
  if (changed == true)
    {
      ResetCw (6); // *** "6" added for logging (m_cwResetCountSetCwMin) ***
    }
}

void
DcfState::SetCwMax (uint32_t maxCw)
{
  bool changed = (m_cwMax != maxCw);

  // Set the Maximum CW
  m_cwMax = maxCw; 

  // Reset the CW if the Max CW has been changed
  if (changed == true)
    {
      ResetCw (7); // *** "7" added for logging (m_cwResetCountSetCwMax) ***
    }
}

uint32_t
DcfState::GetAifsn (void) const
{
  return m_aifsn;
}

uint32_t
DcfState::GetCwMin (void) const
{
  return m_cwMin;
}

uint32_t
DcfState::GetCwMax (void) const
{
  return m_cwMax;
}

void
DcfState::ResetCw (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // ***

  // *** For Sawtooth Backoff only ***
  m_sawtoothRound = 0; // ***
  m_sawtoothCount = 0; // ***

  // Set the Current CW to the Minimum CW
  m_cw = m_cwMin;
}

void
DcfState::ResetCw (uint32_t logVal) // *** (Variation of ResetCw for the logVal) ***
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // ***

  // *** Resetting variables for next packet ***
  // *** For Sawtooth Backoff only ***
  m_sawtoothRound = 0; // ***
  m_sawtoothCount = 0; // ***

  // Set the Current CW to the Minimum CW
  m_cw = m_cwMin;

  // *** For the Client estimate, DcaTxop sends 100 + it's estimate (100 is just as an identifier) ***
  if(logVal>100) // ***
  { 
    // *** Set the CW to this ***
    m_cw = logVal-100; // ***
    m_predictedCw = logVal-100; // ***

    // *** For going over the Max CW ***
    if(m_cw > 1021) // ***
    {
        m_cw = 1023; // ***
        m_predictedCw = 1023; // ***
    }

    std::cout << "Predicted CW as: " << m_cw << std::endl; 

    // *** We have our estimate and are no longer sending noise packets ***
    m_countingClients = false; // ***
  }

  if(m_keepPredictedCw) // ***
  {
    m_cw = m_predictedCw; // ***
  } 

}

// Update the value of the CW variable to take into account a transmission failure, ***
// Either an ACK failure of a RTS/CTS handshake failure. ***
// By default, this triggers a doubling of CW (capped by maxCW). ***
void
DcfState::UpdateFailedCw (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // ***

  //see 802.11-2012, section 9.19.2.5
  m_cw = std::min ( 2 * (m_cw + 1) - 1, m_cwMax);
  
}
void
DcfState::UpdateFailedCw (uint32_t logVal) // *** (Variation of ::UpdateFailedCw for the logVal) ***
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // ***

  if(m_backoffType==0) // *** BE Backoff ***
  {
    //see 802.11-2012, section 9.19.2.5
    m_cw = std::min ( 2 * (m_cw + 1) - 1, m_cwMax);
  }

  if(m_backoffType == 1) // *** Log Backoff ***
  {
    // The formula for logarithmic backoff can be described as
    // New CW = ((1 + 1/log2(Old CW)) * Old CW) and uses a base 2 log.
    // A base 2 log can also be described as log(Old CW)/log(2.0).
    // C++ uses log to return the NATURAL LOGARTIHM 

    double cwDouble = (double) m_cw; // (m_cw) ***
    double newCwDouble = cwDouble; // ***
    newCwDouble += 1.0; // (m_cw + 1) ***

    double newBase = (1.0 +(1.0/(log(newCwDouble)/log(2.0)))); // ***
    double logCwDouble = (newBase * (cwDouble)); // ( X * (m_cw + 1) - 1) ***

    logCwDouble = floor (logCwDouble); // *** 

    uint32_t lowCwUint = (uint32_t) logCwDouble; // ***
  
    m_cw = std::min (lowCwUint, m_cwMax); // ***

  }

  if(m_backoffType == 2) // *** LogLog Backoff ***
  {

    double cwDouble = (double) m_cw, newCwDouble = (double) m_cw; // (m_cw) ***
    newCwDouble += 1.0; // (m_cw + 1) ***

    // Problem to note: The first log-base-2 of the CW, assuming the original
    // min CW is 1, will end up being 1.  This means, taking the log-base-2 again
    // will make the CW constanly 0.  To solve this I'm just adding another 1.0

    double firstLog = (log(newCwDouble)/log(2.0)); // ***
    
    // *** To avoid a constant CW of 1 ***
    if(firstLog==1.0){firstLog += 1.0;} // ***

    double newBase = (1.0 + (1.0/(log(firstLog)/log(2.0)))); // ***
    double logCwDouble = (newBase * cwDouble);

    logCwDouble = floor (logCwDouble); // *** 

    uint32_t lowCwUint = (uint32_t) logCwDouble; // ***

    m_cw = std::min (lowCwUint, m_cwMax); // ***

  }

  if(m_backoffType == 3) // *** Sawtooth Backoff ***
  {

    if(m_sawtoothCount != 0) // ***
    {
        m_cw = std::min (m_cw/2, m_cwMax); // ***
        m_sawtoothCount -= 1; // ***
    }
    
    else // *** m_sawtoothCount DOES == 0 ***
    {
         m_sawtoothRound += 1; // ***
         m_sawtoothCount = m_sawtoothRound; // ***

         m_cw = std::min ((uint32_t) pow(2,m_sawtoothRound), m_cwMax); // ***
    }  

  }

  // *** origial code ***
  //m_cw = std::min ( 2 * (m_cw + 1) - 1, m_cwMax);
}


// Updates Backoff by subtracting the number of slots given from the States BackoffSlots. ***
// Also updates the start time with the new time given. ***
void
DcfState::UpdateBackoffSlotsNow (uint32_t nSlots, Time backoffUpdateBound)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << nSlots << backoffUpdateBound);} // ***

  m_backoffSlots -= nSlots;
  m_backoffStart = backoffUpdateBound;
  
}

// Starts Backoff for a state with the given number slots at the current simulation time. ****
void
DcfState::StartBackoffNow (uint32_t nSlots)
{

  NS_ASSERT (m_backoffSlots == 0);

  m_slotsCountTotal+=nSlots; // ***

  if(m_myLogging){MY_DEBUG (" (no log val) start backoff=" << nSlots << " slots");} // *** myLogging only ***

  m_backoffSlots = nSlots;
  m_backoffStart = Simulator::Now ();
}

void
DcfState::StartBackoffNow (uint32_t nSlots, uint32_t logVal) // *** (Variation of ::StartBackoffNow for the logVal) ***
{
  // *** Commented Out for CountingClients ***
  //NS_ASSERT (m_backoffSlots == 0); ***

  if(m_myLogging){NS_LOG_FUNCTION (this << nSlots);} // ***

  // *** Used to make sure data from EDCA states aren't shown ***
  if(logVal==99){(m_slotsCountEdca = m_slotsCountEdca + nSlots + 1);} // ***

  // Both of these are doing to same thing ***
  // m_slotCountTotal is for the entire duration ***
  (m_slotsCountTotal+=nSlots); // ***

  // *** This Could be an issue? ***
  //(m_curSlots+=nSlots+=1); // *** 

  // *** We know explictely the packet will be dequeued at this time ***
  if(Simulator::Now().GetMicroSeconds() >= 60000000) // ***
  {
    std::cout << "(" << this << ") chose " << nSlots << " slots" << std::endl; // ???

    m_noMoreSlots += nSlots; // ***
  }

  if(m_myLogging){MY_DEBUG (" (log val) start backoff=" << nSlots << " slots");} // *** myLogging only ***

  m_backoffSlots = nSlots;
  m_backoffStart = Simulator::Now ();

}

// Returns the Current CW
uint32_t
DcfState::GetCw (void) const
{
  return m_cw;
}

// Returns the current number of backoff slots
uint32_t
DcfState::GetBackoffSlots (void) const
{
  return m_backoffSlots;
}

// Returns the time at which backoff started
Time
DcfState::GetBackoffStart (void) const
{
  return m_backoffStart;
}

// Used to check if the state has requested access to the channel
bool
DcfState::IsAccessRequested (void) const
{
  return m_accessRequested;
}

// Sets that the state has requested access to the channel
void
DcfState::NotifyAccessRequested (void)
{
  m_accessRequestedCount++; // ***
  
  m_accessRequested = true;
}

// Notify a DcfState that it has been granted access to the medium.
void
DcfState::NotifyAccessGranted (void)
{
  NS_ASSERT (m_accessRequested);

  // Counting how many times access was granted
  m_accessGrantedCount++; // ***

  // Since it has been granted access, it is no longer requesting it.
  m_accessRequested = false;

  // Ivokoes DcaTxop::NotifyAccessGranted
  DoNotifyAccessGranted ();
}

// Called by DcfManager to notify a DcfState subclass that a normal collision occured.
// That is, that the medium was busy when access was requested.
// This is called from DcfManager::RequestAccess (state)
void
DcfState::NotifyCollision (void)
{ 
  // Invokes DcaTxop::NotifyCollision
  DoNotifyCollision ();
}

// Called by DcfManager to notify a DcfState subclass that an 'internal' collision occured. 
// That is, that the backoff timer of a higher priority DcfState expired at the same time
// and that access was granted to this higher priority DcfState.
// ** With DCA, since there is only one state, there should never be an internal collision. **
void
DcfState::NotifyInternalCollision (void)
{
  // Invokes DcaTxop::NotifyInternalCollision (which calls DcaTxop::NotifyCollision)
  DoNotifyInternalCollision ();
}

// Called by DcfManager to notify a DcfState subclass that a channel switching occured.
void
DcfState::NotifyChannelSwitching (void)
{
  // Invokes DcaTxop::NotifyChannelSwitching
  DoNotifyChannelSwitching ();
}

// Called by DcfManager to notify a DcfState subclass that the device has begun to sleep. ***
void
DcfState::NotifySleep (void)
{
  // Invokes DcaTxop::NotifySleep
  DoNotifySleep ();
}

// Called by DcfManager to notify a DcfState subclass that the device has begun to wake up. ***
void
DcfState::NotifyWakeUp (void)
{
  // Invokes DcaTxop::NotifyWakeUp
  DoNotifyWakeUp ();
}


/**
 * Listener for NAV events. Forwards to DcfManager
 */
class LowDcfListener : public ns3::MacLowDcfListener
{
public:
  /**
   * Create a LowDcfListener for the given DcfManager.
   *
   * \param dcf
   */
  LowDcfListener (ns3::DcfManager *dcf)
    : m_dcf (dcf)
  {
  }
  virtual ~LowDcfListener ()
  {
  }
  virtual void NavStart (Time duration)
  {
    m_dcf->NotifyNavStartNow (duration);
  }
  virtual void NavReset (Time duration)
  {
    m_dcf->NotifyNavResetNow (duration);
  }
  virtual void AckTimeoutStart (Time duration)
  {
    m_dcf->NotifyAckTimeoutStartNow (duration);
  }
  virtual void AckTimeoutReset ()
  {
    m_dcf->NotifyAckTimeoutResetNow ();
  }
  virtual void CtsTimeoutStart (Time duration)
  {
    m_dcf->NotifyCtsTimeoutStartNow (duration);
  }
  virtual void CtsTimeoutReset ()
  {
    m_dcf->NotifyCtsTimeoutResetNow ();
  }

private:
  ns3::DcfManager *m_dcf;  //!< DcfManager to forward events to
};


/**
 * Listener for PHY events. Forwards to DcfManager
 */
class PhyListener : public ns3::WifiPhyListener
{
public:
  /**
   * Create a PhyListener for the given DcfManager.
   *
   * \param dcf
   */
  PhyListener (ns3::DcfManager *dcf)
    : m_dcf (dcf)
  {
  }
  virtual ~PhyListener ()
  {
  }
  virtual void NotifyRxStart (Time duration)
  {
    m_dcf->NotifyRxStartNow (duration);
  }
  virtual void NotifyRxEndOk (void)
  {
    m_dcf->NotifyRxEndOkNow ();
  }
  virtual void NotifyRxEndError (void)
  {
    m_dcf->NotifyRxEndErrorNow ();
  }
  virtual void NotifyTxStart (Time duration, double txPowerDbm)
  {
    m_dcf->NotifyTxStartNow (duration);
  }
  virtual void NotifyMaybeCcaBusyStart (Time duration)
  {
    m_dcf->NotifyMaybeCcaBusyStartNow (duration);
  }
  virtual void NotifySwitchingStart (Time duration)
  {
    m_dcf->NotifySwitchingStartNow (duration);
  }
  virtual void NotifySleep (void)
  {
    m_dcf->NotifySleepNow ();
  }
  virtual void NotifyWakeup (void)
  {
    m_dcf->NotifyWakeupNow ();
  }

private:
  ns3::DcfManager *m_dcf;  //!< DcfManager to forward events to
};


/****************************************************************
 *      Implement the DCF manager of all DCF state holders
 ****************************************************************/

DcfManager::DcfManager ()
  : m_lastAckTimeoutEnd (MicroSeconds (0)),
    m_lastCtsTimeoutEnd (MicroSeconds (0)),
    m_lastNavStart (MicroSeconds (0)),
    m_lastNavDuration (MicroSeconds (0)),
    m_lastRxStart (MicroSeconds (0)),
    m_lastRxDuration (MicroSeconds (0)),
    m_lastRxReceivedOk (true),
    m_lastRxEnd (MicroSeconds (0)),
    m_lastTxStart (MicroSeconds (0)),
    m_lastTxDuration (MicroSeconds (0)),
    m_lastBusyStart (MicroSeconds (0)),
    m_lastBusyDuration (MicroSeconds (0)),
    m_lastSwitchingStart (MicroSeconds (0)),
    m_lastSwitchingDuration (MicroSeconds (0)),
    m_rxing (false),
    m_sleeping (false),
    m_slotTimeUs (0),
    m_sifs (Seconds (0.0)),
    m_phyListener (0),
    m_lowListener (0),

    m_manTxStartCount (0), // ***

    m_manRxStartCount (0), // ***
    m_manRxEndOkCount (0), // ***
    m_manRxEndErrorCount (0), // ***
 
    m_carrierSensing (true), // ***

    m_countingClients (false), // ***

    m_timeLogging (false), // ***
    m_myLogging (false) // ***
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***
}

DcfManager::~DcfManager ()
{
  delete m_phyListener;
  delete m_lowListener;
  m_phyListener = 0;
  m_lowListener = 0;

  NS_LOG_INFO (" "); // ***

  NS_LOG_FUNCTION (this); // ***

  NS_LOG_INFO ("-----------------------------------"); // ***
  
  NS_LOG_INFO ("Times device started TX: " << m_manTxStartCount); // ***
  NS_LOG_INFO ("Times device started RX: " << m_manRxStartCount); // ***

  NS_LOG_INFO (" "); // ***

  NS_LOG_INFO ("Times device RX ended OK: " << m_manRxEndOkCount); // ***
  NS_LOG_INFO ("Times device RX ended with an error: " << m_manRxEndErrorCount); // ***

  NS_LOG_INFO (" "); // ***

  NS_LOG_INFO ("Duration of last TX: " << m_lastTxDuration); // ***

  NS_LOG_INFO ("-----------------------------------"); // ***
  NS_LOG_INFO (" "); // ***
}

// Sets the WifiPhy listener for this DcfManager
void
DcfManager::SetupPhyListener (Ptr<WifiPhy> phy)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << phy);} // *** myLogging only ***

  // If there already exists a PHY listener, delete it
  if (m_phyListener != 0)
    {
      delete m_phyListener;
    }
  
  // Set up the new PHY listener
  m_phyListener = new PhyListener (this);
  phy->RegisterListener (m_phyListener);
}

// Removes the WifiPhy listener for this DcfManager
void
DcfManager::RemovePhyListener (Ptr<WifiPhy> phy)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << phy);} // *** myLogging only ***

  // If there exists a PHY listener
  if (m_phyListener != 0)
    {
      // Unregister the listener and delete it
      phy->UnregisterListener (m_phyListener);
      delete m_phyListener;

      m_phyListener = 0;
    }
}

// Sets up the MacLow listener for this DcfManager
void
DcfManager::SetupLowListener (Ptr<MacLow> low)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << low);} // *** myLogging only ***
  
  // If there already exists a MAC low listener, delete it
  if (m_lowListener != 0)
    {
      delete m_lowListener;
    }

  // Register the new MAC low listener
  m_lowListener = new LowDcfListener (this);
  low->RegisterDcfListener (m_lowListener);
}

// Setting the slot time the manager uses (in Microsconds)
void
DcfManager::SetSlot (Time slotTime)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << slotTime);} // *** myLogging only ***

  m_slotTimeUs = slotTime.GetMicroSeconds ();
}

// Setting the Short Interframe Sapce time the manager uses
void
DcfManager::SetSifs (Time sifs)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << sifs);} // *** myLogging only ***

  m_sifs = sifs;
}

// Sets EifsNoDifs; the duration of an EIFS minus DIFS ***
// (Extended Interframe Space, DCF Interframe Space) ***
void
DcfManager::SetEifsNoDifs (Time eifsNoDifs)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << eifsNoDifs);} // *** myLogging only ***

  m_eifsNoDifs = eifsNoDifs;
}

Time
DcfManager::GetEifsNoDifs () const
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  return m_eifsNoDifs;
}

// Adds a new DcfState to the DcfManager. The priority of the states
// is set as they are added; the first state added has the highest priority,
// the second has the second highest and so on.
// ** Again, for DCA there should only be one state **
void
DcfManager::Add (DcfState *dcf)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << dcf);} // *** myLogging only ***

  m_states.push_back (dcf);
}

// Returns the most recent time (the largest) from the times given
Time
DcfManager::MostRecent (Time a, Time b) const
{
  if(m_myLogging){NS_LOG_FUNCTION (this << a << b);} // *** myLogging only ***

  return Max (a, b);
}

Time
DcfManager::MostRecent (Time a, Time b, Time c) const
{
  if(m_myLogging){NS_LOG_FUNCTION (this << a << b << c);} // *** myLogging only ***

  Time retval;
  retval = Max (a, b);
  retval = Max (retval, c);

  return retval;
}

Time
DcfManager::MostRecent (Time a, Time b, Time c, Time d) const
{
  if(m_myLogging){NS_LOG_FUNCTION (this << a << b << c << d);} // *** myLogging only ***

  Time e = Max (a, b);
  Time f = Max (c, d);
  Time retval = Max (e, f);

  return retval;
}

Time
DcfManager::MostRecent (Time a, Time b, Time c, Time d, Time e, Time f) const
{
  if(m_myLogging){NS_LOG_FUNCTION (this << a << b << c << d << e << f);} // *** myLogging only ***

  Time g = Max (a, b);
  Time h = Max (c, d);
  Time i = Max (e, f);
  Time k = Max (g, h);
  Time retval = Max (k, i);

  return retval;
}

Time
DcfManager::MostRecent (Time a, Time b, Time c, Time d, Time e, Time f, Time g) const
{
  if(m_myLogging){NS_LOG_FUNCTION (this << a << b << c << d << e << f << g);} // *** myLogging only ***

  Time h = Max (a, b);
  Time i = Max (c, d);
  Time j = Max (e, f);
  Time k = Max (h, i);
  Time l = Max (j, g);
  Time retval = Max (k, l);

  return retval;
}

// Check if the device is busy sending or receiving or NAV busy.
bool
DcfManager::IsBusy (void) const
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  // PHY busy
  if (m_rxing)
    {
      if(m_myLogging){NS_LOG_INFO ("**BUSY**:PHY busy RXing");} // ***

      return true;
    }

  Time lastTxEnd = m_lastTxStart + m_lastTxDuration;

  // If the transmission will end after the current simulation time
  if (lastTxEnd > Simulator::Now ())
    {
      if(m_myLogging){NS_LOG_INFO ("**BUSY**: TX end after Simulator::Now");} // ***

      return true;
    }

  // NAV busy
  Time lastNavEnd = m_lastNavStart + m_lastNavDuration;
  
  if (lastNavEnd > Simulator::Now ())
    {
      if(m_myLogging){NS_LOG_INFO ("** BUSY NAV **");} // ***

      std::cout << "! ! NAV BUSY ! !" << std::endl; // ***
      
      // *** THIS IS WHERE WE TURN NAV ON ***
      return true;
    }
 

  if(m_myLogging){NS_LOG_INFO ("**NOT BUSY**");} // ***

  // If returning false, then the device was NOT busy
  return false;
}

// Notify the DcfManager that a specific DcfState needs access to the medium.
// The DcfManager is then responsible for starting an access timer and invoking 
// DcfState::DoNotifyAccessGranted when the access is granted; if it ever gets granted.
void
DcfManager::RequestAccess (DcfState *state)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << state);} // *** myLogging only ***

  //Deny access if in sleep mode
  if (m_sleeping)
    {
      if(m_myLogging){NS_LOG_INFO ("Manager is in sleep mode, deny access");} // ***

      return;
    }

  if(m_myLogging){NS_LOG_INFO ("Updating Backoff (from RequestAccess)");} // ***

  UpdateBackoff ();

  NS_ASSERT (!state->IsAccessRequested ());

  if(m_myLogging){NS_LOG_INFO ("Notifying state (" << state << ") that access is requested");} // ***

  state->NotifyAccessRequested ();

  /**
   * If there is a collision, generate a backoff
   * by notifying the collision to the user.
   */
  if (state->GetBackoffSlots () == 0
      && IsBusy ())
    {
      if(m_myLogging){MY_DEBUG ("medium is busy: collision");} // *** myLogging only ***

      /* someone else has accessed the medium.
       * generate a backoff.
       */
      state->NotifyCollision ();
    }

  if(m_myLogging){NS_LOG_INFO ("Granting access");} // ***

  // Grant access to a state
  DoGrantAccess ();

  // Schedule when DcfManager::AccessTimeout will be called
  DoRestartAccessTimeoutIfNeeded ();
}

// Grant access to the DCF. ***
void
DcfManager::DoGrantAccess (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  uint32_t k = 0;

  for (States::const_iterator i = m_states.begin (); i != m_states.end (); k++)
    {
      DcfState *state = *i;

      // If the State has requested access and its Backoff will end before the current simulation time
      if (state->IsAccessRequested ()
          && GetBackoffEndFor (state) <= Simulator::Now () )
        {
          /**
           * This is the first dcf we find with an expired backoff and which
           * needs access to the medium. i.e., it has data to send.
           */
          if(m_myLogging){NS_LOG_INFO ("Notifying state (" << state << ") that access is requested");} // *** myLogging only ***

          i++; //go to the next item in the list.
          k++;

          // Creating a vector of DcfStates for notifying internall collisions
          std::vector<DcfState *> internalCollisionStates;

          for (States::const_iterator j = i; j != m_states.end (); j++, k++)
            {
              DcfState *otherState = *j;

              // If the other states have requested access too
              if (otherState->IsAccessRequested ()
                  && GetBackoffEndFor (otherState) <= Simulator::Now ())
                {
                  if(m_myLogging){MY_DEBUG ("dcf " << k << " needs access. backoff expired. internal collision. slots=" << otherState->GetBackoffSlots ());} // *** myLogging only ***

                  /**
                   * all other dcfs with a lower priority whose backoff
                   * has expired and which needed access to the medium
                   * must be notified that we did get an internal collision.
                   */
                  internalCollisionStates.push_back (otherState);
                }
            }

          /**
           * Now, we notify all of these changes in one go. It is necessary to
           * perform first the calculations of which states are colliding and then
           * only apply the changes because applying the changes through notification
           * could change the global state of the manager, and, thus, could change
           * the result of the calculations.
           */
 
          // First, notify the first state found that it has been granted access
          state->NotifyAccessGranted ();
 
          // Itterate through the internalCollisionStates and notify each 
          // of them of the internal collision.
          for (std::vector<DcfState *>::const_iterator k = internalCollisionStates.begin ();
               k != internalCollisionStates.end (); k++)
            {
              (*k)->NotifyInternalCollision ();

            }
          break;
        }
      i++;
    }
}

// Called when access timeout should occur, e.g. backoff procedure expired.
void
DcfManager::AccessTimeout (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  if(m_myLogging){NS_LOG_INFO ("Updating backoff (from AccessTimeout)");} // ***
  // Update Backoff slots
  UpdateBackoff ();

  // Grant access to a state
  DoGrantAccess ();

  // Schedule when this function (DcfManager::AccessTimeout) will be called again
  DoRestartAccessTimeoutIfNeeded ();
}

// Access will never be granted to the medium before the time returned. ***
Time
DcfManager::GetAccessGrantStart (void) const
{
 if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  Time rxAccessStart;

  // If not receiving
  if (!m_rxing)
    {
      if(m_myLogging){NS_LOG_INFO ("Manager is not RXing");} // ***

      rxAccessStart = m_lastRxEnd + m_sifs;

      // If there as a problem with the last RX, tack on the EifsNoDifs time  
      if (!m_lastRxReceivedOk)
        {
          rxAccessStart += m_eifsNoDifs;
        }
    }
  
  // Else (it is recieving)
  else
    {
      rxAccessStart = m_lastRxStart + m_lastRxDuration + m_sifs;
    }

  Time busyAccessStart = m_lastBusyStart + m_lastBusyDuration + m_sifs;
  Time txAccessStart = m_lastTxStart + m_lastTxDuration + m_sifs;
  Time navAccessStart = m_lastNavStart + m_lastNavDuration + m_sifs;
  Time ackTimeoutAccessStart = m_lastAckTimeoutEnd + m_sifs;
  Time ctsTimeoutAccessStart = m_lastCtsTimeoutEnd + m_sifs;
  Time switchingAccessStart = m_lastSwitchingStart + m_lastSwitchingDuration + m_sifs;
  Time accessGrantedStart = MostRecent (rxAccessStart,
                                        busyAccessStart,
                                        txAccessStart,
                                        navAccessStart,
                                        ackTimeoutAccessStart,
                                        ctsTimeoutAccessStart,
                                        switchingAccessStart
                                        );
  
  if(m_myLogging){NS_LOG_INFO ("access grant start=" << accessGrantedStart <<
               ", rx access start=" << rxAccessStart <<
               ", busy access start=" << busyAccessStart <<
               ", tx access start=" << txAccessStart <<
               ", nav access start=" << navAccessStart);} // *** myLogging only ***
  
  return accessGrantedStart;
}

// Return the time when the backoff procedure started for the given DcfState. ***
Time
DcfManager::GetBackoffStartFor (DcfState *state)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << state);} // *** myLogging only ***

  Time mostRecentEvent = MostRecent (state->GetBackoffStart (),
                                     GetAccessGrantStart () + MicroSeconds (state->GetAifsn () * m_slotTimeUs));

  return mostRecentEvent;
}

// Return the time when the backoff procedure ended (or will end) for the state. ***
Time
DcfManager::GetBackoffEndFor (DcfState *state)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << state);} // ***
 
  // Calculates the end time as: Start Time + (Slots * Slot Time)
  return GetBackoffStartFor (state) + MicroSeconds (state->GetBackoffSlots () * m_slotTimeUs);
}

// *** This is called by DcfManager::NotifyRxStartNow and DcfManager::NotifyMaybeCcaBusyStartNow ***
// *** This calls NotifyWakeUp for the state which has been 'modified' to notify DcaTxop noise was heard ***
void 
DcfManager::CountClients (void) // ***
{

  uint32_t k = 0; // ***

  // *** Iterate through the states, the only one we care about is the DCA one. ***
  for (States::const_iterator i = m_states.begin (); i != m_states.end (); i++, k++) // ***
  { 

    DcfState *state = *i; // ***

    // *** If it's the DCA state and it's still counting clients, report the noise. ***
    if(!state->IsEdca () && state->IsCountingClients ()) // ***
    {
        state->NotifyWakeUp (); // ***
    } 
  }
}

// Updates backoff slots for all DcfStates. ***
void
DcfManager::UpdateBackoff (void)
{

//std::cout<<"-------------"<<m_slotTimeUs<<"-----------"<<m_sifs;
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  uint32_t k = 0;

  if(m_myLogging){NS_LOG_INFO ("Iterating through states...");} // ***
  for (States::const_iterator i = m_states.begin (); i != m_states.end (); i++, k++)
    {
      // Temporary state for each iteration
      DcfState *state = *i;

      Time backoffStart = GetBackoffStartFor (state);

      // If the backoff started before the current simulation time
      if (backoffStart <= Simulator::Now ())
        {
          uint32_t nus = (Simulator::Now () - backoffStart).GetMicroSeconds ();
          uint32_t nIntSlots = nus / m_slotTimeUs;

          /*
           * EDCA behaves slightly different to DCA. For EDCA we
           * decrement once at the slot boundary at the end of AIFS as
           * well as once at the end of each clear slot
           * thereafter. For DCA we only decrement at the end of each
           * clear slot after DIFS. We account for the extra backoff
           * by incrementing the slot count here in the case of
           * EDCA. The if statement whose body we are in has confirmed
           * that a minimum of AIFS has elapsed since last busy
           * medium.
           */
          if (state->IsEdca ())
            {
              nIntSlots++;
            }

          // Chooses between the minimum of the calcualted slots and the states current backoff slots
          // So, if it calculates 8 slots, but only 7 are left, it will go with 7.
          uint32_t n = std::min (nIntSlots, state->GetBackoffSlots ());

          if(m_myLogging){MY_DEBUG ("dcf " << k << " dec backoff slots=" << n);} // *** myLogging only ***

          // Calculates a new BackoffStart time as: Current Backoff start + (N slots * Slot Time)
          Time backoffUpdateBound = backoffStart + MicroSeconds (n * m_slotTimeUs);

          // Updates the states backoff slots with N and the new BackoffStart time
          state->UpdateBackoffSlotsNow (n, backoffUpdateBound);
        }
    }
}

// Is there a DcfState which needs to access the medium, & if there is one, ***
// how many slots for AIFS+backoff does it require? ***
void
DcfManager::DoRestartAccessTimeoutIfNeeded (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  /**
   * Is there a DcfState which needs to access the medium, and,
   * if there is one, how many slots for AIFS+backoff does it require ?
   */
  bool accessTimeoutNeeded = false;

  // Start by expecting the backoff to end at the end of the simulation
  Time expectedBackoffEnd = Simulator::GetMaximumSimulationTime ();

  for (States::const_iterator i = m_states.begin (); i != m_states.end (); i++)
    {
      // Temporary state for each iteration
      DcfState *state = *i;

      if (state->IsAccessRequested ())
        {
          if(m_myLogging){NS_LOG_INFO ("state: " << state << " has requested access");} // ***

          Time tmp = GetBackoffEndFor (state);

          // If the states backoff will end after the current Simulation time, it needs an access timeout
          if (tmp > Simulator::Now ())
            {
              accessTimeoutNeeded = true;
              expectedBackoffEnd = std::min (expectedBackoffEnd, tmp);
            }
        }
    }

  // If the state needs and access timeout
  if (accessTimeoutNeeded)
    {

      if(m_myLogging){MY_DEBUG ("expected backoff end=" << expectedBackoffEnd);} // *** myLogging only ***

      Time expectedBackoffDelay = expectedBackoffEnd - Simulator::Now ();

      // m_accessTimeout is the EventId for shceduling timeouts in the manager
      if (m_accessTimeout.IsRunning ()
          && Simulator::GetDelayLeft (m_accessTimeout) > expectedBackoffDelay)
        {

          m_accessTimeout.Cancel ();
        }

      // if the scheduled even has experied, schedule a new call to DcfManager::AccessTimeout
      if (m_accessTimeout.IsExpired ())
        {

          m_accessTimeout = Simulator::Schedule (expectedBackoffDelay,
                                                 &DcfManager::AccessTimeout, this);
        }
    }

}

// Notify the DCF that a packet reception started for the expected duration. ***
void
DcfManager::NotifyRxStartNow (Time duration)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << duration);} // *** myLogging only ***

  // Calls the function to notify the State and DcaTxop of this.
  if(m_countingClients)
  { 
    CountClients();
  } // !!!

  if(m_myLogging){MY_DEBUG ("rx start for=" << duration);} // *** myLogging only ***

  if(m_myLogging){NS_LOG_INFO ("Updating backoff (from NotifyRxStartNow)");} // ***
  UpdateBackoff ();

  // The last time a RX started was now
  m_lastRxStart = Simulator::Now ();

  // It is expected to last the given duration
  m_lastRxDuration = duration; // ****

  // The manager is currently RXing
  m_rxing = true;

  if(m_timeLogging){std::cout << Simulator::Now().GetMicroSeconds() << "s " << this << ": Rx start for " << m_lastRxDuration << std::endl;} // ***

  m_manRxStartCount++; // ***
}

// Notify the DCF that a packet reception was just completed successfully. ***
void
DcfManager::NotifyRxEndOkNow (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  if(m_myLogging){MY_DEBUG ("rx end ok");} // *** myLogging only ***

  if(m_timeLogging){std::cout << Simulator::Now().GetMicroSeconds() << "s " << this << ": Rx ended Ok " << std::endl;} // ***

  // The last time a RX ended was now
  m_lastRxEnd = Simulator::Now ();

  // It recieved a packet without issues
  m_lastRxReceivedOk = true;

  // The manager is no longer RXing
  m_rxing = false;

  m_manRxEndOkCount++; // ***
}

// Notify the DCF that a packet reception was just completed unsuccessfully. ***
void
DcfManager::NotifyRxEndErrorNow (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  if(m_myLogging){MY_DEBUG ("rx end error");} // *** myLogging only ***

  // The last time a RX ended was now
  m_lastRxEnd = Simulator::Now ();

  // There was and error with the last RX
  m_lastRxReceivedOk = false;

  // The manager is no longer RXing
  m_rxing = false;

  m_manRxEndErrorCount++; // ***
}

// Notify the DCF that a packet transmission was just started and ***
// is expected to last for the specified duration. ***
void
DcfManager::NotifyTxStartNow (Time duration)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << duration);} // *** myLogging only ***

  std::cout << "(" << this << ") " << Simulator::Now().GetMicroSeconds() << std::endl; // ???

  if(m_timeLogging){std::cout << Simulator::Now().GetMicroSeconds() << "us " << this << ": Tx start for " << duration << std::endl;} // ***

  // If the manager is RXing
  if (m_rxing)
    {

      //this may be caused only if PHY has started to receive a packet
      //inside SIFS, so, we check that lastRxStart was maximum a SIFS ago

      NS_ASSERT (Simulator::Now () - m_lastRxStart <= m_sifs); // ****      

      // The last time a RX ended was now
      m_lastRxEnd = Simulator::Now ();

      m_lastRxDuration = m_lastRxEnd - m_lastRxStart;

      // It finished RXing successfully
      m_lastRxReceivedOk = true;

      // It is no longer RXing
      m_rxing = false;
    }

  if(m_myLogging){MY_DEBUG ("tx start for " << duration);} // *** myLogging only ***

  if(m_myLogging){NS_LOG_INFO ("Updating Backoff (from NotifyTxStartNow)");} // ***
  UpdateBackoff ();

  // The last time a TX started was now
  m_lastTxStart = Simulator::Now ();
  
  // The Tx is expected to last the given duration
  m_lastTxDuration = duration;

  m_manTxStartCount++; // ***
}

// Notify the DCF that a CCA busy period has just started. ***
void
DcfManager::NotifyMaybeCcaBusyStartNow (Time duration)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << duration);} // *** myLogging only ***

  if(m_countingClients)
  {
    CountClients();
  } //!!!

  if(m_myLogging){MY_DEBUG ("busy start for " << duration);} // *** myLogging only ***

  if(m_myLogging){NS_LOG_INFO ("Updating Backoff (from NotifyMaybeCcaBusyStartNow)");} // ***
  UpdateBackoff ();

  m_lastBusyStart = Simulator::Now ();

  m_lastBusyDuration = duration;

  if(m_timeLogging){std::cout << Simulator::Now().GetMicroSeconds() << "s " << this << ": Cca busy start for " << m_lastBusyDuration << std::endl;} // ***
}

// Notify the DCF that a channel switching period has just started. ***
// During switching state, new packets can be enqueued in DcaTxop/EdcaTxop ***
// but they won't access to the medium until the end of the channel switching. ***

// ** This is a fairly complex procedure, I feel like we won't focus much on this now **
void
DcfManager::NotifySwitchingStartNow (Time duration)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << duration);} // *** myLogging only ***

  Time now = Simulator::Now ();
  NS_ASSERT (m_lastTxStart + m_lastTxDuration <= now);
  NS_ASSERT (m_lastSwitchingStart + m_lastSwitchingDuration <= now);

  if (m_rxing)
    {
      //channel switching during packet reception
      m_lastRxEnd = Simulator::Now ();
      m_lastRxDuration = m_lastRxEnd - m_lastRxStart;
      m_lastRxReceivedOk = true;
      m_rxing = false;
    }
  if (m_lastNavStart + m_lastNavDuration > now)
    {
      m_lastNavDuration = now - m_lastNavStart;
    }
  if (m_lastBusyStart + m_lastBusyDuration > now)
    {
      m_lastBusyDuration = now - m_lastBusyStart;
    }
  if (m_lastAckTimeoutEnd > now)
    {
      m_lastAckTimeoutEnd = now;
    }
  if (m_lastCtsTimeoutEnd > now)
    {
      m_lastCtsTimeoutEnd = now;
    }

  //Cancel timeout
  if (m_accessTimeout.IsRunning ())
    {
      m_accessTimeout.Cancel ();
    }

  //Reset backoffs
  for (States::iterator i = m_states.begin (); i != m_states.end (); i++)
    {
      DcfState *state = *i;
      uint32_t remainingSlots = state->GetBackoffSlots ();
      if (remainingSlots > 0)
        {
          state->UpdateBackoffSlotsNow (remainingSlots, now);
          NS_ASSERT (state->GetBackoffSlots () == 0);
        }
      state->ResetCw (8); // *** "8" added for logging (m_cwResetCountNotifySwitchingStart) ***
      state->m_accessRequested = false;
      state->NotifyChannelSwitching ();
    }

  if(m_myLogging){MY_DEBUG ("switching start for " << duration);} // *** myLogging only ***

  m_lastSwitchingStart = Simulator::Now ();
  m_lastSwitchingDuration = duration;

}

// Notify the DCF that the device has been put in sleep mode. ***
void
DcfManager::NotifySleepNow (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  // The manager is now sleeping
  m_sleeping = true;

  //Cancel timeout if the scheduler is running
  if (m_accessTimeout.IsRunning ())
    {
      m_accessTimeout.Cancel ();
    }

  //For each state notify that Sleeping has begun
  for (States::iterator i = m_states.begin (); i != m_states.end (); i++)
    {
      DcfState *state = *i;
      state->NotifySleep ();
    }
}

// Notify the DCF that the device has been resumed from sleep mode. ***
// Essentially resets and starts everything over ***
void
DcfManager::NotifyWakeupNow (void)
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  // The manager is no longer sleeping
  m_sleeping = false;

  if(m_myLogging){NS_LOG_INFO ("Iterating through states...");} // ***
  for (States::iterator i = m_states.begin (); i != m_states.end (); i++)
    {
      // Temporary state for each iteration
      DcfState *state = *i;

      uint32_t remainingSlots = state->GetBackoffSlots ();

      // If the state has any remaining slots
      if (remainingSlots > 0)
        {
          // This will make the states backoffSlots 0 and the backoffStartTime the current Sim time
          state->UpdateBackoffSlotsNow (remainingSlots, Simulator::Now ());
          NS_ASSERT (state->GetBackoffSlots () == 0);
        }
     
      // Reset the states CW
      state->ResetCw (9); // *** "9" added for logging (m_cwResetCountNotifyWakeupNow) ***

      // State hasn't requested access
      state->m_accessRequested = false;
      
      // Notify the state that the manager has been woken up
      state->NotifyWakeUp ();
    }
}

// Called at the end of RX. ***
void
DcfManager::NotifyNavResetNow (Time duration)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << duration);} // *** myLogging only ***

  if(m_myLogging){MY_DEBUG ("nav reset for=" << duration);} // *** myLogging only ***

  if(m_timeLogging){std::cout << Simulator::Now().GetMicroSeconds() << "s " << this << ": NAV reset for " << duration << std::endl;} // ***
  
  if(m_myLogging){NS_LOG_INFO ("Updating backof (First time from NotifyNavResetNow)");} // ***
  UpdateBackoff ();

  m_lastNavStart = Simulator::Now ();
  m_lastNavDuration = duration;

  if(m_myLogging){NS_LOG_INFO ("Updating backof (Second time from NotifyNavResetNow)");} // ***
  UpdateBackoff ();
  /**
   * If the nav reset indicates an end-of-nav which is earlier
   * than the previous end-of-nav, the expected end of backoff
   * might be later than previously thought so, we might need
   * to restart a new access timeout.
   */
  DoRestartAccessTimeoutIfNeeded ();
}

void
DcfManager::NotifyNavStartNow (Time duration)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << duration);} // *** myLogging only ***

  NS_ASSERT (m_lastNavStart <= Simulator::Now ());

  if(m_myLogging){MY_DEBUG ("nav start for=" << duration);} // *** myLogging only ***

  if(m_timeLogging){std::cout << Simulator::Now().GetMicroSeconds() << "s " << this << ": NAV start for " << duration << std::endl;} // ***

  if(m_myLogging){NS_LOG_INFO ("Updating backoff (from NotifyNavStartNow)");} // ***
  UpdateBackoff ();

  Time newNavEnd = Simulator::Now () + duration;
  Time lastNavEnd = m_lastNavStart + m_lastNavDuration;

  if (newNavEnd > lastNavEnd)
    {
      m_lastNavStart = Simulator::Now ();
      m_lastNavDuration = duration;
    }
}

// Notify that ACK timer has started for the given duration. ***
void
DcfManager::NotifyAckTimeoutStartNow (Time duration)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << duration);} // *** myLogging only ***

  if(m_timeLogging){std::cout << Simulator::Now().GetMicroSeconds() << "us " << this << ": ACK timeout started for for " << duration << std::endl;} // ***

  NS_ASSERT (m_lastAckTimeoutEnd < Simulator::Now ());

  // ACK timeout will end: Current simulaiton time + given duration
  m_lastAckTimeoutEnd = Simulator::Now () + duration;
}

// Notify that ACK timer has been reset. ***
void
DcfManager::NotifyAckTimeoutResetNow ()
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  // ACK timeout ended now
  m_lastAckTimeoutEnd = Simulator::Now ();

  // Schedule a time to call DcfManager::AccessTimeout
  DoRestartAccessTimeoutIfNeeded ();
}

// Notify that CTS timer has started for the given duration. ***
void
DcfManager::NotifyCtsTimeoutStartNow (Time duration)
{
  if(m_myLogging){NS_LOG_FUNCTION (this << duration);} // *** myLogging only ***

  if(m_timeLogging){std::cout << Simulator::Now().GetMicroSeconds() << "s " << this << ": CTS timeout started for " << duration << std::endl;} // ***

  // CTS Timeout will end: Current simulation time + given druation.
  m_lastCtsTimeoutEnd = Simulator::Now () + duration;
}

// Notify that CTS timer has been reset. ***
void
DcfManager::NotifyCtsTimeoutResetNow ()
{
  if(m_myLogging){NS_LOG_FUNCTION (this);} // *** myLogging only ***

  // CTS timeout ended now
  m_lastCtsTimeoutEnd = Simulator::Now ();

  // Schedule when to call DcfManager::AccessTimeout
  DoRestartAccessTimeoutIfNeeded ();
}

void // !!!
DcfManager::NotCountingClients (void)
{
    m_countingClients = false;
} // !!!

} //namespace ns3


