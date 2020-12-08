#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Nov 28 20:46:59 2020

@author: snegi
"""
import torch.nn.functional as F
import torch.nn as nn
import torch.nn.init as init
from collections import OrderedDict
import pdb


def _weights_init(m):
    classname = m.__class__.__name__
    #print(classname)
    if (isinstance(m, nn.Linear) or isinstance(m, nn.Conv2d)):
        init.kaiming_normal_(m.weight)
        print('Initializing')
        
        
class LeNet5(nn.Module):          
     
    def __init__(self):     
        super(LeNet5, self).__init__()
        # Convolution (In LeNet-5, 32x32 images are given as input. Hence padding of 2 is done below)
        self.conv1 = nn.Conv2d(in_channels=1, out_channels=20, kernel_size=(1,8), stride=1, padding=0, bias=True)
        # Max-pooling
        self.max_pool_1 = nn.MaxPool2d(kernel_size=(1, 2))
        # Convolution
        self.conv2 = nn.Conv2d(in_channels=20, out_channels=50, kernel_size=(1, 8), stride=1, padding=0, bias=True)
        # Max-pooling
        self.max_pool_2 = nn.MaxPool2d(kernel_size=(1, 2)) 
        # Fully connected layer
        self.fc1 = nn.Linear(50*1*230, 500)   # convert matrix with 16*5*5 (= 400) features to a matrix of 120 features (columns)
        self.fc2 = nn.Linear(500, 2)       # Notaken(0), taken(1)
        self.sig = nn.Sigmoid()
        self.apply(_weights_init)
    def forward(self, x):
        # convolve, then perform ReLU non-linearity
        x = F.relu(self.conv1(x))  
        # max-pooling with 1x2 grid 
        x = self.max_pool_1(x) 
        # convolve, then perform ReLU non-linearity
        x = F.relu(self.conv2(x))
        # max-pooling with 2x2 grid
        x = self.max_pool_2(x)
        # first flatten 'max_pool_2_out' to contain 16*5*5 columns
        # read through https://stackoverflow.com/a/42482819/7551231
        x = x.view(-1, x.shape[1]*x.shape[2]*x.shape[3])
        # FC-1, then perform ReLU non-linearity
        x = F.relu(self.fc1(x))
        # FC-2, then perform ReLU non-linearity
        x = self.sig(self.fc2(x))
        
        return x