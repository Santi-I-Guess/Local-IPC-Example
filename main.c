#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h> 
#include <sys/types.h>
#include <unistd.h>

int prep_sockets(const char *usock_path, int *server_sock_fd, int *data_sock_fd) {

        if (access(usock_path, F_OK) == 0) {
                if (unlink(usock_path) == -1) {
                        perror("unlink");
                        return 1;
                }
        }

        *server_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (*server_sock_fd == -1) {
                perror("socket");
                return 1;
        }

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, usock_path, sizeof(addr.sun_path) - 1); 
        int ret; 
        ret = bind(*server_sock_fd, (const struct sockaddr*) &addr, sizeof(addr));
        if (ret == -1) {
                perror("bind");
                return 1;
        }

        ret = listen(*server_sock_fd, 1);
        if (ret == -1) {
                perror("listen");
                return 1;
        }
        
        printf("waiting at socket at %s\n", usock_path);
        *data_sock_fd = accept(*server_sock_fd, NULL, NULL);
        if (*data_sock_fd == -1) {
                perror("accept");
                return 1;
        }

        return 0;
}

int main() {
        const char *usock_path = "/tmp/something.sock";
        
        int server_sock_fd = -1, data_sock_fd = -1;
        if (prep_sockets(usock_path, &server_sock_fd, &data_sock_fd)) {
                return 1;
        }

        while (1) {
                printf("outgoing server mesage: ");
                char buffer[128];
                fgets(buffer, sizeof(buffer), stdin);

                if (write(data_sock_fd, buffer, strlen(buffer) + 1) == -1) {
                        perror("write");
                        return 1;
                }

                sleep(1);
                char message_given[128];
                if (read(data_sock_fd, message_given, 128) == -1) {
                        perror("read");
                        return 1;
                }
                message_given[strcspn(message_given, "\n")] = '\0';

                printf("recieved from client: %s\n", message_given);
                if (!strncmp(message_given, "QUIT", 4)) {
                        break;
                }        
        }
        
        close(data_sock_fd);
        close(server_sock_fd);
        unlink(usock_path);
        return 0;
}
