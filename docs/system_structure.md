# Introduction

This file explains the hierarchical structure of Cactus. Cactus mainly contains the following parts:
- The microarchitecture core (digital part)
- The electronic simulator (analog part)
- A third party qubit state simulator


```
+-- cclight_tb
|    +-- cclight
```

```
QVM
 +-- Digital (cclight)
 |   +-- classical_top (Classical_part)
 |   |   +-- classical_pipeline (Classical_pipeline)
 |   |   +-- meas_reg_file (Meas_reg_file)
 |   |   |   +-- meas_reg_file_rtl/meas_reg_file_slice
 |   |   |
 |   |   +-- icache (ICache)
 |   |       +-- icache_rtl/icache_slice
 |   |
 |   +-- quantumt_top (Quantum_pipeline)
 |   |   +-- quantum_pipeline_independent (Q_tech_ind)
 |   |   |   +-- Q_decoder
 |   |   |   |  +-- q_decoder_bin
 |   |   |   |  +-- q_decoder_asm
 |   |   |   +-- VLIW_pipelane
 |   |   |   |  +-- mask_reg_file
 |   |   |   |  +-- addr_mask_decoder
 |   |   |   |  +-- op_decoder
 |   |   |   +-- meas_issue_gen
 |   |   |   +-- op_combiner
 |   |   |
 |   |   +-- quantum_pipeline_dependent (Q_tech_dep)
 |   |   |	 +-- timing_control_unit
 |   |   |	 |  +-- event_queue_manager
 |   |   |	 +-- fast_condition_execution
 |   |
 |   +-- msmt_result_analysis
 |   +-- Exe_flag_gen
 |       +-- <Fce_logic>, each per qubit
 |
 +-- Analog Digital Interface (ADI)
 |   +-- analog_digital_convert
 |   +-- msmt_result_gen
 |
 +-- Qubit Simulator
```

```

```
