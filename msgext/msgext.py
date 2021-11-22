#!/usr/bin/env python3

import rosbag
import sys
import argparse
import os

base_path = 'workspace' #base folder location, defaults to 'workspace' in working directory 

def mkdir(string):
    direct = base_path
    direct_arr = string.split('/')

    for i in direct_arr:
        direct = (direct + '/' + i)
        if i in direct_arr[:-1]:
            try:
                os.mkdir(direct)
                print('Created: ' + direct)
            except FileExistsError:
                print(' ')
    return(direct)


def main():

    try:
        os.mkdir(base_path)
    except FileExistsError:
        print('INFO: Workspace already exists')
     
    types = {}

    for bagname in sys.argv[1:]:
        bag = rosbag.Bag(bagname)
        for topic, msg, t in bag.read_messages():
            types[msg._type] = msg._full_text 

    for t in types:

        # Print msg def to std out first #
        print ("Message type:", t)
        print ("Message text:")
        print (types[t])
        print ()
        ##################################
        
        direct = mkdir(t)
        f = open(direct + '.msg', 'w+')
        lines = types[t].split('MSG: ',-1)
        first = True
        for x in lines:
            if first:
                f.write(x)
                first = False
                f.close()
            else:
                sublines = x.split('\n',-1)
                path = sublines[0].strip('MSG: ')
                
                subtype = mkdir(path)
                f = open(subtype + '.msg', 'w+')
                first2 = True
                for i in sublines:
                    if(first2):
                        first2 = False
                    else:
                         f.write(i + '\n')
                f.close()
      
      


    
