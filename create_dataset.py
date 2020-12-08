#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Nov 27 23:06:15 2020

@author: snegi
"""

import pdb
import random
import numpy as np
import os
import argparse
import csv 


parser = argparse.ArgumentParser()
parser.add_argument('--fn', type=str, default='SHORT-FP-1.cbp4.gz.txt')
args = parser.parse_args()

#filename= '/home/nano01/a/snegi/Projects/ca-565-project/CSE 6421 Branch Prediction/bpc6421AU16/traces/dataset/SHORT-FP-1.cbp4.gz.txt'
filename = '/home/nano01/a/snegi/Projects/ca-565-project/CSE 6421 Branch Prediction/bpc6421AU16/traces/dataset/'+args.fn

print('Benchmark: {}'.format(args.fn))
if not os.path.exists(filename.split('dataset/')[0]+'dataset/'+'final_dataset/'):    
    os.mkdir(filename.split('dataset/')[0]+'dataset/'+'final_dataset/')

pc_value=[]
history_value=[]
ga_value=[]
hr_temp=0

#lhr bits=16 and taking 2:11 bits of PC to 
pc=[]
pc_bits = 32

ghr=[]
ghr_bits=512

local_history=[0]*2**10
lhr=[]
lhr_bits=16

ga=[]
ga_bits=48*8

output_conv=[]
index=0
with open(filename) as f:
    line = f.readline()
    
    line = line.split(" ")

#    while line:
    while index<10:
        if line[0] == '':
            break
#############################################
        #store pc values
        pc_value.append(bin(int(line[1])).split('b')[1])
     
#############################################  
        #store history value
        branch_dir=line[7].split('\n')[0]
        output_conv.append([str(1-int(branch_dir)), branch_dir])
        #left shift by 1 bit
        hr_temp<<1
        #if branch taken add 1
        if branch_dir == '1':
            hr_temp+=1
        history_value.append(hr_temp)
##################################################
        ind = int(bin(int(line[1])).split('b')[1][2:12], 2)
        if local_history[ind] ==0:
            local_history[ind] = [branch_dir]
        else:
            local_history[ind] = [branch_dir] + local_history[ind]
        if len(local_history[ind]) > 16:
            local_history[ind].pop()
            
        lhr.append(['0']*(16-len(local_history[ind])) + local_history[ind] if len(local_history[ind]) < 16 else local_history[ind][0:16])
        
##############################################
        if len(ga_value) == 0: 
            ga_value.append(bin(int(line[1])).split('b')[1][-8:])
        else:
            ga_temp = ga_value[index-1] + bin(int(line[1])).split('b')[1][-8:]
            ga_value.append(ga_temp if len(ga_temp) < ga_bits else  ga_temp[-ga_bits:])
        
        
        line = f.readline()
        line = line.split(" ") 
        index+=1         

#pdb.set_trace()

#pdb.set_trace()
print('d')
#storing the pc (32 bits), ghr (512 bits), lhr (16 bits) and ga (48*8 bits) values

for i in range(len(pc_value)):
    #pc
    pc.append(['0']*(pc_bits-len(pc_value[i])) + list(pc_value[i]) if len(pc_value[i]) < pc_bits else list(pc_value[i]))
    
    #ghr
    ghr.append( ['0']*(ghr_bits-len(bin(history_value[i]).split('b')[1])) + list(bin(history_value[i]).split('b')[1]) if len(bin(history_value[i]).split('b')[1]) < ghr_bits else list(bin(history_value[i]).split('b')[1][-ghr_bits:]))

    #lhr
#    lhr.append( ['0']*(lhr_bits-len(bin(history_value[i]).split('b')[1])) + list(bin(history_value[i]).split('b')[1]) if len(bin(history_value[i]).split('b')[1]) < lhr_bits else list(bin(history_value[i]).split('b')[1][-lhr_bits:]))
    
    #ga
    ga.append( ['0']*(ga_bits-len(ga_value[i])) +list(ga_value[i]) if len(ga_value[i]) < ga_bits else list(ga_value[i]) ) 

    
#pdb.set_trace()
print('d')



in_conv=[]

for j in range(len(pc)):
    in_conv.append(pc[j]+ghr[j]+lhr[j]+ga[j])   #944 bits input
    
in_array=np.array(in_conv, dtype=float)
out_array=np.array(output_conv, dtype=float)
#
#for i in range(len(in_conv)):
#    for j in range(len(in_conv[0])):
#        in_array[i][j]=float(in_conv[i][j])
#
#for i in range(len(in_conv)):
#    out_array[i]=float(output_conv[i])
        
train_size = int(len(in_conv)*0.9)
test_size = len(in_conv) - train_size
indices   = list(range(len(in_conv)))
split     = test_size

random_seed = 0
np.random.seed(random_seed)
np.random.shuffle(indices)
random.seed(random_seed)
train_indexes, test_indexes = indices[split:], indices[:split]

#pdb.set_trace()

#inserting column names
colin= [x for x in range(len(in_conv[0]))]
colout= [y for y in range(len(output_conv[0]))]

with open (filename.split('dataset/')[0]+'dataset/'+'final_dataset/'+'train_in.csv', 'a') as f:
    wr = csv.writer(f, dialect='excel')
    wr.writerow(colin)
    for i in train_indexes:
        wr = csv.writer(f, dialect='excel')
        wr.writerow(in_conv[i])

with open (filename.split('dataset/')[0]+'dataset/'+'final_dataset/'+'train_label.csv', 'a') as f:
    wr = csv.writer(f, dialect='excel')
    wr.writerow(colout)
    for i in train_indexes:
        wr = csv.writer(f, dialect='excel')
        wr.writerow(output_conv[i])
            
            
with open (filename.split('dataset/')[0]+'dataset/'+'final_dataset/'+'test_in.csv', 'a') as f:
    wr = csv.writer(f, dialect='excel')
    wr.writerow(colin)
    for i in test_indexes:
        wr = csv.writer(f, dialect='excel')
        wr.writerow(in_conv[i])

with open (filename.split('dataset/')[0]+'dataset/'+'final_dataset/'+'test_label.csv', 'a') as f:
    wr = csv.writer(f, dialect='excel')
    wr.writerow(colout)  
    for i in test_indexes:
        wr = csv.writer(f, dialect='excel')
        wr.writerow(output_conv[i])            
            
            
            