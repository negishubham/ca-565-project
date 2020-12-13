set -e
set -v

#python train.py -b 2560 -j 12 --epochs 45 --gpus 0,1,2,3 --lr 0.002 --results_dir ./results_allbenchmark/LeNet5/final_dataset_LONG-SPEC2K6-03_002_50epoch --model LeNet5

python train.py -b 2560 -j 12 --epochs 100 --gpus 0,1,2,3 --lr 0.002 --results_dir ./results_allbenchmark/LeNet5/serv/final_dataset_SHORT-SERV-2_100epochs_2 --model LeNet5

