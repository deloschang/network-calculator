/*	network.c	program that can take subnet CIDR and ip address to output network address etc.
                      or automatically find the ip address / subnet mask from ifconfig
                      has both AUTOMATIC and INTERACTIVE mode

	Copyright 2013

	License none

	Project name: network.c
  Version name: v1.0

	This file contains a program that takes in full IP address (e.g. 192.168.5.10) and 
    a network prefix (e.g. 24 in CIDR form /24) and prints out the network address, 
    host portion, full subnet mask and more.

    Automatic mode - retrieve the subnet mask and IP address from ifconfig
    and perform similar calculations
	
	Primary Author:	 Delos Chang
	Date Created:	 4/11/13

	Special considerations:  
    Assumes that getaddr.sh is in the same dir 
    Assumes Linux computer running eth0 (for automatic mode).
    The bash script "getaddr.sh" will overwrite the filename labeled in the constant,
    ADDRESS
    Read the README and TESTING files in this dir for more info
    You can change the bash script name and filename in CONSTANTS

    INTERACTIVE MODE info.
        Full IP address should be like 192.168.2.1 format (with dots)
        Prefix should be like 24 or 32 ( not /24 or /32).

    AUTOMATIC MODE info.
    The bash script will check ifconfig and print out a filename in the format:
    
         <ip_addr>  (e.g. 192.168.1.1)
         <subnet_mask> (e.g. 255.255.255.0)
         
    To learn more check the README and TESTING files

======================================================================*/
// do not remove any of these sections, even if they are empty
//
// ---------------- Open Issues 

// ---------------- System includes e.g., <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// used to test ip address validity
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// ---------------- Local includes  e.g., "file.h"

// ---------------- Constant definitions 
#define IPADDRESS_LENGTH 32    // default length of an IP Address (IPv4)
#define ADDRESS "address.dat"  // filename for bash script to store in
#define BASH "getaddr.sh " ADDRESS // uses the constant from ADDRESS

// ---------------- Macro definitions

// ---------------- Structures/Types 

// ---------------- Private variables 
int menuopt;  // character option for menu

char hostaddress[20]; // for the full ip address

int prefix;  // subnet mask 

FILE *f; // file pointer for the address file (for automatic)

// for automatic mode
char ip_addr[30];
char buffer[30];
char subnet[30];

// ---------------- Private prototypes 

/*====================================================================*/

// prints the main menu and its options
void print_menu(){
    printf("\n");

    // user choices
    printf("Menu \n");
    printf("i - to print the IP address details with user input\n");
    printf("a - to print the IP address details automatically\n");
    printf("e - to exit the program\n");
    printf("enter command: ");
}

// Takes in char *ipAddress (e.g. 192.168.1.1) and returns 0 if not valid
// returns 1 if valid
// e.g. (256.23.12.1) is invalid
int test_valid_ip(char *ipAddress){
    struct sockaddr_in sa;
    
    // will return 1 if valid, 0 if not
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));

    return result;
}

// tests if prefix is valid
// 1 if valid 
// 0 if invalid
int test_valid_prefix(int prefix){
  // must be between 0 and 32
  if ( ( prefix >= 0) && ( prefix <= 32 ) ){
    return 1;

  } else {

    return 0;
  }
}

// this function will convert an unsigned long into IP address format
// e.g. 3232236800 ==> 192.168.5.0
void convert_bin_to_ip(int long_ip) {
    unsigned char octets[4];
    octets[0] = long_ip & 0xFF;
    octets[1] = (long_ip >> 8) & 0xFF;
    octets[2] = (long_ip >> 16) & 0xFF;
    octets[3] = (long_ip >> 24) & 0xFF;	
    printf("%d.%d.%d.%d\n", octets[3], octets[2], octets[1], octets[0]);        
}

// this function will calculate the host given the full address and subnet mask
// first it inverts the masks, then ANDs with the full address
void calculate_host(unsigned long full_address_bin, unsigned long subnet_mask){

  uint32_t inverted_mask = ~(subnet_mask); // invert mask using NOT operator
  unsigned long inverted = inverted_mask; 

  unsigned long host_bits = full_address_bin & inverted; // AND together
  
  // convert the host binary to IP address
  convert_bin_to_ip(host_bits); 
}

// converts the submask binary to a prefix by counting the 1 bits
// e.g. 255.255.255.0 => 24
int mask_to_prefix(unsigned long submask_bin){
  int prefix = 0;
  while (submask_bin){
    prefix += (submask_bin & 0x01);
    submask_bin >>= 1;
  }

  return prefix;
}

// given a prefix CIDR (e.g. 24), print the subnet mask (255.255.255.0)
// return subnet mask in binary
unsigned long prefix_to_mask(int *prefix){
  unsigned long submask_bin = (0xFFFFFFFF << (IPADDRESS_LENGTH - *prefix)) & 0xFFFFFFFF;
  printf("%lu.%lu.%lu.%lu\n", submask_bin >> 24, (submask_bin >> 16) & 0xFF, (submask_bin >> 8) & 0xFF, submask_bin & 0xFF);

  return submask_bin;
}


