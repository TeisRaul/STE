#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081 // Schimbat portul la 9090
#define BUFFER_SIZE 1024

void list_files(int socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    write(socket, "ls\n", 3);
    printf("Lista fișierelor disponibile:\n");

    while (read(socket, buffer, BUFFER_SIZE) > 0) {
        printf("%s", buffer);
        if (strstr(buffer, "\n") != NULL) {
            break;
        }
        memset(buffer, 0, BUFFER_SIZE);
    }
}

void get_file(int socket, const char *file_name) {
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    snprintf(command, BUFFER_SIZE, "get %s\n", file_name);
    write(socket, command, strlen(command));

    FILE *file = fopen(file_name, "wb");
    if (file == NULL) {
        perror("Eroare la crearea fișierului local");
        return;
    }

    printf("Descărcare fișier: %s\n", file_name);
    ssize_t bytes_read;
    while ((bytes_read = read(socket, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, 1, bytes_read, file);
    }

    fclose(file);
    printf("Fișier descărcat cu succes.\n");
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Adresă invalidă");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Eroare la conectare");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Conectat la server.\n");

    char command[BUFFER_SIZE];
    while (1) {
        printf("Comandă (ls / get <nume_fisier> / exit): ");
        fgets(command, BUFFER_SIZE, stdin);

        command[strcspn(command, "\n")] = 0; // Elimină newline

        if (strncmp(command, "ls", 2) == 0) {
            list_files(client_socket);
        } else if (strncmp(command, "get ", 4) == 0) {
            get_file(client_socket, command + 4);
        } else if (strcmp(command, "exit") == 0) {
            printf("Deconectare...\n");
            break;
        } else {
            printf("Comandă necunoscută.\n");
        }
    }

    close(client_socket);
    return 0;
}
