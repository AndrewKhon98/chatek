#include <fcntl.h>              
#include <unistd.h>
#include <pthread.h>
#include <thread>
//#include <stdio.h>
//#include <sys/stat.h>     
#include <netinet/in.h> 
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
int flag_receive_request = 1;
 
int flag_wait_connect = 1;
int server;
 
//queue<string> req_on_proc;
//queue<string> res_on_send;
//pthread_t recv_req;
//pthread_t proc_req;
//pthread_t send_res;
pthread_t wait_con;
int b = 0;
int* retval1 = &b;
int retval2 = 0;
int retval3 = 0;
int retval4 = 0;
std::set<int> client_set;
struct my_arg
{
    int client;
    std::queue<std::string> req_on_proc;
    std::queue<std::string> res_on_send;
    pthread_t proc_req;
    pthread_t send_res;
    int flag_process_request;
    int flag_send_response;
    pthread_mutex_t* mutex1;
    pthread_mutex_t* mutex2;
};
 
void* receive_request(void* arg)
{
    my_arg* argument = (my_arg*)arg;
    printf("enter thread receive_request\n");
    while(flag_receive_request)
    {
        char buf[buf_size];//        
        if(recv(argument->client, buf, sizeof(buf), 0) > 0)
        {
            pthread_mutex_lock(argument->mutex1);
            std::string str = std::string(buf);
            int g = argument->client % 5 + 31;
            printf("\033[%dm", g);
            printf("received %s", str.c_str());    
            printf("\033[0m\n");   
            (argument->req_on_proc).push(str);
            pthread_mutex_unlock(argument->mutex1);
        }
        else
        {
                perror("serv receive");
                client_set.erase(argument->client);
                argument->flag_process_request = 0;
                argument->flag_send_response = 0;
                argument->client = 0;
                pthread_join(argument->proc_req, NULL);
                pthread_join(argument->send_res, NULL);
                break;
        }
    }
    argument->flag_process_request = 0;
    argument->flag_send_response = 0;
    pthread_join(argument->proc_req, (void**)&retval2);
    pthread_join(argument->send_res, (void**)&retval3);
    delete argument;
    printf("closed thread receive_request\n");
    int* a = new int (12);
    pthread_exit((void*)a);
   
}
 
void* process_request(void* arg)
{ 
    my_arg* argument = (my_arg*)arg;
    printf("enter thread process_request\n");
    while(argument->flag_process_request)
    {
        if(!(argument->req_on_proc).empty())
        {
           
            std::string str = (argument->req_on_proc).front();
            if(!str.empty())
            {
                (argument->req_on_proc).pop();
                time_t curtime = time(NULL);
                std::string responce(curTime());
                responce += " ";
                responce += str;
                (argument->res_on_send).push(responce);
            }  
            //else
                //printf("request queue empty");
        }
    }
    printf("closed thread process_request\n");
    int* a = new int (1);
    pthread_exit((void*)a);
       
}
 
void* send_response(void* arg)
{
    my_arg* argument = ((my_arg*)arg);
    printf("enter thread send_response\n");
    while(argument->flag_send_response)
    {
        if(!(argument->res_on_send).empty())
        {
            pthread_mutex_lock(argument->mutex2);
            std::string str = (argument->res_on_send).front();
            if(!str.empty())
            {
                (argument->res_on_send).pop();
                pthread_mutex_unlock(argument->mutex2);
                for(auto i: client_set)
                if(i > 0)
                    if(send(i, str.c_str(), str.size() + 1, MSG_NOSIGNAL) > 0)
                    {
                        //printf("client %d\n", clnt);
                        int g = argument->client % 5 + 31;
                        //printf("\033[%dm", g);
                        //printf("sent     %s\n\n", str.c_str());
                        //printf("\033[0m\n");
                    }
                    else
                        perror("serv sent");
                
            }
            //else
                //printf("request queue empty");
        }
    }
    printf("closed thread send_response\n");
    int* a = new int (15);
    pthread_exit((void*)a);
   
}
 
void* wait_connect(void* arg)
{
    printf("enter thread wait_connect\n");
    while(flag_wait_connect)
    {
        my_arg* arg = new my_arg;
        arg->mutex1 = new pthread_mutex_t;
        arg->mutex2 = new pthread_mutex_t;
        pthread_mutex_init(arg->mutex1, NULL);
        pthread_mutex_init(arg->mutex2, NULL);
        int status = fcntl(server, F_SETFL, O_NONBLOCK);
        arg->client = accept(server, (struct sockaddr*)NULL, NULL);
        //client_set.insert(arg->client);
        arg->flag_process_request = 1;
        arg->flag_send_response = 1;
        if (status == -1)
            perror("fcntl");
        if(arg->client > 0)
        {
            client_set.insert(arg->client);
            pthread_t recv_req;
           
           
            if(!pthread_create(&recv_req, NULL, receive_request, (void*)arg))
                printf("created thread recv_req\n");
            if(!pthread_create(&arg->proc_req, NULL, process_request, (void*)arg))
                printf("created thread proc_req\n");
            if(!pthread_create(&arg->send_res, NULL, send_response, (void*)arg))
                printf("created thread send_res\n");
            //ghjdthrf yf pfrhsnbt gjnjrjd
            //pthread_join(recv_req, (void**)&retval[0]);
            //pthread_join(proc_req, (void**)&retval[1]);
            //pthread_join(send_res, (void**)&retval[2]);
            //pthread_exit((void*)15);
        }
        else
            delete arg;
    }
    int* a = new int(115);
    pthread_exit((void*)a);
    printf("closed thread wait_connect\n");
}
 
int main()
{
    unlink("server_socket");
    struct sockaddr_in serv_addr;
   
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(1335);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
    server = socket(AF_INET, SOCK_STREAM, 0);
    //client = socket(AF_UNIX, SOCK_STREAM, 0);
    if(!bind(server, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
        printf("binded\n");
    if(!listen(server, 5))
        printf("listened\n");
   
    if(!pthread_create(&wait_con, NULL, wait_connect, NULL))
        printf("created thread wait_con\n");
    getchar();
    flag_receive_request = 0;
    flag_wait_connect = 0;
    //pthread_join(recv_req, (void**)&retval[0]);
    //pthread_join(proc_req, (void**)&retval[1]);
    //pthread_join(send_res, (void**)&retval[2]);

    pthread_join(wait_con, (void**)&retval1);
    close(server);
    //printf("ret_code1: %d\n", retval[0]);
    //printf("ret_code2: %d\n", retval[1]);
    //printf("ret_code3: %d\n", retval[2]);
    printf("ret_code: %d\n", retval1);
    printf("ret_code: %d\n", retval2);
    printf("ret_code: %d\n", retval3);
    printf("ret_code: %d\n", retval4);
    return 0;
}