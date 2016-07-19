#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <fstream>

#include <cassert>
#include <thread>
#include <mutex>
#include <queue>
#include <cmath>
#include <cerrno>
#include <cfenv>

#include <map>
#include <utility>

#include <stdexcept>      // std::invalid_argument

#include <sstream>
#include <iostream>

#include <atomic>

#include <xmmintrin.h> //__mm_pause()

#include "net_utils.h"

#define MAXEVENTS 64

#define DEFAULT_BUFFER_SIZE 4096
#define DEFAULT_RING_BUFFER_SIZE 1024

#define X_MAX (500.0)
#define X_MIN (-500.0)
#define Y_MAX (500.0)
#define Y_MIN (-500.0)





// Event data includes the socket FD,
// a pointer to a fixed sized buffer,
// number of bytes written and 
// a boolean indicating whether the worker thread should stop.
struct event_data {
	int fd;
	char* buffer;
	int written;
	bool stop;

	event_data():
		fd(-1),
		buffer(new char[DEFAULT_RING_BUFFER_SIZE]),
		written(0),
		stop(false) {

		}

	~event_data() {
//		delete[] buffer;
//		std::cout << __func__ << std::endl;
	}
};





std::mutex currentPlayers_mutex;



int processGetRequest(std::string& path, std::string& resp, int fd) {

	/*std::string */resp= "HTTP/1.0 200\r\nDate: Fri, 20 Dec 2016 23:59:59 GMT\r\nServer: lab5 \r\nContent-Length: ";
	//string path="";
	//path =request.path;

	//find file
	std::cout << __func__ << ": " << path << std::endl;
	int page= open(path.c_str(),O_RDONLY);

	if(page<0){
		perror("open");
//		std::cout<< "fd = " << *fd <<" 404 \n";
		std::cout<< "404 \n";
		resp = "HTTP/1.0 404 File Not Found\r\nServer: lab5 \r\nContent-Length: 0\r\nContent-type: text/html\r\n\r\n";

		send(fd, resp.c_str(), resp.length(), 0);
		close(fd);
		

		return 0; 
	}
	//get size
	FILE * pageF= fdopen(page,"rb"); 
	fseek(pageF, 0L, SEEK_END);
	int sz = ftell(pageF);
	fseek(pageF, 0L, SEEK_SET);

	//form content length
	std::stringstream ss;
	ss<<resp<<sz-1<<"\r\n";
	resp=ss.str(); 

	//make response
	
	if(path.find(".gif")!=std::string::npos)
		resp += "Content-type: image/gif\r\n\r\n";
	else if(path.find(".png")!=std::string::npos)
		resp += "Content-type: image/png\r\n\r\n";
	else if(path.find(".jpg")!=std::string::npos)
		resp += "Content-type: image/jpeg\r\n\r\n";
	else
		resp += "Content-type: text/html\r\n\r\n";

	//write response
//	write( *fd, resp.c_str(), resp.length());


	int total=0;      
	char buff[1024];
	int readBytes = 0;
	int er;
	
	ss.str(std::string());

	//send file
	std::ifstream hfile(path);
//	hfile.open(path);
	std::string tmp_str = "";

	send(fd, resp.c_str(), resp.length(), 0); 
	while(getline(hfile,tmp_str)) {
		if (tmp_str.empty()) {
			ss << std::endl;
			//ss << "\r\n";
			//tmp_str = "\r\n";
			tmp_str = "\r\n";
			send(fd, tmp_str.c_str(), tmp_str.length(), 0); 
			continue;
		}
		//std::cout << tmp_str << "\r\n";
		std::cout << tmp_str << std::endl;
		ss << tmp_str << "\r\n";
		send(fd, tmp_str.c_str(), tmp_str.length() ,0 ); 
	}
	resp += ss.str();
	hfile.close();
	close(page);
	close(fd);
	return 0;
	do{

		std::cout<<"page fd "<< page <<"\n";
		readBytes= read(page, buff, sizeof(buff));
		std::cout<<"read bytes "<<readBytes<<"\n";

		if(readBytes<0){
			perror("read");

			break;
		}
		total+=readBytes;
		ss << buff;
		//er=  send( *fd, buff, readBytes,0 );   
		//std::cout<<"sent bytes "<<er<<"\n";
		//if (er==-1){
		//	perror("send");
		//}
		//else if( er != readBytes){
		//	std::cout<<"Read write miss match\n";
		//}

	}while(readBytes>0);
	
	//resp += ss.str();
	resp += "123123\r\n";
	
	close(page);
	return 0;
}	

	
char *reply_hdr_ok = 
"HTTP/1.0 200 OK\n"
"Date: Thu, 19 Feb 2009 12:27:04 GMT\n"
"Server: http_server/1.0.0\n"
"Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\n"
"ETag: \"56d-9989200-1132c580\"\n"
"Content-Type: text/html\n"
"Content-Length: 15\n"
"Accept-Ranges: bytes\n"
"Connection: close\n"
"\n";
//"sdfkjsdnbfkjbsf";

int http_handler(std::queue<struct event_data>* ring_buffer, std::string dir) {
	printf("start %s\n", __func__);
	int num_events_processed = 0;
	int s = -1;
	while (true) {
		while(ring_buffer->size() == 0) {
			//sleep(10);
			_mm_pause();
			//printf("%s: ring_buffer is empty, lets sleep\n", __func__);
		}
		while(ring_buffer->size() > 0) {
			auto e_data = ring_buffer->front();
			auto client_fd = e_data.fd;
			auto buffer = e_data.buffer;
			if (e_data.stop)
				goto exit_consumer;

			
			auto buffer_length = e_data.written;
			assert(client_fd != -1);
			//assert(buffer_length > 0);
/*
			s = write (1, buffer, buffer_length);
			if (s == -1) {
				perror ("write");
				abort ();
			}
*/
	//				ring_buffer->pop();

			//while(getline(ss, tmp_line)) {
			//	std::cout << tmp_line << std::endl;
			//}
			std::string in_s = std::string(e_data.buffer);
			std::cout << in_s << std::endl;
			

			size_t pos = in_s.find("GET");
			if(pos == std::string::npos) {
				std::cout << "not GET method" << std::endl;
				ring_buffer->pop();
				++num_events_processed;
			}
			else {
				std::cout << "GET method found" << std::endl;
			}
			
			std::string file_path = "";			
			size_t pos_slash = in_s.find("\/");
			size_t pos_space = in_s.find(" ", pos_slash);
			std::cout << pos_slash << " " << pos_space << std::endl;
			//if( (pos_space - pos_slash) == 1)
			file_path = in_s.substr(pos_slash, (pos_space-pos_slash));
			std::cout << file_path << std::endl;
			file_path.insert(0,dir);
			std::cout << file_path << std::endl;
			
			
			std::string out_str;
			processGetRequest(file_path, out_str, client_fd);
			std::cout << "RESPONSE:\n" << out_str << std::endl;
			
			//s = write(client_fd, out_str.c_str(), out_str.size());
//			s = send(client_fd, out_str.c_str(), out_str.size(),0);

			//s = write(client_fd, "123", 4);
//			std::cout << s  << std::endl;
//			if (s == -1) {
//				perror ("echo");
//				abort ();
//			}
			

			ring_buffer->pop();
			++num_events_processed;
			
		}
	}
	printf("stop %s\n", __func__);
exit_consumer:
	printf("Finished processing all events. Server shutting down. Num events processed = %d\n", num_events_processed);
	return 1;
}




void event_loop(int epfd,
		int sfd,
		//processor::RingBuffer<event_data>* ring_buffer) {
		std::queue < struct event_data >* ring_buffer) {
	int n, i;
	int retval;

	struct epoll_event event, current_event;
	// Buffer where events are returned.
	struct epoll_event* events = static_cast<epoll_event*>(calloc(MAXEVENTS, sizeof event));

	while (true) {

		n = epoll_wait(epfd, events, MAXEVENTS, -1);
//#if 0
		if(-1 == n) {
			printf("%s: %s\n", __func__, strerror(errno));
			if (EINTR == errno)
			{
				/* Handle shutdown request here. */ 
				break;
			}
			else {
				//TODO: Handle other errors
			}
		}	
//#endif
		for (i = 0; i < n; i++) {
			current_event = events[i];

			if ((current_event.events & EPOLLERR) ||
					(current_event.events & EPOLLHUP) ||
					(!(current_event.events & EPOLLIN))) {
				// An error has occured on this fd, or the socket is not ready for reading (why were we notified then?).
				//fprintf(stderr, "epoll error\n");
				close(current_event.data.fd);
			} else if (current_event.events & EPOLLRDHUP) {
				// Stream socket peer closed connection, or shut down writing half of connection.
				// We still to handle disconnection when read()/recv() return 0 or -1 just to be sure.
				printf("Closed connection on descriptor vis EPOLLRDHUP %d\n", current_event.data.fd);
				// Closing the descriptor will make epoll remove it from the set of descriptors which are monitored.
				close(current_event.data.fd);
			} else if (sfd == current_event.data.fd) {
				// We have a notification on the listening socket, which means one or more incoming connections.
				while (true) {
					struct sockaddr in_addr;
					socklen_t in_len;
					int infd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

					in_len = sizeof in_addr;
					// No need to make these sockets non blocking since accept4() takes care of it.
					infd = accept4(sfd, &in_addr, &in_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
					if (infd == -1) {
						if ((errno == EAGAIN) ||
								(errno == EWOULDBLOCK)) {
							break;  // We have processed all incoming connections.
						} else {
							perror("accept");
							break;
						}
					}

					// Print host and service info.
					retval = getnameinfo(&in_addr, in_len,
							hbuf, sizeof hbuf,
							sbuf, sizeof sbuf,
							NI_NUMERICHOST | NI_NUMERICSERV);
					if (retval == 0) {
						printf("Accepted connection on descriptor %d (host=%s, port=%s)\n", infd, hbuf, sbuf);
					}

					// Register the new FD to be monitored by epoll.
					event.data.fd = infd;
					
					// Register for read events, disconnection events and enable edge triggered behavior for the FD.
					event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
					retval = epoll_ctl(epfd, EPOLL_CTL_ADD, infd, &event);
					if (retval == -1) {
						perror("epoll_ctl");
						abort();
					}
				}
			} else {
				printf("Some event occcurs on descriptor %d\n", current_event.data.fd);
				// We have data on the fd waiting to be read. Read and  display it.
				// We must read whatever data is available completely, as we are running in edge-triggered mode
				// and won't get a notification again for the same data.
				bool should_close = false, done = false;

				while (!done) {
					ssize_t count;
					// Get the next ring buffer entry.
					//struct event_data *entry = (struct event_data *)malloc(sizeof(struct event_data));
					struct event_data entry;
					// Read the socket data into the buffer associated with the ring buffer entry.
					// Set the entry's fd field to the current socket fd.
					count = read(current_event.data.fd, entry.buffer, DEFAULT_BUFFER_SIZE);
					entry.written = count;
					entry.fd = current_event.data.fd;

					if (count == -1) {
						// EAGAIN or EWOULDBLOCK means we have no more data that can be read.
						// Everything else is a real error.
						if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
							perror("read");
							should_close = true;
						}
						done = true;
					} else if (count == 0) {
						// Technically we don't need to handle this here, since we wait for EPOLLRDHUP. We handle it just to be sure.
						// End of file. The remote has closed the connection.
						should_close = true;
						done = true;
					} else {
						printf("Read form socket %d: %d bytes\n", current_event.data.fd, (int)count);
						// Valid data. Process it.
						// Check if the client want's the server to exit.
						// This might never work out even if the client sends an exit signal because TCP might
						// split and rearrange the packets across epoll signal boundaries at the server.
						bool stop = (strncmp(entry.buffer, "exit", 4) == 0);
						entry.stop = stop;

						// Publish the ring buffer entry since all is well.
						//ring_buffer->publish(next_write_index);
						printf("Ring buffer size = %d\n", (int)ring_buffer->size());
						ring_buffer->push(entry);
						printf("Ring buffer size = %d\n", (int)ring_buffer->size());
						if (stop)
							goto exit_loop;
					}
				}


				if (should_close) {
					printf("Closed connection on descriptor %d\n", current_event.data.fd);
					// Closing the descriptor will make epoll remove it from the set of descriptors which are monitored.
					close(current_event.data.fd);
				}
			}
		}
	}
exit_loop:
	free(events);
}


