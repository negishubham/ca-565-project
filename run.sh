#Part1-b
#./build/X86/gem5.opt -d part1/b/output configs/example/se.py -c part1/b/a.out  --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache 

#Part1-c
#scons-3 -j4 ./build/X86/gem5.opt
#./build/X86/gem5.opt -d part1/c/output1 configs/example/se.py -c part1/c/a.out  --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache 
#./build/X86/gem5.opt -d part1/c/output2 configs/example/se.py -c part1/c/a.out  --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache 
#./build/X86/gem5.opt -d part1/c/output3 configs/example/se.py -c part1/c/a.out  --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache 
#./build/X86/gem5.opt -d part1/c/output4 configs/example/se.py -c part1/c/a.out  --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache 
#./build/X86/gem5.opt -d part1/c/output5 configs/example/se.py -c part1/c/a.out  --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache 
#./build/X86/gem5.opt -d part1/c/output6 configs/example/se.py -c part1/c/a.out  --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache 



#Part2 
# 100%
#scons-3 -j4 ./build/ARM/gem5.opt
#./build/ARM/gem5.opt -d part2/sjeng/output_100 configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#./build/ARM/gem5.opt -d part2/bzip2/output_100 configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#./build/ARM/gem5.opt -d part2/libquantum/output_100 configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache

# 50%
#./build/ARM/gem5.opt -d part2/sjeng/output_50 configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#./build/ARM/gem5.opt -d part2/bzip2/output_50 configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#./build/ARM/gem5.opt -d part2/libquantum/output_50 configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache

# 0%
#./build/ARM/gem5.opt -d part2/sjeng/output_0 configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#./build/ARM/gem5.opt -d part2/bzip2/output_0 configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#./build/ARM/gem5.opt -d part2/libquantum/output_0 configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache



#Part3 - baseline output
#scons-3 -j4 ./build/ARM/gem5.opt
#./build/ARM/gem5.opt -d part3/sjeng/baseline_output configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#./build/ARM/gem5.opt -d part3/bzip2/baseline_output configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#./build/ARM/gem5.opt -d part3/libquantum/baseline_output configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache

# Split output
#scons-3 -j4 ./build/ARM/gem5.opt
./build/ARM/gem5.opt -d part3/sjeng/split_output configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
./build/ARM/gem5.opt -d part3/bzip2/split_output configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
./build/ARM/gem5.opt -d part3/libquantum/split_output configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache


