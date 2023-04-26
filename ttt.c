#define _POSIX_C_SOURCE 200809L
#include "myutils.h"

int connect_to_server(char *host, char *portStr);

int connect_to_server(char *host, char *portStr){
    struct addrinfo hints, *potentialAddrList, *potentialAddr;
    int status, sock;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(host, portStr, &hints, &potentialAddrList);
    if (status != 0) {
        err_die(gai_strerror(status));
    }

    // attempt to create socket
    for (potentialAddr = potentialAddrList; potentialAddr != NULL; potentialAddr = potentialAddr->ai_next){
        sock = socket(potentialAddr->ai_family, potentialAddr->ai_socktype, potentialAddr->ai_protocol);

        if (sock < 0) continue;
        
        //bind successful socket; now bind
        status = connect(sock, potentialAddr->ai_addr, potentialAddr->ai_addrlen);
        if (status != 0) {
            close(sock);
            continue;
        }

        //good socket found
        break;
    }

    freeaddrinfo(potentialAddrList);

    return sock;

}


int main (int argc, char** argv){
    int sock, bytes;
    char buf;

    if (argc < 3) {
        printf("Specify host and service\n");
        err_die("Usage: ./sclient <host> <port>");
    }

    sock = connect_to_server(argv[1], argv[2]);
    if (sock < 0) err_die("Unable to connect to server");

    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (int[]){1}, sizeof(int));

    for (;;){
        bytes = read(STDIN_FILENO, &buf, 1);
        if (bytes < 0) {
                err_die("Unable to read from stdin");
            }

        if (bytes == 0) {
            printf("bytes = 0\n");
            fflush(stdout);
            break;
        }

        if (buf != '\n'){
            bytes = write(sock, &buf, 1);
            if (bytes < 0) {
                err_die("Unable to write to socket");
            }

            if (bytes == 0) {
                printf("Server closed connection\n");
                fflush(stdout);
                break;
            }
        } else {
            while (buf != '|'){
                read(sock, &buf, 1);
                printf("%c", buf);
                fflush(stdout);
            }
        }

    }

    close(sock);
    return 0;
}