/*

@author Hasan Kamal

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#define BUFFER_SZ 1000
#define DEBUG 0

int maximum(int a, int b){
	return a > b ? a : b;
}

void handle_data_just_read(int sck_fd, char *data, int num_bytes){
	char out_buf[BUFFER_SZ];
	int out_ptr = 0;

	char IAC = (char)255; //IAC Telnet byte consists of all ones

	for(int i = 0; i < num_bytes; i++){
		if(data[i] == IAC){
			//command sequence follows
			if(DEBUG){ printf("IAC detected\n"); }
			out_buf[out_ptr++] = IAC;
			i++;
			switch(data[i]){
				case ((char)251):
					//WILL as input
					if(DEBUG){ printf("WILL detected\n"); }
					out_buf[out_ptr++] = (char)254; //DON'T, rejecting WILL
					i++;
					out_buf[out_ptr++] = data[i];
					break;
				case ((char)252):
					//WON'T as input
					if(DEBUG){ printf("getting won't as input, weird!\n"); }
					break;
				case ((char)253):
					//DO as input
					if(DEBUG){ printf("DO detected\n"); }
					out_buf[out_ptr++] = (char)(char)252; //WON'T, rejecting DO
					i++;
					out_buf[out_ptr++] = data[i];
					break;
				case ((char)254):
					//DON'T as input
					if(DEBUG){ printf("getting don't as input, weird!\n"); }
					break;
				default:
					//not a option negotiation, a genereal TELNET command like IP, AO, etc.
					if(DEBUG){ printf("general TELNET command %x\n", (int)data[i]); }
					break;
			}
		}else{
			//is data
			printf("%c", data[i]);
		}
	}

	fflush(stdout); //need to flush output here as printf doesn't do so unless '\n' is encountered, and we're printing char-by-char the real data
	
	if(write(sck_fd, out_buf, out_ptr) == -1){
		printf("error write()'ing\n");
		exit(0);
	}

	if(DEBUG){ printf("packet sent!\n\n"); }
}

void start_communication(int sck_fd){
	char inp_buf[BUFFER_SZ];
	char *user_command = NULL;
	size_t user_max_length = 0;
	
	fd_set fds;
	FD_ZERO(&fds);
	int mx;
	while(1){

		FD_SET(fileno(stdin), &fds);
		FD_SET(sck_fd, &fds);
		mx = maximum(fileno(stdin) + 1, sck_fd + 1);

		select(mx, &fds, NULL, NULL, NULL); //IO Multiplexing

		if(FD_ISSET(fileno(stdin), &fds)){
			//read from standard input
			if(DEBUG){ printf("reading from stdin\n"); }
			int num_read = getline(&user_command, &user_max_length, stdin);
			if(num_read == -1){
				printf("error getline()'ing\n");
				exit(0);
			}
			write(sck_fd, user_command, num_read);
		}

		if(FD_ISSET(sck_fd, &fds)){
			//read from TCP socket
			int num_read = read(sck_fd, inp_buf, BUFFER_SZ);
			if(DEBUG){ printf("reading from socket(%d bytes read)\n", num_read); }
			if(num_read == -1){
				printf("error read()'ing\n");
				exit(0);
			}else if(num_read == 0){
				printf("EOF reached, connection closed?\n");
				exit(0);
			}
			handle_data_just_read(sck_fd, inp_buf, num_read);
		}
	}
}

int main(int argc, char *argv[]){

	if(argc != 3){
		printf("usage error, please enter exactly 2 arguments\n");
		printf("format: ./a.out <ip_address> <port_no>\n");
		return 0;
	}

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	struct addrinfo *results;

	if(getaddrinfo(argv[1], argv[2], &hints, &results) != 0){
		printf("error getaddrinfo()'ing, translation error\n");
		return 0;
	}

	struct addrinfo *addr_okay;
	int succeeded = 0;
	int sck_fd = socket(AF_INET, SOCK_STREAM, 0);
	for(addr_okay = results; addr_okay != NULL; addr_okay = addr_okay->ai_next){
		if(connect(sck_fd, addr_okay->ai_addr, addr_okay->ai_addrlen) >= 0){
			succeeded = 1;
			printf("connected successfully!\n");
			break;
		}
	}
	freeaddrinfo(results);

	if(!succeeded){
		printf("error, coudln't connect() to any of possible addresses\n");
		return 0;
	}

	start_communication(sck_fd);
	close(sck_fd);

	return 0;
}