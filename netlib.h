/*
TODO:
- Add error code constants

*/


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

/**
 * Connects to target host via TCP and returns UNIX file descriptor.
 * 
 * char* target:      IP or web address of host to connect to (e.g. "192.168.0.1", "www.example.com")
 * char* target_port: Port to connect to at target (e.g. "80", "1729")
 * 
 * return:            Returns UNIX file descriptor to which one can write. Returns error code upon failure (error codes below)
 * 
 * {error codes}:
 *  Unable to resolve address =>   -1
 *  Unable to set up socket =>     -2
 *  Unable to connect to server => -3
 */
int tconnect(char* target, char* target_port)
{
	int retfd;
	struct addrinfo hints, *servinfo;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if(getaddrinfo(target, target_port, &hints, &servinfo) != 0)
	{
		return -1;
	}
	
	if((retfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
	{
		return -2;
	}
	
	if(connect(retfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		return -3;
	}
	freeaddrinfo(servinfo);
	return retfd;
}


/**
 * Disconnects TCP stream in both directions.
 * 
 */
void tdisconnect(int targetfd)
{
	shutdown(targetfd, 2);
}


/**
* Sends data via TCP to a target host.
* 
* int targetfd:    UNIX file descriptor of target (With TCP connection established)
* char* bytes:     Pointer to the bytes which will be sent.
* int &bytes_size: Number of bytes to send (size of [char* bytes]).
* 
* return:          Returns 0 upon success and error code upon failure
* 
* {error codes}:
*  Unable to send data =>                     -1
*/
int tsend(int targetfd, char* bytes, int bytes_size)
{
	int bytes_sent = 0;
	do
	{
		bytes_sent = send(targetfd, bytes, bytes_size, 0);
		if(bytes_sent == -1) return -1;
	}while(bytes_sent < bytes_size);
	return 0;
}

/**
* Recieve data from host via TCP.
* 
* int targetfd:    UNIX file descriptor of target (With TCP connection established)
* char* bytes:     Pointer to the bytes where the recieved data will be written.
* int &bytes_size: Number of bytes allocated at [char* bytes]. Will be set to amount of bytes recieved!
* 
* return:          Returns 0 upon success and error code upon failure
* 
* {error codes}:
*  Recieved no data / target disconnected =>  -2
*/
int trecv(int targetfd, char* bytes, int *bytes_size)
{
	if((*bytes_size = recv(targetfd, bytes, *bytes_size, 0)) < 1)
	{
		return -2;
	}
	return 0;
}

/**
* Sends data via TCP to a target host and recieves response data from host.
* 
* int targetfd:    UNIX file descriptor of target (With TCP connection established)
* char* bytes:     Pointer to the bytes which will be sent. Returning bytes will be written there aswell!
* int &bytes_size: Number of bytes allocated at [char* bytes]. Will be set to amount of bytes recieved!
* 
* return:          Returns 0 upon success and error code upon failure
* 
* {error codes}:
*  Unable to send data =>                     -1
*  Recieved no data / target disconnected =>  -2
*/
int tsend_recv(int targetfd, char* bytes, int *bytes_size)
{
	int bytes_sent = 0;
	do
	{
		bytes_sent = send(targetfd, bytes, *bytes_size, 0);
		if(bytes_sent == -1) return -1;
	}while(bytes_sent < *bytes_size);
	
	if((*bytes_size = recv(targetfd, bytes, *bytes_size, 0)) < 1)
	{
		return -2;
	}
	return 0;
}

/**
 * Creates a TCP host on port [PORT] and returns UNIX file descriptor.
 * 
 * const char* PORT: The port on which to listen for incoming connections (as a C string [a.k.a. char pointer])
 * 
 * return:           Returns UNIX file descriptor on which one can listen for & accept incomming connections. Returns error code upon failure (error codes below)
 * 
 * {error codes}:
 *  Unable to resolve address =>                           -1
 *  Unable to set up UNIX files descriptor =>              -2
 *  Unable to bind to port =>                              -3
 *  Unable to force bind to port (Enable reuse of port) => -4
 */
int tcreate_host(const char* PORT)
{
	int retfd;
	struct addrinfo hints, *servinfo;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if(getaddrinfo(NULL, PORT, &hints, &servinfo) != 0)
	{
		return -1;
	}
	
	if((retfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
	{
		return -2;
	}
	
	int yes = 1;
	if (setsockopt(retfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		return -4;
	}
	
	if(bind(retfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		return -3;
	}
	
	freeaddrinfo(servinfo);
	
	return retfd;
}


/**
 * Listens on given UNIX file descriptor form incomming connections accepts the first it gets. Returns file descriptor.
 * This function will block until either an error occured or a connection was successfully established!
 * 
 * int sockfd:        UNIX file descriptor on which the Kernel is listening for incoming connections
 * const int BACKLOG: The amount of connections the queue will hold. Should be equal to or less than real backlog, which is defined by the kernel (e.g. 128 for linux kernel version 2.x)
 * 
 * return:           Returns UNIX file descriptor over which one can communicate with connecting node. Returns error code upon failure (error codes below)
 * 
 * {error codes}:
 *  Unable to listen for incoming connection =>  -1
 *  Unable to accept incoming connection =>      -2
 */
int tlisten_accept(int sockfd, const int BACKLOG)
{
	int retfd;
	struct sockaddr_storage conn_addr;
	socklen_t conn_addr_size = sizeof conn_addr;
	
	if(listen(sockfd, BACKLOG) == -1)
	{
		return -1;
	}
	
	if((retfd = accept(sockfd, (struct sockaddr*)&conn_addr, &conn_addr_size)) == -1)
	{
		return -2;
	}
	
	return retfd;
}


/**
 * Listens on given UNIX file descriptor form incomming connections accepts the first it gets. Returns file descriptor.
 * This function will block until either an error occured or a connection was successfully established!
 * 
 * int sockfd:                    UNIX file descriptor on which the Kernel is listening for incoming connections
 * const int BACKLOG:             The amount of connections the queue will hold. Should be equal to or less than real backlog, which is defined by the kernel (e.g. 128 for linux kernel version 2.x)
 * struct sockaddr_storage &addr: Reference to struct in which information about connecting node will be saved
 * socklen_t &addr_size:          Size of [addr]
 * 
 * return:                        Returns UNIX file descriptor over which one can communicate with connecting node. Returns error code upon failure (error codes below)
 * 
 * {error codes}:
 *  Unable to listen for incoming connection =>  -1
 *  Unable to accept incoming connection =>      -2
 */
int tlisten_accept_a(int sockfd, const int BACKLOG, struct sockaddr_storage *addr, socklen_t *addr_size)
{
	int retfd;
	
	if(listen(sockfd, BACKLOG) == -1)
	{
		return -1;
	}
	
	if((retfd = accept(sockfd, (struct sockaddr*)addr, addr_size)) == -1)
	{
		return -2;
	}
	
	return retfd;
}


/**
 * Resolve target address and return UNIX file descriptor.
 * 
 * const char* target:          IP or web address of host to send to (e.g. "192.168.0.1", "www.example.com")
 * const char* target_port:     Port to send to at target (e.g. "80", "1729")
 * struct addrinfo *targetinfo: Pointer to addrinfo structure in which infos about the target will be saved (required for usend)
 * 
 * return:                      Returns UNIX file descriptor to socket over which one can send UDP packages. Returns error code upon failure (error codes below)
 * 
 * {error codes}:
 *  Unable to resolve address => -1
 *  Unable to set up socket =>   -2
 */
int usock(const char* target, const char* target_port, struct addrinfo **targetinfo)
{
	struct addrinfo hints;
	int retfd;
	
	memset(&hints, 0, sizeof hints);
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if(getaddrinfo(target, target_port, &hints, targetinfo) != 0)
	{
		return -1;
	}
	
	if((retfd = socket((*targetinfo)->ai_family, (*targetinfo)->ai_socktype, (*targetinfo)->ai_protocol)) == -1)
	{
		return -2;
	}
	return retfd;
}


/**
 * Send data via UDP.
 * 
 * int sockfd:                  Socket over which packets will be send
 * struct addrinfo *targetinfo: Pointer to addrinfo structure in which infos about the target will be saved (required for usend)
 * const char* data             Pointer to data which will be send
 * const int DATA_SIZE          Size of [data] memory block
 * 
 * return:                      Returns bytes sent (might not be equal to DATA_SIZE)
 */
int usend(int sockfd, struct addrinfo *targetinfo, const char* data, const int DATA_SIZE)
{
	return sendto(sockfd, data, DATA_SIZE, 0, targetinfo->ai_addr, targetinfo->ai_addrlen);
}


/**
 * Send UDP packet to target host and return bytes sent.
 * 
 * const char* target:      IP or web address of host to send to (e.g. "192.168.0.1", "www.example.com")
 * const char* target_port: Port to send to at target (e.g. "80", "1729")
 * const char* data:        Pointer to data which will be send
 * const int DATA_SIZE:     Size of [data] memory block
 * 
 * return:                  Returns bytes sent (might not be equal to DATA_SIZE). Returns error code upon failure (error codes below)
 * 
 * {error codes}:
 *  Unable to resolve address => -1
 *  Unable to set up socket =>   -2
 *  Unable to send data =>       -3
 */
int usend_once(const char* target, const char* target_port, const char* data, const int DATA_SIZE)
{
	struct addrinfo hints, *servinfo;
	int sockfd;
	int retbytes;
	
	memset(&hints, 0, sizeof hints);
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if(getaddrinfo(target, target_port, &hints, &servinfo) != 0)
	{
		return -1;
	}
	if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
	{
		freeaddrinfo(servinfo);
		return -2;
	}
	if((retbytes = sendto(sockfd, data, DATA_SIZE, 0, servinfo->ai_addr, servinfo->ai_addrlen)) == -1)
	{
		freeaddrinfo(servinfo);
		return -3;
	}
	freeaddrinfo(servinfo);
	close(sockfd);
	return retbytes;
}


/**
 * Creates a UDP host on port [PORT] and returns UNIX file descriptor.
 * 
 * const char* PORT: The port on which to listen for incoming connections (as a C string [a.k.a. char pointer])
 * 
 * return:           Returns UNIX file descriptor on which one can listen for & accept incomming connections. Returns error code upon failure (error codes below)
 * 
 * {error codes}:
 *  Unable to resolve address =>                           -1
 *  Unable to set up UNIX files descriptor =>              -2
 *  Unable to bind to port =>                              -3
 *  Unable to force bind to port (Enable reuse of port) => -4
 */
int ucreate_host(const char* PORT)
{
	int retfd;
	struct addrinfo hints, *servinfo;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	
	if(getaddrinfo(NULL, PORT, &hints, &servinfo) != 0)
	{
		return -1;
	}
	
	if((retfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
	{
		return -2;
	}
	
	int yes = 1;
	if (setsockopt(retfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
	{
		return -4;
	}
	
	if(bind(retfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		close(retfd);
		return -3;
	}
	
	freeaddrinfo(servinfo);
	
	return retfd;
}


/*
void tpollfds(int *fds, int biggest_fd)
{
	fds = select(biggest_fd+1, &fds, NULL, NULL, 0);
}
*/
