#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <dirent.h>

#include <arpa/inet.h>

#include <sys/stat.h>

#include <time.h> 

#include "dependencies.h"

#include "pgmread.c"

#define amountOfArgs 4

#define BUFFERSIZE 1450

/**
 * Global variables
*/
int lowerBound = 0;
int upperBound = 7;


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
 * errorHandler
 * * Helperfunction the does perror and exit
 * @param rtrn : Return value
 * @param message : Error message
 * @return void
 */
void errorHandler(int ret, char *msg) {
	if (ret == -1) {
		perror(msg);
		exit(EXIT_FAILURE);
	}
}

/**
 * sendACK
 * * Helperfunction that sends ACK packet
 * @param SequenceNumber : Sequence number of packet
 * @param socket : Socket to send message to
 * @param socketaddr : Sockaddr struct for socket
 * @return Void
*/
void sendACK(unsigned int SequenceNumber, int socket, struct sockaddr* socketaddr) {
	int receiver;
	unsigned char * serial = malloc(sizeof(struct packetProtocol));
	memset(serial, 0, sizeof(struct packetProtocol));
	struct packetProtocol tempProtocol;
	tempProtocol.flag = 0x2;
	tempProtocol.length = 7; // Bytes
	tempProtocol.lastACKSequence = SequenceNumber;
	serial[0] = (tempProtocol.length >> 24) & 0xFF;
	serial[1] = (tempProtocol.length >> 16) & 0xFF;
	serial[2] = (tempProtocol.length >> 8) & 0xFF;
	serial[3] = tempProtocol.length & 0xFF;
	serial[5] = SequenceNumber;
	serial[6] = tempProtocol.flag;
	receiver = sendto(
		socket,  /*Socket*/
		serial, /*Bytes som skal sendes*/
		tempProtocol.length, /* Lenged av bytes som skal sendes */
		0, /*Flag*/
		(struct sockaddr *)socketaddr, /*IP og port*/
		sizeof(struct sockaddr_in) /*Størrelse på adresse struct*/
		);
	printf("Sendt %d bytes!\n", receiver);
	free(serial);

}

/**
 * createPacket
 * * Function that created a packet
 * @param serial : Serial that is needed to create packet
 * @return pointer to packetProtocol struct
*/
struct packetProtocol * createPacket(unsigned char * serial) {
	unsigned int lengthOfPacket = (serial[0]<<24) + (serial[1]<<16) + (serial[2]<<8) + serial[3];
	struct packetProtocol * Packet = malloc(lengthOfPacket);
	Packet->length = lengthOfPacket;
	Packet->currentSequence = serial[4];
	Packet->lastACKSequence = serial[5];
	Packet->flag = serial[6];
	Packet->payload = &serial[8];
	return Packet;
}

/**
 * createPayload
 * * Function that created a packet
 * @param serial : Serial that is needed to create packet
 * @param size : Length of serial
 * @return pointer to Payload struct
*/
struct Payload * createPayload(unsigned char * Serial, unsigned int size) {
	struct Payload * ImagePayload = malloc(size);
	ImagePayload->requestID = (Serial[0]<<24) + (Serial[1]<<16) +(Serial[2]<<8) + Serial[3];
	ImagePayload->lengthOfFilename = (Serial[4]<<24) + (Serial[5]<<16) +(Serial[6]<<8) + Serial[7];
	ImagePayload->imgSize = (Serial[33]<<24) + (Serial[34]<<16) +(Serial[35]<<8) + Serial[36];
	char tempfileName[ImagePayload->lengthOfFilename];
	strncpy(tempfileName, &Serial[8], 25);
	strcpy(ImagePayload->filename, tempfileName);
	return ImagePayload;
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
	printf("\n\n");
}

