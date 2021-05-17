#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>


void upload_html(const char * filename, int soc);
void upload_html(const char * filename, int soc)
{
    
    FILE *f; 

    f = fopen(filename,"rb");
    if (f == NULL)
    {
        char error[] = "HTTP/1.1 404 Not found\r\nConnection: close\r\n";
        send(soc, error, sizeof(error), 1);
        fclose(f);
        //send(soc, "HTTP/1.1 404")
    }
    else
    {

        int size, j;
        char  size_str[100];
        char info[] = "HTTP/1.1 200 OK\r\nContent-Language: ru\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\nContent-Length: ";
        fseek(f, 0L, SEEK_END);
        size = ftell(f);

        rewind(f);
        sprintf(size_str,"%d",size);
        printf("%s\n", size_str);
        char * together = new char[sizeof(info) + sizeof(size_str)+6];
        strcpy(together, info);
        strcat(together, size_str);
        strcat(together, "\r\n\n");
        for(j = 0; j < sizeof(together); j++)
        {
            if (together[j] == ' ')
            {
                together[j] = '\0';
                break;
            }
        }
        
        send(soc, together, strlen(together), 0);
        /*int size;
        char info[] = "HTTP/1.1 200 OK\nContent-Language: ru\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n";
        send(soc, info, sizeof(info) , 0);*/
        
        
        char buff1[100];
        buff1[0] = '\0';
        while(!feof(f))
        {
            size = fread(buff1, 1 , 100, f);
            
            printf("%s\n",buff1);
            send(soc, buff1, size, 0);
        }

        fclose(f);
     }
     
}

void upload_jpg(const char * filename, int soc);
void upload_jpg(const char * filename, int soc)
{
    FILE *f;
    f = fopen(filename,"rb");
    if (f == NULL)
    {
        printf("Hello world\n");
        fclose(f);
        //send(soc, "HTTP/1.1 404")
    }
    else
    {
        int size, j;
        char  size_str[1000];
        char info[] = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nConnection: close\r\nContent-Length: ";
        fseek(f, 0L, SEEK_END);
        size = ftell(f);

        rewind(f);
        sprintf(size_str,"%d",size);
        printf("%s\n", size_str);
        char * together = new char[sizeof(info) + sizeof(size_str)+4];
        strcpy(together, info);
        strcat(together, size_str);
        strcat(together, "\n\n");
        for(j = 0; j < sizeof(together); j++)
        {
            if (together[j] == ' ')
            {
                together[j] = '\0';
                break;
            }
        }
        
        send(soc, together, strlen(together), 0);

        char buff1[1000];
        
        buff1[0] = '\0';
        while(!feof(f))
        {
            size = fread(buff1, 1 , 1000, f);
            if (size != 10000)
                buff1[size] = '\0';
        //printf("%s\n",buff1);
            send(soc, buff1, size, 0);
        }

        fclose(f);
     }
     
}



void parce(char * request, int sock);
void parce(char * request, int sock)
{
    char *filename, *buf, *extentsion;
    char html[] = "html", jpg[] = "jpg";
    //request[sizeof(request)] = '\0';
    if (request[0] == 'G')
    {
        buf = strchr(request , '/') + 1;
        //printf("%s", work);
        filename = strtok(buf," " );
        //tok = strtok(NULL, " ");
        buf = strchr(buf, '.') + 1;
        extentsion = strtok(buf, " ");
        printf("%s",extentsion);
        if (!(strcmp(extentsion, html)))
        {
            upload_html(filename, sock);
        }
        if ( !(strcmp(extentsion, jpg)))
        {
            upload_jpg(filename, sock);
        }
        
        //printf("%s",tok);
    }
}

int main()


{
    int sock;
    sockaddr_in peer_addr;
    sockaddr_in addr;
    char buf[1000];
    
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("Error in socket creation\n");
        return 1;
    }
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // waiting from any address
    addr.sin_port = htons (8089);
    addr.sin_family = AF_INET;
    printf("socket ok\n");


    
    int ret = bind (sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        printf("Cannot bind to port\n");
        close(sock);
        return -1;
    }
    printf("bind ok\n");



    if (listen(sock , 4) != 0) //initialize queue for clients
    {
        printf("cannot listen\n");
        close (sock);
        return -2;
    }


    socklen_t peer_addr_size;
    int sock_peer = 1;
    while(1)
    {
        sock_peer = accept(sock, (struct sockaddr *)&peer_addr, &peer_addr_size); //sock continues to listen for other clients, chats with client via sock_peer    
    
        if (sock_peer <= 0)
        {
           printf("cant accept\n");
         close(sock);
         return -1;
     }// , the prog will be looped
        printf ("accept OK\n");


         ret = recv(sock_peer, buf , sizeof(buf) - 1, 0); 
         if (ret <= 0)
        {
            printf("connection failed during recv\n");
            close(sock);
            close(sock_peer);
            return 0;
        }   
        buf[ret] = '\0'; // bad, server can return more than sizeof(buf) chars
        //printf("%s\n", buf);
        parce(buf, sock_peer);
        //upload("welcome.html",sock_peer);
        //printf("%s\n", buf);
        
        //send(sock_peer, "PONG", 4 ,0);
        close(sock_peer);
            }
            close(sock);

            return 0;

}

 
