/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ------------------------------------------------------------------ */

#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
 
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

#include <pthread.h>
#if defined(__linux__) || defined(__APPLE__)
	#include <unistd.h>
#endif

#include "engine.h"

#ifdef WITH_VST
PluginHost    G_PluginHost;
#endif

pthread_t     t_network;
struct sockaddr_in si_me, si_other;

int cmdSocket;
CommandHandler  *comms;
	

/* ------------------------------------------------------------------ */


void setup_udp(int argc, char **argv) {

    //create a UDP socket
    if ((cmdSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
	    perror("socket failure");
	    exit(1);
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(cmdSocket, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
	    perror("bind failure");
	    exit(1);
    }     

}

void *thread_network(void *arg) {

	if ( !kernelAudio::audioStatus ) {
		pthread_exit(NULL);
		return NULL;
	}

	socklen_t slen = sizeof(si_other) ;
	int  recv_len;
	char buf[BUFLEN];
	String *response;

	while (!control::quit)	{

		fprintf(stderr, "Waiting for data...");
         
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(cmdSocket, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			perror("recvfrom failure");
			pthread_exit(NULL);
		}
         
		//print details of the client/peer and the data received
		fprintf(stderr, "Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		fprintf(stderr, "Data: %s\n" , buf);
   
		response = comms->parse( buf, recv_len );

		//now reply the client with the response data if there
		if( response && response->len() &&
			sendto(cmdSocket, response->gets(), response->len(), 0, (struct sockaddr*) &si_other, slen) == -1)
		{
			perror("sendto failure");
			pthread_exit(NULL);
	    }

		delete response;
    }
 
	close(cmdSocket);

	pthread_exit(NULL);
	return NULL;
}



int main(int argc, char **argv) {

	int ret = 0;
	bool runonce = true;
	comms = new CommandHandler();

	while( runonce ) 
	{
		runonce = false;

		init::prepareParser();
		init::prepareKernelAudio();
		init::prepareKernelMIDI();

		setup_udp(argc, argv);
	
		pthread_create(&t_network, NULL, thread_network, NULL);

		init::startKernelAudio();
	
		pthread_join(t_network, NULL);

		init::shutdown();

		if( control::restart ) 
		{
			runonce = true;
			control::restart = false;
			control::quit = false;
		}
	}

	return ret;
}
