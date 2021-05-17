#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include <memory.h>
#include <unistd.h>
#include <sys/select.h>
#include <map>
using namespace std;

double time_now (void);
int add_to_fds (fd_set* fds, struct mem_ip* mp);
int filesize (FILE *f);
int type (char* path, char* type_format);
int path (char* request, char* cpath);
int send_site (int sock);
int send_image_type(int sock, char* path, char* type_format);
int send_text_type (int sock, char* path, char* type_format);
int send_app_type (int sock, char* path, char* type_format);
int request(char* request, int sock_peer);
int reaction (char* request, int sock_peer);
int server (int sock, int sock_ip);


struct mem_ip
{
  int ip;
  int flag;
  int sock;
};

double time_now(void)
{
    struct timeval tv2;
    struct timezone tz;
    
    gettimeofday(&tv2, &tz);
    
    return tv2.tv_sec+tv2.tv_usec/1000000.0;
}

int filesize (FILE *f)
{
   int save_pos, size_of_file;

   save_pos = ftell (f);
   fseek (f, 0L, SEEK_END);
   size_of_file = ftell (f);
   fseek (f, save_pos, SEEK_SET);
   return (size_of_file);
}

int type (char* path, char* type_format)
{
	int i, j, count = 0;

    for (i=strlen(path)-1; i>-1; i--)
    {
      if (path[i] != '.')
        count++;

      if (path[i] == '.') 
      {
        for (j=0; j<count; j++)
        	type_format[j] = path[i+j+1];

        type_format[count] = '\0';
        break;
      }

      if (count == strlen(path))
      {
      	printf (">>>404<<<\n");
      	printf(">>>error in type find\n");
      	break;
      }
    }

	return 1;
}

int path (char* request, char* cpath)
{
	int i=0, j=0, count = 0;
        
    while (request[i] != '\n' && count <= 10000)
	{
	   count++;
	   i++;
	}

	if ((request[0]=='G')&&(request[1]=='E')&&(request[2]=='T'))
	{
	   for (i=5; i<count-10; i++)
	   {
	   		if (request[i]=='?')
	   			break;
		    cpath[j]=request[i];
		    j++;
	   }

	    cpath[j]='\0';

		//printf("\n>>>PATH: %s\n", cpath);     	
	}
	else
	{
		printf (">>>404<<<\n");
		printf(">>>no path found\n");
		return -1;
	}
	
	return 1;
}

int send_site (int sock)
{
	FILE* f;

	char buf_begin[]="HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: ";
	char buf_end[]="\r\nConnection: close\r\n\r\n";
	char csize[100];
	
	int size;

	f = fopen ("site.html", "rb");
	if (!f)
	{
		printf(">>>can't open file\n");
		return -1;
	}

	size = filesize (f);
	
	if (size>1e14)
	{
		printf(">>>inappropriate format of uploading file\n");
		return -1;
	}
	else if ((size>0)&&(size<1e14))
	{
		sprintf (csize,"%d",size); 
	  	csize[strlen(csize)]='\0';

		send (sock,buf_begin,strlen(buf_begin),0);
		send (sock,csize,strlen(csize),0);
		send (sock,buf_end,strlen(buf_end),0);

		char buf[1000];

		while (!(feof(f)))
		{
			int n = fread (buf,sizeof(char),1000,f);
			buf[n]='\0';

			send(sock,buf,n,0);
		}

		fclose (f);
		printf(">>>main page uploaded\n");
	}
	else
	{
		printf(">>>error in entering main page\n");
		return -2;
	}

	return 1;
}

