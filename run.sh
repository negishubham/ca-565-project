#Part1-b
#./build/X86/gem5.opt -d part1/b/output configs/example/se.py -c part1/b/a.out  --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache 

#Part1-c

#for PART1C in 1 2 3 4 5 6
#do
#    sed -i 's/part1_c =.*/part1_c = '"$PART1C"'/' src/cpu/minor/MinorCPU.py
#    scons-3 -j4 ./build/X86/gem5.opt
#    ./build/X86/gem5.opt -d part1/c/output$PART1C configs/example/se.py -c part1/c/a.out  --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache 
#done



#Part2

#sed -i 's/part1_c =.*/part1_c = 1/' src/cpu/minor/MinorCPU.py
#for PART2 in 0 50 100
#do
#    sed -i 's/    branchPredRate = Param.Unsigned(.*/    branchPredRate = Param.Unsigned('"$PART2"',/' src/cpu/minor/MinorCPU.py 
#    scons-3 -j4 ./build/ARM/gem5.opt
#    ./build/ARM/gem5.opt -d part2/sjeng/output_$PART2 configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#    ./build/ARM/gem5.opt -d part2/bzip2/output_$PART2 configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#    ./build/ARM/gem5.opt -d part2/libquantum/output_$PART2 configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#done 



#Part3 - baseline output
#cp -r part2/sjeng/output_100 part3/sjeng
#cp -r part2/bzip2/output_100 part3/bzip2
#cp -r part2/libquantum/output_100 part3/libquantum
#mv part3/sjeng/output_100 part3/sjeng/output_baseline
#mv part3/bzip2/output_100 part3/bzip2/output_baseline
#mv part3/libquantum/output_100 part3/libquantum/output_baseline

# Split output
sed -i 's/    branchPredRate = Param.Unsigned(.*/    branchPredRate = Param.Unsigned(100,/' src/cpu/minor/MinorCPU.py 
scons-3 -j4 ./build/ARM/gem5.opt
./build/ARM/gem5.opt -d part3/sjeng/output_split configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
./build/ARM/gem5.opt -d part3/bzip2/output_split configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
./build/ARM/gem5.opt -d part3/libquantum/output_split configs/spec2k6/run.py -b libquantum --maxinsts=100000000 --cpu-type=MinorCPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache


