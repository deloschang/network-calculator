#!/bin/bash
# Script name: getaddr.sh
#
# Description: grabs the ip address and subnet mask from ifconfig
#
# Command line options: ./getaddr <filename>
#   the filename to save the ipaddress and subnet mask to
#
# Input:  filename to save the ip address /subnet mask to
#
# Output: will fill the file with address and masek from parsing
#
#
# Special Considerations
#   Called by network.c for "automatic mode"
#   works for ethernet (eth0) in the foramt
#   inet <ip address>  netmask <netmask here>
#   if the filename alrady exists, it will overwrite
#
# Pseudocode
# Parses the ifconfig eth0 for the ip address and subnet mask
# Sends this file to the argument filename output
# e.g. ./getaddr address.dat will send the ipaddress/subnet mask
# to the address.dat


# check for sufficient arguments (1 filename)
if (( $# != 1 )); then
  echo "Error. Need a filename input parameter. e.g.: address.dat"
  exit 1
fi

# grab ip address from ifconfig
ip_addr=`ifconfig eth0 | grep 'inet' | head -n 1 | awk '{print $2}'`

# append ip address to file
# clear file if it exists already
echo $ip_addr > $1
echo "IP address saved.";

# grab the subnet mask from ifconfig
subnet_mask=`ifconfig eth0 | grep 'inet' | head -n 1 | awk '{print $4}'`

# append subnet mask to file
echo $subnet_mask >> $1
echo "Subnet mask saved.";

# successful return status
exit 0
