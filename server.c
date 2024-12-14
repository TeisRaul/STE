#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8081 // Schimbat portul la 9090
#define BUFFER_SIZE 1024
#define DIRECTORY "./ProiectSTE" // Directorul cu fișiere

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Primește comanda de la client
    read(client_socket, buffer, BUFFER_SIZE);

    if (strncmp(buffer, "ls", 2) == 0) {
        // Trimite lista de fișiere
        DIR *dir = opendir(DIRECTORY);
        if (dir == NULL) {
            perror("Eroare la deschiderea directorului");
            write(client_socket, "Eroare la deschiderea directorului\n", 32);
            close(client_socket);
            return;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                write(client_socket, entry->d_name, strlen(entry->d_name));
                write(client_socket, "\n", 1);
            }
        }
        closedir(dir);

    } else if (strncmp(buffer, "get ", 4) == 0) {
        // Preia numele fișierului
        char file_path[BUFFER_SIZE];
        snprintf(file_path, BUFFER_SIZE, "%s/%s", DIRECTORY, buffer + 4);
        file_path[strcspn(file_path, "\n")] = 0; // Elimină newline

        FILE *file = fopen(file_path, "rb");
        if (file == NULL) {
            perror("Eroare la deschiderea fișierului");
            write(client_socket, "Eroare: fișierul nu există\n", 27);
            close(client_socket);
            return;
        }

        // Trimite fișierul
        while (!feof(file)) {
            size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
            write(client_socket, buffer, bytes_read);
        }
        fclose(file);
    } else {
        write(client_socket, "Comandă invalidă\n", 18);
    }

    close(client_socket);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Eroare la bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Eroare la listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Serverul rulează pe portul %d...\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket == -1) {
            perror("Eroare la accept");
            continue;
        }

        printf("Client conectat: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}