void terminate(int sig) {
	//sig = sig; /* Cheat compiler to not give a warning about an unused variable. */
	printf("SIGTERM: Terminate all threads\n");
}
/*
int signal_handler_set(int sig, void (*sa_handler ) (int) ) {
	struct sigaction sa = {0};
	sa.sa_handler = sa_handler;
	return sigaction(sig, &sa, NULL);
}

*/
char * srv_getopt_str = "h:p:d:f";
void print_usage() {
	printf("%s\n", "http_server");
	printf("usage:\n");
	printf(" %s -h -p -d\n", "http_server");
	printf("example:\n");
	printf(" $ %s -h 127.0.0.1 -p 8080 -d ./directory/ \n", "http_server");
}

struct server_options{
	std::string host;
	unsigned int port;
	std::string dir;
	bool fork;
	server_options() : host("127.0.0.1"), port(80), dir("./"), fork (true) {}
};

struct server_options srv_opts;

int main (int argc, char *argv[]) {
	if(argc == 1) { // если запускаем без аргументов, выводим справку
		print_usage();
		return 0;
	}
	int sfd, epfd, retval;
	int rez=0;

//	opterr=0;
	while ((rez = getopt(argc,argv, srv_getopt_str)) != -1) {
		switch (rez){
			case 'h':
				printf("host = %s\n", optarg);
				srv_opts.host = std::string(optarg);
				break;
			case 'p':
				printf("port = %s\n", optarg);
				srv_opts.port = atoi(optarg);
				break;
			case 'd':
				printf("directory = %s\n", optarg);
				srv_opts.dir = std::string(optarg);
				break;
			case 'f':
				printf("no fork\n");
				srv_opts.fork = false;
				break;
			case '?':
				printf("Error found !\n");
				return 0;
				break;
		}
	}

	printf("Server options:\nhost=%s\nport=%u\ndir=%s\nfork=%d\n",
								 srv_opts.host.c_str(),
								 srv_opts.port,
								 srv_opts.dir.c_str(),
								 srv_opts.fork ? 1 : 0
								);
	// Our ring buffer.
	//auto ring_buffer = new processor::RingBuffer<event_data>(DEFAULT_RING_BUFFER_SIZE);
	//std::queue<struct event_data>* ring_buffer = new std::queue<struct event_data>(DEFAULT_RING_BUFFER_SIZE);
	std::queue<struct event_data>* ring_buffer = new std::queue<struct event_data>();



	sfd = create_and_bind(srv_opts.host.c_str(), srv_opts.port);
	if (sfd == -1)
		abort ();


	retval = make_socket_non_blocking(sfd);
	if (retval == -1)
		abort ();

	retval = listen(sfd, SOMAXCONN);
	if (retval == -1) {
		perror ("listen");
		abort ();
	}

	epfd = epoll_create1(0);
	if (epfd == -1) {
		perror ("epoll_create");
		abort ();
	}

	// Register the listening socket for epoll events.
	{
		struct epoll_event event;
		event.data.fd = sfd;
		event.events = EPOLLIN | EPOLLET;
		retval = epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &event);
		if (retval == -1) {
			perror ("epoll_ctl");
			abort ();
		}
	}


	pid_t child = 0;
	if(srv_opts.fork) {
		child = fork();
		if(child == -1) {
			perror("fork");
			return -1;
		} else if(child > 0) { // parent, returned child pid
			return 0;
		}
	}
	// Start the worker thread.
	//std::thread t{process_messages, ring_buffer};
	std::thread t(http_handler, ring_buffer, srv_opts.dir);

	// Start the event loop.
	event_loop(epfd, sfd, ring_buffer);

	// Our server is ready to stop. Release all pending resources.
	t.join();
	close(sfd);
	delete ring_buffer;

	return EXIT_SUCCESS;
}