int send_text_type (int sock, char* path, char* type_format)
{
	printf(">>>%s uploaded\n", path);
	//printf(">>>type of uploading file: %s\n\n", type_format);

	FILE* f;

	char buf_begin[]="HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/";
	char buf_med[]="; charset=UTF-8\r\nContent-Length: ";
	char buf_end[]="\r\nConnection: close\r\n\r\n";
	char csize[100];
	
	int size, i;

	f = fopen (path, "rb");
	if (!f)
	{
		printf(">>>can't open file\n");
		return -1;
	}

	size = filesize (f);
	fclose (f);
	
	if (size>1e14)
	{
		printf(">>>inappropriate format of uploading file\n");
		return -1;
	}
	else if ((size>0)&&(size<1e6))
	{
		f = fopen (path, "rb");
		if (!f)
		{
			printf(">>>can't open file\n");
			return -1;
		}

		char format[20];

		if  ((type_format[0]=='c')&&(type_format[1]=='s')&&(type_format[2]=='s'))
		{
			sprintf (format,"%s", "css"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='j')&&(type_format[1]=='s'))	
		{
			sprintf (format,"%s", "javascript"); 
	  		format[strlen(format)]='\0';
		}

		if  
	  		((type_format[0]=='p')&&(type_format[1]=='h')&&(type_format[2]=='p'))
		{
			sprintf (format,"%s", "php"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='h')&&(type_format[1]=='t')&&(type_format[2]=='m')&&(type_format[3]=='l'))
		{
			sprintf (format,"%s", "html"); 
	  		format[strlen(format)]='\0';
		}

		sprintf (csize,"%d",size); 
	  	csize[strlen(csize)]='\0';

		send (sock,buf_begin,strlen(buf_begin),0);
		send (sock,format,strlen(format),0);
		send (sock,buf_med,strlen(buf_med),0);
		send (sock,csize,strlen(csize),0);
		send (sock,buf_end,strlen(buf_end),0);

	    char buf[1000];

		while (!(feof(f)))
		{
			int n = fread (buf,sizeof(char),1000,f);
			buf[n]='\0';

			send(sock,buf,n,0);
		}

		fclose (f);
	}
	else
	{
		printf(">>>error in entering main page\n");
		return -2;
	}

	return 1;
}

int send_image_type (int sock, char* path, char* type_format)
{
  printf(">>>%s uploaded\n", path);
  //printf(">>>type of uploading file: %s\n\n", type_format);

  FILE* f;

	char buf_begin[]="HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: image/";
	char buf_med[]="; charset=UTF-8\r\nContent-Length: ";
	char buf_end[]="\r\nConnection: close\r\n\r\n";
	char csize[100];
	
	int size;
	
	f = fopen (path, "rb");
	if (!f)
	{
		printf(">>>can't open file\n");
		return -1;
	}

	size = filesize (f);
	fclose (f);
	
	if (size>1e14)
	{
		printf(">>>inappropriate format of uploading file\n");
		return -1;
	}
	else if ((size>0)&&(size<1e14))
	{
		f = fopen (path, "rb");
		if (!f)
		{
			printf(">>>can't open file\n");
			return -1;
		}

		char format[20];

		if  ((type_format[0]=='s')&&(type_format[1]=='v')&&(type_format[2]=='g'))
		{
			sprintf (format,"%s", "svg+xml"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='g')&&(type_format[1]=='i')&&(type_format[2]=='f'))	
		{
			sprintf (format,"%s", "gif"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='p')&&(type_format[1]=='n')&&(type_format[2]=='g'))
		{
			sprintf (format,"%s", "png"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='j')&&(type_format[1]=='p')&&(type_format[2]=='g'))
		{
			sprintf (format,"%s", "jpg"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='i')&&(type_format[1]=='c')&&(type_format[2]=='o'))
		{
			sprintf (format,"%s", "ico"); 
	  		format[strlen(format)]='\0';
		}

		sprintf (csize,"%d",size); 
	  	csize[strlen(csize)]='\0';

		send (sock,buf_begin,strlen(buf_begin),0);
		send (sock,format,strlen(format),0);
		send (sock,buf_med,strlen(buf_med),0);
		send (sock,csize,strlen(csize),0);
		send (sock,buf_end,strlen(buf_end),0);

		char buf[1000];

		while (!(feof(f)))
		{
			int n = fread (buf,sizeof(char),1000,f);
			buf[n]='\0';

			send(sock,buf,n,0);
		}

		fclose (f);

		//printf(">>>SIZE OF FILE: %d\n", size);
	}
	else
	{
		printf(">>>error in entering main page\n");
		return -2;
	}

	return 1;
}

