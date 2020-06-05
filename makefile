all: compile

compile:
	clear
	@echo "\033[92m*** Compiling code - Client ***\033[0m"
	gcc Client.c -o ./Client
	@echo "\033[92m*** Compiling code - Server ***\033[0m"
	gcc Server.c -o ./Server
	@echo "\033[92m*** Compiling finished ***\033[0m"

commitCompile:
	clear
	@echo "\033[92m*** Compiling code - Client ***\033[0m"
	gcc -g -Wunused-variable Client.c -o ./Client 
	@echo "\033[92m*** Compiling code - Server ***\033[0m"
	gcc -g -Wunused-variable Server.c -o ./Server
	@echo "\033[92m*** Compiling finished ***\033[0m"

runClient:
	clear
	./Client 127.0.0.1 1234 list_of_filenames.txt 5

runServer:
	clear
	./Server 1234 server_files/ outputFile.txt

memCheckClient:
	clear
	valgrind --leak-check=full --show-leak-kinds=all ./Client 127.0.0.1 1234 list_of_filenames.txt 5
memCheckClientSmall:
	clear
	valgrind ./Client 127.0.0.1 1234 list_of_filenames.txt 5

memCheckServer:
	clear
	valgrind --leak-check=full --show-leak-kinds=all ./Server 1234 server_files/ outputFile.txt

memCheckServerSmall:
	clear
	valgrind ./Server 1234 server_files/ outputFile.txt





clean:
	rm ./Client 
	rm ./Server
	rm ./outputFile.txt
tar:
	tar -czvf 15430.tgz .