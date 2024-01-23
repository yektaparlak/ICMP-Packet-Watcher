/* Prepared by Yekta Parlak - September 2022 */

 // Steps:
 // 1. Use the function IcmpCreateFile to create an Icmp handle.
 // 2. Construct the API parameters. (IcmpHandle, ipaddr, SendData, sizeof(SendData), &ipOptions, ReplyBuffer, ReplySize, Timeout)
 // 3. Call the function IcmpSendEcho to send.
 // 4. Close the Icmp handle obtained by IcmpCreateFile.

 /* 1. IcmpCreateFile();  => Opens a handle on which IPv4 ICMP echo requests can be issued.
    2. IcmpSendEcho();    => Sends an IPv4 ICMP echo request and returns any echo response replies and the call returns when the time-out has expired or the reply buffer is filled.
    3. IcmpCloseHandle(); => Closes a handle opened by a call to the IcmpCreateFile or  IcmpCreateFile functions.*/

 /* To compile: g++ *.cpp -o pingapi.exe -lws2_32 -fPIC -static -static-libgcc -static-libstdc++ C:\Windows\System32\iphlpapi.dll
    and then enter: ./pingapi DestinationIP (such as ./pingapi 1.1.1.1) */

#define _WINSOCK_DEPRECATED_NO_WARNINGS //to suppress "inet_addr" deprecation warnings

#include <winsock2.h>
#include <iphlpapi.h>                                                                                   // IP helper function library: <<<< https://docs.microsoft.com/en-us/windows/win32/iphlp/ip-helper-functions?redirectedfrom=MSDN >>>>
#include <icmpapi.h>
#include <stdio.h>
#include <iostream>
#include <handleapi.h>                                                                                  
#include <ipexport.h>

using namespace std; 

#pragma comment(lib, "iphlpapi.lib")                                                                    // The IcmpSendEcho function is exported from the Icmp.dll on Windows 2000. The IcmpSendEcho function is exported from the Iphlpapi.dll which provides all the functions of the ICMP protocol
#pragma comment(lib, "ws2_32.lib")
                                                         
bool CheckIPAddr (unsigned long ipaddr)    
    {
        cout << "Checking IP address using CheckIPAddr function" << endl<< '\n';
        if (ipaddr == INADDR_NONE) 
        {
            return false;
        }
        else
        {
            return true;
        }
    }

