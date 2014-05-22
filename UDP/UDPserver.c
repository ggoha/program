#include "UDP.h"

int main(int argc, char**argv)
{
   int sockfd, n;
   struct sockaddr_in servaddr, cliaddr;
   socklen_t len;
   char mesg[SIZE_BUFF];

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   memset(&servaddr, 0, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=INADDR_ANY;
   servaddr.sin_port=htons(PORT);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);	
   time_start();		
   for (int i=0; i<COUNT_ITEMS-1; ++i)
   {
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
/*      mesg[n] = 0;
      printf("%s",mesg);
*/ 
   }
   long int time =time_stop(), size=COUNT_ITEMS*SIZE_BUFF/(1024*1024.0);
   printf("Size: %ld Mb, Time: %ld ms, Speed: %ld Mb/s", size, time, 1000*size/time);	
   close(sockfd);
   return 0;
}
