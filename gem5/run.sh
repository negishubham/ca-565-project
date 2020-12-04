

#sed -i 's/    branchPred = Param.BranchPredictor(.*/    branchPred = Param.BranchPredictor(BiModeBP(numThreads =/' src/cpu/o3/O3CPU.py 
#scons -j4 ./build/ARM/gem5.opt
#./build/ARM/gem5.opt -d output/bimode/sjeng configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=DerivO3CPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache
#./build/ARM/gem5.opt -d output/bimode/bzip2 configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=DerivO3CPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache

#sed -i 's/    branchPred = Param.BranchPredictor(.*/    branchPred = Param.BranchPredictor(TournamentBP(numThreads =/' src/cpu/o3/O3CPU.py 
#scons -j4 ./build/ARM/gem5.opt
#./build/ARM/gem5.opt -d output/tournament/sjeng configs/spec2k6/run.py -b sjeng --maxinsts=100000000 --cpu-type=DerivO3CPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache

#sed -i 's/    branchPred = Param.BranchPredictor(.*/    branchPred = Param.BranchPredictor(YAGSBP(numThreads =/' src/cpu/o3/O3CPU.py 
scons -j4 ./build/ARM/gem5.opt
./build/ARM/gem5.opt -d output/YAGS/sjeng configs/project_spec2k6/run.py -b sjeng --maxinsts=1000000 --cpu-type=DerivO3CPU --l1d_size=32kB --l1i_size=32kB --caches --l2cache
#./build/ARM/gem5.opt -d output/YAGS/sjeng configs/project_spec2k6/run.py -b sjeng --maxinsts=1000000 --cpu-type=DerivO3CPU --l1d_size=32kB --l1i_size=32kB --caches --l2cache

#./build/ARM/gem5.opt -d output/YAGS/bzip2 configs/spec2k6/run.py -b bzip2 --maxinsts=100000000 --cpu-type=DerivO3CPU --l1d_size=32kB --l1i_size=32kB --caches --l2cache

