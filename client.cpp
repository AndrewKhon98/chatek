#include <fcntl.h>              
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>      
#include <sys/types.h>
#include <sys/socket.h>
#include <queue>        
#include <string>
#include <stdlib.h>
#include <time.h>
#include <sys/un.h>
#define buf_size 16
 
using namespace std;
 
int flag_send_request = 1;
int flag_receive_response = 1;
int flag_wait_connect = 1;
int s;
struct sockaddr_un addr;
queue<string> req_on_proc;
queue<string> res_on_send;
char buf1[buf_size];
char buf2[buf_size];
pthread_t send_req;
pthread_t recv_res;
pthread_t wait_con;
 
void* send_request(void* )
{
    printf("enter thread send_request\n");
    srand(time(0));
    while(flag_send_request)//
    {      
        for(int i = 0; i < buf_size; ++i)
            buf1[i] = rand() % 79 + 48;
        buf1[buf_size - 1] = 0;
        if(send(s, buf1, buf_size, MSG_NOSIGNAL) > 0)
            printf("request:  %s\n", buf1);
        else
            perror("client request");
        sleep(1);
    }
    pthread_exit((void*)14);
    printf("closed thread send_request\n");
}
 
void* receive_response(void* )
{
    printf("enter thread receive_response\n");
    while(flag_receive_response)//
    {
       
        if(recv(s, buf2, buf_size, 0) > 0)
            printf("response: %s\n\n", buf2);
        else
            perror("serv response");
        sleep(1);
    }
    pthread_exit((void*)15);
    printf("closed thread receive_response\n");
}
 
void* wait_connect(void* )
{
    printf("enter thread wait_connect\n");
    while(flag_wait_connect)
    {
        if(!connect(s, (struct sockaddr *)&addr, sizeof(addr)))
        {
            if(!pthread_create(&send_req, NULL, send_request, NULL))
                printf("created thread send_req\n");
            if(!pthread_create(&recv_res, NULL, receive_response, NULL))
                printf("created thread recv_res\n");
            pthread_exit((void*)16);
        }
    }
    printf("closed thread wait_connect\n");
}
 
int main()
{
   
    int retval[3];
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "server_socket");
  //addr.sin_port = htons(1337);
  //addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//inet_addr("127.0.0.1");
 
    s = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
   
    if(!pthread_create(&wait_con, NULL, wait_connect, NULL))
        printf("created thread wait_con\n");
    getchar();
    flag_send_request = 0;
    flag_receive_response = 0;
    flag_wait_connect = 0;
    pthread_join(send_req, (void**)&retval[0]);
    pthread_join(recv_res, (void**)&retval[1]);
    pthread_join(wait_con, (void**)&retval[2]);
 
    shutdown(s, 2);
    printf("ret_code1: %d\n", retval[0]);
    printf("ret_code1: %d\n", retval[1]);
    printf("ret_code1: %d\n", retval[2]);
    close(s);
    return 0;
}