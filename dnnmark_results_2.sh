         
            grep -ri L2_total_cache_accesses lab_8_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri L2_total_cache_misses lab_8_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri total.*dram.*reads lab_8_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri total.*dram.*writes  lab_8_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1;
            grep -ri L2_total_cache_accesses lab_16_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri L2_total_cache_misses lab_16_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri total.*dram.*reads lab_16_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri total.*dram.*writes  lab_16_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1;
            grep -ri L2_total_cache_accesses lab_64_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri L2_total_cache_misses lab_64_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri total.*dram.*reads lab_64_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri total.*dram.*writes  lab_64_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1;
            grep -ri L2_total_cache_accesses lab_128_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri L2_total_cache_misses lab_128_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri total.*dram.*reads lab_128_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri total.*dram.*writes  lab_128_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1;
            grep -ri L2_total_cache_accesses lab_8192_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri L2_total_cache_misses lab_8192_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri total.*dram.*reads lab_8192_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1; grep -ri total.*dram.*writes  lab_8192_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1 | head -1;
            grep -ri L2_total_cache_accesses dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1| head -1; grep -ri L2_total_cache_misses dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1| head -1; grep -ri total.*dram.*reads dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1| head -1; grep -ri total.*dram.*writes  dnnmark_test_SqueezeNet.out | cut -f2- -d= | tail -1| head -1;
            
        
tail -2 | head -1; lab_8_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out;
tail -2 | head -1; lab_16_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out;
tail -2 | head -1; lab_64_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out;
tail -2 | head -1; lab_128_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out;
tail -2 | head -1; lab_8192_SqueezeNet_dnnmark_test_SqueezeNet/results/dnnmark_test_SqueezeNet.out;
tail -2 | head -1; dnnmark_test_SqueezeNet.out;

L1I_total_cache_accesses     = 2160892855
L1I_total_cache_misses       = 3023738
L1D_total_cache_accesses     = 252278141
L1D_total_cache_misses       = 102260372
L1C_total_cache_accesses     = 44236224
L1C_total_cache_misses       = 151907
L1T_total_cache_accesses     = 125938752
L1T_total_cache_misses       = 68222544
L2_total_cache_accesses      = 354305832
L2_total_cache_misses        = 153018412
total.*dram.*reads           = 115542367
total.*dram.*writes          = 16015450
icnt_total_pkts_simt_to_mem  = 159863222
icnt_total_pkts_mem_to_simt  = 325962461
gpgpu_n_shmem_insn           = 8189288448
gpu_tot_sim_insn             = 132647130576
gpgpu_n_load_insn            = 1294506315
gpgpu_n_store_insn           = 324206592
LAB_total_cache_accesses     = 223387569
LAB_total_cache_misses       = 10383055

54607092-3573361
24223004-1550460
28170747


105451610
123579572
lab; source setup_environment; cd /nobackup/condor_preyesh/microbench; ./histogram |& tee histogram_lab16.log

47691984
47144343
46733667
46923541
45653567
44306027
54352162

644473253
601921068
545802465
548226903
526045022
498713930
1047511975

nvcc -O3 -arch sm_61 main.cu -o allSyncPrims-1kernel_baseline -I/p/hal/private/installs/cuda-8.0//include -L/p/hal/private/installs/cuda-8.0//lib64 


L2_total_cache_accesses = 61454573
L2_total_cache_misses =   8455350
total dram reads =        6562458
total dram writes =       1121947


