//
// C++ Implementation: seqcontext
//
// Description: 
//
//
// Author: John Derry <johndderry@yahoo.com>, (C) 2015
//
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <alsa/asoundlib.h>
#include "seqcontext.h"
#include "midibase.h"
#include "midistream.h"

/*******************************************************
 *
 *	CLASS AddressList
 *	for list of destination ALSA sequencer client/ports
 * 
 *******************************************************/
  
AddressList::AddressList()
{
	head = (AddrListElem *)0;
}

AddressList::~AddressList()
{
	AddrListElem *next;
	while( head )
	{
		next = head->next;
		delete head;
		head = next;
	}
}

void AddressList::append(snd_seq_addr_t *a)
{
	AddrListElem *p, *ae = new AddrListElem;
	ae->addr.client = a->client;
	ae->addr.port = a->port;
	ae->next = (AddrListElem *)0;
	if( !head )	head = ae;
	else
	{
		p = head;
		while( p->next ) p = p->next;
		p->next = ae;
	}
	return; 
}

int AddressList::length()
{
	/* get dynamic length */
	int l = 0;
	AddrListElem *p = head;
	while( p )
	{
		p = p->next; l++;
	}
	return l;
}

snd_seq_addr_t *AddressList::index(int dev)
{
	AddrListElem *p = head;
	while( dev-- && p )
	{
		p = p->next;
	}
	if( p ) return &(p->addr);
	else 
		return (snd_seq_addr_t *)0;
}

/**************************************
 *
 *	CLASS SEQCONTEXT
 * 
 **************************************/

/*
 * helper function for SeqContext
 */
  
snd_seq_addr_t *parseclientport( char *address )
{
	snd_seq_addr_t *ret = (snd_seq_addr_t *)0;
	char *tmpstr, holdc, *start, *p = address;	

	while( (holdc = *p) && isdigit(holdc) ) p++;
	if( p == 0 )
	{
		/* just a client but no port */
		ret = new snd_seq_addr_t;
		ret->client = atoi( address );
		ret->port = 0;
		return ret;
	}
	if( *p == 0 ) return ret;
	if( *p != ':' )
	{
		fprintf(stderr,"bad format\n");
		return ret;
	}
	ret = new snd_seq_addr_t;
	start = p + 1;
	tmpstr = new char[p - address + 1];
	strncpy( tmpstr, address, p - address );
	tmpstr[p - address] = '\0'; 
	//*p = '\0';
	ret->client = atoi( tmpstr );
	//*p = holdc;
	ret->port = atoi( start );
	return ret;
}	

SeqContext::SeqContext()
{
	/*
	 * Create the sequencer context. Connected = 3 if all goes well
	 */	 
	connected = verbose = 0;

	handle = (snd_seq_t *)0;

	if (snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
		fprintf(stderr, "Could not open sequencer: %s", snd_strerror(errno));
		return;
		}
	
	connected++;

	if( (queue = snd_seq_alloc_queue(handle)) == 0 ) {
		fprintf(stderr, "Failure to allocate queue\n");
		return;		
	}
	if( (client = snd_seq_client_id(handle)) == 0 ) {
		fprintf(stderr, "Failure to get client address");
		return;		
	}
	
	connected++;

	destlist = new AddressList;
	source.client = client;
	source.port = 0;

	if( seq_new_port('e') < 0 ) {
		fprintf(stderr, "Could not open sequencer port: %s", snd_strerror(errno));
		return;
	}
	
	if( verbose ) {		
		fprintf(stderr, "Opened alsa sequencer for client %d and created queue\n", client );		
	}

	connected++;
}

SeqContext::~SeqContext()
{
		snd_seq_event_t ev;
		unsigned long t;

		snd_seq_drop_output(handle);

		t = 0;
		if( connected == 3 ) {
			seq_midi_event_init( &ev, t, 0);
			seq_midi_control( &ev, 0, MIDI_CTL_ALL_SOUNDS_OFF, 0);
			seq_send_to_all( &ev);

#ifdef USE_DRAIN
		snd_seq_drain_output(handle);
#else
		snd_seq_flush_output(handle);
#endif
		}
		if( queue )
			snd_seq_free_queue(handle, queue);

		snd_seq_close(handle);
}

void SeqContext::name( char *name ) {
	snd_seq_set_client_name(handle, name);
}

