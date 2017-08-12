/*

@author Hasan Kamal

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#define BUFFER_SZ 1000

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

	char inp_buf[BUFFER_SZ];
	char out_buf[BUFFER_SZ];
	char *user_command = NULL;
	size_t user_max_length = 0;
	char IAC = (char)255; //IAC Telnet byte consists of all ones
	while(1){
		int num_read = read(sck_fd, inp_buf, BUFFER_SZ);
		if(num_read >= 0){
				
			printf("input taken\n");
			int out_ptr = 0;
			int has_data = 0;
			for(int i = 0; i < num_read; i++){
				if(inp_buf[i] == IAC){
					//command sequence follows
					printf("IAC detected\n");
					out_buf[out_ptr++] = IAC;
					i++;
					switch(inp_buf[i]){
						case ((char)251):
							//WILL as input
							printf("WILL detected\n");
							out_buf[out_ptr++] = (char)254; //DON'T, rejecting WILL
							i++;
							out_buf[out_ptr++] = inp_buf[i];
							break;
						case ((char)252):
							//WON'T as input
							printf("getting won't as input, weird!\n");
							break;
						case ((char)253):
							//DO as input
							printf("DO detected\n");
							out_buf[out_ptr++] = (char)(char)252; //WON'T, rejecting DO
							i++;
							out_buf[out_ptr++] = inp_buf[i];
							break;
						case ((char)254):
							//DON'T as input
							printf("getting don't as input, weird!\n");
							break;
						default:
							//not a option negotiation, a genereal TELNET command like IP, AO, etc.
							printf("general TELNET command %x\n", (int)inp_buf[i]);
							break;
					}
				}else{
					//is data
					has_data = 1;
					printf("%c", inp_buf[i]);
				}
				//printf("%x ", inp_buf[i]);
			}
			//printf("\n");

			if(!has_data){
				//out_buf[out_ptr++] = '\n';
				if(write(sck_fd, out_buf, out_ptr) == -1){
					printf("error write()'ing\n");
					return 0;
				}
			}else{
				//take input from user first
				// if(getline(&user_command, &user_max_length, stdin) == -1){
				// 	printf("error getline()'ing\n");
				// 	return 0;
				// }
				int num_char = getline(&user_command, &user_max_length, stdin);
				if(num_char >= 0){
					for(int i = 0; i < num_char; i++)
						out_buf[out_ptr++] = user_command[i];
				}
				if(write(sck_fd, out_buf, out_ptr) == -1){
					printf("error write()'ing\n");
					return 0;
				}
			}

			printf("sent a packet!\n\n");

		}else{
			printf("error read()'ing\n");
			return 0;
		}
	}

	return 0;
}