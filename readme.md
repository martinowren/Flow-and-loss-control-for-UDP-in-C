# Client / Server program to send images
## Task is to implement Flow control and loss recovery in C
This program sends images from client to server and then the server checks if they exit on the server. Then it writes to the file outputFile.txt if they do exist or not.
   
The task at hand was to implement Flow control and Loss recovery for a program that sends pictures as UDP packets. To see the complete assignment, please contact me at martin.owren@gmail.com
   
Please read the section about assumptions at the bottom before running the project

## Getting Started

These instructions will get you started running this project on your local machine
Read pseudoKode.txt for a look into how I started to implement sliding window

### Prerequisites

The complete project ZIP must be unzipped for the project to work

### Installing

Open a terminal window in the directory of the project.

Step 1: Run the make command

```
make
```

Step 2: Start the server

```
make runServer
```

Step 3: Open a second terminal and start the client
```
make runClient
```

## Running the tests

### Valgrind

#### Client

To run valgrind on the **Client** with --leak-check=full --show-leak-kinds=all and 5% drop do the following command

```
make memCheckClient
```
To run valgrind on the **Client** with and 5% drop do the following command

```
make memCheckClientSmall
```

#### Server

To run valgrind on the **Server** with --leak-check=full and --show-leak-kinds=all do the following command

```
make memCheckServer
```
To run valgrind on the **Server** do the following command

```
make memCheckServerSmall
```


## Assumptions

### Timer
The Timer is stated in the task to be a 5 second / 5000 millisecond from when the oldest package that has yet to been ACK was sent. 
In this implementation there is a 5 second / 5000 millisecond timer from then the window was sent. This is because the difference in time would be so short, it would be more or less unnoticable
When tested, the time diff from when the first package in a window was sent, and the last one was sent. Was equal to < 0.00. That time implementation was tested with 2 x time_t structs and difftime. 

### Missing file in server_files
On purpose, I removed the file bark-3.pgm in the folder where the server checks the files

### Extra variable in Payload struct
I have added an extra variable called imgSize in the payload struct. This is just to make the life easier for the developer. 


## Authors

* **Martin Owren** - *Complete task* - [Martin Owren](https://martinowren.com)