int __cdecl main(int argc, char **argv) 
    {
        
     // Express and initialize variables

    HANDLE IcmpHandle;                                                                                   // Handle from IcmpCreateFile(). File handling in C++ is a mechanism to store the output of a program in a file and help perform various operations on it. Files help store these data permanently on a storage device.
    unsigned long ipaddr = INADDR_NONE;                                                                  // The destination IP address to be pinged
    DWORD dwRetVal = 0;                                                                                  // For calling function of IcmpSendEcho and DWORD means unsigned "double word.
    DWORD dwError = 0;
    // Payload to send                                                                                   /* Because you get the size of the array including a null terminator. with  '\0'  */                                                                                                                                                  
    char SendData[32] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',                    /* C strings don't store their own length, so a 'null terminator' (the character \0) is used to mark the end of the string. */                                                 
                         'p','q','r','s','t','u','v','w','a','b','c','d','e','f','g','h','i'};           /* About The Issue: https://stackoverflow.com/questions/72270575/hidden-byte-in-unsigned-char   */                                                 
                                                                                                         /* Alternative type: char SendData[32] = "abcdefghijklmnopqrstuvwabcdefghi"; */       
    LPVOID ReplyBuffer = NULL;                                                                 
    static int Number = 0;

    DWORD Timeout = 10000;                                                                                // Timeout is 10 seconds.
    static int TTL = 128;

        // Testout the parameters
        if (argc != 2) 
        {       
            cout<<'\n';
            cout<<argv[1]<<" is an invalid IPv4 address usage\n"<< argv[0];
            return 1;
        }


    // Parse the destination IP address                                                                   // Lookup: https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-inet_addr
    ipaddr = inet_addr(argv[1]);                                                                          // It converts a string containing an IPv4 dotted-decimal address into a proper address for the IN_ADDR structure.

        if (false == CheckIPAddr(ipaddr)) 
        {
            cout<<'\n';
            cout<<argv[1]<<" is an invalid IPv4 address usage\n"<< argv[0];
            return 1;
        }
    

    // If ICMP echo request handle return with an error 
    IcmpHandle = IcmpCreateFile();                                                                        
        if (IcmpHandle == INVALID_HANDLE_VALUE)
        {
            cout<<"Unable to open handle.\n";
            cout<<"IcmpCreatefile returned error code:\n"<< GetLastError();                               // Return value of 1 indicates failure, try to get error info. Lookup here to check the error code definitions: https://docs.automic.com/documentation/webhelp/english/ARA/11.2/AE/11.2/All%20Guides/Content/uczaia.htm
            return 1;
        }    


    // Allocating memory space  
    DWORD ICMP_ERROR_SIZE = 8;                                                                           // Reply buffer size for exactly 1 echo reply, payload data and buffer should also be large enough to also hold  more bytes of data (the size of an ICMP error message)
    DWORD ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData) + ICMP_ERROR_SIZE;                      // A buffer to hold any replies to the echo request and LPVOID means is void*. 
    ReplyBuffer = (void*) malloc(ReplySize);                                                     
                                                                                               
        if (ReplyBuffer == NULL) 
        {
            cout<<"Unable to allocate memory for reply buffer\n";
            return 1;
        }    

    IP_OPTION_INFORMATION ipOptions;
    memset(&ipOptions, 0, sizeof(IP_OPTION_INFORMATION));

    ipOptions.Ttl = TTL;                                                                                  // Default TTL value for Windows is 128. It is adjustable.


    int n;
    cout<<"Enter time interval in milliseconds:"<<endl;
    cin>>n;
 

    long long int m;
    cout<<"Enter ping count:"<<endl;
    cin>>m;

        // Keep looping for sending ping request
        for (long long int i = 0; i < m; ++i)    
        {            
            // Call the function of IcmpSendEcho to make echo request
            dwRetVal = IcmpSendEcho(IcmpHandle, ipaddr,(LPVOID) SendData, sizeof(SendData), &ipOptions, ReplyBuffer, ReplySize, Timeout);

            if (dwRetVal != 0) 
            {
                // It converts an IP address as a string to an integer value in network byte order.
                PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
                struct in_addr ipAddr;                                                                                                                        
                ipAddr.S_un.S_addr = pEchoReply->Address;                                           

                cout<<" Sent ICMP echo request to "<< argv[1]<<" with "<< sizeof(SendData) <<" Bytes payload"<<'\n';

                    if (dwRetVal > 1) 
                    {
                        cout<<" successfully received ICMP response"<< dwRetVal<< '\n';
                    }    
                        else 
                        {    
                            cout<<" Received "<<  dwRetVal<<" ICMP response successfully"<< '\n';
                        }    
                            cout<<" Round Trip Time (RTT): "<< pEchoReply->RoundTripTime<< " ms"<< '\n';  
                            cout<<" Time Interval: "<< n<< " ms"<< '\n';  
                            cout<<" Ping Count: "<<++Number<< '\n';
                            cout<<" TTL: "<<TTL<< '\n';

                            if (pEchoReply->Status == 0)
                            {
                                cout << "OK" << endl;
                            }

                            else
                            {
                                cout << "FAILED!" << endl;
                            }

                                switch (pEchoReply->Status)
                                {
                                    case IP_REQ_TIMED_OUT:
                                    cout<<"(Request timed out)\n";
                                    break;

                                        case IP_DEST_HOST_UNREACHABLE:
                                        cout<<"(Destination host was unreachable)\n"; 
                                        break;

                                    case IP_DEST_NET_UNREACHABLE:
                                    cout<<"(Destination Network was unreachable)\n";
                                    break;

                                        case IP_BUF_TOO_SMALL:
                                        cout<<"(The reply buffer was too small.)\n";
                                        break;

                                    case IP_DEST_PROT_UNREACHABLE:
                                    cout<<"(The destination protocol was unreachable.)\n";
                                    break;

                                        case IP_DEST_PORT_UNREACHABLE:
                                        cout<<"(The destination port was unreachable.)\n";
                                        break;

                                    case IP_NO_RESOURCES:
                                    cout<<"(Insufficient IP resources were available.)\n";
                                    break;

                                        case IP_BAD_OPTION:
                                        cout<<"(A bad IP option was specified.)\n";
                                        break;
                                    
                                    case IP_HW_ERROR:
                                    cout<<"(A hardware error occurred.)\n";
                                    break;
                                    
                                        case IP_PACKET_TOO_BIG:
                                        cout<<"(The packet was too big.)\n";
                                        break;

                                    case IP_BAD_REQ:
                                    cout<<"(A bad request.)\n";
                                    break;

                                        case IP_BAD_ROUTE:
                                        cout<<"(A bad route.)\n";
                                        break;

                                    case IP_TTL_EXPIRED_TRANSIT:
                                    cout<<"(The time to live (TTL) expired in transit.)\n";
                                    break;

                                        case IP_TTL_EXPIRED_REASSEM:
                                        cout<<"(The time to live expired during fragment reassembly.)\n";
                                        break;

                                    case IP_PARAM_PROBLEM:
                                    cout<<"(A parameter problem.)\n";
                                    break;

                                        case IP_SOURCE_QUENCH:
                                        cout<<"(Datagrams are arriving too fast to be processed and datagrams may have been discarded.)\n";
                                        break;

                                    case IP_OPTION_TOO_BIG:
                                    cout<<"(An IP option was too big.)\n";
                                    break;

                                        case IP_BAD_DESTINATION:
                                        cout<<"(A bad destination.)\n";
                                        break;

                                    case IP_GENERAL_FAILURE:
                                    cout<<"(A general failure. This error can be returned for some malformed ICMP packets.)\n";
                                    break;
                                    
                                        default:
                                        cout<<"\n";
                                        break;
                                } 
            }                
                                    else 
                                    {
                                        cout<<"IcmpSendEcho returned error code: "<<GetLastError()<< '\n';   
                                        dwError = GetLastError();                                         

                                        switch (dwError)
                                        {
                                            case IP_REQ_TIMED_OUT:
                                            cout<<" Request timed out\n";
                                            break;
                                            
                                                default:
                                                cout<<"Extended error returned: \n"<<dwError;
                                                break;
                                        }   
                                                return 1;    
                                    }  
            int milli_seconds=n; 
            Sleep(milli_seconds);   
        }  
    free(ReplyBuffer);                                                                                         // free() will correctly deallocate the memory that malloc() allocated
    IcmpCloseHandle(IcmpHandle);                                                                               // Close the echo request file handle. Use "CloseHandle" to close handles returned by calls to the "CreateFile" function. 
    return 0;  
}                         




