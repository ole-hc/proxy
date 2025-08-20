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
    string request = "";
    char buffer[BUFFER_SIZE];
    int read_bytes = 0;
    char* end_of_request = nullptr;
    while ((read_bytes = recv(client, buffer, sizeof(buffer), 0)) > 0) 
    {
        request.append(buffer, read_bytes);

        // stop reading when end of http : \r\n\r\n is reached
        end_of_request = &request.back();

        if(request.size() >= 4 && request.compare(request.size() - 4, 4, "\r\n\r\n") == 0)
            break;
    }
    // parse http header to "dict"
    // find all line breaks in the header
    vector<int> positions;
    string line_break = "\r\n";
    int substr_position = request.find(line_break, 0);
    while (substr_position != string::npos)
    {
        positions.push_back(substr_position);
        substr_position = request.find(line_break, substr_position + 1);
    }
    // create dict 
    int pos_colon = 0;
    int start_substr = 1;
    string key = "";
    string value = "";
    key = "Tpye";
    value = request.substr(0, positions.at(0));
    start_substr = (positions.at(0) + 3);
    message_dict[key].push_back(value);

    for (size_t i = 1; i < positions.size() - 1; i++)
    {
        key = "";
        value = "";

        pos_colon = request.find(":", start_substr);
        key = request.substr(start_substr - 1, pos_colon - start_substr + 1);
        value = request.substr(pos_colon + 2, positions.at(i) - pos_colon - 1);
        start_substr = (positions.at(i) + 3);

        message_dict[key].push_back(value);
    }
    
    cout << "Msg: " << request << endl;
    
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