int send_app_type (int sock, char* path, char* type_format)
{
  printf(">>>%s uploaded\n", path);
  //printf(">>>type of uploading file: %s\n\n", type_format);

  FILE* f;

	char buf_begin[]="HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: application/";
	char buf_med[]="; charset=UTF-8\r\nContent-Length: ";
	char buf_end[]="\r\nConnection: close\r\n\r\n";
	char csize[100];
	
	int size;
	
	f = fopen (path, "rb");
	if (!f)
	{
		printf(">>>can't open file\n");
		return -1;
	}

	size = filesize (f);
	fclose (f);
	
	if (size>1e14)
	{
		printf(">>>inappropriate format of uploading file\n");
		return -1;
	}
	else if ((size>0)&&(size<1e14))
	{
		f = fopen (path, "rb");
		if (!f)
		{
			printf(">>>can't open file\n");
			return -1;
		}

		char format[20];

		if  ((type_format[0]=='e')&&(type_format[1]=='o')&&(type_format[2]=='t'))
		{
			sprintf (format,"%s", "vnd.ms-fontobject"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='t')&&(type_format[1]=='t')&&(type_format[2]=='f'))	
		{
			sprintf (format,"%s", "font-ttf"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='o')&&(type_format[1]=='t')&&(type_format[2]=='f'))
		{
			sprintf (format,"%s", "font-otf"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='w')&&(type_format[1]=='o')&&(type_format[2]=='f')&&(type_format[3]=='f'))
		{
			sprintf (format,"%s", "font-woff"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='w')&&(type_format[1]=='o')&&(type_format[2]=='f')&&(type_format[3]=='f')&&(type_format[4]=='2'))
		{
			sprintf (format,"%s", "font-woff"); 
	  		format[strlen(format)]='\0';
		}

		if  ((type_format[0]=='j')&&(type_format[1]=='s')&&(type_format[2]=='o')&&(type_format[3]=='n'))
		{
			sprintf (format,"%s", "json"); 
	  		format[strlen(format)]='\0';
		}

		sprintf (csize,"%d",size); 
	  	csize[strlen(csize)]='\0';

		send (sock,buf_begin,strlen(buf_begin),0);
		send (sock,format,strlen(format),0);
		send (sock,buf_med,strlen(buf_med),0);
		send (sock,csize,strlen(csize),0);
		send (sock,buf_end,strlen(buf_end),0);

		char buf[1000];

		while (!(feof(f)))
		{
			int n = fread (buf,sizeof(char),1000,f);
			buf[n]='\0';

			send(sock,buf,n,0);
		}

		fclose (f);
	}
	else
	{
		printf(">>>error in entering main page\n");
		return -2;
	}

	return 1;
}

int request(char* request, int sock)
{
	struct timeval tv;
  	int ret;
  	double start;

  	start = time_now();

	ret = recv(sock, request, 800, 0);
	if (ret < 0)
	{
		printf (">>>404<<<\n");
	  	printf(">>>recv error\n");
	    return -1;
	}
	if (time_now()-start > 30)
	{
		printf (">>>too much time awaiting\n");
		return -2;
	}

	request[ret] = '\0';

	//printf("\n\n>>>request from socket:\n\n%s\n\n", request);

	return 1;  
}

