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


#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <iostream>
#include <WS2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Function to check if the provided IP address is valid
bool CheckIPAddr(unsigned long ipaddr) {
    cout << "Checking IP address using CheckIPAddr function" << endl << '\n';
    return ipaddr != INADDR_NONE;
}

// Function to handle ICMP response status codes
void HandleICMPStatus(int statusCode) {
    switch(statusCode) {
        case IP_SUCCESS:
            cout << "Ping was successful!" << endl;
            break;
        case IP_DEST_NET_UNREACHABLE:
            cout << "Destination network is unreachable." << endl;
            break;
        case IP_DEST_HOST_UNREACHABLE:
            cout << "Destination host is unreachable." << endl;
            break;
        // Add more cases for other status codes
        default:
            cout << "Unknown ICMP status code: " << statusCode << endl;
            break;
    }
}

// Function to handle errors from IcmpSendEcho
void HandleICMPSendEchoError(DWORD errorCode) {
    switch(errorCode) {
        case IP_REQ_TIMED_OUT:
            cout << "Ping request timed out." << endl;
            break;
        case IP_DEST_NET_UNREACHABLE:
            cout << "Destination network is unreachable." << endl;
            break;
        // Add more cases for other error codes
        default:
            cout << "Unknown ICMP error code: " << errorCode << endl;
            break;
    }
}

int main(int argc, char** argv) {
    // Declarations and initializations
    HANDLE IcmpHandle;
    unsigned long ipaddr = INADDR_NONE;
    DWORD dwRetVal = 0;
    DWORD dwError = 0;
    const char SendData[32] = "abcdefghijklmnopqrstuvwxyz12345";
    LPVOID ReplyBuffer = NULL;
    static int Number = 0;
    DWORD Timeout = 10000;
    static int TTL = 128;

    // Check if the correct number of arguments is provided
    if (argc != 2) {
        cout << "Invalid usage. Please provide a valid IPv4 address." << endl;
        return 1;
    }

    // Convert the provided IP address to binary form
    ipaddr = inet_pton(AF_INET, argv[1], &ipaddr);

    // Check if the provided IP address is valid
    if (!CheckIPAddr(ipaddr)) {
        cout << "Invalid IPv4 address: " << argv[1] << endl;
        return 1;
    }

    // Create a handle for sending ICMP echo requests
    IcmpHandle = IcmpCreateFile();
    if (IcmpHandle == INVALID_HANDLE_VALUE) {
        cout << "Unable to open handle." << endl;
        cout << "IcmpCreatefile returned error code: " << GetLastError() << endl;
        return 1;
    }

    // Allocate memory for storing reply data
    DWORD ICMP_ERROR_SIZE = 8;
    DWORD ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData) + ICMP_ERROR_SIZE;
    ReplyBuffer = malloc(ReplySize);

    if (ReplyBuffer == NULL) {
        cout << "Unable to allocate memory for reply buffer" << endl;
        return 1;
    }

    // Set IP options for the ICMP request
    IP_OPTION_INFORMATION ipOptions;
    memset(&ipOptions, 0, sizeof(IP_OPTION_INFORMATION));
    ipOptions.Ttl = TTL;

    // Input interval time in milliseconds
    int intervalMillis;
    cout << "Enter time interval in milliseconds: ";
    cin >> intervalMillis;

    // Choose between unlimited ping or manual ping count
    char choice;
    cout << "Type 't' for unlimited ping or 'm' for entering manual ping count: ";
    cin >> choice;

    long long int pingCount;
    if (choice == 't') {
        pingCount = -1; // -1 indicates unlimited ping
    }
    else if (choice == 'm') {
        cout << "Enter ping count: ";
        cin >> pingCount;
    }
    else {
        cout << "Invalid choice. Exiting..." << endl;
        return 1;
    }

    // Loop for sending ping requests
    for (long long int i = 0; (pingCount == -1 || i < pingCount); ++i) {
        dwRetVal = IcmpSendEcho(IcmpHandle, ipaddr, (LPVOID)SendData, sizeof(SendData), &ipOptions, ReplyBuffer, ReplySize, Timeout);

        // Process the response if no error occurred
        if (dwRetVal != 0) {
            PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
            struct in_addr ipAddr {};
            ipAddr.S_un.S_addr = pEchoReply->Address;

            cout << "Sent ICMP echo request to " << argv[1] << " with " << sizeof(SendData) << " Bytes payload" << endl;

            if (dwRetVal > 1) {
                cout << "Successfully received ICMP response: " << dwRetVal << endl;
            }
            else {
                cout << "Received " << dwRetVal << " ICMP response successfully" << endl;
            }
            cout << "Round Trip Time (RTT): " << pEchoReply->RoundTripTime << " ms" << endl;
            cout << "Time Interval: " << intervalMillis << " ms" << endl;
            cout << "Ping Count: " << ++Number << endl;
            cout << "TTL: " << TTL << endl;

            HandleICMPStatus(pEchoReply->Status); // Handle different status codes...
        }
        // Handle error if IcmpSendEcho fails
        else {
            cout << "IcmpSendEcho returned error code: " << GetLastError() << endl;
            dwError = GetLastError();
            HandleICMPSendEchoError(dwError); // Handle different error codes...
        }
        Sleep(intervalMillis); // Sleep for specified interval before sending next ping
    }

    // Free allocated memory and close the ICMP handle
    free(ReplyBuffer);
    IcmpCloseHandle(IcmpHandle);
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
