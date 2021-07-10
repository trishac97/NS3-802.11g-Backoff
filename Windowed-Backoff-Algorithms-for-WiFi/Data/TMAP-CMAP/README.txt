README

The data files for BEB and STB are given here.  The initial activity is due to ARP. The packet transmissions start at 6,000,000 microseconds. 

In the BEB file 0x15b77c0 denotes the AP (note that it never chooses a slot).

At 6,000,000 microseconds, all packets send and collide. The value m_cw (see dcf-manager.cc code for m_backoffType == 0 starting on line 265) updates to 1. When the first window is run, the window size becomes m_cw = 2(1+1) - 1 = 3, allowing for slots 0, 1, 2, or 3 to be chosen. The window sizes increase accordingly.

For STB, we see something similar. After the initial transmission, The m_cw value gets updated to 2^1, allowing for slots 0, 1 ,2 in the first window. Again, the window sizes increase accordingly.

Note that after sending, a MISSED ACK message will come back at roughly 121 microseconds later. For example, at the 60,000,000 microsec mark, we have:

(0x123bea0) 60000000 <-- this client starts transmission now.
(0x123f880) 60000000
(0x12432a0) 60000000
(0x1246d00) 60000000
(0x124a740) 60000000
(0x124e1d0) 60000000
(0x1251c10) 60000000
(0x12a3fe0) 60000000
(0x12a7ab0) 60000000
(0x12ab580) 60000000
(0x12aefd0) 60000000
(0x12b2a20) 60000000
(0x12b6410) 60000000
(0x12b9e80) 60000000
(0x12bd8f0) 60000000
(0x12c1360) 60000000
(0x12c4dd0) 60000000
(0x12c89b0) 60000000
(0x12cc400) 60000000
(0x12cfe50) 60000000
(0x123c220) Missed ACK 60000121 <-- this client determined failure of transmission here
(0x123c3f0) chose 0 slots <-- this client chooses slot 0 for the next window.
(0x123fc00) Missed ACK 60000121
etc.

Calculating the transmission delay + preamble + propagation delay + processing on the AP side, we are left with something that is close to the expected ACK timeout of 75 microseconds.

Also note that after that point, we have:

(0x123bea0) 60000155
(0x123f880) 60000155
(0x12432a0) 60000155
(0x1246d00) 60000155
(0x12a7ab0) 60000155
(0x12aefd0) 60000155
(0x12c89b0) 60000155
(0x12cc400) 60000155
(0x123c220) Missed ACK 60000276

And 276 - 155 = 121 again, and we have the delay + ACK timeout. Then we see (repeating the first two lines):

(0x12cc400) 60000155
(0x123c220) Missed ACK 60000276
(0x123c3f0) chose 3 slots
(0x123fc00) Missed ACK 60000276
(0x123fdd0) chose 4 slots
(0x1243620) Missed ACK 60000276
(0x1242350) chose 3 slots
(0x1247080) Missed ACK 60000276
(0x1245da0) chose 1 slots
(0x12a7e30) Missed ACK 60000276
(0x12a6b70) chose 2 slots
(0x12af350) Missed ACK 60000276
(0x12ae090) chose 3 slots
(0x12c8d30) Missed ACK 60000276
(0x12c7960) chose 7 slots
(0x12cc780) Missed ACK 60000276
(0x12cb4c0) chose 6 slots
(0x12a3fe0) 60000294
(0x12b2a20) 60000294
(0x12c4dd0) 60000294

Note that these 3 devices send at 294, but this is NOT a DIFS separation between 276; it is only a gap of 18 microseconds. They paused earlier, while the group at 155 transmitted.  After that group at 155 finished transmitting, they waited for a DIFS and started sending.

Therefore, as specified, packets don't always wait for an ACK timeout. The ones that sent will do so, of course. But the ones that paused, will wait for DIFS, then unpause their countdown in backoff. This is supported by the standard, which states:


