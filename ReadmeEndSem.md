# ENDSEM PROJECT
	- Implementation of KAWS: Coordinate Kernel-Aware Warp Scheduling and Warp Sharing Mechanism for Advanced GPUs.
 	  Link to the paper - http://xml.jips-k.org/full-text/view?doi=10.3745/JIPS.01.0084

14:30 03-11-2023
Fetch and decode - Current version of gpgpu sim has two i-buffer entries for each warp. Each buffer entry has a valid bit, ready bit, decoded instruction. In simulator rather than ready bit a readiness check is performed.
Instruction Issue - warp is not waiting for a barrier, instruction is valid, no scoreboard collisions, operand access stage is not stalled.
SIMT stack - Used to see if there is a control divergence.
Scoreboard - Looks for RAW and WAW hazards.
Register access and the operand collector - After decoding, a hardware called collector unit is allocatoed to buffer the source operands of the instruction. Each collector unit receives one operand per cycle. 

Friday 20 October 2023 10:57:54 PM 

Software Model:

SIMT Core Cluster class - Contains an array of SIMT core objects in m_core. 
SIMT core class - Implemented with the class shader_core_ctx in shader.cc/h.
	- Collection of shd_warp_t objects which models the simulation state of each warp in the core.
	- SIMT stack to handle branch divergence.
	- shader_core_ctx::create_schedulers (../src/gpgpu-sim/shader.cc - line 165) parse the strings and detectes the type of scheduler.

Fetch and Decode:(Fig.3)
	- Implemented as an array of shd_warp_t() objects inside shadder_core_ctx. 
	
Sunday 22 October 2023 11:12:03 PM 
	
	
Issue:
	- Each core has cofigurable number of scheduler units. Function shader_core_ctx::issue() (../src/gpgpu-sim/shader.cc - line 1027) iterates over these units where each of them executes scheduler_unit_cycle() in RR fashion.
	
SIMT stack: Each scheduler unit has an array of SIMT stacks. Each SIMT stack corresponds to one warp. This class is implemented in shader.h file. used for control hazards.

Operand collector:
	- Implemented using class opndcoll_rfu_t (../src/gpgpu-sim/shader.h - line 563)
	- allocate_cu() function is responsible to allocate warp_inst_t to a free operand collector unit.
	

CTA scheduling :
	1. The issuing of thread block to SIMT core occurs in occurs in 			shader_core_ctx::issue_block2core().(..src/gpgpu-sim/gpu-sim.cc - line 1576)
	2. Maximum no.of thread blocks that can be concurrently scheduled on a core is calculated by the function shader_core_config::max_cta().(..src/gpgpu-sim/shader.cc - line 3229)
		depends on : no.of threads per thread block, per-thread reg. usage, shared memory usage, configured limit.
	3. In shader_core_ctx::issue_block2core(), the thread block size is first padded to be an exact multiple of warp size. 


Some other classes which may be of some use:
	- shader.h (line - 2269) exec_shader_core_ctx used in function,  exec_simt_core_cluster::create_shader_core_ctx
	- scheduler_unit class
	

Sunday 28 November 2023 10:11:38 PM 

Implemented scheduler part:
	Steps Followed:
		1.Identify the location where we are deciding the type of scheduler.There is a function in the class shader_core_ctx named create_schedulers in which we are deciding the type of scheduler. Implement KAWS similar to other schedulers in that function. We found that there is an enumerator named concrete_ scheduler include KAWS in that enumerator. There are derived classes from scheduler_unit class named lrr_scheduler, two_level_scheduler etc.., we also have to create a class for our KAWS. In this class we have virtual function named order_warps which orders the warp based on some criterion(in our case we have to do it based on progress).
		2. Now for the implementation of order_warps function we have to write a comparator function, for comparing two warps we have to first determine the progress of CTA to which they belong to. For progress tracking we define an unsigned integer array named m_cta_progress (public variable - tracks the no.of instructions completed in that CTA) with size of MAX_CTA_PER_SHADER in shader_core_ctx class. Now the tracking is similar to m_cta_stats (tracks the number of threads completed in the CTA). So, in issueblock2_core function of shader_core_ctx class initialise the counter to zero.(../src/gpgpu-sim/gpu-sim.cc - line 1666, ../src/gpgpu-sim/shader.cc - line 139).
		3. Now increment the counter whenever the 
instruction is issued(../src/gpgpu-sim/shader.cc - line 1418).
		4. Now for the comparision of two warps first get the shader whuch the warps belongs to and then compare the counters of the respective ctas using cta_id.(../src/gpgpu-sim/shader.cc - line 1496).
		
Sunday 05 November 2023 10:54:01 PM 

Implementation of warp sharing mechanism:
	Step Followed:
		1.Identify the part of the code where the instructions are categorized on the basis of different operations(SP,SFU,MEM). This is only done in scheduler_unit::cycle() function.In this function we are looking for free registers using had_free function which returns bool value accordingly.
		2. Assuming the model to be subcore every scheduler unit has only a single register_set of each type. Earlier we were checking only for one scheduler_unit of which the cycle function is called. Now what we will check for all the scheduler units and store the id of scheduler unit whose register_set id free in a referenced variable.
		3. The stored id then passed into the issue_warp function. 

## Analysis using benchmarks BFS, Pathfinder, backprop, hotspot, 3DCV, 3MM
  ![Performance Analysis](https://github.com/AbeedShaikM/COA/assets/111953087/474130e5-cba3-4b49-9549-61458b5f4234)
  ![Performance Analysis_GTO](https://github.com/AbeedShaikM/COA/assets/111953087/004f7d96-1915-490e-b7ad-c7ec97bef615)
  ![Performance Analysis_LRR](https://github.com/AbeedShaikM/COA/assets/111953087/6afcd876-e37a-40ac-9c84-9e62d384fad6)
  ![Performance Analysis_KAWS](https://github.com/AbeedShaikM/COA/assets/111953087/a8064841-949e-4c00-bf2a-a5a8ed5062d0)