int reaction (char* request, int sock_peer)
{
	char cpath[1000];
	char ctype_format[5];
	int err;

	err = path(request, cpath);
		if (err < 0)
	    {
	    	printf (">>>404<<<\n");
		    printf(">>>error in path find\n");
	  	}

	if (strlen(cpath)==0)
	{
		err = send_site (sock_peer);
		if (err < 0)
	    {
	    	printf (">>>404<<<\n");
		    printf(">>>error in entering main page\n");
	  	}
	}

	else if (strlen(cpath)>0)
	{
	  	err = type (cpath, ctype_format);
		if (err < 0)
	    {
	    	printf (">>>404<<<\n");
		    printf(">>>error in type find\n");
	  	}

	  	if  (   
	  		((ctype_format[0]=='c')&&(ctype_format[1]=='s')&&(ctype_format[2]=='s'))
	  		||
	  		((ctype_format[0]=='j')&&(ctype_format[1]=='s'))
	  		||
	  		((ctype_format[0]=='p')&&(ctype_format[1]=='h')&&(ctype_format[2]=='p'))
	  		||
	  		((ctype_format[0]=='h')&&(ctype_format[1]=='t')&&(ctype_format[2]=='m')&&(ctype_format[3]=='l'))
	  		)
	  	{
	  		err = send_text_type (sock_peer, cpath, ctype_format);
	  		if (err < 0)
		    {
		    	printf (">>>404<<<\n");
			    printf(">>>error in sending\n");
		  	}
	  	}

	  	if  (   
	  		((ctype_format[0]=='s')&&(ctype_format[1]=='v')&&(ctype_format[2]=='g'))
	  		||
	  		((ctype_format[0]=='g')&&(ctype_format[1]=='i')&&(ctype_format[2]=='f'))
	  		||
	  		((ctype_format[0]=='p')&&(ctype_format[1]=='n')&&(ctype_format[2]=='g'))
	  		||
	  		((ctype_format[0]=='j')&&(ctype_format[1]=='p')&&(ctype_format[2]=='g'))
	  		||
	  		((ctype_format[0]=='i')&&(ctype_format[1]=='c')&&(ctype_format[2]=='o'))
	  		)
	  	{
	  		err = send_image_type (sock_peer, cpath, ctype_format);
	  		if (err < 0)
		    {
		    	printf (">>>404<<<\n");
			    printf(">>>error in sending\n");
		  	}
	  	}

	  	if  (   
	  		((ctype_format[0]=='e')&&(ctype_format[1]=='o')&&(ctype_format[2]=='t'))
	  		||
	  		((ctype_format[0]=='t')&&(ctype_format[1]=='t')&&(ctype_format[2]=='f'))
	  		||
	  		((ctype_format[0]=='o')&&(ctype_format[1]=='t')&&(ctype_format[2]=='f'))
	  		||
	  		((ctype_format[0]=='w')&&(ctype_format[1]=='o')&&(ctype_format[2]=='f')&&(ctype_format[3]=='f'))
	  		||
	  		((ctype_format[0]=='w')&&(ctype_format[1]=='o')&&(ctype_format[2]=='f')&&(ctype_format[3]=='f')&&(ctype_format[4]=='2'))
	  		||
	  		((ctype_format[0]=='j')&&(ctype_format[1]=='s')&&(ctype_format[2]=='o')&&(ctype_format[3]=='n'))
	  		)
	  	{
	  		err = send_app_type (sock_peer, cpath, ctype_format);
	  		if (err < 0)
		    {
		    	printf (">>>404<<<\n");
			    printf(">>>error in sending\n");
		  	}
	  	}
	}
	else
	{
		printf(">>>error in request processing\n");
	}

	return 1;
}

int add_to_fds (fd_set* fds, struct mem_ip* mp)
{
	int i;

	for (i=0;i<20;i++)
	{
			//printf(">>>mp[%d].sock = %d\n", i, mp[i].sock);
			FD_SET (mp[i].sock, fds);
	}

	return 0;
}

