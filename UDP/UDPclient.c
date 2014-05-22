#include "UDP.h"

int main(int argc, char**argv)
{
   if (argc>2)
		write(2, "Too many arguments", strlen("Too many arguments"));
   int sockfd;
   struct sockaddr_in servaddr;
   char sendline[SIZE_BUFF];
   for (int i=0; i<SIZE_BUFF; ++i)
		sendline[i]='!';

   sockfd=socket(AF_INET,SOCK_DGRAM,0);
   memset(&servaddr, 0, sizeof(servaddr));

   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(PORT);

   for (int i=0; i<COUNT_ITEMS; ++i)
      sendto(sockfd, sendline, strlen(sendline), 0 ,(struct sockaddr *)&servaddr,sizeof(servaddr));
   close(sockfd);
   return 0;
}
