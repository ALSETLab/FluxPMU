# FluxPMU
An Open Source Maker's Guide of an DIY PMU

FluxPMU is a "grassroots" educational project that aims  to  enable  students  to  explore  and  buildan  open  source,  low  cost  PMU.  

Building  from  the  legacy  of [OpenPMU  V1](https://ieeexplore.ieee.org/document/6463452),  FluxPMU  provides  a Do-it-Yourself (DIY) guide with  all  documentation  and  software  sources  required  to  build FluxPMU.

FluxPMU provides only a refinement of OpenPMU V1's design, and it is NOT to be viewed as an evolution on the design of OpenPMU. For the latest iteration of the OpenPMU device, please visit: [http://www.openpmu.org/](http://www.openpmu.org/).

## Cite this work:

This repository is accompanied by a draft of the paper:
> E. Williamson, L. Vanfretti, P. Adhikari, J. W. Dziuba, and D. Laverty, “FluxPMU - A Maker’s Guide of a DIY Synchronized Phasor Measurement Unit,” submitted for review, IEEE PES ISGT NA 2021, Feb. 15-18, Virtual Event, 2021.

The paper can be found on this ``root`` directory, it is named [20200930_Overview_draft.pdf](https://github.com/ALSETLab/FluxPMU/blob/master/20200930_Overview_draft.pdf). The paper has been submitted for review to the [IEEE PES ISGT NA 2021](https://ieee-isgt.org/), if and when the paper is published, this repository will be updated to provide a link to the final version of the paper.

When citing the paper above, we also recommend citing the following publication:
> D. M. Laverty, R. J. Best, P. Brogan, I. Al Khatib, L. Vanfretti and D. J. Morrow, "The OpenPMU Platform for Open-Source Phasor Measurements," in IEEE Transactions on Instrumentation and Measurement, vol. 62, no. 4, pp. 701-709, April 2013, doi: 10.1109/TIM.2013.2240920.

## Repository Organization

### Where to start?
After reading the paper(s) listed above, the entire "Maker's Guide" can be found in the director ``./00_Documentation/00_Makers Guide.pdf``, this will give you an overview of the different resources associated with this repository.

### Organization
This repository contains more than the main "Maker's Guide", it also provides updated source code for all software required for the FluxPMU, documentation for the software, hardware assembly guides, etc.

The resources have been organized as follows:
  - ``00_Documentation`` this is the major folder, it includes the "Maker's Guide" as a single .pdf, [20200930_Overview_draft.pdf](https://github.com/ALSETLab/FluxPMU/blob/master/20200930_Overview_draft.pdf), and three sub-folders. The sub-folders include:
      - ``01_HardwareGuides`` contains assembly instructions, PIC micro-controller configuration, wiring, etc.
      - ``02_TestingGuides`` contains documents showing how to test the FluxPMU using different instruments such as a frequency generator, an Analog Discovery Board, etc.
      - ``03_LabVIEW_VI_Documentation`` contains individual documentation for all LabVIEW VI files in .pdf. You can also find this documentation [online](https://alsetlab.github.io/S3DK/docs/Code_Flow_Chart.html), however, it is also placed here for version tracking and maintainance purposes.
  - ``01_SourceCode`` contains two folders, one with all the updated (64-bit) software related to LabVIEW and the PIC micro-controller (v5p1) files used.
  - ``02_HardwareSourceFiles`` contains a part list/bill of materials with costs based on US-based suppliers and costs in US dollars, as well as the design BRD files for the PCBs.
  - ``10_LegacyDocumentation`` contains the original documents we used when getting FluxPMU started.

## Acknowledgement
This work was supported in part by Dominion Energy Virginia and in part by the Center of Excellence for NEOM Research at the King Abdullah University of Science and Technology under grant OSR-2019-CoE-NEOM- 4178.12.

## No Warranty
(c) David Laverty, Emmett Williamson, Luigi Vanfretti, Prottay M. Adhikari, Jerry W. Dziuba.

All materials under this repository are distributed under the GPL v3 license.

