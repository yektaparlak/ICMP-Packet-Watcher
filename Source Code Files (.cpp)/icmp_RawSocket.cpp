/* Prepared by Yekta Parlak - August 2022 */

/* Steps:
1- Define the ICMP header structure
2- Checksum calculation of ICMP packet
3- Create an original socket with a protocol type of IPPROTO_ICMP, set the properties of the socket.
4- Create and initialize ICMP packets.
5- Call the sendto function to send an ICMP request to the remote host.
6- Call the recvfrom function to receive the ICMP response.


/* To compile: g++ *.cpp -o pingraw.exe -lws2_32 -fPIC -static -static-libgcc -static-libstdc++
   To run: ./pingraw + IP adress  (for instance ./pingraw 1.1.1.1) */

#include <stdio.h>
#include <Ws2tcpip.h>
#include <Winsock2.h>
#include <iostream>
#pragma comment (lib, "ws2_32.lib") 										// For linking the dynamic library of WinSock2   

using namespace std;  

#define DataLength 32 																				

typedef struct ICMP_Header  
{ 
    unsigned char   icmp_type;												// The message type, for example [0 - echo reply], [8 - echo request] ------------------------------->Type description occupies 8 bits
    unsigned char   icmp_code;												// This is significant when sending an error message (unreach), and specifies the kind of error ----->Code description occupies 8 bits
    unsigned short  icmp_checksum;											// The checksum for the icmp header + data. same as the IP checksum. -------------------------------->Checksum description occupies 16 bits
    unsigned short  icmp_id;												// An identifier that uniquely identifies this request, usually set to the process ID
    unsigned short  icmp_sequence;											// Identifies the sequence of echo messages, if more than one is sent. (as knwon as serial number)	
	unsigned long   icmp_timestamp; 										// latency (Its data size counts inside the data buffer as well)
} ICMP_Header; 


// Checksum calculation.
unsigned short checksum(unsigned short* buff, int size)
{
	unsigned long cksum = 0;							 					//initialize the internet checksum to 0 at the beginning
	while(size>1)										 					//Sum all the words together, adding the final byte if size is odd
	{					
		cksum += *buff++;								 					//Add the data to be verified bit by bit every 16 bits and save it in cksum
		size -= sizeof(unsigned short);					 					//After adding every 16 bits, subtract 16 from the amount of checksum data
	}					

	if(size)											 					//If the data to be checked is an odd number, after the loop, the content of the last byte needs to be added to the previous result

	cksum += *(char*)buff;					

    cksum = (cksum >> 16) + (cksum & 0xffff);    							//The previous result produced a carry, and the carry needs to be added to the final result.
																			//Adds the left and right 16-bit words in the 32-bit integer. It basically splits the number in half and adds the two halves together. sum>>16 gives you the left half, and sum & 0xffff gives you the right half.


	cksum += (cksum >> 16);							    					//Then when these 2 are added together, they could possible overflow.
       																		//This line below, adds the overflow back into the original number.

	return (unsigned short)(~cksum);				    					// Return the bitwise complement of the resulting mishmash
}



