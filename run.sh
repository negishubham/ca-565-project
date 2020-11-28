
scons-3 -j4 ./build/ARM/gem5.opt
./build/ARM/gem5.opt -d output/baseline/sjeng configs/spec2k6/run.py -b sjeng --maxinsts=1000000 --cpu-type=DerivO3CPU --l1d_size=64kB --l1i_size=16kB --caches --l2cache