int server (int sock, int sock_ip)
{
	struct sockaddr_in client_addr;
	socklen_t pointer;
	struct mem_ip* mp;
	fd_set fds;
	struct timeval tv;

	int sock_peer, err, res, i, j, k, max, count=1;
	int* sum;
	double start;
	char crequest[800];
	

	if (listen(sock, 4) < 0)
	{
    	printf(">>>error in socket listening\n");
    	return -1;;
    }

    start = time_now();

    mp = new mem_ip[4096];
	if (!mp)
	{
		printf(">>>allocation error\n");
	  	return -1;
	}

	mp[0].ip = sock_ip;
	mp[0].flag = 1;
	mp[0].sock = sock;
    for (i = 1; i < 4096; i++)
	{
		mp[i].ip = 0;
		mp[i].flag = 0;
		mp[i].sock = 0;
	}

	sum = new int[4096];
	if (!sum)
	{
		printf(">>>allocation error\n");
	  	return -1;
	}
	for (i = 0; i < 4096; i++)
		sum[i] = 0;

	max = sock;

    while(1)
    {
    	FD_ZERO (&fds);
    	err = add_to_fds (&fds, mp);
    	if (err<0)
  		{
			printf("\n\n>>>error in coping to fds structure\n"); 
			return -4;
  		}

    	res = select(max+1,&fds,NULL,NULL,NULL);
    	//printf("\n\n>>>select --> number of changed sockets: %d\n", res);

  		if (res<0)
  		{
			printf(">>>[-] select failed and returned %d\n", res);
			return -5;
  		}
  		else if(res == 0)
  		{
  			//printf("\n>>>no sockets changed\n\n");
			continue;
  		}
		else if (res>0)
		{
			if (FD_ISSET(sock, &fds))
			{
	      		pointer = sizeof(client_addr);
		    	sock_peer = accept (sock,(struct sockaddr*)&client_addr,&pointer);

		    	if (sock_peer < 0) 
		    	{ 
		    		printf (">>>404<<<\n");
		    	    printf(">>>error in socket acception\n");
		          	continue;
		   		} 
		   		//printf(">>>[+] connection with socket successfull\n");
		   		//printf(">>>accepting %d socket\n", sock_peer);
		   		//printf(">>>%d socket with its ip added in %d row of struct mp\n", sock_peer, count);

		   		if (sock_peer>max)
		   			max = sock_peer;

		   		mp[count].ip = (int)client_addr.sin_addr.s_addr;
		   		mp[count].flag = 1;
		   		mp[count].sock = sock_peer;

		   		sum[count] = 0;
		   		
		   		for (i=1; i<count+1; i++)
		   		{
					if (mp[i].ip==(int)client_addr.sin_addr.s_addr)
		   			{
		   				if (mp[i].sock != sock_peer)
		   					sum[count] += mp[i].flag;
		   			}
		   		}

		   		//printf(">>>number of opened sockets on %d ip: %d\n", mp[count].ip, sum[count]);
		   		
		   		if (sum[count]>10)
				{
				   	printf(">>>too much connections from ip address: %d\n", mp[count].ip);
				   	//printf(">>>closing %d socket", sock_peer);
				   	mp[count].flag = 0;
				    close (sock_peer);

				    //printf(">>>worktime on the %d socket: %lf secs\n", sock_peer, time_now()-start);
				    continue;
				}
		    
		    	struct linger linger_opt = {0, 0};
		    	setsockopt(sock_peer, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));

		  		//printf(">>> current step/row in struct mp: %d\n\n",count);
		  		count++;
	    	}



	    	for (i=0;i<max+1;i++)
			{
				if (i!=mp[0].sock)
				{
					if (FD_ISSET(i, &fds))
					{
						//printf(">>>using %d socket\n", i);
			      		err = request (crequest, i);
				    	if (err < 0)
				    	{
				    		printf (">>>404<<<\n");
					    	printf (">>>error in request\n");
					    	continue;
				  		}

				  		err = reaction (crequest, i);
				    	if (err < 0)
				    	{
				    		printf (">>>404<<<\n");
					    	printf(">>>error in reaction\n");
					    	continue;
				  		}

				  		for (j=0;j<max+1;j++)
				  		{
				  			if (i == mp[j].sock)
				  			{
				  				if (mp[j].flag!=0)
				  				{
				  					mp[j].flag=0;

				  					for (k=j;k<max;k++)
				  					{
				  						mp[k].ip = mp[k+1].ip;
				  						mp[k].flag = mp[k+1].flag;
				  						mp[k].sock = mp[k+1].sock;
				  					}
				  					count--;

				  					break;
				  				}
				  				else
				  					continue;
				  			}
				  		}

				  		close(i);
		    			//printf(">>>closing %d socket\n",i);

		    			//printf(">>>worktime on the %d socket: %lf secs\n", i,time_now()-start);
			    	}
				}
			}

			/*printf("\nvvv contents of struct mp:\n");
	    	for (i=0; i<count; i++)
	   		{
	   			printf(">>>mp.ip[%d]: %d\n", i,mp[i].ip);
	   			printf(">>>mp[%d].flag = %d\n", i,mp[i].flag);
	   			printf(">>>mp.sock[%d]: %d\n", i,mp[i].sock);
	   		}*/
	   		// printf ("\n");

	   		if (time_now()-start > 1800)
		    	printf (">>>too much time awaiting one of clients\n");
		}
    }

    delete mp;
    delete sum;
	return 1;
}

int main (void)
{
	struct sockaddr_in addr;
	struct hostent *h;

	int sock, sock_ip, err;
	char adr[] = "192.168.0.10";

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) 
	{
	  printf(">>>error in socket creation\n");
	  return -2;
	}

	h = gethostbyname (adr);
    if (h == NULL) {
        printf("address error\n");
        return -1;
    }
    
    memcpy (&(addr.sin_addr.s_addr), h -> h_addr_list[0], 4);

    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET;

    signal(SIGCHLD,SIG_IGN);

    struct linger linger_opt = {1,0}; 
    setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));

    err = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (err < 0)
    {
    	printf (">>>404\n");
	    printf(">>>error in binding\n");
	    close(sock);
	    return -3;
  	}

  	sock_ip = (int)addr.sin_addr.s_addr;

  	//printf(">>>main sock ip: %d\n", sock_ip);
  	printf("\n");

  	err = server (sock, sock_ip);
  	if (err < 0)
    {
	    printf(">>>error in work of server\n");
	    close(sock);
	    return err;
  	}

  	printf (">>>closing main socket\n>>>good bye\n");
  	close (sock);
	return 0;
}