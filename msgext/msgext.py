#!/usr/bin/env python3

from rosbag.bag import Bag
import rospy
import rosbag
import sys
import argparse
import os
import glob
import yaml

class msg:
    def __init__(self,msg_name,msg_body,msg_sub_path):
        self.msg_name = msg_name
        self.msg_body = msg_body
        self.msg_sub_path = msg_sub_path

def mkdir(string,workspace_name):
    dir = workspace_name
    dir_arr = string.split('/')

    for i in dir_arr:
        dir = (dir + '/' + i)
        if i in dir_arr[:-1]:
            try:
                os.mkdir(dir)
                print('Created: ' + dir)
            except FileExistsError:
                print(' ')
    return(dir)

def main():
    
    parser = argparse.ArgumentParser(description='Read, analyze and build temp workspaces containing .msg definitions from a ROS .bag file,.', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        'bag', 
        nargs='+', 
        help='Path of a bag file or a folder of bag files.')
    args = parser.parse_args()

    filename = args.bag[0]

    bag = rosbag.Bag(filename)

    for connection in bag._connections.values():
        print(connection.msg_def)
        break

if __name__ == "__main__":
    main()