set -v
set -e

bm1=SHORT-FP-
bm2=SHORT-INT-
bm3=SHORT-MM-
bm4=SHORT-SERV-
bm5=LONG-SPEC2K6-0

make clean
make

for ((i=1; i<4; i++))
do
   filename=../traces/$bm1$i.cbp4.gz
   ./predictor $filename
done

for ((i=1; i<4; i++))
do
   filename=../traces/$bm2$i.cbp4.gz
   ./predictor $filename
done

for ((i=1; i<4; i++))
do
   filename=../traces/$bm3$i.cbp4.gz
   ./predictor $filename
done

for ((i=1; i<4; i++))
do
   filename=../traces/$bm4$i.cbp4.gz
   ./predictor $filename
done

for ((i=0; i<10; i++))
do
   filename=../traces/$bm5$i.cbp4.gz
   ./predictor $filename
done