int main(int argc, char *argv[ ]) 
{ 

	/***** Start WinSock ****/				              		            // read it: https://docs.microsoft.com/en-us/windows/win32/winsock/initializing-winsock
	WSADATA wsaData;                                  			            // The WSADATA structure contains information about the Windows Sockets implementation.
	int ret;
	ret = WSAStartup(MAKEWORD(2,2), &wsaData);					            // read it: https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-sendto?source=recommendations
	
	
		if((ret=WSAStartup(MAKEWORD(1,0),&wsaData))!=0)
		{
			cout<<"Error to start WinSock 2.2";
			exit(0);
		}

	char szDestIp[256] ={0}; 									            // Store the IP address or domain name to be pinged

	    // If a user typed more than 1 argument, check it.
		if (argc > 2)
		{
		 	cout<<"\nCheck your argument again!\n"<<endl;
		 	return -1;
		}

	strcpy(szDestIp,argv[1]);
		
	unsigned long ulDestIP = inet_addr(szDestIp);   			            //Converts a dotted decimal IP address to a 32-bit binary representation of the IP address
															                //In other words, inet_addr function converts the network host address (such as 192.168.1.10) to a network endian binary value


      	// Check the written destination IP address whether in right format or not.			    
		if(ulDestIP == INADDR_NONE)
		{
			cout<<"\nWrong IP address format. Host not found!\n"<<endl;
			WSACleanup();
			return -1;
		}
	

    // Create a raw socket for sending and receiving ICMP packets 
    SOCKET sRaw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);    		        /*It is a standard command to create a datagram raw socket.
                                                                            From the moment that it is created, you can send any ICMP packets over it, and
                                                                            receive any ICMP packets that the host received after that socket was created. 
                                                                            The protocol number for ICMP is 1. */
															                // read it: http://www.cs.binghamton.edu/~steflik/cs455/rawip.txt

																		  	// About IPPROTO_IP: It creates a socket that sends/receives raw data for IPv4-based protocols.


	/*Set receive timeout*/
	int Timeout=10000; 																			// Set the receiving timeout, skip receiving if it is not received within 10 seconds
	ret = setsockopt(sRaw, SOL_SOCKET, SO_RCVTIMEO, (char*)&Timeout, sizeof(Timeout));        	
																								 /* SO_RCVTIMEO is an option to set a timeout value for input operations.

	/* The sockaddr_in structure specifies the address family.
	The server, port and IP address of the server to be connected to. */                                                                                           
	sockaddr_in RecvAddr;
	RecvAddr.sin_family = AF_INET; 	  				                 // address family					                										
	RecvAddr.sin_port = htons(0); 						             // port numebr										
	RecvAddr.sin_addr.s_addr = inet_addr(szDestIp); 	             // destination IP adress	        /* Extra Note: */
																						         		// SOCKET socket(
																							     		// int af,		    -> Used to specify the address format used by the socket, only AF_INET is supported in WInsock
																							     		// int type,	    -> Used to specify the type of socket
																							     		// int protocol	-> Used with the type parameter to specify the protocol type used. Can be IPPROTO_ICMP
																							     		// );

	// ICMP packet creation
	char buff[sizeof(ICMP_Header) + DataLength + 8];				// This buffer should also be large enough to also hold 8 more bytes of data (the size of an ICMP error message) (rfc792 says).
	ICMP_Header* pIcmp = (ICMP_Header*)buff; 						/* Cast the binary data to our struct. Means, convert the array to the ICMP header type */


	//Fill in the ICMP packet data                                  // read it: https://book.huihoo.com/iptables-tutorial/x1078.htm
	pIcmp->icmp_type = 8;	                                        /* The type field contains the ICMP type of the packet. This is always different from ICMP type to type.
																	For example ICMP Destination Unreachable packets will have a type 3 set to it. For a complete listing of
																	the different ICMP types, see the ICMP types appendix. This field contains 8 bits total. */

	pIcmp->icmp_code = 0;										    /* All ICMP types can contain different codes as well. Some types only have a single code, while others have
																	several codes that they can use. For example, the ICMP Destination Unreachable (type 3) can have at least
																	code 0, 1, 2, 3, 4 or 5 set. Each code has a different meaning. */

	pIcmp->icmp_id = (USHORT)::GetCurrentProcessId(); 				/*Get process number as ID  uniquely. It can be adjustable specifically */

	pIcmp->icmp_checksum = 0; 										/*The Checksum is a 16 bit field containing a one's complement of the ones complement of the headers starting with the ICMP type and down.
																	While calculating the checksum, the checksum field should be set to zero */

	pIcmp->icmp_sequence = 0; 										/* serial number */

	pIcmp->icmp_timestamp = 0; 				


	// Fill data part
	memset(&buff[sizeof(ICMP_Header)], 'Y', DataLength);          // Embed inside the data part with a single letter multiple times
		
																										
	// Start sending and receiving ICMP packets 
	USHORT nSeq = 1;   			   								  //Sequence number of the sent ICMP packet
	char recvBuf[1024] = { 0 };	   								  //Define the receive buffer. We need a pointer to the memory space where the binary data of the packet is stored
	sockaddr_in from;   		   								  //Save the source address of the received data
	int nLen = sizeof(from);       								  //address length
	ICMP_Header* pRecvIcmp; 	   								  //pointer to the header of the ICMP message. It means, cast binary data to a data structure
	static int Number = 0;				 
	bool bTimeout = FALSE;
		
		int n;
		cout<<"Enter time interval in milliseconds:"<<endl;
		cin>>n;



		long long int m;
		cout<<"Enter ping count:"<<endl;
		cin>>m;

	for (long long int i = 0; i < m; ++i) 
 	
	{ 

		int nRet; 

		
		pIcmp->icmp_checksum  = 0;
		pIcmp->icmp_timestamp = GetTickCount();
		pIcmp->icmp_sequence  = nSeq++; 
		pIcmp->icmp_checksum  = checksum((unsigned short*)buff, sizeof(ICMP_Header) + DataLength); 



		// The sendto function sends data to a specific destination.
		/*  read it: https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-sendto?source=recommendations */
		nRet = sendto(sRaw, buff, sizeof(ICMP_Header) + DataLength, 0, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr)); 


		// The recvfrom function receives a datagram, and stores the source address.
		/* read it: https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-recvfrom?source=recommendations */
		nRet = recvfrom(sRaw, recvBuf, 1024, 0, (sockaddr*)&from, &nLen); 



			if(nRet == SOCKET_ERROR)
			{
				if(WSAGetLastError() == WSAETIMEDOUT)
				{
					cout<<" timed out!\n";
					bTimeout=TRUE;   //receive time out
					break;
				}
				cout<<"Receiving failed! Error code:"<<WSAGetLastError()<<endl;
				return -1;

			}
		

		// parsing(ayristirma) the received ICMP packet
		int nTick = GetTickCount(); 


		ICMP_Header* pRecvIcmp = (ICMP_Header*)(recvBuf + 20);  		   					    //The received data contains 20 bytes of IP header. If we type cast binary data to structs we have to add the length of the last header to the data pointer to get the beginning of the next header
																	    						/*   next_header = (struct header*) (buffer + sizeof(struct header));   */

			cout<<'\n';
			cout<< DataLength <<" Bytes payload, sent to "<<inet_ntoa(from.sin_addr)<<'\n';     //  inet_ntoa() function converts the binary value transmitted by the network into a dot-decimal IP address (standard ASCII point-separated address,)
			cout<<"Round Trip Time (RTT): "<<nTick -pRecvIcmp->icmp_timestamp<<" ms"<< '\n';
			cout<<"Time Interval: "<< n<< " ms"<< '\n';
			cout<<"Ping Count: "<<++Number<< '\n';
					
			if (SOCKET_ERROR == 0)
            	{
            	    cout << "FAILED!" << endl;
				}
            	else
            	{
            	    cout << "OK" << endl;
            	}

				int milli_seconds=n; 																								
		Sleep(milli_seconds);    
  } 
	closesocket(sRaw);
	WSACleanup();
 	return 0;
} 


