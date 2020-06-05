#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <sys/stat.h>

#include <sys/poll.h>

#include <libgen.h>

#include <time.h>

#include "dependencies.h"

#include "send_packet.c"

#define amountOfArgs 5

#define fileBufferLength 1300

#define SENDMESSAGE 0

/**
 * Global variables
*/
int payloadRequestIdSerial = 0;
int lowerBound = 0;
int upperBound = 0;
struct fifo * HEAD;
clock_t timer;
struct timeval timeout;
fd_set readfds;

/**
 * Lenkeliste
 * * Struct for the linked list
 * @data next : Pointer to next in linked list
 * @data serializedPacket :  Serialized packet
 * @data lengthOfPacket : Length of the serialized packet
*/
struct fifo {
	struct fifo * next;
	unsigned char serializedPacket[1450];
	unsigned char seqNumber;
	int lengthOfPacket;
};

/**
 * addToFifi
 * * Adds fifo struct to HEAD link list
 * @param serializedPacket : Serialized packet
 * @param seqNumber : Sequence number
 * @param lengthOfPacket : Length of the serialized packet
 * @return void 
*/
void addToFifo(unsigned char * serializedPacket,unsigned char seqNumber,int lengthOfPacket) {
	if(HEAD == NULL) {
		struct fifo * newNode;
		newNode = (struct fifo *)malloc(sizeof(struct fifo));
		newNode->seqNumber = seqNumber;
		memcpy(&newNode->serializedPacket[0], serializedPacket, lengthOfPacket);

		newNode->lengthOfPacket = lengthOfPacket;
		HEAD = newNode;
		HEAD->next = NULL;
		return;
	}

	struct fifo * newNode;
	newNode = malloc(sizeof(struct fifo) + lengthOfPacket);
	newNode->lengthOfPacket = lengthOfPacket;
	newNode->seqNumber = seqNumber;
	memcpy(&newNode->serializedPacket[0], serializedPacket, lengthOfPacket);
	
	if(HEAD->next == NULL) {
		HEAD->next = newNode;
		newNode->next = NULL;
	} else {
		struct fifo * temp = HEAD;
		while(temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = newNode;
		newNode->next = NULL;
	}
}

/**
 * removeFromFifo
 * * Removed the fifo struct that is the HEAD
 * @return void 
*/
void removeFromFifo() {
	if(HEAD == NULL) {
		return;
	}
	if(HEAD->next == NULL) {
		free(HEAD); 
		HEAD = NULL;
		return;
	}
	struct fifo * temp = HEAD->next;
	free(HEAD);
	HEAD = temp;
}

/**
 * printFifoList
 * * Prints the complete list of the linked list
 * @return void 
*/
void printFifoList() {
	if(HEAD == NULL) {
		printf("Linked list is empty");
		return;
	}
	if(HEAD->next == NULL) {
		printf("FULLLINKED LIST->%i\n", HEAD->seqNumber);
		return;
	}
	struct fifo * temp = HEAD;
	printf("FULLLINKED LIST->%i", HEAD->seqNumber);
	while(temp->next != NULL) {
		temp = temp->next;
		printf("->%i", temp->seqNumber); 
	}
	printf("\n");
}

/**
 * sendPacketsInFifo
 * * Sends the packets that are in the linked list to the server
 * @param socket : Socket to be used in send_packet
 * @param socketAddress : The sockaddr struct for send packet
 * @return void 
*/
void sendPacketsInFifo(int socket, struct sockaddr * socketAddress) {
	if(HEAD == NULL) {
		return;
	}
	if(HEAD->next == NULL) {
		int receiver;
		receiver = send_packet(
			socket,  /*Socket*/
			HEAD->serializedPacket, /*Bytes som skal sendes*/
			HEAD->lengthOfPacket, /* Lengden av bytes som skal sendes */
			0, /*Flag*/
			socketAddress, /*IP og port*/
			sizeof(struct sockaddr_in) /*Størrelse på adresse struct*/
			);
	}
	struct fifo * temp = HEAD;
	while(temp->next != NULL) {
		int receiver;
		receiver = send_packet(
			socket,  /*Socket*/
			temp->serializedPacket, /*Bytes som skal sendes*/
			temp->lengthOfPacket, /* Lengden av bytes som skal sendes */
			0, /*Flag*/
			socketAddress, /*IP og port*/
			sizeof(struct sockaddr_in) /*Størrelse på adresse struct*/
			);
		// perror("SENDTOPACKET");
		temp = temp->next;
	}
}

/**
 * errorHandler
 * * Helperfunction the does perror and exit
 * @param rtrn : Return value
 * @param message : Error message
 * @return void
 */
void errorHandler(int rtrn, char * message) {
	if (rtrn == -1) {
		perror(message);
		exit(EXIT_FAILURE);
	}
}

/**
 * inputErrorHandler
 * * Helper function that checks if input is valid
 * @param args : Amount of arguments
 * @return null or EXIT
 */ 
int inputErrorHandler(int args) {
	if(args > amountOfArgs) {
		printf("Too many inputs. Please read readme.md file for instruction");
		return EXIT_FAILURE;
	} else if(args < amountOfArgs) {
		printf("Too few inputs. Please read readme.md file for instructions");
		return EXIT_FAILURE;
	}
}

/**
 * getFileSize
 * * Helper function that gets the file size
 * @param pathToFile : Path to the file that needs to be checked
 * @return Size of file
*/
int getFileSize(char * pathToFile) {
	struct stat fileInfo;
	if(stat(pathToFile, &fileInfo) == 0) {
		return fileInfo.st_size;
	}
}

/**
 * serializePayload
 * * Function that serializes the payload
 * @param payld : Payload struct to serialize
 * @return The serialized unsigned char *
*/
unsigned char * serializePayload(struct Payload * payld) {
	unsigned char * serial = malloc(37 + payld->imgSize);
	
	// RequestID
	serial[0] = (payld->requestID >> 24) & 0xFF;
	serial[1] = (payld->requestID >> 16) & 0xFF;
	serial[2] = (payld->requestID >> 8) & 0xFF;
	serial[3] = payld->requestID & 0xFF;

	// lengthOfFilename
	serial[4] = (payld->lengthOfFilename >> 24) & 0xFF;
	serial[5] = (payld->lengthOfFilename >> 16) & 0xFF;
	serial[6] = (payld->lengthOfFilename >> 8) & 0xFF;
	serial[7] = payld->lengthOfFilename & 0xFF;

	// filename
	memcpy(&serial[8], payld->filename, 25);
	// imagesize
	serial[33] = (payld->imgSize >> 24) & 0xFF;
	serial[34] = (payld->imgSize >> 16) & 0xFF;
	serial[35] = (payld->imgSize >> 8) & 0xFF;
	serial[36] = payld->imgSize & 0xFF;
	// File
	memcpy(&serial[37], payld->imagePayload,payld->imgSize);

	return serial;
}

/**
 * serializePacketProtocol
 * * Function that serializes the packet
 * @param packet : Packet struct to serialize
 * @param sizeOfPayload : Size of Packet
 * @return The serialized unsigned char *
*/
unsigned char * serializePacketProtocol(struct packetProtocol * packet, unsigned int sizeOfPayload ) {
	unsigned char * serial = malloc(8 + sizeOfPayload);
	// Length
	serial[0] = (packet->length >> 24) & 0xFF;
	serial[1] = (packet->length >> 16) & 0xFF;
	serial[2] = (packet->length >> 8) & 0xFF;
	serial[3] = packet->length & 0xFF;

	// currentSequence
	serial[4] = packet->currentSequence;

	// lastACKSequence
	serial[5] = packet->lastACKSequence;

	// flag
	serial[6] = packet->flag;

	// unused
	serial[7] = 0x7f;

	// Payload
	memcpy(&serial[8], packet->payload, sizeOfPayload);
	
	return serial;
}


/**
 * getCharArrayFromImageFile
 * * Function that reads the file and retruns a buffer
 * @param fileName : Name of the file
 * @param fileSize : Size of the file
 * @return Unsigned char buffer
*/
unsigned char * getCharArrayFromImageFile(char * fileName, int fileSize) {
	unsigned char * tempImageCharHolder = malloc(fileSize);
	FILE *imagepointer = fopen(fileName, "r");
	if(!imagepointer) {
		perror("File not found");
		printf("\n--- %s ---\n\n", fileName);
		exit(1);
	}
	char temp;
	size_t n = 0;
	while((temp = fgetc(imagepointer)) != EOF) {
		tempImageCharHolder[n++] = (char) temp;
	}
	fclose(imagepointer);

	return tempImageCharHolder;
}

/**
 * receiveMessage
 * * Recvfrom in a helper function
 * @param socket : Socket to listen to
 * @return Void
*/
void receiveMessage(int socket) {
	char buf[255];
	int receiver;
	receiver = recvfrom(socket, buf, 255 - 1, 0, NULL, NULL);
	errorHandler(receiver, "Receivermessage");
	buf[receiver] = '\0';
	char flag = buf[6];
	unsigned char seqNumber = buf[5];
	if(flag == 0x1) {
		printf("Contains data\n");
	} else if(flag == 0x2) {
		printf("ACK message for packet number: %i\n", seqNumber);
	}
}

/**
 * getAck
 * * Recvfrom in a helper function
 * @param socket : Socket to listen to
 * @return Sequence number of the ACK
*/
int getAck(int socket) {
	char buf[255];
	int receiver;
	receiver = recvfrom(socket, buf, 255 - 1, 0, NULL, NULL);
	errorHandler(receiver, "Receivermessage");
	buf[receiver] = '\0';
	char flag = buf[6];
	if(flag == 0x2) {

	unsigned char seqNumber = buf[5];
	printf("ACK message for packet number: %i\n", seqNumber);
	return seqNumber;
	}
}

/**
 * sendTerminationMessage
 * * Send in a helper function, send termination packet to server
 * @param socket : Socket to listen to
 * @param socketAddress : The sockaddr struct for send packet
 * @return Void
*/
void sendTerminationMessage(int socket, struct sockaddr * socketAddress) {
	int receiver;
	char * serial = malloc(7);
	struct packetProtocol tempProtocol;
	tempProtocol.length = 7;
	tempProtocol.flag = 0x4;
	serial[0] = (tempProtocol.length >> 24) & 0xFF;
	serial[1] = (tempProtocol.length >> 16) & 0xFF;
	serial[2] = (tempProtocol.length >> 8) & 0xFF;
	serial[3] = tempProtocol.length & 0xFF;
	serial[4] = '0'; //Trenger ikke seq num
	serial[5] = '0'; //Trenger ikke ack num
	serial[6] = tempProtocol.flag;

	receiver = send_packet(
		socket,  /*Socket*/
		serial, /*Bytes som skal sendes*/
		tempProtocol.length, /* Lengden av bytes som skal sendes */
		0, /*Flag*/
		socketAddress, /*IP og port*/
		sizeof(struct sockaddr_in) /*Størrelse på adresse struct*/
		);
	free(serial);
}

/**
 * printPacketInformation
 * * Helperfunction that prints out the packet information
 * @param Packet : Packet to be printed
 * @param ImagePayload : Payload to be printed
 * @return Void
*/
void printPacketInformation(struct packetProtocol * Packet, struct Payload * ImagePayload) {
	printf("*** Packet ***\n");
	printf("Length: %i\n", Packet->length);
	printf("currentSequence: %i\n", Packet->currentSequence);
	printf("lastACKSequence: %i\n", Packet->lastACKSequence);
	printf("flag: ");
	int i;
	for(i = 0; i < 8; i++) {
		printf("%d", !!((Packet->flag << i) & 0x80));
	}
	printf("\n--Payload--\n");
	printf("RequestID: %i\n", ImagePayload->requestID);
	printf("LengthOfFileName: %i\n", ImagePayload->lengthOfFilename);
	printf("Filename: %s\n", ImagePayload->filename);
	printf("Imagesize: %i\n", ImagePayload->imgSize);

}

/**
 * createPacket
 * * Function that created a packet
 * @param length : Length of packet
 * @param curQeq : Sequence number of packets
 * @param ACKseq : ACK number
 * @param flag : Flag
 * @return pointer to packetProtocol struct
*/
struct packetProtocol * createPacket(unsigned int length, unsigned char curSeq,unsigned char ACKseq,unsigned char flag) {
	struct packetProtocol * packetProto = malloc(sizeof(struct packetProtocol));
	packetProto->length = length;
	packetProto->currentSequence = curSeq;
	packetProto->lastACKSequence = 0;
	packetProto->flag = flag;
	packetProto->unused = 0x7f;
	packetProto->payload = 0;
	return packetProto;
}

/**
 * createPayload
 * * Function that created a packet
 * @param rId : Request ID
 * @param lenOfFile : Length of filename
 * @param fileName : The filename
 * @param iSize : Imagesize
 * @return pointer to Payload struct
*/
struct Payload * createPayload(unsigned int rId, unsigned int lenOfFile, char * fileName, unsigned int iSize) {
	struct Payload * payloadPointer = malloc(sizeof(struct Payload));
	payloadPointer->requestID = rId;
	payloadPointer->lengthOfFilename = lenOfFile;
	memset(payloadPointer->filename, 0, 25);
	memcpy(payloadPointer->filename, fileName, lenOfFile);
	payloadPointer->imgSize = iSize;
	payloadPointer->imagePayload = 0;
	return payloadPointer;
}

/**
 * main
 */
int main(int argc, char	* argv[]) {
	// Used for UDP
	int socket1, receiver;
	char tempArrayForFiles[fileBufferLength];
	
	struct in_addr hostAdress;
	struct sockaddr_in socketAddressStruct;
	HEAD = NULL; // Init of empty variable;
	
	// Args
	char * inputIpadress;
	unsigned int portNumber;
	FILE *filepointer;
	float dropPercentage;

	// Sliding window
	int windowSize = 7;
	
	// Errorcheck the inputs
	inputErrorHandler(argc);
	inputIpadress = argv[1];
	portNumber = atoi(argv[2]);
	filepointer = fopen(argv[3], "r");
	dropPercentage = atof(argv[4]);
	if (dropPercentage > 20 || dropPercentage < 0) {
		printf("Please enter correct input drop percentage. Value between 0 - 20\n");
		exit(1);
	}
	set_loss_probability(dropPercentage/100);

	inet_pton(AF_INET, inputIpadress, &hostAdress);
	socketAddressStruct.sin_family = AF_INET;
	socketAddressStruct.sin_port = htons(portNumber);
	socketAddressStruct.sin_addr = hostAdress;
	char fileNameArray[250][25];
	size_t i = 0;
	socket1 = socket(AF_INET, SOCK_DGRAM, 0);
	errorHandler(socket1, "Socketerror");
				timeout.tv_sec = 5;
			timeout.tv_usec = 0;

	tempArrayForFiles;
	while(fgets(tempArrayForFiles,fileBufferLength, filepointer) != NULL) {
		if (tempArrayForFiles[strlen(tempArrayForFiles) - 1] == '\n') {
			tempArrayForFiles[strlen(tempArrayForFiles) - 1] = '\0';
		}
		memcpy(fileNameArray[i++], tempArrayForFiles, sizeof(tempArrayForFiles));
	}
	
	size_t filesSent = 0;
	size_t packetsCreated = 0;
	while(filesSent != i) {

		while( ((upperBound - lowerBound) < windowSize) && (packetsCreated != i)) {
		int fileSize = getFileSize(fileNameArray[packetsCreated]);
		errorHandler(fileSize, "Filesizeerror");

			/* Payload */
			struct Payload * payloadPointer = createPayload(
				payloadRequestIdSerial,
				strlen(basename(fileNameArray[packetsCreated])),
				basename(fileNameArray[packetsCreated]),
				fileSize
			);
			unsigned char *imageCharArray = getCharArrayFromImageFile(fileNameArray[packetsCreated],fileSize);
			payloadPointer->imagePayload = imageCharArray;
			unsigned char * serialOfPayloud = serializePayload(payloadPointer);

			/* packetProtocol */ 
			struct packetProtocol * packetProto = createPacket(
				8 + 37 + payloadPointer->imgSize,
				payloadRequestIdSerial++,
				0,
				0x1
			);
			packetProto->payload = serialOfPayloud;
			unsigned char * serialOfPacket = serializePacketProtocol(packetProto, 37 + payloadPointer->imgSize);
			// printPacketInformation(packetProto, payloadPointer);
			receiver = send_packet(
				socket1,  /*Socket*/
				serialOfPacket, /*Bytes som skal sendes*/
				packetProto->length, /* Lengden av bytes som skal sendes */
				0, /*Flag*/
				(struct sockaddr *)&socketAddressStruct, /*IP og port*/
				sizeof(struct sockaddr_in) /*Størrelse på adresse struct*/
				);
			addToFifo(serialOfPacket, packetProto->currentSequence, packetProto->length);
			packetsCreated++;
			free(imageCharArray);
			free(payloadPointer);
			free(packetProto);
			free(serialOfPayloud);
			free(serialOfPacket);
			upperBound++;
		}

		struct pollfd fds[1];
		fds[0].fd = socket1;
		fds[0].events = POLLIN;
		if(poll(fds, 1, 5000) > 0) {
			if (getAck(socket1) == lowerBound) {
				removeFromFifo();
				lowerBound++;
				filesSent++;
			}
		} else if (filesSent != i) {
			sendPacketsInFifo(socket1, (struct sockaddr *)&socketAddressStruct);
		}		
	}
	errorHandler(receiver, "Sendtoerror");

	/* EXIT program */
	printf("/* EXIT program */\n");
	fclose(filepointer);
	sendTerminationMessage(socket1, (struct sockaddr *)&socketAddressStruct);
	close(socket1);


	return 0;
}