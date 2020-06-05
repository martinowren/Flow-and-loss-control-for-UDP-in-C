#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

/**
 * 
 * 
 * Bytes: 4 + 1 + 1 + 1 + 1 + 1337 = 1345 (Mindre enn ett datagram)
*/
struct packetProtocol {
	unsigned int length; // total length of the packet including the payload in bytes
	unsigned char currentSequence; // sequence number of this packet
	unsigned char lastACKSequence; // sequence number of the last received packet (ACK)
	unsigned char flag;
	/* 
		0x1: 1 if this packet contains data
		0x2: 1 if this packet contains an ACK
		0x4: 1 1 if this packet closes the connection
		All other bits: 0
	*/
	unsigned char unused; // unused, must always contain the value 0x7f
	unsigned char * payload; // Payload struct
};

/**
 * 
 * 
 * Bytes: 4 + 4 + 25 + 4 + 4 = 32 + 1300 = 1337
*/
struct Payload {
	unsigned int requestID; // unique number of the request
	unsigned int lengthOfFilename; // length of the filename in bytes (including final 0)
	char filename[25]; // Filename
	unsigned int imgSize; // Image size
	unsigned char * imagePayload; //Payload
};

#endif