int SeqContext::seq_new_port(char cap)
{
	if( cap == 'r' )
		source.port = snd_seq_create_simple_port(handle, NULL,
					 SND_SEQ_PORT_CAP_READ |
					 SND_SEQ_PORT_CAP_SUBS_READ,
					 SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if( cap == 'w' )
		source.port = snd_seq_create_simple_port(handle, NULL,
					 SND_SEQ_PORT_CAP_WRITE |
					 SND_SEQ_PORT_CAP_SUBS_WRITE,
					 SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if( cap == 'd' )
		source.port = snd_seq_create_simple_port(handle, NULL,
					 SND_SEQ_PORT_CAP_DUPLEX,
					 SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if( cap == 'e' )
		source.port = snd_seq_create_simple_port(handle, NULL,
					 SND_SEQ_PORT_CAP_WRITE |
					 SND_SEQ_PORT_CAP_SUBS_WRITE |
					 SND_SEQ_PORT_CAP_READ,
					 SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if (source.port < 0) {
		fprintf(stderr,"Error creating port %s\n", snd_strerror(errno));
		return -1;
	}
	port_count++;

	if ( verbose ) 
		fprintf(stderr,"Created new port :%d with mode %c\n", source.port, cap);

	return source.port;
}

void SeqContext::seq_destroy_port(int port)
{
}

int SeqContext::seq_connect_add( char *clientport )
{
	int  			ret;
	snd_seq_addr_t	*addr;
	
	addr = parseclientport( clientport );

	if( verbose ) 
		fprintf(stderr,"Adding clientport = %s; ", clientport);
		
	destlist->append( addr );

	ret = snd_seq_connect_to(handle, source.port, addr->client, addr->port);

	if( ret < 0 ) 
		fprintf(stderr,"Error connecting to client at %d:%d\n", addr->client, addr->port);
	else 
		if( verbose ) 
			fprintf(stderr,"connected to client %d:%d\n", addr->client, addr->port);
		
	return ret;
}

/*
 * Set the initial time base and tempo. 
 *    resolution - Ticks per quarter note or realtime resolution
 *    tempo     - Beats per minute
 *    realtime  - True if absolute time base
 */
int SeqContext::seq_init_tempo(int resolution, int tempo, int realtime)
{
	snd_seq_queue_tempo_t *qtempo;
	int  ret;

	snd_seq_queue_tempo_alloca(&qtempo);
	memset(qtempo, 0, snd_seq_queue_tempo_sizeof());
	snd_seq_queue_tempo_set_ppq(qtempo, resolution);
	snd_seq_queue_tempo_set_tempo(qtempo, 60*1000000/tempo);

	ret = snd_seq_set_queue_tempo(handle, queue, qtempo);

	if( verbose ) 
		fprintf(stderr,"seq queue tempo: res=%d, tempo=%d, realtime=%d\n", resolution, tempo, realtime );

	return ret;
}

/*
 * Send the event to the specified client and port.
 * 
 *  Arguments:
 *    ctxp      - Client context
 *    ev        - Event to send
 *    client    - Client to send the event to
 *    port      - Port to send the event to
 */
int SeqContext::seq_sendto(snd_seq_event_t *ev, int client, int port)
{
	ev->source = source;
	ev->queue = queue;
	ev->dest.client = client;
	ev->dest.port = port;
	seq_write(ev);

	return 0;
}

/*
 * seq_send_to_all:
 * Send the event to all the connected devices across all
 * possible channels. The messages are sent to the blocking
 * control port and so should not have timestamps in the
 * future. This function is intended for all-sounds-off type
 * controls etc.
 *  
 *  Arguments:
 * 	ctxp: Client context
 * 	ep: Event prototype to be sent
 */
int SeqContext::seq_send_to_all(snd_seq_event_t *ep)
{
	int  dev;
	int  chan;
	snd_seq_addr_t *addr;

	for (dev = 0; ; dev++) {
		addr = seq_dev_addr(dev);
		if (addr == NULL)
			break; /* No more */

		ep->queue = queue;
		ep->dest = *addr;
		for (chan = 0; chan < 16; chan++) {
			set_channel(ep, chan);
			(void) seq_write( ep);
		}
	}

	return 0;
}

/*
 * seq_dev_addr:
 * Return the address corresponding to the specified logical
 * device.
 */
snd_seq_addr_t *SeqContext::seq_dev_addr(int dev)
{
	snd_seq_addr_t *addr;

	if ( dev >= destlist->length() )
		return NULL;

	addr = destlist->index( dev );

	return addr;
}

/*
 * Start the queue
 */
void SeqContext::seq_start_queue()
{
#ifdef USE_DRAIN
	snd_seq_drain_output(handle);
#else
	snd_seq_flush_output(handle);
#endif

	snd_seq_start_queue(handle, queue, 0);

	if( verbose ) fprintf(stderr, "Queue started\n");

}

/*
 * Stop the queue
 */
void SeqContext::seq_stop_queue()
{
	snd_seq_stop_queue(handle, queue, 0);

	if( verbose ) fprintf(stderr, "Queue stopped\n");

}

/*
 * Write out the event. This routine blocks until
 * successfully written. 
 * 
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event
 */
int SeqContext::seq_write(snd_seq_event_t *ep)
{
	int  err = 0;

	err = snd_seq_event_output(handle, ep);

	if( err < 0 ) fprintf(stderr, "Error: SeqEvent output failure, err=%d\n", err);
	else
		if( verbose ) fprintf(stderr, " SeqEvent outputted, cnt=%d\n", err);

	return err;
}

/*
 * Return the handle to the underlying snd_seq stream.
 *  Arguments:
 *    ctxp      - Application context
 */
snd_seq_t *SeqContext::seq_handle()
{
 	return handle;
}

void SeqContext::set_channel(snd_seq_event_t *ep, int chan)
{

	if( verbose ) 
		fprintf(stderr, "set channel: event type=%d; channel = %d\n", ep->type, chan);

	switch (ep->type) {
	case SND_SEQ_EVENT_NOTE:
	case SND_SEQ_EVENT_NOTEON:
	case SND_SEQ_EVENT_NOTEOFF:
		ep->data.note.channel = chan;
		break;
	case SND_SEQ_EVENT_KEYPRESS:
	case SND_SEQ_EVENT_PGMCHANGE:
	case SND_SEQ_EVENT_CHANPRESS:
	case SND_SEQ_EVENT_PITCHBEND:
	case SND_SEQ_EVENT_CONTROL14:
	case SND_SEQ_EVENT_NONREGPARAM:
	case SND_SEQ_EVENT_REGPARAM:
	case SND_SEQ_EVENT_CONTROLLER:
		ep->data.control.channel = chan;
		break;
	default:
		if (snd_seq_ev_is_channel_type(ep))
			fprintf(stderr,"Missed a case in set_channel");
		break;
	}
}
/*
 * Initialize a midi event from the context. The source and
 * destination addresses will be set from the information
 * stored in the context, which depends on the previous
 * connection calls. In addition the time and channel are set.
 * This should be called first before any of the following
 * functions.
 * 
 *  Arguments:
 *    ctxp      - Client application
 *    ep        - Event to init
 *    time      - Midi time
 *    devchan   - Midi channel
 */
void 
SeqContext::seq_midi_event_init(snd_seq_event_t *ep, 
        unsigned long time, int devchan)
{
	int  dev;

	/* our client index is encoded in upper bits of devchan	 */
	dev = devchan >> 4;

	/*
	 * If insufficient output devices have been registered, then we
	 * just scale the device back to fit in the correct range.  This
	 * is not necessarily what you want.
	 */
	if (dev >= destlist->length())
		dev = dev % destlist->length();

	if( verbose ) 
		fprintf(stderr, "init midi event: time=%d source=%d:%d;", 
			(int)time, source.client, source.port);
	
	snd_seq_ev_clear(ep);
	snd_seq_ev_schedule_tick(ep, queue, 0, time);
	ep->source = source;
	if (destlist->length() > 0) {
		ep->dest = *(destlist->index(dev));
		if( verbose ) 
			fprintf(stderr, "dev=%d; destination=%d:%d\n", dev, ep->dest.client, ep->dest.port);
	}
}

/*
 * Send a note event.
 *  Arguments:
 *    ctxp      - Client context
 *    ep        - Event template
 *    note      - Pitch of note
 *    vel       - Velocity of note
 *    length    - Length of note
 */
void 
SeqContext::seq_midi_note(snd_seq_event_t *ep, int devchan, int note, int vel, 
        int length)
{
	ep->type = SND_SEQ_EVENT_NOTE;

	ep->data.note.channel = devchan & 0xf;
	ep->data.note.note = note;
	ep->data.note.velocity = vel;
	ep->data.note.duration = length;

	if( verbose ) 
		fprintf(stderr, "midi note: note=%d velocity=%d length=%d", note, vel, length );

	seq_write(ep);
}

/*
 * Send a note on event.
 *  Arguments:
 *    ctxp      - Client context
 *    ep        - Event template
 *    note      - Pitch of note
 *    vel       - Velocity of note
 *    length    - Length of note
 */
void 
SeqContext::seq_midi_note_on(snd_seq_event_t *ep, int devchan, int note, int vel, 
        int length)
{
	ep->type = SND_SEQ_EVENT_NOTEON;

	ep->data.note.channel = devchan & 0xf;
	ep->data.note.note = note;
	ep->data.note.velocity = vel;
	ep->data.note.duration = length;

	if( verbose ) 
		fprintf(stderr, "midi note on: note=%d velocity=%d length=%d", note, vel, length );

	seq_write(ep);
}

/*
 * Send a note off event.
 *  Arguments:
 *    ctxp      - Client context
 *    ep        - Event template
 *    note      - Pitch of note
 *    vel       - Velocity of note
 *    length    - Length of note
 */
void 
SeqContext::seq_midi_note_off(snd_seq_event_t *ep, int devchan, int note, int vel, 
        int length)
{
	ep->type = SND_SEQ_EVENT_NOTEOFF;

	ep->data.note.channel = devchan & 0xf;
	ep->data.note.note = note;
	ep->data.note.velocity = vel;
	ep->data.note.duration = length;

	if( verbose ) 
		fprintf(stderr, "midi note off: note=%d velocity=%d length=%d", note, vel, length );

	seq_write(ep);
}

/*
 * Send a key pressure event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    note      - Note to be altered
 *    value     - Pressure value
 */
void 
SeqContext::seq_midi_keypress(snd_seq_event_t *ep, int devchan, int note, 
        int value)
{
	ep->type = SND_SEQ_EVENT_KEYPRESS;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.param = note;
	ep->data.control.value = value;
	seq_write(ep);
}

/*
 * Send a control event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    control   - Controller to change
 *    value     - Value to set it to
 */
void 
SeqContext::seq_midi_control(snd_seq_event_t *ep, int devchan, int control, 
        int value)
{
	ep->type = SND_SEQ_EVENT_CONTROLLER;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.param = control;
	ep->data.control.value = value;
	seq_write(ep);
}

/*
 * Send a program change event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    program   - Program to set
 */
void 
SeqContext::seq_midi_program(snd_seq_event_t *ep, int devchan, int program)
{
	ep->type = SND_SEQ_EVENT_PGMCHANGE;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.value = program;
	seq_write(ep);
}

/*
 * Send a channel pressure event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    pressure  - Pressure value
 */
void 
SeqContext::seq_midi_chanpress(snd_seq_event_t *ep, int devchan, int pressure)
{
	ep->type = SND_SEQ_EVENT_CHANPRESS;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.value = pressure;
	seq_write(ep);
}

/*
 * Send a pitchbend message. The bend parameter is centered on
 * zero, negative values mean a lower pitch.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    bend      - Bend value, centered on zero.
 */
void 
SeqContext::seq_midi_pitchbend(snd_seq_event_t *ep, int devchan, int bend)
{
	ep->type = SND_SEQ_EVENT_PITCHBEND;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.value = bend;
	seq_write(ep);
}

/*
 * Send a tempo event. The tempo parameter is in microseconds
 * per beat.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    tempo     - New tempo in usec per beat
 */
void 
SeqContext::seq_midi_tempo(snd_seq_event_t *ep, int tempo)
{
	ep->type = SND_SEQ_EVENT_TEMPO;

	ep->data.queue.queue = queue;
	ep->data.queue.param.value = tempo;
	ep->dest.client = SND_SEQ_CLIENT_SYSTEM;
	ep->dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
	seq_write(ep);
}

/*
 * Send a sysex event. The status byte is to distiguish
 * continuation sysex messages.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    status    - Status byte for sysex
 *    data      - Data to send
 *    length    - Length of data
 */
void 
SeqContext::seq_midi_sysex(snd_seq_event_t *ep, int status, 
        unsigned char *data, int length)
{
	unsigned char *ndata;
	int  nlen;

	ep->type = SND_SEQ_EVENT_SYSEX;

	ndata = new unsigned char[length + 1];
	nlen = length +1;

	ndata[0] = status;
	memcpy(ndata+1, data, length);

	snd_seq_ev_set_variable(ep, nlen, ndata);

	seq_write(ep);

	delete ndata;
}

/*
 * Send an echo event back to the source client at the specified
 * time.
 * 
 *  Arguments:
 *    ctxp      - Application context
 *    time      - Time of event
 */
void 
SeqContext::seq_midi_echo(unsigned long time)
{
	snd_seq_event_t ev;

	seq_midi_event_init(&ev, time, 0);
	/* Loop back */
	ev.dest = source;
	seq_write(&ev);
}