int main(int argc, char *argv[]){

  // if user doesn't use command 'e' for exit, keep showing menu
  do{
    // filter the CR after the getchar entry
    // because \n is also counted as a character
    if (menuopt != '\n'){
      print_menu();
    } 

    // get the character from user input
    menuopt = getchar();

    switch (menuopt){
      case 'i':
        //////////// INTERACTIVE MODE ////////////
        printf("\nEnter host address : ");
        scanf("%19s", hostaddress); // arrays already pointers so no &

        // Loop until valid host address
        if (!test_valid_ip(hostaddress)){
          printf("\n Invalid IP address. Try again. \n");
          printf("     e.g.: 129.170.213.202");

          // send extra characters to "invalid input"
          // return to menu
          break;
        }

        printf("\nEnter prefix: ");
        scanf("%19d", &prefix); 

        // Loop until valid prefix
        if (!test_valid_prefix(prefix)){
          printf("\n Invalid prefix.");
          printf("     e.g.: 24");

          // send extra characters to "invalid input"
          // return to menu
          break;
        }


        // print output
        printf("\nAddresses:\n");
        printf("Full IP Address:\t%s\n", hostaddress);

        // convert the subnet binary into numbers by right shifting and ANDing
        printf("Subnet Mask    :\t");
        // print the subnet mask (e.g. 255.255.255.0)
        unsigned long submask_bin = prefix_to_mask(&prefix);

        // Calculate the Network address
        printf("Network Address:\t");
        unsigned long network_unsigned = inet_addr(hostaddress); // converted to network order
        unsigned long ip_address_bin = ntohl(network_unsigned); // convert to host byte order
        unsigned long network_address_bin = ip_address_bin & submask_bin; // network address in bin

        convert_bin_to_ip(network_address_bin); // convert network address to IP


        // Calculate the Host portion
        printf("Host Portion   :\t");
        calculate_host(ip_address_bin, submask_bin);


        // Prefix portion
        printf("Prefix         :\t%d\n", prefix);
        break;

      case 'a':
        //////////// AUTOMATIC MODE ////////////

        // run address.sh bash script in current dir, which will return
        // the ip address and subnet mask to the address.dat file
        system(BASH);  // open the bash file (default: getaddr.sh)

        // the bash file (default: getaddr.sh) will take a filename (default: address.dat)
        // In the file, there will be a full IP address and subnet mask in the form:
        // <ip_addr>  (e.g. 192.168.1.1)
        // <subnet_mask> (e.g. 255.255.255.0)

        f = fopen(ADDRESS, "r"); 

        if (!f) {
          printf("No such file found when running bash script");
          exit(1);
        } else {
          // scanf all the character in the file until \n or 20 chars
          // validated IP address earlier, so there should be no overflows
          fscanf(f, "%20[^\n]", ip_addr); 

          // check if ip address exists
          if (ip_addr == NULL){
            printf(" No IP address found!");
            break;
          }

          // defensive check to make sure address is valid
          if (!test_valid_ip(ip_addr)){
            printf("\n Invalid IP address. Try again. \n");
            printf("     e.g.: 129.170.213.202");

            // send extra characters to "invalid input"
            // return to menu
            break;
          }

          // get first line and start at next line
          // validated IP address earlier, so there should be no overflows
          fgets(buffer, 30, f); // as long as ip and subnet are less than 30 chars
          fscanf(f, "%20[^\n]", subnet); 

          // check if subnet exists
          if (subnet == NULL){
            printf(" No subnet found!");
          }

          // check subnet mask is in correct form
          if (!test_valid_ip(subnet)){
            printf("\n Invalid Subnet Mask. Try again. \n");
            printf("     e.g.: 255.255.252.0");

            break;
          }

          // print output
          printf("\nAddresses:\n");
          printf("Full IP Address:\t%s\n", ip_addr);

          printf("Subnet Mask    :\t%s\n", subnet);

          // convert subnet mask to its binary representation
          unsigned long temp_submask_bin = inet_addr(subnet);
          unsigned long submask_bin = ntohl(temp_submask_bin);
          
          // Calculate the Network address using the binary subnet mask
          printf("Network Address:\t");
          unsigned long network_unsigned = inet_addr(ip_addr); // converted to network order
          unsigned long ip_address_bin = ntohl(network_unsigned); // convert to host byte order
          unsigned long network_address_bin = ip_address_bin & submask_bin; // network address in bin

          convert_bin_to_ip(network_address_bin); // convert network address to IP

          // Calculate host portion
          printf("Host Portion   :\t");
          calculate_host(ip_address_bin, submask_bin);

          // Prefix portion
          prefix = mask_to_prefix(submask_bin);
          printf("Prefix         :\t%d\n", prefix);
        }
        break;

      // exit
      case 'e':
        break;

      case '\n':
        break;

      // not 'i', 'a' or 'e'
      // Tell user that it was an invalid input and return to menu
      default:
        printf("\n\n---------------------------------------- \n");
        printf("\n| Not a valid input. Returning to menu. | \n");
        printf("----------------------------------------- \n");
        break;

    }

  } while (menuopt != 'e');

  // user character was an 'e'
  // Exit the menu 
  printf(".. exiting\n");
  exit(0);
}
