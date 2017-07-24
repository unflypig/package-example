#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main( int argc, char **argv )
{
	char			*ptr, **pptr;
	char			str[INET_ADDRSTRLEN];
	struct hostent		*hptr;
	struct in_addr		* addr;
	struct sockaddr_in	saddr;

	/* 取参数 */
	while ( --argc > 0 )
	{
		ptr = *++argv;                                                                              /* 此时的ptr是ip地址 */
		if ( !inet_aton( ptr, &saddr.sin_addr ) )                                                   /* 调用inet_aton()，将ptr点分十进制转in_addr */
		{
			printf( "Inet_aton error\n" );
			return(1);
		}
		
		if ( (hptr = gethostbyaddr( (void *) &saddr.sin_addr, 4, AF_INET ) ) == NULL )      /* 把主机信息保存在hostent中 */
		{	
			//printf( "gethostbyaddr error for addr:%s\n", ptr );
			//printf( "h_errno %d\n", h_errno );
			//return(1);
			printf( "%s:unknown\n",ptr);
			continue;
		}
		
		printf( "%s:%s\n",ptr,hptr->h_name );
		/* 正式主机名 */
	}
		return 0;	
}




