README
Updated June 23, 2021 

This document contains notes to myself (Maxwell Young) about our paper that involves NS3 (v 3.25) experiments. The details of these experiments are likely to be forgotten quickly, so this will help me recall details, if need be. It also serves as a README file to others, if others have an interest.

This part of the document discusses our experiments with different slot durations and SIFS durations. Getting these set up correctly required significant effort, so I want to record what we learned.

In regular-wifi-mac.cc:

case WIFI_PHY_STANDARD_80211G:
m_erpSupported = false; 

Note that we had this set to true before, so it was forcing 9 microseconds, and this is the default NS3 setting.

If you wish to change the slot duration, you need to set this to false. Otherwise, it will default to 9 microseconds regardless of what is said in the wifi-mac.cc.

I tested this by setting m_erpSupported = false. Then, I set the slot duration from within nonQos.cc to be roughly 700,000 microsecs. This greatly increased the run time, where some stations that I looked at had a "Packet time" of more than 52,000. This was for 10 stations and 64 byte packets, so this is enormous.

SIFS, on the other hand, does not seem to be forced in the same way. If you leave it alone, it will default to 10,000. Our plan should be to use the default settings of slot duration of 9 microseconds (m_erpSupported=true) or 20 microseconds (m_erpSupported=false) , using a SIFS=10 microseconds regardless.

The issue is: how do we decide which slot time to use? Inside of regular-wifi-mac.cc, there is the following:

------------
void
  351 WifiMac::Configure80211g (void)
  352 {
  353   SetSifs (MicroSeconds (10));
  354   // Slot time defaults to the "long slot time" of 20 us in the standard
  355   // according to mixed 802.11b/g deployments.  Short slot time is enabled
  356   // if the user sets the ShortSlotTimeSupported flag to true and when the BSS
  357   // consists of only ERP STAs capable of supporting this option.
  358   SetSlot (MicroSeconds (20));
  359   SetEifsNoDifs (MicroSeconds (10 + 304));
  360   SetPifs (MicroSeconds (10 + 20));
  361   SetCtsTimeout (MicroSeconds (10 + 304 + 20 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
  362   SetAckTimeout (MicroSeconds (10 + 304 + 20 + GetDefaultMaxPropagationDelay ().GetMicroSeconds () * 2));
  363 }
------------

I suspect that someone mistyped and, instead of meaning ShortSlotTimeSupported (which doesn't seem to exist, I looked), they mean m_erpSupported.  ERP stands for extended rate PHY. I found the following paper, which seems helpful:

The IEEE 802.11g Standard for High Data Rate WLANs by Dimitris Vassis, George Kormentzas, Angelos Rouskas, and Ilias Maglogiannis.

If you look at Table 2 and read the accompanying text, you'll see that 9 microseconds are for the case where all stations are running 802.11g and OFDM is in use. Given that our setup consists of only 802.11g enabled devices, this should apply.

Since m_erpSupported = true by default, I suspect we should be able to use this.  On the other hand, the text in the Configure80211g() function above gives me pause, and also this:

------------
void ns3::WifiMac::Configure80211g(void)
This method sets 802.11g standards-compliant defaults for following attributes: Sifs, Slot, EifsNoDifs, Pifs, CtsTimeout, and AckTimeout.

There is no support for short slot time.

Definition at line 351 of file wifi-mac.cc.
------------

which, if you click on the link, takes you back to the conflicting text above. The term "support" is ambiguous here. We may not be utilizing the most efficient PHY layer method, but in terms of the functionality of backoff, this doesn't seem critical.

I believe that our main result is not be impacted by this. That is, for reasonable-sized slot times (on the order of, say, 10 to 50 microseconds) you see the same behavior. Of course, if you set the slot time to be quite large, then you are increasing the cost associated with contention window (CW) slots relative to the total time. But our analysis of these two costs remains true nonetheless, so if you want a huge slot time and small packets (or, conversely, use small slot times and huge packets), our model addresses the overall time. But given the current applications, particularly WiFi, choosing a slot duration of roughly 10 microseconds is justifiable.

That said, to be safe, we are ALSO running with the 20 microsecond slot time, SIFS=10 microsecs, which is a default setting for when 802.11g cohabits a network with devices running an older standard. We will include our plots with 9 microsecond and SIFS=16 microseconds, along with the 20 microsecond slot time, SIFS=10 microsecs to show that our main finding --- these new algorithms underperform BEB due to more collisions --- is robust to this aspect. This seems like a safe way to proceed.

