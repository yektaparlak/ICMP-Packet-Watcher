# ICMP-Packet-Watcher
Featuring distinct tailoring for Windows (via Win32 API Approach) and Linux (via Raw Socket Approach), ICMP Packet Watcher is a network monitoring tool developed during an Aselsan internship in September 2022.

Using the ***Wireshark program***, I conducted a detailed monitoring and analysis of ICMP packet information. It can be found the corresponding analysis details in the report I authored, beginning from page 10.

<br>
<br>

>### To run via CMD for "icmp_Win32_API" 
>> <table><tr><td> icmp_Win32_API + DestinationIP (such as icmp_Win32_API 1.1.1.1) </td></tr></table>

--> To compile: g++ *.cpp -o icmp_Win32_API.exe -lws2_32 -fPIC -static -static-libgcc -static-libstdc++ C:\Windows\System32\iphlpapi.dll
<img src="https://github.com/yektaparlak/ICMP-Packet-Watcher/blob/main/gif_file_2.gif" width="800"/>


<br>

>### To run via CMD for "icmp_RawSocket"
>> <table><tr><td> icmp_RawSocket + DestinationIP  (such as icmp_RawSocket 1.1.1.1) </td></tr></table>

--> To compile: g++ *.cpp -o icmp_RawSocket.exe -lws2_32 -fPIC -static -static-libgcc -static-libstdc++
<img src="https://github.com/yektaparlak/ICMP-Packet-Watcher/blob/main/gif_file_1.gif" width="800"/>
<br>

## About the Problem 
We were looking for a method to check the connection status of radios used in digital communication. We needed a system that would give us feedback on whether the connection between us and the radio was established.

<img width="420" alt="image" src="https://github.com/yektaparlak/ICMP-Packet-Watcher/assets/111290340/09530918-dd20-453d-9df1-6485a7db214f">

<br>

## Solution
The solution to the problem was to develop a program that could diagnose the liveness of network communications. In order to develop the program that would make this possible, the Internet Control Protocol (ICMP) was used.  The aim of the project was to use this protocol to instantiate whether the network connection is active or not.

<br>

## Overview

Two devices are connected over the internet and exchange data via data packets or datagrams. ICMP is an error reporting that determines whether the data has reached the destination, and errors in communication between two network devices are reported to the source device as an ICMP message. 

It is a protocol that helps the management of the Internet protocol (IP). It is specified by the RFC 792 standard. As written in the (IANA) Internet Assigned Numbers Authority documentation, the IP Protocol number assigned for ICMP is 1. On top of it, ICMP operates at the Network Layer (Layer 3) of the OSI (Open Systems Interconnection) model.

As RFC 792 ICMP Standard Documentation says, ICMP information is placed in IP packets.

RFC 1122 Requirements for Internet Servers Standard Documentation, Section 3.2. states, "Each Host must implement the ICMP message function."

Also, ICMP information does not include a port. Because it is not in layer 4 where protocols such as TCP or UDP are located.

The ICMP packet is encapsulated in an IPv4 packet. The general composition of the IP datagram is as follows:
<img width="420" alt="image" src="https://github.com/yektaparlak/ICMP-Packet-Watcher/assets/111290340/16941c7c-7732-473e-828a-0d7135e3d414">


<br>

## ICMP Common Message Format

| Field Name | Size (Bytes) | Description |
|------------|--------------|-------------|
| Type       | 1            | Identifies the ICMP message type. For ICMPv6, values from 0 to 127 are error messages and values 128 to 255 are informational messages. Common values for this field are given in the table in the topic on ICMP message classes and types. |
| Code       | 1            | Identifies the “subtype” of the message within each ICMP message Type value. Thus, up to 256 “subtypes” can be defined for each message type. Values for this field are shown in the individual ICMP message type topics. |
| Checksum   | 2            | 16-bit checksum field that is calculated in a manner similar to the IP header checksum in IPv4. It provides error detection coverage for the entire ICMP message. |
| Message Body / Data | 32 (default) | Contains the specific fields used to implement each message type. This is the unique part of the message. |

<br>

## How Does It Work?
The ICMP protocol provides communication via Echo Request & Echo Reply, also known as Ping. A general description of how it works is given below. ICMP sends an "echo request" (type 8) packet to the device to be monitored. If the target device receives this packet successfully, it sends the "echo reply" (type 0) message packet to the source device.
1. Start the Echo Request process.
2. Send the Echo Request.
3. Wait for Echo Reply.
4. Receive Echo Reply.
5. Report the result.

