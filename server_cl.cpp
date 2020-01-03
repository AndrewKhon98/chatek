#include <fcntl.h>              
#include <unistd.h>
//#include <pthread.h>
#include <thread>
#include <mutex>
//#include <stdio.h>
//#include <sys/stat.h>      
#include <sys/types.h>
#include <sys/socket.h>
#include <queue>        
#include <string>
#include <sys/un.h>
//#include <features.h>
#include <errno.h>
#include <set>
 
#define buf_size 1024

 
std::string curTime()
{
    struct tm *u;    
    time_t currentTime = time(NULL);
    u = localtime(&currentTime);
    char S[10];
    strftime(S, 10, "%T", u);
    std::string time(S);
    return time;
}
struct my_arg
{
    int client;    
    int flag_send_response;
    int flag_process_request;
    int flag_receive_request;
    std::mutex* mutex1;
    std::mutex* mutex2;    
    std::thread* proc_req;
    std::thread* send_res;    
    std::queue<std::string> req_on_proc;
    std::queue<std::string> res_on_send;
};

class server
{
    void receive_request(my_arg* arg);
    void process_request(my_arg* arg);
    void send_response(my_arg* arg);
    void wait_connect();
    int server_descriptor;
    int flag_wait_connect;
    std::set<int> client_set;
public:
    void start();
    server();
};

server::server() : flag_wait_connect(1)
{
    unlink("server_socket");
    struct sockaddr_un serv_addr; 
    serv_addr.sun_family = AF_UNIX;
    //addr.sin_port = htons(1337);
    //addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
    strcpy(serv_addr.sun_path, "server_socket");
    server_descriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if(!bind(server_descriptor, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
        printf("binded\n");
    if(!listen(server_descriptor, 5))
        printf("listened\n");
    int status = fcntl(server_descriptor, F_SETFL, O_NONBLOCK);
    if (status == -1)
            perror("fcntl");
}
std::set<int> client_set;

 
void server::receive_request(my_arg* arg)
{
    printf("enter thread receive_request\n");
    while(arg->flag_receive_request)
    {
        char buf[buf_size];//        
        if(recv(arg->client, buf, sizeof(buf), 0) > 0)
        {
            arg->mutex1->lock();
            std::string str = std::string(buf);
            int g = arg->client % 5 + 31;
            printf("\033[%dm", g);
            printf("received %s", str.c_str());    
            printf("\033[0m\n");   
            arg->req_on_proc.push(str);
            arg->mutex1->unlock();
        }
        else
        {
                perror("serv receive");
                client_set.erase(arg->client);
                arg->flag_process_request = 0;
                arg->flag_send_response = 0;
                arg->client = 0;
                arg->proc_req->join();
                arg->send_res->join();
                break;
        }
    }
    arg->flag_process_request = 0;
    arg->flag_send_response = 0;
    arg->proc_req->join();
    arg->send_res->join();
    delete arg;
    printf("closed thread receive_request\n");   
}
 
void server::process_request(my_arg* arg)
{ 
    printf("enter thread process_request\n");
    while(arg->flag_process_request)
    {
        if(!(arg->req_on_proc).empty())
        {
           
            std::string str = (arg->req_on_proc).front();
            if(!str.empty())
            {
                (arg->req_on_proc).pop();
                time_t curtime = time(NULL);
                std::string responce(curTime());
                responce += " ";
                responce += str;
                (arg->res_on_send).push(responce);
            }  
            //else
                //printf("request queue empty");
        }
    }
    printf("closed thread process_request\n");
    int* a = new int (1);
    pthread_exit((void*)a);
       
}
 
void server::send_response(my_arg* arg)
{
    printf("enter thread send_response\n");
    while(arg->flag_send_response)
    {
        if(!(arg->res_on_send).empty())
        {
            arg->mutex2->lock();
            std::string str = (arg->res_on_send).front();
            if(!str.empty())
            {
                (arg->res_on_send).pop();
                arg->mutex2->unlock();
                for(auto i: client_set)
                if(i > 0)
                    if(send(i, str.c_str(), str.size() + 1, MSG_NOSIGNAL) > 0)
                    {
                        int g = arg->client % 5 + 31;
                    }
                    else
                        perror("serv sent");
                
            }
        }
    }
    printf("closed thread send_response\n");
    int* a = new int (15);
    pthread_exit((void*)a);
   
}
 
void server::wait_connect()
{
    printf("enter thread wait_connect\n");
    while(flag_wait_connect)
    {
        my_arg* arg = new my_arg;
        arg->mutex1 = new std::mutex;
        arg->mutex2 = new std::mutex;
        arg->client = accept(server_descriptor, (struct sockaddr*)NULL, NULL);
        arg->flag_process_request = 1;
        arg->flag_send_response = 1;
        arg->flag_receive_request = 1;
        if(arg->client > 0)
        {
            client_set.insert(arg->client);
            std::thread recv_req(&server::receive_request, this, arg);
            std::thread proc_req(&server::process_request, this, arg);
            std::thread send_res(&server::send_response, this, arg);
        }
        else
            delete arg;
    }
    printf("closed thread wait_connect\n");
}
 
void server::start()
{
    
    
    std::thread wait_con(&server::wait_connect, this);
    getchar();
    flag_wait_connect = 0;
    wait_con.join();
    close(server_descriptor);
}

int main()
{
    server Server;
    Server.start();
    return 0;
}