/* Data must include min. 8 bytes of data for ICMP error messages. The data portion is mandatory in reply packets, if there is a data portion in the
/* request packet. The answering system must send a reply containing exactly the same data portion. The RFC792 does not specify if the data portion
/* in the request packet is mandatory. The Data in this schema is not marked as (optional) as in many other RFCs. */
																				
 /*  0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
     |     Type      |        Code   |          Checksum            |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
     |           Identifier          |        Sequence Number       |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
     |     Data ...														  // read it: https://superuser.com/questions/1086312/why-does-icmp-echo-request-and-echo-reply-packets-contain-a-data-portion 
     +-+-+-+-+-                                                          */  
																				
																				
/* However, sending a ping with zero bytes of data is entirely possible. On Windows try: ping <destination> -l 0
																				
To put in a nutshell, this proves that the Data-portion in an ICMP-Echo-Request is optional.  */


/* To inject our own packets, all we need to know is the structures of the
protocols that need to be included. Below you will find a short introduction to
the ICMP header. */

/* The data types/sizes we need to use are:
 unsigned char      --> 1 byte (8 bits),
 unsigned short int --> 2 bytes (16 bits)
 unsigned int       --> 4 bytes (32 bits) */
// read it : http://www.cs.binghamton.edu/~steflik/cs455/rawip.txt

 /* IP itself has no mechanism for establishing and maintaining a connection, or
 even containing data as a direct payload. Internet Control Messaging Protocol
is an addition to IP to carry error, routing and control messages and
 data, and is often considered as a protocol of the network layer. */	


/* ============================================================================== */


/* The usage of Timestamp:

read it: https://www.rfc-editor.org/rfc/rfc1122 

The use of the ICMP Timestamps is optional, and may, or may not, be implemented.
You may find many hosts or network devices do not implement this option.  */