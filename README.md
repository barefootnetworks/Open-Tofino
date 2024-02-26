Open Tofino
===========
We are openly publishing these files to make it possible for anyone developing
Tofino-specific P4 data plane programs (both P4<sub>14</sub> and P4<sub>16</sub> versions) and related control plane 
applications to openly publish their data plane and control plane code. 

License
=======
The files are licensed under Creative Commons Attribution-NoDerivs (CC BY-ND
4.0) license (https://creativecommons.org/licenses/by-nd/4.0/legalcode). The
text of the license can also be found in the LICENSE file.

Contents
========

This repository contains the files that describe the P4 target architecture for Intel&reg; Tofino&trade; and Intel &reg; Tofino 2&trade;.
These files are copies of the corresponding files in the corresponding Intel&reg; P4
Studio SDE release (see VERSION file), explicitly licensed under CC BY-ND 4.0.

FAQs
========

**Q: What is being allowed?** <br>
*A: You can openly publish your data plane code written for Tofino in both P4<sub>14</sub> and P4<sub>16</sub>, as well as the accompanying control plane code. You can openly publish your own code written for Tofino, e.g. by putting it on Github, attaching to a publication, etc.* 

**Q: What can I *not* publish?** <br>
*A: You are not allowed to publish Intel software, e.g. the P4Studio SDE in source or in binary. You are also not allowed to publish the artifacts produced by the P4 compiler and other P4Studio tools, such as Tofino binary file (tofino.bin), context.json and bfrt.json, logs, visualizations, etc.*

**Q: Do I need an NDA to develop this code?** <br>
*A: Yes, you do need an NDA and SLA to get access to documentation and P4 Studio release so you can develop the control plane and data plane code.*

**Q: What is Intel's vision for being more open about Tofino?** <br>
*A: Just as Intel helped create one of the biggest software ecosystems in the computing industry, it is approaching the networking industry in a similar way. Thus, Intel believes Ethernet switch ASICs should have a software ecosystem that is as open and easy to participate in as CPUs.  This announcement is just the first step. Stay tuned!*

**Q: Can you share examples of others who have taken advantage of Open Tofino?** <br>
*A: Here is a list of repos, which showcase several projects using Open Tofino in ongoing research:*
<ul>
  <li><a href=https://github.com/Princeton-Cabernet/p4-projects/blob/master/AES-tofino>AES-tofino</a>: AES encryption on Tofino switch</li>
  <li><a href=https://github.com/Princeton-Cabernet/BeauCoup>BeauCoup</a>: Run multiple distinct-counting queries on Tofino</li>
  <li><a href=https://github.com/harvard-cns/cheetah-release>Cheetah</a>: Use Tofino to accelerate Spark queries</li>
  <li><a href=https://github.com/chipmunk-project/chipmunk-tofino>Chipmunk</a>: Use program synthesis to generate P4 code</li>
  <li><a href=https://github.com/Princeton-Cabernet/p4-projects/blob/master/ConQuest-tofino>ConQuest-tofino</a>: ConQuest queue analysis on Tofino switch</li>
  <li><a href=https://github.com/eniac/Mantis>Mantis</a>: Generates reactive P4 program and C++ agent</li>
  <li><a href=https://github.com/cornell-netlab/MicroP4>MicroP4</a>: Modularized data-plane programming</li>
  <li><a href=https://github.com/netx-repo/NetLock>NetLock</a>: Using P4 switch for lock management</li>
  <li><a href=https://github.com/Princeton-Cabernet/p4-projects/blob/master/ONTAS>ONTAS</a>: Traffic anonymization on BMV2 / Tofino</li>
  <li><a href=https://github.com/ralfkundel/p4-codel>P4-CoDel</a>: Implementation of CoDel AQM for BMV2 and Tofino</li>
  <li><a href=https://github.com/ralfkundel/p4sta>P4STA</a>: Load generation and timestamping framework for Tofino/Netronome/BMV2</li>
  <li><a href=https://github.com/Princeton-Cabernet/p4-projects/blob/master/PRECISION-tofino>PRECISION-tofino</a>: The PRECISION heavy-hitter algorithm on Tofino switch</li>
  <li><a href=https://github.com/Princeton-Cabernet/p4-projects/blob/master/RTT-tofino>RTT-tofino</a>: TCP Round-Trip Time measurement on Tofino switch</li>
</ul>

**Q: How often is Open Tofino updated?** <br>
*A: Next update of Open Tofino will be on a need basis* 