kernel_name = _Z24scalePackedTensor_kernelIffEv19cudnnTensor4dStructPT_T0_
kernel_name = _ZN5cudnn6detail12dgrad_engineIfLi512ELi6ELi5ELi3ELi3ELi3ELb1EEEviiiPKT_iS4_iPS2_18kernel_grad_paramsiifiii
kernel_name = _ZN5cudnn6detail23activation_bw_4d_kernelIffLi128ELi1ELi4ENS0_9relu_funcIfL21cudnnNanPropagation_t1ELb0EEEEEv17cudnnTensorStructPKT_S8_S5_S8_PS6_T0_SA_iSA_
kernel_name = _ZN5cudnn6detail17wgrad_alg0_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1ELi512EEEviiiPKT_iPS2_S4_18kernel_grad_paramsifiiii
kernel_name = _Z24scalePackedTensor_kernelIffEv19cudnnTensor4dStructPT_T0_
kernel_name = _ZN5cudnn6detail12dgrad_engineIfLi512ELi6ELi5ELi3ELi3ELi3ELb1EEEviiiPKT_iS4_iPS2_18kernel_grad_paramsiifiii
kernel_name = _ZN5cudnn6detail23activation_bw_4d_kernelIffLi128ELi1ELi4ENS0_9relu_funcIfL21cudnnNanPropagation_t1ELb0EEEEEv17cudnnTensorStructPKT_S8_S5_S8_PS6_T0_SA_iSA_
kernel_name = _ZN5cudnn6detail17wgrad_alg0_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1ELi512EEEviiiPKT_iPS2_S4_18kernel_grad_paramsifiiii
kernel_name = _Z24scalePackedTensor_kernelIffEv19cudnnTensor4dStructPT_T0_
kernel_name = _ZN5cudnn6detail12dgrad_engineIfLi512ELi6ELi5ELi3ELi3ELi3ELb1EEEviiiPKT_iS4_iPS2_18kernel_grad_paramsiifiii
kernel_name = _ZN5cudnn6detail23activation_bw_4d_kernelIffLi128ELi1ELi4ENS0_9relu_funcIfL21cudnnNanPropagation_t1ELb0EEEEEv17cudnnTensorStructPKT_S8_S5_S8_PS6_T0_SA_iSA_
kernel_name = _ZN5cudnn6detail17wgrad_alg0_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1ELi512EEEviiiPKT_iPS2_S4_18kernel_grad_paramsifiiii
kernel_name = _Z24scalePackedTensor_kernelIffEv19cudnnTensor4dStructPT_T0_
kernel_name = _ZN5cudnn6detail12dgrad_engineIfLi512ELi6ELi5ELi3ELi3ELi3ELb1EEEviiiPKT_iS4_iPS2_18kernel_grad_paramsiifiii
kernel_name = _Z45pooling_bw_kernel_max_nchw_fully_packed_smallIffLi2EL21cudnnNanPropagation_t1EEv17cudnnTensorStructPKT_S1_S4_S1_S4_S1_PS2_18cudnnPoolingStructT0_S7_N5cudnn15reduced_divisorES9_
kernel_name = _ZN5cudnn6detail23activation_bw_4d_kernelIffLi128ELi1ELi4ENS0_9relu_funcIfL21cudnnNanPropagation_t1ELb0EEEEEv17cudnnTensorStructPKT_S8_S5_S8_PS6_T0_SA_iSA_
kernel_name = _ZN5cudnn6detail17wgrad_alg0_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1ELi512EEEviiiPKT_iPS2_S4_18kernel_grad_paramsifiiii
kernel_name = _Z24scalePackedTensor_kernelIffEv19cudnnTensor4dStructPT_T0_
kernel_name = _ZN5cudnn6detail12dgrad_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1EEEviiiPKT_iS4_iPS2_18kernel_grad_paramsiifiii
kernel_name = _ZN5cudnn6detail23activation_bw_4d_kernelIffLi128ELi1ELi4ENS0_9relu_funcIfL21cudnnNanPropagation_t1ELb0EEEEEv17cudnnTensorStructPKT_S8_S5_S8_PS6_T0_SA_iSA_
kernel_name = _ZN5cudnn6detail17wgrad_alg0_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1ELi512EEEviiiPKT_iPS2_S4_18kernel_grad_paramsifiiii
kernel_name = _Z24scalePackedTensor_kernelIffEv19cudnnTensor4dStructPT_T0_