/**
 * compareImages
 * * Function that implements Image_compare
 * @param outputFilePointer : File descriptor
 * @param buffer : Image buffer to compare with
 * @param payLoadName : Name of the payload
 * @param pathToFiles : Path to where the files are
 * @return Void
*/
void compareImages(FILE * outputFilePointer, unsigned char * buffer, unsigned char * payLoadName, char * pathToFiles) {
	struct Image * imageFromClient = readPGMfile(buffer);
	struct dirent * directoryPointer;
	DIR *dfd;
	if((dfd = opendir(pathToFiles)) == NULL) {
		printf("Cannot open directory %s\n", pathToFiles);
		exit(1);
	}
	while(((directoryPointer = readdir(dfd)) != NULL)) {
		if(!(strlen(directoryPointer->d_name) < 3)) {
			char fileNameWithPath[100];
			strcpy(fileNameWithPath, pathToFiles);
			char * path = directoryPointer->d_name;
			strcat(fileNameWithPath, path);
			
			FILE *filepointer = fopen(fileNameWithPath,"r");
			if(!filepointer) {
				perror("File not found");
				printf("\n--- %s ---\n\n", fileNameWithPath);
				exit(1);
			}
			char temp;
			unsigned char * imageBuffer = malloc(1300);
			size_t n = 0;
			while((temp = fgetc(filepointer)) != EOF) {
				imageBuffer[n++] = (char) temp;
			}
			struct Image * imageFromServer = readPGMfile(imageBuffer);
			int statusOfCompare = Image_compare(imageFromClient, imageFromServer);
			if(statusOfCompare == 1) {
				char outPutString[100] ="";
				strcat(outPutString, payLoadName);
				strcat(outPutString, " ");
				strcat(outPutString, fileNameWithPath);
				strcat(outPutString, "\n");
				fputs(outPutString, outputFilePointer);
				printf("Correct file: %s\n", outPutString);

				Image_free(imageFromServer);
				fclose(filepointer);
				free(imageBuffer);
				Image_free(imageFromClient);
				closedir(dfd);
				return;
			}
				Image_free(imageFromServer);
				free(imageBuffer);
				fclose(filepointer);
			
		}
	}
	char outPutString[100] ="";
	strcat(outPutString, payLoadName);
	strcat(outPutString, " UNKOWN\n");
	fputs(outPutString, outputFilePointer);
	closedir(dfd);
	Image_free(imageFromClient);
	printf("Could not find file %s\n", outPutString);
	return;
}

/**
 * main
 */
int main(int argc, char *argv[]) {
	int socket1, receiver;
	char buf[BUFFERSIZE];
	struct in_addr ipadresse;
	struct sockaddr_in adresse;
	struct sockaddr_in fraAdresse;

	unsigned int portNumber;
	inputErrorHandler(argc);
	portNumber = atoi(argv[1]);
	
	FILE* outputFilePointer = fopen(argv[3], "w");

	inet_pton(AF_INET, "127.0.0.1", &ipadresse);
	adresse.sin_family = AF_INET;
	adresse.sin_port = htons(portNumber);
	adresse.sin_addr = ipadresse;
	socket1 = socket(AF_INET, SOCK_DGRAM, 0);
	errorHandler(socket1, "socket");
	receiver = bind(socket1, (struct sockaddr *)&adresse, sizeof(struct sockaddr_in));
	errorHandler(receiver, "bind");

	int address_length = sizeof(&fraAdresse);
	receiver = recvfrom(socket1, buf, BUFFERSIZE - 1, 0, (struct sockaddr*)&fraAdresse, &address_length);
	errorHandler(receiver, "read");
	while(buf[6] != 0x4) {
		if((int)buf[4] == lowerBound) {
			lowerBound++;
			buf[receiver] = '\0';
			struct packetProtocol * Packet = createPacket(buf);
			unsigned char imagePayloadArray[Packet->length - 12];

			struct Payload * ImagePayload = createPayload(Packet->payload, sizeof(imagePayloadArray));
			ImagePayload->imagePayload = &Packet->payload[37];
			// printPacketInformation(Packet, ImagePayload);
			sendACK(Packet->currentSequence, socket1, (struct sockaddr *)&fraAdresse);
			compareImages(outputFilePointer, ImagePayload->imagePayload, ImagePayload->filename, argv[2]);
			upperBound++;
			free(ImagePayload);
			free(Packet);
		}
		receiver = recvfrom(socket1, buf, BUFFERSIZE - 1, 0, (struct sockaddr*)&fraAdresse, &address_length);
		errorHandler(receiver, "read");
	}
	fclose(outputFilePointer);
	close(socket1);


	return EXIT_SUCCESS;
}