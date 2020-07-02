#!/bin/bash
old=43
for size in 43 52 59
do
mkdir checkpoint_files/alexnet/checkpoint_files_"$size"/checkpoint_files
mv checkpoint_files/alexnet/checkpoint_files_"$size"/global_mem* checkpoint_files/alexnet/checkpoint_files_"$size"/checkpoint_files/
sed -i s'/resume_kernel $old/resume_kernel $size/' configs/default.config
sed -i s'/checkpoint_files_"$old"/checkpoint_files_"$size"/' dnnmark_test_alexnet/dnnmark_test_alexnet.sub
./condor_jobs_Squeezenet.sh
sleep(1m)
old =$size
done
