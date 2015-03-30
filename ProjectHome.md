This project contains an open-source simulation framework based on the ns-3 platform (http://www.nsnam.org) for datacenter network architectures. Using the framework, a datacenter network (DCN) topology of interest can be simulated and its corresponding efficiency performance can be easily produced or reproduced.

This research initiative is a collaborative effort between Nanyang Technological University, Singapore and A\*STAR Data Storage Institute, Singapore. It aims to facilitate fair performance reproducibility of DCN architectures and at the same time, eases the tasks of understanding, evaluating and extending these architectures. We welcome and acknowledge contributions from anyone who wishes to improve the NTU-DSI-DCN simulation framework for the benefit of the research community. We can be contacted at ntu.dsi.dcn@gmail.com.

Project members:
  * Daji Wong
  * Kiam Tian Seow
  * Chuan Heng Foh
  * Renuga Kanagavelu
  * Ngoc Linh Vu

### Advantages of the framework ###
  1. It is low cost, without having to invest a lot into hardware to build DCN architectures for simulation purposes
  1. It provides transparent performance benchmarks of known DCN architectures
  1. It enables unbiased comparative performance simulations of DCN architectures, without the tedium of developing existing DCN simulators from scratch.
  1. Researchers can use this framework as the basis to extend or create new DCN architectures for their research work

### DCN architectures available in the framework ###
  1. Fat-tree architecture
  1. BCube architecture

(More DCN architectures such as VL2, DCell and Portland will be added in the future)

### Getting started ###
The project's wiki section contains the steps needed to download and install the framework, as well as the steps needed to run the experiments stated in the paper.

The fat-tree implementation based on ns-3, as well as the BCube implementation on ns-3 can be found in this framework.

A Linux machine with Mercurial (http://mercurial.selenic.com) is needed to run the simulation framework.



### Publications ###

If you find our work useful or relevant in some ways, do cite our publications which are listed below:

1) D. Wong, K.T. Seow, C.H. Foh and R. Kanagavelu, “Towards Reproducible Performance Studies of Datacenter Network Architectures Using An Open-Source Simulation Approach”, Proceedings of the IEEE Global Communications Conference (GLOBECOM’13), December 2013, Atlanta, GA, USA. [[Download as PDF](https://drive.google.com/file/d/0B_2UOgK6adKGRmxGVHRtTkRoaWc/edit?usp=sharing)]

### Acknowledgements ###

We would like to thank the following researchers for their contributions to the NTU-DSI-DCN simulation framework. More details can be found on the "Getting Started" wiki page of this project.

Arfath Ahamed - Arfath Ahamed has made available the source codes for simulating the fat-tree architecture on ns-3.21 with NetAnim. Arfath Ahamed is currently a student pursuing a Masters degree in Masters of Technology (CS) at BIET, Davangere, Karnataka, India [[Download Fat-tree](https://drive.google.com/file/d/0B_2UOgK6adKGajM5ODhuV01yN0U/view?usp=sharing)] [[Download Fat-tree by Bilal et al](https://drive.google.com/file/d/0B_2UOgK6adKGSWlnOTM5a0k3YjQ/view?usp=sharing)] [[Download Fat-tree by Al-Fares](https://drive.google.com/file/d/0B_2UOgK6adKGQklfS1ZiaFRxejQ/view?usp=sharing)] (Contributed on 16 Jan 2015)