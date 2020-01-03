//#include <fcntl.h>    //          
#include <unistd.h>//
//#include <stdio.h>
//#include <sys/stat.h>      
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>//
#include <queue>        //
#include <string>//
//#include <stdlib.h>
//#include <time.h>
#include <sys/un.h>
#include <thread>//
#include <iostream>//
#include <arpa/inet.h>
#define buf_size 1024
 

std::string parseInput()
{
    std::string input;
    char c = 0;
    while(true)
    {
        c = std::cin.get();
        if(c == 10) break;//enter
        if(c == 8)//backspace
        {
            input.pop_back();
        }
        else
        {
            input += c;
        }
    }
    return input;
}
class client
{    
    void send_request();
    void receive_response();
    void wait_connect();
    struct sockaddr_in addr;
    std::queue<std::string> req_on_proc;
    std::queue<std::string> res_on_send;
    std::string nickname;
    char buf1[buf_size];
    char buf2[buf_size];
    int flag_send_request;
    int flag_receive_response;
    int flag_wait_connect;
    int client_socket;
public:
    client();
    std::string getNickname() { return nickname;}
    void start();
};
/*//int flag_send_request = 1;
int flag_receive_response = 1;
int flag_wait_connect = 1;
int s;
struct sockaddr_un addr;
std::queue<std::string> req_on_proc;
std::queue<std::string> res_on_send;
char buf1[buf_size];
char buf2[buf_size];*/

client::client() : flag_receive_response(1), flag_send_request(1), flag_wait_connect(1)
{
    memset(buf1,'\0', buf_size);
    memset(buf2,'\0', buf_size);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1337);
    addr.sin_addr.s_addr = inet_addr("82.179.72.71");
    client_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
}
void client::send_request( )
{
    printf("enter thread send_request\n");
    //srand(time(0));
    while(flag_send_request)
    {      
        memset(buf1, '\0', buf_size);
        strcpy(buf1, parseInput().c_str());
        std::cout <<"\033[F\r";
        //std::cin >> buf1;//TODO
        if(!strcmp(buf1, "\\exit"))
        {
            flag_send_request = 0;
            flag_receive_response = 0;           
            break;
        }
        //std::cout << buf1 << std::endl;
        std::string request = nickname;
        request += ": ";
        request += buf1;
        
        if(!send(client_socket, request.c_str(), buf_size, MSG_NOSIGNAL) > 0)
            perror("client request");
        sleep(1);
    }
    printf("closed thread send_request\n");
}
 
void client::receive_response( )
{
    printf("enter thread receive_response\n");
    while(flag_receive_response)//
    {
       
        if(recv(client_socket, buf2, buf_size, 0) > 0)
            std::cout << buf2 << std::endl;
        //else
            //perror("serv response");
        sleep(1);
    }
    printf("closed thread receive_response\n");
}
 
void client::wait_connect( )
{
    printf("enter thread wait_connect\n");
    std::thread send_req (&client::send_request, this);
    std::thread recv_res (&client::receive_response, this);
    while(flag_wait_connect)
    {
        
        if(connect(client_socket, (struct sockaddr*)&addr, sizeof(addr)))
        {
            //perror("connect");
            //if(!pthread_create(&send_req, NULL, send_request, NULL))
                //printf("created thread send_req\n");
            
            //if(!pthread_create(&recv_res, NULL, receive_response, NULL))
                //printf("created thread recv_res\n");
            
            //pthread_exit((void*)16);
        }
    }
    send_req.join();
    recv_res.join();
    printf("closed thread wait_connect\n");
}

void client::start()
{
    if(connect(client_socket, (struct sockaddr*)&addr, sizeof(addr)))
    {
        //perror(connect);
    }
    std::cout << "Connected\nEnter your nickname: ";
    nickname = parseInput();
    std::thread send_req (&client::send_request, this);
    std::thread recv_res (&client::receive_response, this);
    while(flag_send_request);
    flag_send_request = 0;
    flag_receive_response = 0;
    flag_wait_connect = 0;

    send_req.join();
    recv_res.join();
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
}
int main()
{
   
    client Client;
    Client.start();
    return 0;
}