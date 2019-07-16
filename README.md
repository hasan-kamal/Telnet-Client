# Telnet-Client

### Introduction
- This is an implementation of a [telnet](https://en.wikipedia.org/wiki/Telnet) client in C
- Client is implemented using IO multiplexing (see [`select()`](https://notes.shichao.io/unp/ch6/) system call)

### Repository structure
-  `src/` contains the source code
    - `src/telnet_client.c` contains the telnet client implementation 
- `compile.sh` bash shell-script can be used to compile the client from source

### Build
- First compile by `cd`-ing into the root of this repository and then running command  `./compile.sh`
- Run `./bin/telnet_client.out <ip_address> <port_no>` to start the telnet client and connect to telnet server at IP address `ip_address`, port number `port_no`

### Example Runs
- Using the following sample commands, you can connect to some interesting [open servers](http://www.telnet.org/htm/places.htm) and see the client in action
    - `./bin/telnet_client.out towel.blinkenlights.nl 23` (Star Wars asciimation)
    - `./bin/telnet_client.out india.colorado.edu 13` (get the time)
        ```bash
        Hasans-MacBook-Pro:Telnet-Client hasankamal$ ./bin/telnet_client.out india.colorado.edu 13
        connected successfully!

        58680 19-07-16 13:24:15 50 0 0 442.0 UTC(NIST) * 
        EOF reached, connection closed?
        Hasans-MacBook-Pro:Telnet-Client hasankamal$
        ```

### References
1. [Unix Socket API](http://man7.org/linux/man-pages/man2/socket.2.html)
2. [Telnet protocol specification](https://tools.ietf.org/html/rfc854)
3. [IO multiplexing using `select()` system call](https://notes.shichao.io/unp/ch6/)