/* SEVERAL REMARKS:


/* Error Code Links:    https://docs.automic.com/documentation/webhelp/english/ARA/11.2/AE/11.2/All%20Guides/Content/uczaia.htm
                        https://docs.microsoft.com/en-us/windows/win32/api/ipexport/ns-ipexport-icmp_echo_reply
                        https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes  */

//////////////////////////////////////////////////////////////////////////////////////////////
// Routine Name:
//
//     IcmpCreateFile
//
// Routine Description:
//
//     Opens a handle on which ICMP Echo Requests can be issued.
//
// Arguments:
//
//     None.
//
// Return Value:
//
//     An open file handle or INVALID_HANDLE_VALUE. Extended error information
//     is available by calling GetLastError().
//
////////////////////////////////////////////////////////////////////////////////////////////
//
// Routine Name:
//
//     IcmpCloseHandle
//
// Routine Description:
//
//     Closes a handle opened by ICMPOpenFile.
//
// Arguments:
//
//     IcmpHandle  - The handle to close.
//
// Return Value:
//
//     TRUE if the handle was closed successfully, otherwise FALSE. Extended
//     error information is available by calling GetLastError().
//
////////////////////////////////////////////////////////////////////////////////////////////
//
// Routine Name:
//
//     IcmpSendEcho
//
// Routine Description:
//
//     Sends an ICMP Echo request and returns any replies. The
//     call returns when the timeout has expired or the reply buffer
//     is filled.
//
// Arguments:
//
//     IcmpHandle           - An open handle returned by ICMPCreateFile.
//
//     DestinationAddress   - The destination of the echo request.
//
//     RequestData          - A buffer containing the data to send in the
//                            request.
//
//     RequestSize          - The number of bytes in the request data buffer.
//
//     RequestOptions       - Pointer to the IP header options for the request.
//                            May be NULL.
//
//     ReplyBuffer          - A buffer to hold any replies to the request.
//                            On return, the buffer will contain an array of
//                            ICMP_ECHO_REPLY structures followed by the
//                            options and data for the replies. The buffer
//                            should be large enough to hold at least one
//                            ICMP_ECHO_REPLY structure plus
//                            MAX(RequestSize, 8) bytes of data since an ICMP
//                            error message contains 8 bytes of data.
//
//     ReplySize            - The size in bytes of the reply buffer.
//
//     Timeout              - The time in milliseconds to wait for replies.
//
// Return Value:
//
//     Returns the number of ICMP_ECHO_REPLY structures stored in ReplyBuffer.
//     The status of each reply is contained in the structure. If the return
//     value is zero, extended error information is available via
//     GetLastError().
//
////////////////////////////////////////////////////////////////////////////////////////////
