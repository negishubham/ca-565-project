#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Nov 28 19:47:07 2020

@author: snegi
"""
import pandas as pd
import numpy as np
import os
import torch
import torch.nn as nn
from torch.autograd import Variable
from torchvision import transforms
from torch.utils.data.dataset import Dataset  # For custom datasets

from models import *
from utils import *
import time
import argparse
import random
import pdb

import models
    
    
    
    
    
    
    
    
if __name__ == "__main__":
    # prepare the options
    parser = argparse.ArgumentParser()
    parser.add_argument('--cpu', action='store_true',
            help='set if only CPU is available')
    parser.add_argument('--arch', action='store', default='mlp3layer',
            help='the architecture for the network: resnet')
    parser.add_argument('--model', '-a', metavar='MODEL', default='LeNet5')

    parser.add_argument('--lr', action='store', default=1e-3, type=float, 
            help='the intial learning rate')
    parser.add_argument('--momentum', action='store', default=0.9, type=float, 
            help='momentum')
    parser.add_argument('--evaluate', action='store_true',
            help='evaluate the model')
    parser.add_argument('-b', '--batch_size', default=32, type=int,
                    metavar='N', help='mini-batch size (default: 256)')
                       
    parser.add_argument('-j', '--workers', default=8, type=int, metavar='N',
                    help='number of data loading workers (default: 8)')
    parser.add_argument('--epochs', default=30, type=int, metavar='N',
                    help='number of total epochs to run')
    parser.add_argument('--pretrained', action='store', default=None,
            help='the path to the pretrained model')        
    parser.add_argument('--gpus', default='0',
                    help='gpus used for training - e.g 0,1,3')

    parser.add_argument('--save', metavar='SAVE', default='',
                    help='saved folder')
    parser.add_argument('--results_dir', metavar='RESULTS_DIR', default='./final_results',
                    help='results dir')
    parser.add_argument('--input_size', type=int, default=None,
                    help='image input size')
    parser.add_argument('--print-freq', '-p', default=5, type=int,
                    metavar='N', help='print frequency (default: 20)')
    parser.add_argument('--weight-decay', '--wd', default=1e-4, type=float,
                    metavar='W', help='weight decay (default: 5e-4)')
    parser.add_argument('--start-epoch', default=0, type=int, metavar='N',
                    help='manual epoch number (useful on restarts)')
    parser.add_argument('--save-every', dest='save_every',
                    help='Saves checkpoints at every specified number of epochs',
                    type=int, default=10)    
    best_prec1 = 0
    class CustomDatasetFromCSV(Dataset):
        def __init__(self, input_path, label_path, transform=None):
#            pdb.set_trace()
            self.data = pd.read_csv(input_path)
    #        pdb.set_trace()
            self.labels = pd.read_csv(label_path)
    
            self.transform = transform
    
        def __getitem__(self, index):

#            input_vec = np.asarray(self.data.iloc[index][0:944])
#            pdb.set_trace()
            input_vec = np.asarray(self.data.iloc[index][0:944]).reshape(1, 1, 944)
            
            output_vec = torch.from_numpy(np.asarray(self.labels.iloc[index]))
            
            if self.transform is not None:
                input_vec_as_tensor = torch.from_numpy(input_vec).float()
            
            return (input_vec_as_tensor, output_vec)
    
        def __len__(self):
            return len(self.data.index)
    
    args = parser.parse_args()
    print('==> Options:',args)
    save_path = os.path.join(args.results_dir, args.save)
    if not os.path.exists(save_path):
        os.makedirs(save_path)

    new_manual_seed = 0
    torch.manual_seed(new_manual_seed)
    torch.cuda.manual_seed_all(new_manual_seed)
    np.random.seed(new_manual_seed)
    torch.backends.cudnn.deterministic = True  
    torch.backends.cudnn.benchmark = False     
    random.seed(new_manual_seed)
    os.environ['PYTHONHASHSEED'] = str(new_manual_seed)
    os.environ['CUDA_VISIBLE_DEVICES'] = args.gpus
    
#    model = LeNet5()
    model = models.__dict__[args.model]()
    print(model)
    model.cuda()
    pdb.set_trace()
    if args.gpus and len(args.gpus) > 1:
        model = torch.nn.DataParallel(model)     
    transformations = transforms.Compose([transforms.ToTensor()])
  
#    direc = '/home/nano01/a/snegi/Projects/ca-565-project/cbp4/bpc6421AU16/traces/dataset/final_dataset/'
    
    direc = '/home/nano01/a/snegi/Projects/ca-565-project/cbp4/bpc6421AU16/traces/dataset/final_dataset_LONG-SPEC2K6-06/'
#    pdb.set_trace()
#    direc = '/home/nano01/a/snegi/Projects/ca-565-project/cbp4/bpc6421AU16/traces/dataset/' + args.results_dir.split('/')[len(args.results_dir.split('/'))-1]+ '/'

    
    train_file = [direc + 'train_in.csv', direc + 'train_label.csv']
    
    test_file = [direc + 'test_in.csv', direc + 'test_label.csv']    
    

    print('Train file: ', train_file[0])
    print('Test file: ', test_file[0])
    
    if not args.evaluate:
        custom_train_from_csv = CustomDatasetFromCSV(train_file[0], train_file[1], transformations)
    custom_test_from_csv = CustomDatasetFromCSV(test_file[0], test_file[1], transformations)

    if not args.evaluate:
        train_loader = torch.utils.data.DataLoader(dataset=custom_train_from_csv,num_workers=args.workers,
                                                    batch_size=args.batch_size,
                                                    shuffle=True)
    test_loader = torch.utils.data.DataLoader(dataset=custom_test_from_csv,num_workers=args.workers,
                                                    batch_size=args.batch_size,
                                                    shuffle=False)    
    
    
    
    criterion = nn.CrossEntropyLoss().cuda()
    
    optimizer = torch.optim.SGD(model.parameters(), args.lr, momentum=args.momentum, weight_decay=args.weight_decay)

#    setup_logging(os.path.join(save_path, 'log.txt'))
    results_file = os.path.join(save_path, 'log.%s')
    results = ResultsLog(results_file % 'txt', results_file % 'html')


#    optimizer = torch.optim.Adam(model.parameters(), lr=args.lr)
#    scheduler = torch.optim.lr_scheduler.StepLR(optimizer, step_size=1, gamma=0.2)

    def train(train_loader, model, criterion, optimizer, epoch):
        """
            Run one train epoch
        """
        batch_time = AverageMeter()
        data_time = AverageMeter()
        losses = AverageMeter()
        top1 = AverageMeter()
        
        # switch to train mode
        
        model.train()
        
        end = time.time()
        for i, (input, target) in enumerate(train_loader):
        
            # measure data loading time
            data_time.update(time.time() - end)
        
            target = target.cuda()
            input_var = input.cuda()
            target_var = target

            # compute output
#            pdb.set_trace()
            output = model(input_var)
            loss = criterion(output, torch.max(target_var, 1)[1])
        
            # compute gradient and do SGD step
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
        
            output = output.float()
            loss = loss.float()
            # measure accuracy and record loss
            prec1 = accuracy(output.data, torch.max(target_var, 1)[1])[0]
            losses.update(loss.item(), input.size(0))
            top1.update(prec1.item(), input.size(0))
        
            # measure elapsed time
            batch_time.update(time.time() - end)
            end = time.time()
        
            if i % args.print_freq == 0:
                print('Epoch: [{0}][{1}/{2}]\t'
                      'Time {batch_time.val:.3f} ({batch_time.avg:.3f})\t'
                      'Data {data_time.val:.3f} ({data_time.avg:.3f})\t'
                      'Loss {loss.val:.6f} ({loss.avg:.6f})\t'
                      'Prec@1 {top1.val:.3f} ({top1.avg:.3f})'.format(
                          epoch, i, len(train_loader), batch_time=batch_time,
                          data_time=data_time, loss=losses, top1=top1))              
        return losses.avg, top1.avg 
    
    
    def validate(val_loader, model, criterion):
        """
        Run evaluation
        """
        batch_time = AverageMeter()
        losses = AverageMeter()
        top1 = AverageMeter()
        
        # switch to evaluate mode
        model.eval()
        
        end = time.time()
        with torch.no_grad():
            for i, (input, target) in enumerate(val_loader):
                target = target.cuda()
                input_var = input.cuda()
                target_var = target.cuda()
        
                # compute output
                output = model(input_var)
                loss = criterion(output, torch.max(target_var, 1)[1])
        
                output = output.float()
        #            pdb.set_trace()
                loss = loss.float()
        
                # measure accuracy and record loss
                prec1 = accuracy(output.data, torch.max(target_var, 1)[1])[0]
                losses.update(loss.item(), input.size(0))
                top1.update(prec1.item(), input.size(0))
        
                # measure elapsed time
                batch_time.update(time.time() - end)
                end = time.time()
        
                if i % args.print_freq == 0:
                    print('Test: [{0}/{1}]\t'
                          'Time {batch_time.val:.3f} ({batch_time.avg:.3f})\t'
                          'Loss {loss.val:.4f} ({loss.avg:.4f})\t'
                          'Prec@1 {top1.val:.3f} ({top1.avg:.3f})'.format(
                              i, len(val_loader), batch_time=batch_time, loss=losses,
                              top1=top1))
                
        print(' * Prec@1 {top1.avg:.3f}'
              .format(top1=top1))
        return losses.avg, top1.avg
    
    def save_checkpoint(state, is_best, filename='checkpoint.pth.tar'):
        """
        Save the training model
        """
        torch.save(state, filename)
    
    class AverageMeter(object):
        """Computes and stores the average and current value"""
        def __init__(self):
            self.reset()
        
        def reset(self):
            self.val = 0
            self.avg = 0
            self.sum = 0
            self.count = 0
        
        def update(self, val, n=1):
            self.val = val
            self.sum += val * n
            self.count += n
            self.avg = self.sum / self.count
    
    
    def accuracy(output, target, topk=(1,)):
        """Computes the precision@k for the specified values of k"""
        maxk = max(topk)
        batch_size = target.size(0)
        
        _, pred = output.topk(maxk, 1, True, True)
        pred = pred.t()
        correct = pred.eq(target.view(1, -1).expand_as(pred))
        
        res = []
        for k in topk:
            correct_k = correct[:k].view(-1).float().sum(0)
            res.append(correct_k.mul_(100.0 / batch_size))
        return res    


    def adjust_learning_rate(optimizer, epoch):
        update_list = [15,30]
        lr_list = [1e-2,1e-3]
        if epoch in update_list:
            for param_group in optimizer.param_groups:
                index_epoch = update_list.index(epoch)
            optimizer.param_groups[0]['lr'] = lr_list[index_epoch]
        return

    for epoch in range(args.start_epoch, args.epochs):
        adjust_learning_rate(optimizer, epoch)
        # train for one epoch
        print('current lr {:.5e}'.format(optimizer.param_groups[0]['lr']))
        train_result= train(train_loader, model, criterion, optimizer, epoch)

        test_result = validate(test_loader, model, criterion)

        is_best = test_result[1] > best_prec1
        best_prec1 = max(test_result[1], best_prec1)
        
        if epoch > 0 and epoch % args.save_every == 0:
            save_checkpoint({
                'epoch': epoch + 1,
                'state_dict': model.state_dict(),
                'best_prec1': best_prec1,
            }, is_best, filename=os.path.join(args.results_dir, 'checkpoint.th'))

        save_checkpoint({
            'state_dict': model.state_dict(),
            'best_prec1': best_prec1,
        }, is_best, filename=os.path.join(args.results_dir, 'model.th'))
        if is_best:
            save_checkpoint({
                'state_dict': model.state_dict(),
                'best_prec1': best_prec1,
                'epoch': epoch+1,
            }, is_best, filename=os.path.join(args.results_dir, 'best_model.th'))            
        print('Best Accuracy:{}'.format(best_prec1))

        results.add(epoch=epoch, train_acc=train_result[1], train_loss=train_result[0], test_acc=test_result[1])
        results.save()
#        scheduler.step()
    

    
    