gpu_sim_cycle = 4110
gpu_sim_cycle = 944182
gpu_sim_cycle = 13387
gpu_sim_cycle = 940487
gpu_sim_cycle = 3155
gpu_sim_cycle = 918219
gpu_sim_cycle = 11724
gpu_sim_cycle = 1132176
gpu_sim_cycle = 3292
gpu_sim_cycle = 918780
gpu_sim_cycle = 12079
gpu_sim_cycle = 1073876
gpu_sim_cycle = 3156
gpu_sim_cycle = 913523
gpu_sim_cycle = 67500
gpu_sim_cycle = 48667
gpu_sim_cycle = 2804207
gpu_sim_cycle = 9679
gpu_sim_cycle = 2859012
gpu_sim_cycle = 50057
gpu_sim_cycle = 2834505
gpu_sim_cycle = 9725


gpu_sim_cycle = 4110
gpu_sim_cycle = 995351
gpu_sim_cycle = 13749
gpu_sim_cycle = 2587237
gpu_sim_cycle = 3206
gpu_sim_cycle = 961566
gpu_sim_cycle = 12254
gpu_sim_cycle = 2460473
gpu_sim_cycle = 3239
gpu_sim_cycle = 951384
gpu_sim_cycle = 11827
gpu_sim_cycle = 2567146
gpu_sim_cycle = 3390
gpu_sim_cycle = 959639
gpu_sim_cycle = 68393
gpu_sim_cycle = 48822
gpu_sim_cycle = 3985470
gpu_sim_cycle = 11536
gpu_sim_cycle = 3035997
gpu_sim_cycle = 54883

      L1D_total_cache_accesses =   50176
        L1D_total_cache_accesses = 50176 
        L1D_total_cache_accesses = 202240  
        L1D_total_cache_accesses = 2499072
        L1D_total_cache_accesses = 2549248
        L1D_total_cache_accesses = 2549248
        L1D_total_cache_accesses = 2701312
        L1D_total_cache_accesses = 4998144
        L1D_total_cache_accesses = 5048320
        L1D_total_cache_accesses = 5048320
        L1D_total_cache_accesses = 5200384
        L1D_total_cache_accesses = 7497216
        L1D_total_cache_accesses = 7547392
        L1D_total_cache_accesses = 7547392
        L1D_total_cache_accesses = 8070656
        L1D_total_cache_accesses = 8678912
        L1D_total_cache_accesses = 20291072
        L1D_total_cache_accesses = 20491776
        L1D_total_cache_accesses = 20491776
        L1D_total_cache_accesses = 21100032

 50176
 50176
 202240
 2499072
 2549248
 2549248
 2701312
 4998144
 5048320
 5048320
 5200384
 7497216
 7547392
 7547392
 8070656
 8678912
 20291072
 20491776
 20491776
 21100032

