#include "bibs.h"

#define PORT 80
#define IP_ADDR "127.0.0.1"
#define REMOTE_IP_ADDR "192.168.178.161"
#define BUFFER_SIZE 1024

void signalHandler(int sig) {
    cout << "Interrupt signal -- Programm stopped\n";
    exit(sig);
}

void handleClient(int &client) {
    unordered_map<string, vector<string>> message_dict;
    // http parser 
    string current_line = "";
    string line_overflow = "";
    string key = "Type";
    string value = "";

    char buffer[BUFFER_SIZE];
    int read_bytes = 0;
    bool header_done = false;
    while (!header_done) 
    {
        // read port buffer ; break if empty
        int read_bytes = recv(client, buffer, sizeof(buffer), 0);
        current_line.append(buffer, read_bytes);

        bool buffer_done = false;
        while(!buffer_done) {
            int line_break_pos = current_line.find("\r\n", 0);
            // line break in current part of msg
            if(line_break_pos != string::npos) {
                line_overflow = current_line.substr(line_break_pos + 2);
                current_line = current_line.substr(0, line_break_pos + 1);

                // only in first line
                if(key == "Type") {
                    message_dict[key].push_back(current_line);
                    key = "";
                }
                else {
                    int pos_colon = current_line.find(":", 0);
                    if(pos_colon != string::npos) {
                        key = current_line.substr(0 , pos_colon);
                        value = current_line.substr(pos_colon + 2, current_line.size() - pos_colon - 2);

                        message_dict[key].push_back(value);
                    }
                }
                current_line = line_overflow;
                
                if(current_line.find("\r\n") == string::npos)
                    buffer_done = true;
            }
        }

        if(current_line.empty())
            header_done = true;
    }
    
    cout << "Ausgabe des dict ---- \n";
    // Test ausgabe dicts:
    for (const auto& key_values : message_dict)
    {
        const string key = key_values.first;
        const vector<string>& values = key_values.second;

        for (const auto& value : values)
        {
            cout << key << ": " << value << "\n";
        }
        
    }
    
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