<br>

## Comparison of Networking Approaches

| Feature                                | Raw Socket Approach (Linux)                           | Win32 API Approach (Windows)                        |
| -------------------------------------- | ----------------------------------------------------- | -------------------------------------------------- |
| **Target Platform**                    | Designed for Linux-based systems                      | Designed for Windows-based systems                 |
| **Implementation Method**              | Utilizes the raw socket interface in the Linux kernel | Utilizes the Win32 API, specifically Winsock and Iphlpapi libraries |
| **Flexibility**                        | Provides granular control, allowing for extensive customization at the network level | Offers a high degree of flexibility through the comprehensive Win32 API |
| **Ease of Use**                        | Best suited for users with in-depth knowledge of networking protocols | Provides a user-friendly development experience in Windows-centric environments |
| **Networking Features**                | Facilitates detailed control over packet construction, ideal for complex networking scenarios | Leverages Windows-specific networking features and utilities, enhancing overall functionality |

<br>

## Approach Analysis
In order to carried out the analysis, there were two types of sockets used to get more sophistication and practise.
 - The first method was to use a predefined protocol in the IP header, such as Win32 API approach.
 - The second method was to use a custom protocol in the IP header. Raw sockets provide the ability to manage down-level transports namely Raw Socket Approach.

## I. Raw Socket Approach (for Linux Platform)
 > <img width="430" alt="image" src="https://github.com/yektaparlak/ICMP-Packet-Watcher/assets/111290340/a93049ec-df80-4363-b276-88c768ea6d74">
  
### *****Steps:***** 
1. Define the ICMP header structure
2. Checksum calculation of ICMP packet
3. Create an original socket with a protocol type of IPPROTO_ICMP, set the properties of the socket.
4. Create and initialize ICMP packets.
5. Call the sendto function to send an ICMP request to the remote host.
6. Call the recvfrom function to receive the ICMP response.


### Program Code Input View
<img width="460" alt="image" src="https://github.com/yektaparlak/ICMP-Packet-Watcher/assets/111290340/53863f29-3a16-4380-9fc5-4872921328a1">


### Program Code Output View (Successful Connection)
<img width="458" alt="image" src="https://github.com/yektaparlak/ICMP-Packet-Watcher/assets/111290340/baa30b0f-832c-4760-9e5a-33ed2b6ab073">

### Program Code Output View (Failed Connection)
<img width="453" alt="image" src="https://github.com/yektaparlak/ICMP-Packet-Watcher/assets/111290340/80c605ff-c429-48a5-a6e4-04728778ff08">

<br>
<br>

## II. Win32 API Approach (for Windows Platform)
> <img width="430" alt="image" src="https://github.com/yektaparlak/ICMP-Packet-Watcher/assets/111290340/65f13121-d501-45a7-8fb3-c7fa2575cb65">


### *****Steps:***** 
1. Use the function IcmpCreateFile to create an Icmp handle.
2. Construct the API parameters. (IcmpHandle, ipaddr, SendData, sizeof(SendData), &ipOptions, ReplyBuffer, ReplySize, Timeout)
3. Call the function IcmpSendEcho to send.
4. Close the Icmp handle obtained by IcmpCreateFile.
 
    1. IcmpCreateFile();  => Opens a handle on which IPv4 ICMP echo requests can be issued.
    2. IcmpSendEcho();    => Sends an IPv4 ICMP echo request and returns any echo response replies and the call returns when the time-out has expired or the reply buffer is filled.
    3. IcmpCloseHandle(); => Closes a handle opened by a call to the IcmpCreateFile or  IcmpCreateFile functions.

### Program Code Input View
<img width="460" alt="image" src="https://github.com/yektaparlak/ICMP-Packet-Watcher/assets/111290340/f713925b-f6a3-4afc-b1de-395707f9d36c">


### Program Code Output View (Successful Connection)
<img width="461" alt="image" src="https://github.com/yektaparlak/ICMP-Packet-Watcher/assets/111290340/eb2d4f4b-2330-4878-aff5-5bf9a8c4de8b">


### Program Code Output View (Failed Connection)
<img width="449" alt="image" src="https://github.com/yektaparlak/ICMP-Packet-Watcher/assets/111290340/1ab32ee0-b00d-498d-9c13-20cda41729f8">