kernel_name = _ZN5cudnn6detail17wgrad_alg0_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1ELi512EEEviiiPKT_iPS2_S4_18kernel_grad_paramsifiiii
kernel_name = _Z24scalePackedTensor_kernelIffEv19cudnnTensor4dStructPT_T0_
kernel_name = _ZN5cudnn6detail12dgrad_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1EEEviiiPKT_iS4_iPS2_18kernel_grad_paramsiifiii
kernel_name = _ZN5cudnn6detail23activation_bw_4d_kernelIffLi128ELi1ELi4ENS0_9relu_funcIfL21cudnnNanPropagation_t1ELb0EEEEEv17cudnnTensorStructPKT_S8_S5_S8_PS6_T0_SA_iSA_
kernel_name = _ZN5cudnn6detail17wgrad_alg0_engineIfLi128ELi6ELi7ELi3ELi3ELi5ELb1ELi512EEEviiiPKT_iPS2_S4_18kernel_grad_paramsifiiii
kernel_name = _Z24scalePackedTensor_kernelIffEv19cudnnTensor4dStructPT_T0_
kernel_name = _ZN5cudnn6detail12dgrad_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1EEEviiiPKT_iS4_iPS2_18kernel_grad_paramsiifiii
kernel_name = _ZN5cudnn6detail23activation_bw_4d_kernelIffLi128ELi1ELi4ENS0_9relu_funcIfL21cudnnNanPropagation_t1ELb0EEEEEv17cudnnTensorStructPKT_S8_S5_S8_PS6_T0_SA_iSA_
kernel_name = _ZN5cudnn6detail17wgrad_alg0_engineIfLi128ELi6ELi7ELi3ELi3ELi5ELb1ELi512EEEviiiPKT_iPS2_S4_18kernel_grad_paramsifiiii
kernel_name = _Z24scalePackedTensor_kernelIffEv19cudnnTensor4dStructPT_T0_
kernel_name = _ZN5cudnn6detail12dgrad_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1EEEviiiPKT_iS4_iPS2_18kernel_grad_paramsiifiii
kernel_name = _Z45pooling_bw_kernel_max_nchw_fully_packed_largeIffLi3EL21cudnnNanPropagation_t1EEv17cudnnTensorStructPKT_S1_S4_S1_S4_S1_PS2_18cudnnPoolingStructT0_S7_N5cudnn15reduced_divisorES9_i
kernel_name = _ZN5cudnn6detail17lrnBackward_evenCILi5EffEEvNS0_17LrnBackwardParamsIT0_T1_EE
kernel_name = _ZN5cudnn6detail23activation_bw_4d_kernelIffLi128ELi1ELi4ENS0_9relu_funcIfL21cudnnNanPropagation_t1ELb0EEEEEv17cudnnTensorStructPKT_S8_S5_S8_PS6_T0_SA_iSA_
kernel_name = _ZN5cudnn6detail17wgrad_alg0_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1ELi512EEEviiiPKT_iPS2_S4_18kernel_grad_paramsifiiii
kernel_name = _Z24scalePackedTensor_kernelIffEv19cudnnTensor4dStructPT_T0_
kernel_name = _ZN5cudnn6detail12dgrad_engineIfLi128ELi6ELi8ELi3ELi3ELi5ELb1EEEviiiPKT_iS4_iPS2_18kernel_grad_paramsiifiii
kernel_name = _Z45pooling_bw_kernel_max_nchw_fully_packed_largeIffLi3EL21cudnnNanPropagation_t1EEv17cudnnTensorStructPKT_S1_S4_S1_S4_S1_PS2_18cudnnPoolingStructT0_S7_N5cudnn15reduced_divisorES9_i
kernel_name = _ZN5cudnn6detail17lrnBackward_evenCILi5EffEEvNS0_17LrnBackwardParamsIT0_T1_EE
kernel_name = _ZN5cudnn6detail23activation_bw_4d_kernelIffLi128ELi1ELi4ENS0_9relu_funcIfL21cudnnNanPropagation_t1ELb0EEEEEv17cudnnTensorStructPKT_S8_S5_S8_PS6_T0_SA_iSA_
kernel_name = _Z13gen_sequencedI17curandStateXORWOWfiXadL_Z21curand_uniform_noargsIS0_EfPT_iEEEvPT_PT0_mmT1_
kernel_name = _Z13gen_sequencedI17curandStateXORWOWfiXadL_Z21curand_uniform_noargsIS0_EfPT_iEEEvPT_PT0_mmT1_
kernel_name = _Z13gen_sequencedI17curandStateXORWOWfiXadL_Z21curand_uniform_noargsIS0_EfPT_iEEEvPT_PT0_mmT1_
kernel_name = _ZN5cudnn6detail17wgrad_alg0_engineIfLi128ELi6ELi7ELi3ELi3ELi5ELb1ELi512EEEviiiPKT_iPS2_S4_18kernel_grad_paramsifiiii
gpu_sim_cycle = 1414051
gpu_sim_cycle = 7776
gpu_sim_cycle = 1270388
gpu_sim_cycle = 31596
gpu_sim_cycle = 1727412
gpu_sim_cycle = 7051
gpu_sim_cycle = 1744661
gpu_sim_cycle = 28301
gpu_sim_cycle = 1248562
gpu_sim_cycle = 4826
gpu_sim_cycle = 1149390
gpu_sim_cycle = 117318
gpu_sim_cycle = 203809
gpu_sim_cycle = 221852
gpu_sim_cycle = 3549464
gpu_sim_cycle = 7124
gpu_sim_cycle = 2702484
gpu_sim_cycle = 181899
gpu_sim_cycle = 159667
gpu_sim_cycle = 162411
gpu_sim_cycle = 96848
gpu_sim_cycle = 96635
gpu_sim_cycle = 52223
gpu_sim_cycle = 1483527
