#include "bibs.h"

#define PORT 8080
#define IP_ADDR "127.0.0.1"
#define BUFFER_SIZE 1024

void signalHandler(int sig) {
    cout << "Interrupt signal -- Programm stopped\n";
    exit(sig);
}

void handleClient(int &client) {
    char buffer[BUFFER_SIZE];
    read(client, buffer, BUFFER_SIZE);
    cout << "Message from Client: " << client << " -- " << buffer << "\n";
    cout << "READ -- END -- closing thread... \n";
}

int main(void) {
    signal(SIGINT, signalHandler);

    // create TCP socket
    int listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    // re-use socket --> no delay after restart
    int opt = 1;
    setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // init server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = inet_addr(IP_ADDR);
    serv_addr.sin_port = htons(PORT);
    bind(listening_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    // activate socket
    listen(listening_socket, 10);
    vector<int> clients;
    
    // main loop
    while (true)
    {
        // accecpt new client and add to vector
        struct sockaddr_in client_addr;
        unsigned int client_length = sizeof(client_addr);

        int client_socket = accept(listening_socket, (struct sockaddr*)&client_addr, &client_length);

        if(client_socket < 0)
            cout << "ERROR while connecting !!\n";
        else {
            cout << "Connection successful\n";
            clients.push_back(client_socket);
            thread client_thread(handleClient, ref(client_socket));
            client_thread.detach();
        }
    }

    return 0;
}