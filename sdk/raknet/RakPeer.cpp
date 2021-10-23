/// \file
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#include "RakNetDefines.h"
#include "RakPeer.h"
#include "NetworkTypes.h"

#ifdef _WIN32
//#include <Shlwapi.h>
#include <process.h>
#else
#define closesocket close
#include <unistd.h>
#include <pthread.h>
#endif
#include <ctype.h> // toupper
#include <string.h>
#include "GetTime.h"
#include "PacketEnumerations.h"
#include "DS_HuffmanEncodingTree.h"
#include "Rand.h"
#include "PluginInterface.h"
#include "StringCompressor.h"
#include "StringTable.h"
#include "NetworkIDGenerator.h"
#include "NetworkTypes.h"
#include "SHA1.h"
#include "RakSleep.h"
#include "RouterInterface.h"
#include "RakAssert.h"

#ifdef _WIN32
#include <windows.h>
#else
#define OutputDebugString NULL
#endif

#if !defined ( __APPLE__ ) && !defined ( __APPLE_CC__ )
#include <malloc.h>
#endif

#ifdef _COMPATIBILITY_1
//
#elif defined(_WIN32)
//
#elif defined(_COMPATIBILITY_2)
#include "Compatibility2Includes.h"
#include <stdlib.h>
#else
#include <stdlib.h>
#endif

#ifdef _MSC_VER
#pragma warning( push )
#endif

#ifdef _WIN32
unsigned __stdcall UpdateNetworkLoop( LPVOID arguments );
#else
void* UpdateNetworkLoop( void* arguments );
#endif

int PlayerIDAndIndexComp( const PlayerID &key, const PlayerIDAndIndex &data )
{
	if (key < data.playerId)
		return -1;
	if (key==data.playerId)
		return 0;
	return 1;
}

// On a Little-endian machine the RSA key and message are mangled, but we're
// trying to be friendly to the little endians, so we do byte order
// mangling on Big-Endian machines.  Note that this mangling is independent
// of the byte order used on the network (which also defaults to little-end).
#ifdef HOST_ENDIAN_IS_BIG
	void __inline BSWAPCPY(unsigned char *dest, unsigned char *source, int bytesize)
	{
	#ifdef _DEBUG
		assert( (bytesize % 4 == 0)&&(bytesize)&& "Something is wrong with your exponent or modulus size.");
	#endif
		int i;
		for (i=0; i<bytesize; i+=4)
		{
			dest[i] = source[i+3];
			dest[i+1] = source[i+2];
			dest[i+2] = source[i+1];
			dest[i+3] = source[i];
		}
	}
	void __inline BSWAPSELF(unsigned char *source, int bytesize)
	{
	#ifdef _DEBUG
		assert( (bytesize % 4 == 0)&&(bytesize)&& "Something is wrong with your exponent or modulus size.");
	#endif
		int i;
		unsigned char a, b;
		for (i=0; i<bytesize; i+=4)
		{
			a = source[i];
			b = source[i+1];
			source[i] = source[i+3];
			source[i+1] = source[i+2];
			source[i+2] = b;
			source[i+3] = a;
		}
	}
#endif

#ifdef SAMPSRV
	void logprintf(char* format, ...);
#else
	unsigned int uiIncomeCookieExchange;
#endif

static const unsigned int COOKIE_XOR_KEY = 0x6969; // Nice!
static const unsigned int SYN_COOKIE_OLD_RANDOM_NUMBER_DURATION = 5000;
static const int MAX_OFFLINE_DATA_LENGTH=400; // I set this because I limit ID_CONNECTION_REQUEST to 512 bytes, and the password is appended to that packet.

//#define _DO_PRINTF

// UPDATE_THREAD_POLL_TIME is how often the update thread will poll to see
// if receive wasn't called within UPDATE_THREAD_UPDATE_TIME.  If it wasn't called within that time,
// the updating thread will activate and take over network communication until Receive is called again.
//static const unsigned int UPDATE_THREAD_UPDATE_TIME=30;
//static const unsigned int UPDATE_THREAD_POLL_TIME=30;

//#define _TEST_AES

Packet *AllocPacket(unsigned dataSize)
{
	Packet *p = (Packet *)malloc(sizeof(Packet)+dataSize);
	p->data=(unsigned char*)p+sizeof(Packet);
	p->length=dataSize;
	p->deleteData=false;
	return p;
}

Packet *AllocPacket(unsigned dataSize, unsigned char *data)
{
	Packet *p = (Packet *)malloc(sizeof(Packet));
	p->data=data;
	p->length=dataSize;
	p->deleteData=true;
	return p;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakPeer::RakPeer()
{
	StringCompressor::AddReference();
	StringTable::AddReference();

#if !defined(_COMPATIBILITY_1)
	usingSecurity = false;
#endif
	memset( frequencyTable, 0, sizeof( unsigned int ) * 256 );
	rawBytesSent = rawBytesReceived = compressedBytesSent = compressedBytesReceived = 0;
	outputTree = inputTree = 0;
	connectionSocket = INVALID_SOCKET;
	MTUSize = DEFAULT_MTU_SIZE;
	trackFrequencyTable = false;
	maximumIncomingConnections = 0;
	maximumNumberOfPeers = 0;
	//remoteSystemListSize=0;
	remoteSystemList = 0;
	bytesSentPerSecond = bytesReceivedPerSecond = 0;
	endThreads = true;
	isMainLoopThreadActive = false;
	// isRecvfromThreadActive=false;
	occasionalPing = false;
	connectionSocket = INVALID_SOCKET;
	myPlayerId = UNASSIGNED_PLAYER_ID;
	allowConnectionResponseIPMigration = false;
	blockOnRPCReply=false;
	//incomingPasswordLength=outgoingPasswordLength=0;
	incomingPasswordLength=0;
	router=0;
	splitMessageProgressInterval=0;
	unreliableTimeout=0;

#if defined (_WIN32) && defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
	recvEvent = INVALID_HANDLE_VALUE;
#endif

#ifndef _RELEASE
	_maxSendBPS=0.0;
	_minExtraPing=0;
	_extraPingVariance=0;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakPeer::~RakPeer()
{
//	unsigned i;

	// Free the ban list.
	ClearBanList();

	Disconnect( 0, 0);


	StringCompressor::RemoveReference();
	StringTable::RemoveReference();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Starts the network threads, opens the listen port
// You must call this before calling SetMaximumIncomingConnections or Connect
// Multiple calls while already active are ignored.  To call this function again with different settings, you must first call Disconnect()
//
// Parameters:
// maxConnections:  Required so the network can preallocate and for thread safety.
// - A pure client would set this to 1.  A pure server would set it to the number of allowed clients.
// - A hybrid would set it to the sum of both types of connections
// localPort: The port to listen for connections on.
// _threadSleepTimer: How many ms to Sleep each internal update cycle (30 to give the game priority, 0 for regular (recommended), -1 to not Sleep() (may be slower))
 // forceHostAddress Can force RakNet to use a particular IP to host on.  Pass 0 to automatically pick an IP
// Returns:
// False on failure (can't create socket or thread), true on success.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::Initialize( unsigned short maxConnections, unsigned short localPort, int _threadSleepTimer, const char *forceHostAddress )
{
	if (IsActive())
		return false;

	unsigned i;

	assert( maxConnections > 0 );

	if ( maxConnections <= 0 )
		return false;

	if ( connectionSocket == INVALID_SOCKET )
	{
		connectionSocket = SocketLayer::Instance()->CreateBoundSocket( localPort, true, forceHostAddress );

		if ( connectionSocket == INVALID_SOCKET )
			return false;

		unsigned short localPort2 = SocketLayer::Instance()->GetLocalPort(connectionSocket);
		if (localPort2!=0)
			localPort=localPort2;
	}

#if defined (_WIN32) && defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
	if (_threadSleepTimer>0)
	{
		recvEvent=CreateEvent(0,FALSE,FALSE,0);
		WSAEventSelect(connectionSocket,recvEvent,FD_READ);
	}	
#endif

	if ( maximumNumberOfPeers == 0 )
	{
		// Don't allow more incoming connections than we have peers.
		if ( maximumIncomingConnections > maxConnections )
			maximumIncomingConnections = maxConnections;

		maximumNumberOfPeers = maxConnections;
		// 04/19/2006 - Don't overallocate because I'm not longer allowing connected pings.
		// The disconnects are not consistently processed and the process was sloppy and complicated.
		// Allocate 10% extra to handle new connections from players trying to connect when the server is full
		//remoteSystemListSize = maxConnections;// * 11 / 10 + 1;

		// remoteSystemList in Single thread
		//remoteSystemList = new RemoteSystemStruct[ remoteSystemListSize ];
		remoteSystemList = new RemoteSystemStruct[ maximumNumberOfPeers ];


		for ( i = 0; i < maximumNumberOfPeers; i++ )
		//for ( i = 0; i < remoteSystemListSize; i++ )
		{
			// remoteSystemList in Single thread
			remoteSystemList[ i ].isActive = false;
			#ifndef _RELEASE
			remoteSystemList[ i ].reliabilityLayer.ApplyNetworkSimulator(_maxSendBPS, _minExtraPing, _extraPingVariance);
			#endif
		}

		// Clear the lookup table.  Safe to call from the user thread since the network thread is now stopped
		remoteSystemLookup.Clear();
	}

	// For histogram statistics
	// nextReadBytesTime=0;
	// lastSentBytes=lastReceivedBytes=0;

	if ( endThreads )
	{
	//	lastUserUpdateCycle = 0;

		// Reset the frequency table that we use to save outgoing data
		memset( frequencyTable, 0, sizeof( unsigned int ) * 256 );

		// Reset the statistical data
		rawBytesSent = rawBytesReceived = compressedBytesSent = compressedBytesReceived = 0;

		updateCycleIsRunning = false;
		endThreads = false;
		// Create the threads
		threadSleepTimer = _threadSleepTimer;

		ClearBufferedCommands();

#if !defined(_COMPATIBILITY_1)
		char ipList[ 10 ][ 16 ];
		SocketLayer::Instance()->GetMyIP( ipList );
		myPlayerId.port = localPort;
		if (forceHostAddress==0 || forceHostAddress[0]==0)
			myPlayerId.binaryAddress = inet_addr( ipList[ 0 ] );
		else
			myPlayerId.binaryAddress = inet_addr( forceHostAddress );
#else
		myPlayerId=UNASSIGNED_PLAYER_ID;
#endif
		{
#ifdef _WIN32

			if ( isMainLoopThreadActive == false )
			{
				unsigned ProcessPacketsThreadID = 0;
#ifdef _COMPATIBILITY_1
				processPacketsThreadHandle = ( HANDLE ) _beginthreadex( NULL, 0, UpdateNetworkLoop, this, 0, &ProcessPacketsThreadID );
#else
				processPacketsThreadHandle = ( HANDLE ) _beginthreadex( NULL, MAX_ALLOCA_STACK_ALLOCATION*2, UpdateNetworkLoop, this, 0, &ProcessPacketsThreadID );
#endif

				//BOOL b =  SetThreadPriority(
				//	processPacketsThreadHandle,
				//	THREAD_PRIORITY_HIGHEST
				//	);

				if ( processPacketsThreadHandle == 0 )
				{
					Disconnect( 0, 0 );
					return false;
				}

				// SetThreadPriority(processPacketsThreadHandle, THREAD_PRIORITY_HIGHEST);

				CloseHandle( processPacketsThreadHandle );

				processPacketsThreadHandle = 0;

			}

#else
			pthread_attr_t attr;
			pthread_attr_init( &attr );
			pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );

			//  sched_param sp;
			//  sp.sched_priority = sched_get_priority_max(SCHED_OTHER);
			//  pthread_attr_setschedparam(&attr, &sp);

			int error;

			if ( isMainLoopThreadActive == false )
			{
				error = pthread_create( &processPacketsThreadHandle, &attr, &UpdateNetworkLoop, this );

				if ( error )
				{
					Disconnect( 0 );
					return false;
				}
			}

			processPacketsThreadHandle = 0;
#endif


			// Wait for the threads to activate.  When they are active they will set these variables to true

			while (  /*isRecvfromThreadActive==false || */isMainLoopThreadActive == false )
				RakSleep(10);

		}
	}

	for (i=0; i < messageHandlerList.Size(); i++)
	{
		messageHandlerList[i]->OnInitialize(this);
	}

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Must be called while offline
// Secures connections though a combination of SHA1, AES128, SYN Cookies, and RSA to prevent
// connection spoofing, replay attacks, data eavesdropping, packet tampering, and MitM attacks.
// There is a significant amount of processing and a slight amount of bandwidth
// overhead for this feature.
//
// If you accept connections, you must call this or else secure connections will not be enabled
// for incoming connections.
// If you are connecting to another system, you can call this with values for the
// (e and p,q) public keys before connecting to prevent MitM
//
// Parameters:
// pubKeyE, pubKeyN - A pointer to the public keys from the RSACrypt class. See the Encryption sample
// privKeyP, privKeyQ - Private keys generated from the RSACrypt class.  See the Encryption sample
// If the private keys are 0, then a new key will be generated when this function is called
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::InitializeSecurity(const char *pubKeyE, const char *pubKeyN, const char *privKeyP, const char *privKeyQ )
{
#if !defined(_COMPATIBILITY_1)
	if ( endThreads == false )
		return ;

	// Setting the client key is e,n,
	// Setting the server key is p,q
	if ( //( privKeyP && privKeyQ && ( pubKeyE || pubKeyN ) ) ||
		//( pubKeyE && pubKeyN && ( privKeyP || privKeyQ ) ) ||
		( privKeyP && privKeyQ == 0 ) ||
		( privKeyQ && privKeyP == 0 ) ||
		( pubKeyE && pubKeyN == 0 ) ||
		( pubKeyN && pubKeyE == 0 ) )
	{
		// Invalid parameters
		assert( 0 );
	}

	seedMT( (unsigned int) RakNet::GetTime() );

	GenerateSYNCookieRandomNumber();

	usingSecurity = true;

	if ( privKeyP == 0 && privKeyQ == 0 && pubKeyE == 0 && pubKeyN == 0 )
	{
		keysLocallyGenerated = true;
		rsacrypt.generateKeys();
	}

	else
	{
		if ( pubKeyE && pubKeyN )
		{
			// Save public keys
			memcpy( ( char* ) & publicKeyE, pubKeyE, sizeof( publicKeyE ) );
			memcpy( publicKeyN, pubKeyN, sizeof( publicKeyN ) );
		}

		if ( privKeyP && privKeyQ )
		{
			BIGHALFSIZE( RSA_BIT_SIZE, p );
			BIGHALFSIZE( RSA_BIT_SIZE, q );
			memcpy( p, privKeyP, sizeof( p ) );
			memcpy( q, privKeyQ, sizeof( q ) );
			// Save private keys
			rsacrypt.setPrivateKey( p, q );
		}

		keysLocallyGenerated = false;
	}
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description
// Must be called while offline
// Disables all security.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::DisableSecurity( void )
{
#if !defined(_COMPATIBILITY_1)
	if ( endThreads == false )
		return ;

	usingSecurity = false;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sets how many incoming connections are allowed.  If this is less than the number of players currently connected, no
// more players will be allowed to connect.  If this is greater than the maximum number of peers allowed, it will be reduced
// to the maximum number of peers allowed.  Defaults to 0.
//
// Parameters:
// numberAllowed - Maximum number of incoming connections allowed.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetMaximumIncomingConnections( unsigned short numberAllowed )
{
	maximumIncomingConnections = numberAllowed;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the maximum number of incoming connections, which is always <= maxConnections
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned short RakPeer::GetMaximumIncomingConnections( void ) const
{
	return maximumIncomingConnections;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sets the password incoming connections must match in the call to Connect (defaults to none)
// Pass 0 to passwordData to specify no password
//
// Parameters:
// passwordData: A data block that incoming connections must match.  This can be just a password, or can be a stream of data.
// - Specify 0 for no password data
// passwordDataLength: The length in bytes of passwordData
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetIncomingPassword( const char* passwordData, int passwordDataLength )
{
	//if (passwordDataLength > MAX_OFFLINE_DATA_LENGTH)
	//	passwordDataLength=MAX_OFFLINE_DATA_LENGTH;

	if (passwordDataLength > 255)
		passwordDataLength=255;

	if (passwordData==0)
		passwordDataLength=0;

	// Not threadsafe but it's not important enough to lock.  Who is going to change the password a lot during runtime?
	// It won't overflow at least because incomingPasswordLength is an unsigned char
	if (passwordDataLength>0)
		memcpy(incomingPassword, passwordData, passwordDataLength);
	incomingPasswordLength=(unsigned char)passwordDataLength;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::GetIncomingPassword( char* passwordData, int *passwordDataLength  )
{
	if (passwordData==0)
	{
		*passwordDataLength=incomingPasswordLength;
		return;
	}

	if (*passwordDataLength > incomingPasswordLength)
		*passwordDataLength=incomingPasswordLength;

	if (*passwordDataLength>0)
		memcpy(passwordData, incomingPassword, *passwordDataLength);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Call this to connect to the specified host (ip or domain name) and server port.
// Calling Connect and not calling SetMaximumIncomingConnections acts as a dedicated client.  Calling both acts as a true peer.
// This is a non-blocking connection.  You know the connection is successful when IsConnected() returns true
// or receive gets a packet with the type identifier ID_CONNECTION_ACCEPTED.  If the connection is not
// successful, such as rejected connection or no response then neither of these things will happen.
// Requires that you first call Initialize
//
// Parameters:
// host: Either a dotted IP address or a domain name
// remotePort: Which port to connect to on the remote machine.
// passwordData: A data block that must match the data block on the server.  This can be just a password, or can be a stream of data
// passwordDataLength: The length in bytes of passwordData
//
// Returns:
// True on successful initiation. False on incorrect parameters, internal error, or too many existing peers
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::Connect( const char* host, unsigned short remotePort, char* passwordData, int passwordDataLength )
{
	// If endThreads is true here you didn't call Initialize() first.
	if ( host == 0 || endThreads || connectionSocket == INVALID_SOCKET )
		return false;

	unsigned numberOfFreeSlots;

	numberOfFreeSlots = 0;

	//if (passwordDataLength>MAX_OFFLINE_DATA_LENGTH)
	//	passwordDataLength=MAX_OFFLINE_DATA_LENGTH;
	if (passwordDataLength>255)
		passwordDataLength=255;

	if (passwordData==0)
		passwordDataLength=0;

	// Not threadsafe but it's not important enough to lock.  Who is going to change the password a lot during runtime?
	// It won't overflow at least because outgoingPasswordLength is an unsigned char
//	if (passwordDataLength>0)
//		memcpy(outgoingPassword, passwordData, passwordDataLength);
//	outgoingPasswordLength=(unsigned char) passwordDataLength;

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return false;
#endif
		if (host==0)
			return false;
	}

	// Connecting to ourselves in the same instance of the program?
	if ( ( strcmp( host, "127.0.0.1" ) == 0 || strcmp( host, "0.0.0.0" ) == 0 ) && remotePort == myPlayerId.port )
		return false;

	return SendConnectionRequest( host, remotePort, passwordData, passwordDataLength );
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Stops the network threads and close all connections.  Multiple calls are ok.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::Disconnect( unsigned int blockDuration, unsigned char orderingChannel )
{
	unsigned i,j;
	bool anyActive;
	RakNet::Time startWaitingTime, time;
//	PlayerID playerId;
	//unsigned short systemListSize = remoteSystemListSize; // This is done for threading reasons
	unsigned short systemListSize = maximumNumberOfPeers;

	if ( blockDuration > 0 )
	{
		for ( i = 0; i < systemListSize; i++ )
		{
			// remoteSystemList in user thread
			NotifyAndFlagForDisconnect(remoteSystemList[i].playerId, false, orderingChannel);
		}

		time = RakNet::GetTime();
		startWaitingTime = time;
		while ( time - startWaitingTime < blockDuration )
		{
			anyActive=false;
			for (j=0; j < systemListSize; j++)
			{
				// remoteSystemList in user thread
				if (remoteSystemList[j].isActive)
				{
					anyActive=true;
					break;
				}
			}

			// If this system is out of packets to send, then stop waiting
			if ( anyActive==false )
				break;

			// This will probably cause the update thread to run which will probably
			// send the disconnection notification

			RakSleep(15);
			time = RakNet::GetTime();
		}
	}

	for (i=0; i < messageHandlerList.Size(); i++)
	{
		messageHandlerList[i]->OnDisconnect(this);
	}

	if ( endThreads == false )
	{
		// Stop the threads
		endThreads = true;
	}

	while ( isMainLoopThreadActive )
		RakSleep(15);

	// remoteSystemList in Single thread
	for ( i = 0; i < systemListSize; i++ )
	{
		// Reserve this reliability layer for ourselves
		remoteSystemList[ i ].isActive = false;

		// Remove any remaining packets
		remoteSystemList[ i ].reliabilityLayer.Reset(false);
	}

	// Clear the lookup table.  Safe to call from the user thread since the network thread is now stopped
	remoteSystemLookup.Clear();

	// Setting maximumNumberOfPeers to 0 allows remoteSystemList to be reallocated in Initialize.
	// Setting remoteSystemListSize prevents threads from accessing the reliability layer
	maximumNumberOfPeers = 0;
	//remoteSystemListSize = 0;

	// Free any packets the user didn't deallocate
	Packet **packet;
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[transferToPacketQueue_Mutex].Lock();
#endif
	packet=packetSingleProducerConsumer.ReadLock();
	while (packet)
	{
		DeallocatePacket(*packet);
		packetSingleProducerConsumer.ReadUnlock();
		packet=packetSingleProducerConsumer.ReadLock();
	}
	packetSingleProducerConsumer.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[transferToPacketQueue_Mutex].Unlock();
#endif

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Lock();
#endif
	for (i=0; i < packetPool.Size(); i++)
		DeallocatePacket(packetPool[i]);
	packetPool.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Unlock();
#endif

	blockOnRPCReply=false;

	if ( connectionSocket != INVALID_SOCKET )
	{
		closesocket( connectionSocket );
		connectionSocket = INVALID_SOCKET;
	}

	ClearBufferedCommands();
	bytesSentPerSecond = bytesReceivedPerSecond = 0;

	ClearRequestedConnectionList();

#if defined (_WIN32) && defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
	if (recvEvent!=INVALID_HANDLE_VALUE)
	{
		CloseHandle( recvEvent );
		recvEvent = INVALID_HANDLE_VALUE;
	}	
#endif

	// Clear out the reliability layer list in case we want to reallocate it in a successive call to Init.
	RemoteSystemStruct * temp = remoteSystemList;
	remoteSystemList = 0;
	delete [] temp;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns true if the network threads are running
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
inline bool RakPeer::IsActive( void ) const
{
	return endThreads == false;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Fills the array remoteSystems with the playerID of all the systems we are connected to
//
// Parameters:
// remoteSystems (out): An array of PlayerID structures to be filled with the PlayerIDs of the systems we are connected to
// - pass 0 to remoteSystems to only get the number of systems we are connected to
// numberOfSystems (int, out): As input, the size of remoteSystems array.  As output, the number of elements put into the array
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::GetConnectionList( PlayerID *remoteSystems, unsigned short *numberOfSystems ) const
{
	int count, index;
	count=0;

	if ( remoteSystemList == 0 || endThreads == true )
	{
		*numberOfSystems = 0;
		return false;
	}

	// This is called a lot so I unrolled the loop
	if ( remoteSystems )
	{
		// remoteSystemList in user thread
		//for ( count = 0, index = 0; index < remoteSystemListSize; ++index )
		for ( count = 0, index = 0; index < maximumNumberOfPeers; ++index )
			if ( remoteSystemList[ index ].isActive && remoteSystemList[ index ].connectMode==RemoteSystemStruct::CONNECTED)
			{
				if ( count < *numberOfSystems )
					remoteSystems[ count ] = remoteSystemList[ index ].playerId;

				++count;
			}
	}
	else
	{
		// remoteSystemList in user thread
		//for ( count = 0, index = 0; index < remoteSystemListSize; ++index )
		for ( count = 0, index = 0; index < maximumNumberOfPeers; ++index )
			if ( remoteSystemList[ index ].isActive && remoteSystemList[ index ].connectMode==RemoteSystemStruct::CONNECTED)
				++count;
	}

	*numberOfSystems = ( unsigned short ) count;

	return 0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sends a block of data to the specified system that you are connected to.
// This function only works while the client is connected (Use the Connect function).
//
// Parameters:
// data: The block of data to send
// length: The size in bytes of the data to send
// bitStream: The bitstream to send
// priority: What priority level to send on.
// reliability: How reliability to send this data
// orderingChannel: When using ordered or sequenced packets, what channel to order these on.
// - Packets are only ordered relative to other packets on the same stream
// playerId: Who to send this packet to, or in the case of broadcasting who not to send it to. Use UNASSIGNED_PLAYER_ID to specify none
// broadcast: True to send this packet to all connected systems.  If true, then playerId specifies who not to send the packet to.
// Returns:
// False if we are not connected to the specified recipient.  True otherwise
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )
{
#ifdef _DEBUG
	assert( data && length > 0 );
#endif

	if ( data == 0 || length < 0 )
		return false;

	if ( remoteSystemList == 0 || endThreads == true )
		return false;

	if ( broadcast == false && playerId == UNASSIGNED_PLAYER_ID )
		return false;

	if (broadcast==false && router && GetIndexFromPlayerID(playerId)==-1)
	{
		return router->Send(data, BYTES_TO_BITS(length), priority, reliability, orderingChannel, playerId);
	}
	else
	{
		SendBuffered(data, length*8, priority, reliability, orderingChannel, playerId, broadcast, RemoteSystemStruct::NO_ACTION);
	}

	return true;
}

bool RakPeer::Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast )
{
#ifdef _DEBUG
	assert( bitStream->GetNumberOfBytesUsed() > 0 );
#endif

	if ( bitStream->GetNumberOfBytesUsed() == 0 )
		return false;

	if ( remoteSystemList == 0 || endThreads == true )
		return false;

	if ( broadcast == false && playerId == UNASSIGNED_PLAYER_ID )
		return false;

	if (broadcast==false && router && GetIndexFromPlayerID(playerId)==-1)
	{
		return router->Send((const char*)bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), priority, reliability, orderingChannel, playerId);
	}
	else
	{
		// Sends need to be buffered and processed in the update thread because the playerID associated with the reliability layer can change,
		// from that thread, resulting in a send to the wrong player!  While I could mutex the playerID, that is much slower than doing this
		SendBuffered((const char*)bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), priority, reliability, orderingChannel, playerId, broadcast, RemoteSystemStruct::NO_ACTION);
	}
	
	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Gets a packet from the incoming packet queue. Use DeallocatePacket to deallocate the packet after you are done with it.  Packets must be deallocated in the same order they are received.
// Check the Packet struct at the top of CoreNetworkStructures.h for the format of the struct
//
// Returns:
// 0 if no packets are waiting to be handled, otherwise an allocated packet
// If the client is not active this will also return 0, as all waiting packets are flushed when the client is Disconnected
// This also updates all memory blocks associated with synchronized memory and distributed objects
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Packet* RakPeer::Receive( void )
{
	Packet *packet = ReceiveIgnoreRPC();
	while (packet && (packet->data[ 0 ] == ID_RPC || (packet->length>sizeof(unsigned char)+sizeof(RakNet::Time) &&
		packet->data[0]==ID_TIMESTAMP && packet->data[sizeof(unsigned char)+sizeof(RakNet::Time)]==ID_RPC)))
	{
		// Do RPC calls from the user thread, not the network update thread
		// If we are currently blocking on an RPC reply, send ID_RPC to the blocker to handle rather than handling RPCs automatically
		HandleRPCPacket( ( char* ) packet->data, packet->length, packet->playerId, packet->playerIndex );
		DeallocatePacket( packet );

		packet = ReceiveIgnoreRPC();
	}

    return packet;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal - Gets a packet without checking for RPCs
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning( disable : 4701 ) // warning C4701: local variable <variable name> may be used without having been initialized
#endif
Packet* RakPeer::ReceiveIgnoreRPC( void )
{
	if ( !( IsActive() ) )
		return 0;

	Packet *packet;
	Packet **threadPacket;
	PluginReceiveResult pluginResult;

	int offset;
	unsigned int i;

	for (i=0; i < messageHandlerList.Size(); i++)
	{
		messageHandlerList[i]->Update(this);
	}

	do 
	{
#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[transferToPacketQueue_Mutex].Lock();
#endif
		// Take all the messages off the queue so if the user pushes them back they are really pushed back, and not just at the end of the immediate write
		threadPacket=packetSingleProducerConsumer.ReadLock();
		while (threadPacket)
		{
			packet=*threadPacket;
			packetSingleProducerConsumer.ReadUnlock();
			threadPacket=packetSingleProducerConsumer.ReadLock();
			packetPool.Push(packet);
		}
#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[transferToPacketQueue_Mutex].Unlock();
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif

#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[packetPool_Mutex].Lock();
#endif
		if (packetPool.Size()==0)
		{
#ifdef _RAKNET_THREADSAFE
			rakPeerMutexes[packetPool_Mutex].Unlock();
#endif
			return 0;
		}

		packet = packetPool.Pop();
#ifdef _RAKNET_THREADSAFE
		rakPeerMutexes[packetPool_Mutex].Unlock();
#endif
		if ( ( packet->length >= sizeof(unsigned char) + sizeof( RakNet::Time ) ) &&
			( (unsigned char) packet->data[ 0 ] == ID_TIMESTAMP ) )
		{
			offset = sizeof(unsigned char);
			ShiftIncomingTimestamp( packet->data + offset, packet->playerId );
		}
		if ( (unsigned char) packet->data[ 0 ] == ID_RPC_REPLY )
		{
			HandleRPCReplyPacket( ( char* ) packet->data, packet->length, packet->playerId );
			DeallocatePacket( packet );
			packet=0; // Will do the loop again and get another packet
		}
		else
		{
			for (i=0; i < messageHandlerList.Size(); i++)
			{
				pluginResult=messageHandlerList[i]->OnReceive(this, packet);
				if (pluginResult==RR_STOP_PROCESSING_AND_DEALLOCATE)
				{
					DeallocatePacket( packet );
					packet=0; // Will do the loop again and get another packet
					break; // break out of the enclosing for
				}
				else if (pluginResult==RR_STOP_PROCESSING)
				{
					packet=0;
					break;
				}
			}
		}

	} while(packet==0);

#ifdef _DEBUG
	assert( packet->data );
#endif

	return packet;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Call this to deallocate a packet returned by Receive, in the same order returned to you from Receive
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::DeallocatePacket( Packet *packet )
{
	if ( packet == 0 )
		return;

	if (packet->deleteData)
		delete packet->data;
	free(packet);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Return the total number of connections we are allowed
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned short RakPeer::GetMaximumNumberOfPeers( void ) const
{
	return maximumNumberOfPeers;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Register a C function as available for calling as a remote procedure call
//
// Parameters:
// uniqueID: A string of only letters to identify this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions
// functionName(...): The name of the C function or C++ singleton to be used as a function pointer
// This can be called whether the client is active or not, and registered functions stay registered unless unregistered with
// UnregisterAsRemoteProcedureCall
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::RegisterAsRemoteProcedureCall( UniqueID uniqueID, void ( *functionPointer ) ( RPCParameters *rpcParms ) )
{
	if ( functionPointer == 0 )
		return;

	rpcMap.AddIdentifierWithFunction(uniqueID, (void*)functionPointer, false);

}

void RakPeer::RegisterClassMemberRPC( UniqueID uniqueID, void *functionPointer )
{
	if (functionPointer == 0)
		return;

	rpcMap.AddIdentifierWithFunction(uniqueID, functionPointer, true);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Unregisters a C function as available for calling as a remote procedure call that was formerly registered
// with RegisterAsRemoteProcedureCall
//
// Parameters:
// uniqueID: A null terminated string to identify this procedure.  Must match the parameter
// passed to RegisterAsRemoteProcedureCall
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::UnregisterAsRemoteProcedureCall( UniqueID uniqueID )
{
	// Don't call this while running because if you remove RPCs and add them they will not match the indices on the other systems anymore
	RakAssert(IsActive()==false);

	rpcMap.RemoveNode(uniqueID);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Calls a C function on the server that the server already registered using RegisterAsRemoteProcedureCall
// If you want that function to return data you should call RPC from that system in the same way
// Returns true on a successful packet send (this does not indicate the recipient performed the call), false on failure
//
// Parameters:
// uniqueID: A string of only letters to identify this procedure.  Recommended you use the macro CLASS_MEMBER_ID for class member functions.  Must match the parameter
// data: The block of data to send
// length: The size in BITS of the data to send
// bitStream: The bitstream to send
// priority: What priority level to send on.
// reliability: How reliability to send this data
// orderingChannel: When using ordered or sequenced packets, what channel to order these on.
// broadcast - Send this packet to everyone.
// playerId: Who to send this packet to, or in the case of broadcasting who not to send it to. Use UNASSIGNED_PLAYER_ID to specify none
// broadcast: True to send this packet to all connected systems.  If true, then playerId specifies who not to send the packet to.
// networkID: For static functions, pass UNASSIGNED_NETWORK_ID.  For member functions, you must derive from NetworkIDGenerator and pass the value returned by NetworkIDGenerator::GetNetworkID for that object.
// replyFromTarget: If 0, this function is non-blocking.  Otherwise it will block while waiting for a reply from the target procedure, which is remtely written to RPCParameters::replyToSender and copied to replyFromTarget.  The block will return early on disconnect or if the sent packet is unreliable and more than 3X the ping has elapsed.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::RPC( UniqueID uniqueID, const char *data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget )
{
	RakAssert(orderingChannel >=0 && orderingChannel < 32);

	if (replyFromTarget && blockOnRPCReply==true)
	{
		// TODO - this should be fixed eventually
		// Prevent a bug where function A calls B (blocking) which calls C back on the sender, which calls D, and C is blocking.
		// blockOnRPCReply is a shared variable so making it unset would unset both blocks, rather than the lowest on the callstack
		// Fix by tracking which function the reply is for.
		return false;
	}

	unsigned *sendList;
//	bool callerAllocationDataUsed;
	unsigned sendListSize;

	// All this code modifies bcs->data and bcs->numberOfBitsToSend in order to transform an RPC request into an actual packet for SendImmediate
//	RPCIndex rpcIndex; // Index into the list of RPC calls so we know what number to encode in the packet
//	char *userData; // RPC ID (the name of it) and a pointer to the data sent by the user
//	int extraBuffer; // How many data bytes were allocated to hold the RPC header
	unsigned remoteSystemIndex, sendListIndex; // Iterates into the list of remote systems
//	int dataBlockAllocationLength; // Total number of bytes to allocate for the packet
//	char *writeTarget; // Used to hold either a block of allocated data or the externally allocated data

	sendListSize=0;
	bool routeSend;
	routeSend=false;

	if (broadcast==false)
	{
#if !defined(_COMPATIBILITY_1)
		sendList=(unsigned *)alloca(sizeof(unsigned));
#else
		sendList = new unsigned[1];
#endif
		remoteSystemIndex=GetIndexFromPlayerID( playerId, false );
		if (remoteSystemIndex!=(unsigned)-1 &&
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ASAP && 
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY && 
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ON_NO_ACK)
		{
			sendList[0]=remoteSystemIndex;
			sendListSize=1;
		}
		else if (router)
			routeSend=true;
	}
	else
	{
#if !defined(_COMPATIBILITY_1)
		sendList=(unsigned *)alloca(sizeof(unsigned)*maximumNumberOfPeers);
#else
		sendList = new unsigned[maximumNumberOfPeers];
#endif

		for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; remoteSystemIndex++ )
		{
			if ( remoteSystemList[ remoteSystemIndex ].isActive && remoteSystemList[ remoteSystemIndex ].playerId != playerId )
				sendList[sendListSize++]=remoteSystemIndex;
		}
	}

	if (sendListSize==0 && routeSend==false)
	{
#if defined(_COMPATIBILITY_1)
		delete [] sendList;
#endif

		return false;
	}
	if (routeSend)
		sendListSize=1;

	RakNet::BitStream outgoingBitStream;
	// remoteSystemList in network thread
	for (sendListIndex=0; sendListIndex < (unsigned)sendListSize; sendListIndex++)
	{
		outgoingBitStream.ResetWritePointer(); // Let us write at the start of the data block, rather than at the end

		if (shiftTimestamp)
		{
			outgoingBitStream.Write((unsigned char) ID_TIMESTAMP);
			outgoingBitStream.Write(RakNet::GetTime());
		}
		
		outgoingBitStream.Write((unsigned char) ID_RPC);

		// Using Write instead of WriteCompressed, because it uses 1 bit less. (8 exact) It's not a huge a deal, but still better, right?
		// If you want to change it back, dont't forget to change the read in RakNet::HandleRPCPacket() also
		//outgoingBitStream.WriteCompressed(uniqueID);
		outgoingBitStream.Write(uniqueID);

		outgoingBitStream.Write((bool) ((replyFromTarget!=0)==true));
		outgoingBitStream.WriteCompressed( bitLength );
		if (networkID==UNASSIGNED_NETWORK_ID)
		{
			// No object ID
			outgoingBitStream.Write(false);
		}
		else
		{
			// Encode an object ID.  This will use pointer to class member RPC
			outgoingBitStream.Write(true);
			outgoingBitStream.Write(networkID);
		}


		if ( bitLength > 0 )
			outgoingBitStream.WriteBits( (const unsigned char *) data, bitLength, false ); // Last param is false to write the raw data originally from another bitstream, rather than shifting from user data
		else
			outgoingBitStream.WriteCompressed( ( unsigned int ) 0 );

		if (routeSend)
			router->Send((const char*)outgoingBitStream.GetData(), outgoingBitStream.GetNumberOfBitsUsed(), priority,reliability,orderingChannel,playerId);
		else
			Send(&outgoingBitStream, priority, reliability, orderingChannel, remoteSystemList[sendList[sendListIndex]].playerId, false);
	}

#if defined(_COMPATIBILITY_1)
	delete [] sendList;
#endif

	if (replyFromTarget)
	{
		blockOnRPCReply=true;
		// 04/20/06 Just do this transparently.
		// We have to be able to read blocking packets out of order.  Otherwise, if two systems were to send blocking RPC calls to each other at the same time,
		// and they also had ordered packets waiting before the block, it would be impossible to unblock.
		// assert(reliability==RELIABLE || reliability==UNRELIABLE);
		replyFromTargetBS=replyFromTarget;
		replyFromTargetPlayer=playerId;
		replyFromTargetBroadcast=broadcast;
	}

	// Do not enter this loop on blockOnRPCReply because it is a global which could be set to true by an RPC higher on the callstack, where one RPC was called while waiting for another RPC
	if (replyFromTarget)
//	if (blockOnRPCReply)
	{
//		Packet *p;
		RakNet::Time stopWaitingTime;
//		RPCIndex arrivedRPCIndex;
//		char uniqueIdentifier[256];
		if (reliability==UNRELIABLE)
			if (playerId==UNASSIGNED_PLAYER_ID)
				stopWaitingTime=RakNet::GetTime()+1500; // Lets guess the ave. ping is 500.  Not important to be very accurate
			else
				stopWaitingTime=RakNet::GetTime()+GetAveragePing(playerId)*3;

		// For reliable messages, block until we get a reply or the connection is lost
		// For unreliable messages, block until we get a reply, the connection is lost, or 3X the ping passes
		while (blockOnRPCReply &&
			((reliability==RELIABLE || reliability==RELIABLE_ORDERED || reliability==RELIABLE_SEQUENCED) ||
			RakNet::GetTime() < stopWaitingTime))
		{

			RakSleep(30);

			if (routeSend==false && ValidSendTarget(playerId, broadcast)==false)
				return false;

			// I might not support processing other RPCs while blocking on one due to complexities I can't control
			// Problem is FuncA calls FuncB which calls back to the sender FuncC. Sometimes it is desirable to call FuncC before returning a return value
			// from FuncB - sometimes not.  There is also a problem with recursion where FuncA calls FuncB which calls FuncA - sometimes valid if
			// a different control path is taken in FuncA. (This can take many different forms)
			/*
			// Same as Receive, but doesn't automatically do RPCs
			p = ReceiveIgnoreRPC();
			if (p)
			{
				// Process all RPC calls except for those calling the function we are currently blocking in (to prevent recursion).
				if ( p->data[ 0 ] == ID_RPC )
				{
					RakNet::BitStream temp((unsigned char *) p->data, p->length, false);
					RPCNode *rpcNode;
					temp.IgnoreBits(8);
					bool nameIsEncoded;
					temp.Read(nameIsEncoded);
					if (nameIsEncoded)
					{
						stringCompressor->DecodeString((char*)uniqueIdentifier, 256, &temp);
					}
					else
					{
						temp.ReadCompressed( arrivedRPCIndex );
						rpcNode=rpcMap.GetNodeFromIndex( arrivedRPCIndex );
						if (rpcNode==0)
						{
							// Invalid RPC format
#ifdef _DEBUG
							assert(0);
#endif
							DeallocatePacket(p);
							continue;
						}
						else
							strcpy(uniqueIdentifier, rpcNode->uniqueIdentifier);
					}

					if (strcmp(uniqueIdentifier, uniqueID)!=0)
					{
						HandleRPCPacket( ( char* ) p->data, p->length, p->playerId );
						DeallocatePacket(p);
					}
					else
					{
						PushBackPacket(p, false);
					}
				}
				else
				{
					PushBackPacket(p, false);
				}
			}
			*/
		}

		blockOnRPCReply=false;
	}

	return true;	
}


#ifdef _MSC_VER
#pragma warning( disable : 4701 ) // warning C4701: local variable <variable name> may be used without having been initialized
#endif
bool RakPeer::RPC( UniqueID uniqueID, RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool shiftTimestamp, NetworkID networkID, RakNet::BitStream *replyFromTarget )
{
	if (bitStream)
		return RPC(uniqueID, (const char*) bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), priority, reliability, orderingChannel, playerId, broadcast, shiftTimestamp, networkID, replyFromTarget);
	else
		return RPC(uniqueID, 0,0, priority, reliability, orderingChannel, playerId, broadcast, shiftTimestamp, networkID, replyFromTarget);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Close the connection to another host (if we initiated the connection it will disconnect, if they did it will kick them out).
//
// Parameters:
// target: Which connection to close
// sendDisconnectionNotification: True to send ID_DISCONNECTION_NOTIFICATION to the recipient. False to close it silently.
// channel: If blockDuration > 0, the disconnect packet will be sent on this channel
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::CloseConnection( const PlayerID target, bool sendDisconnectionNotification, unsigned char orderingChannel )
{
	CloseConnectionInternal(target, sendDisconnectionNotification, false, orderingChannel);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Given a playerID, returns an index from 0 to the maximum number of players allowed - 1.
//
// Parameters
// playerId - The playerID to search for
//
// Returns
// An integer from 0 to the maximum number of peers -1, or -1 if that player is not found
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetIndexFromPlayerID( const PlayerID playerId )
{
	return GetIndexFromPlayerID(playerId, false);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// This function is only useful for looping through all players.
//
// Parameters
// index - an integer between 0 and the maximum number of players allowed - 1.
//
// Returns
// A valid playerID or UNASSIGNED_PLAYER_ID if no such player at that index
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
PlayerID RakPeer::GetPlayerIDFromIndex( int index )
{
	// remoteSystemList in user thread
	//if ( index >= 0 && index < remoteSystemListSize )
	if ( index >= 0 && index < maximumNumberOfPeers )
		if (remoteSystemList[ index ].connectMode==RemoteSystemStruct::CONNECTED) // Don't give the user players that aren't fully connected, since sends will fail
			return remoteSystemList[ index ].playerId;

	return UNASSIGNED_PLAYER_ID;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Bans an IP from connecting. Banned IPs persist between connections.
//
// Parameters
// IP - Dotted IP address.  Can use * as a wildcard, such as 128.0.0.* will ban
// All IP addresses starting with 128.0.0
// milliseconds - how many ms for a temporary ban.  Use 0 for a permanent ban
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::AddToBanList( const char *IP, RakNet::Time milliseconds )
{
	unsigned index;
	RakNet::Time time = RakNet::GetTime();

	if ( IP == 0 || IP[ 0 ] == 0 || strlen( IP ) > 15 )
		return ;

	// If this guy is already in the ban list, do nothing
	index = 0;

	banListMutex.Lock();

	for ( ; index < banList.Size(); index++ )
	{
		if ( strcmp( IP, banList[ index ]->IP ) == 0 )
		{
			// Already in the ban list.  Just update the time
			if (milliseconds==0)
				banList[ index ]->timeout=0; // Infinite
			else
				banList[ index ]->timeout=time+milliseconds;
			banListMutex.Unlock();
			return;
		}
	}

	banListMutex.Unlock();

	BanStruct *banStruct = new BanStruct;
	banStruct->IP = new char [ 16 ];
	if (milliseconds==0)
		banStruct->timeout=0; // Infinite
	else
		banStruct->timeout=time+milliseconds;
	strcpy( banStruct->IP, IP );
	banListMutex.Lock();
	banList.Insert( banStruct );
	banListMutex.Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Allows a previously banned IP to connect.
//
// Parameters
// IP - Dotted IP address.  Can use * as a wildcard, such as 128.0.0.* will ban
// All IP addresses starting with 128.0.0
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::RemoveFromBanList( const char *IP )
{
	unsigned index;
	BanStruct *temp;

	if ( IP == 0 || IP[ 0 ] == 0 || strlen( IP ) > 15 )
		return ;

	index = 0;
	temp=0;

	banListMutex.Lock();

	for ( ; index < banList.Size(); index++ )
	{
		if ( strcmp( IP, banList[ index ]->IP ) == 0 )
		{
			temp = banList[ index ];
			banList[ index ] = banList[ banList.Size() - 1 ];
			banList.RemoveAtIndex( banList.Size() - 1 );
			break;
		}
	}

	banListMutex.Unlock();

	if (temp)
	{
		delete [] temp->IP;
		delete temp;
	}

}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Allows all previously banned IPs to connect.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ClearBanList( void )
{
	unsigned index;
	index = 0;
	banListMutex.Lock();

	for ( ; index < banList.Size(); index++ )
	{
		delete [] banList[ index ]->IP;
		delete [] banList[ index ];
	}

	banList.Clear();

	banListMutex.Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Determines if a particular IP is banned.
//
// Parameters
// IP - Complete dotted IP address
//
// Returns
// True if IP matches any IPs in the ban list, accounting for any wildcards.
// False otherwise.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::IsBanned( const char *IP )
{
	unsigned banListIndex, characterIndex;
	RakNet::Time time;
	BanStruct *temp;

	if ( IP == 0 || IP[ 0 ] == 0 || strlen( IP ) > 15 )
		return false;

	banListIndex = 0;

	if ( banList.Size() == 0 )
		return false; // Skip the mutex if possible

	time = RakNet::GetTime();

	banListMutex.Lock();

	while ( banListIndex < banList.Size() )
	{
		if (banList[ banListIndex ]->timeout>0 && banList[ banListIndex ]->timeout<time)
		{
			// Delete expired ban
			temp = banList[ banListIndex ];
			banList[ banListIndex ] = banList[ banList.Size() - 1 ];
			banList.RemoveAtIndex( banList.Size() - 1 );
			delete [] temp->IP;
			delete temp;
		}
		else
		{
			characterIndex = 0;

#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
			while ( true )
			{
				if ( banList[ banListIndex ]->IP[ characterIndex ] == IP[ characterIndex ] )
				{
					// Equal characters

					if ( IP[ characterIndex ] == 0 )
					{
						banListMutex.Unlock();
						// End of the string and the strings match

						return true;
					}

					characterIndex++;
				}

				else
				{
					if ( banList[ banListIndex ]->IP[ characterIndex ] == 0 || IP[ characterIndex ] == 0 )
					{
						// End of one of the strings
						break;
					}

					// Characters do not match
					if ( banList[ banListIndex ]->IP[ characterIndex ] == '*' )
					{
						banListMutex.Unlock();

						// Domain is banned.
						return true;
					}

					// Characters do not match and it is not a *
					break;
				}
			}

			banListIndex++;
		}
	}

	banListMutex.Unlock();

	// No match found.
	return false;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Send a ping to the specified connected system.
//
// Parameters:
// target - who to ping
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::Ping( const PlayerID target )
{
	PingInternal(target, false);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Send a ping to the specified unconnected system.
// The remote system, if it is Initialized, will respond with ID_PONG.
// The final ping time will be encoded in the following sizeof(RakNet::Time) bytes.  (Default is 4 bytes - See __GET_TIME_64BIT in NetworkTypes.h
//
// Parameters:
// host: Either a dotted IP address or a domain name.  Can be 255.255.255.255 for LAN broadcast.
// remotePort: Which port to connect to on the remote machine.
// onlyReplyOnAcceptingConnections: Only request a reply if the remote system has open connections
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::Ping( const char* host, unsigned short remotePort, bool onlyReplyOnAcceptingConnections )
{
	if ( host == 0 )
		return;

//	if ( IsActive() == false )
//		return;

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return;
#endif
	}

	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	RakNet::BitStream bitStream( sizeof(unsigned char) + sizeof(RakNet::Time) );
	if ( onlyReplyOnAcceptingConnections )
		bitStream.Write((unsigned char)ID_PING_OPEN_CONNECTIONS);
	else
		bitStream.Write((unsigned char)ID_PING);

	bitStream.Write(RakNet::GetTime());

	unsigned i;
	for (i=0; i < messageHandlerList.Size(); i++)
		messageHandlerList[i]->OnDirectSocketSend((const char*)bitStream.GetData(), bitStream.GetNumberOfBitsUsed(), playerId);
	// No timestamp for 255.255.255.255
	SocketLayer::Instance()->SendTo( connectionSocket, (const char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed(), ( char* ) host, remotePort );




}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the average of all ping times read for a specified target
//
// Parameters:
// target - whose time to read
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetAveragePing( const PlayerID playerId )
{
	int sum, quantity;
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem == 0 )
		return -1;

	for ( sum = 0, quantity = 0; quantity < PING_TIMES_ARRAY_SIZE; quantity++ )
	{
		if ( remoteSystem->pingAndClockDifferential[ quantity ].pingTime == 65535 )
			break;
		else
			sum += remoteSystem->pingAndClockDifferential[ quantity ].pingTime;
	}

	if ( quantity > 0 )
		return sum / quantity;
	else
		return -1;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the last ping time read for the specific player or -1 if none read yet
//
// Parameters:
// target - whose time to read
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetLastPing( const PlayerID playerId ) const
{
	RemoteSystemStruct * remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem == 0 )
		return -1;

	if ( remoteSystem->pingAndClockDifferentialWriteIndex == 0 )
		return remoteSystem->pingAndClockDifferential[ PING_TIMES_ARRAY_SIZE - 1 ].pingTime;
	else
		return remoteSystem->pingAndClockDifferential[ remoteSystem->pingAndClockDifferentialWriteIndex - 1 ].pingTime;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the lowest ping time read or -1 if none read yet
//
// Parameters:
// target - whose time to read
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetLowestPing( const PlayerID playerId ) const
{
	RemoteSystemStruct * remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem == 0 )
		return -1;

	return remoteSystem->lowestPing;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Ping the remote systems every so often.  This is off by default
// This will work anytime
//
// Parameters:
// doPing - True to start occasional pings.  False to stop them.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetOccasionalPing( bool doPing )
{
	occasionalPing = doPing;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// All systems have a block of data associated with them, for user use.  This block of data can be used to easily
// specify typical system data that you want to know on connection, such as the player's name.
//
// Parameters:
// playerId: Which system you are referring to.  Pass the value returned by GetInternalID to refer to yourself
//
// Returns:
// The data passed to SetRemoteStaticData stored as a bitstream
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakNet::BitStream * RakPeer::GetRemoteStaticData( const PlayerID playerId )
{
	if ( playerId == myPlayerId )
		return & localStaticData;

	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, false );

	if ( remoteSystem )
		return &(remoteSystem->staticData);
	else
		return 0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// All systems have a block of data associated with them, for user use.  This block of data can be used to easily
// specify typical system data that you want to know on connection, such as the player's name.
//
// Parameters:
// playerId: Whose static data to change.  Use your own playerId to change your own static data
// data: a block of data to store
// length: The length of data in bytes
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetRemoteStaticData( const PlayerID playerId, const char *data, const int length )
{
	if ( playerId == myPlayerId )
	{
		localStaticData.Reset();

		if ( data && length > 0 )
			localStaticData.Write( data, length );
	}
	else
	{
		RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, false, true );

		if ( remoteSystem == 0 )
			return;

		remoteSystem->staticData.Reset();
		remoteSystem->staticData.Write( data, length );
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sends your static data to the specified system.  This is automatically done on connection.
// You should call this when you change your static data.
// To send the static data of another system (such as relaying their data) you should do this normally with Send
//
// Parameters:
// target: Who to send your static data to.  Specify UNASSIGNED_PLAYER_ID to broadcast to all
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SendStaticData( const PlayerID target )
{
	SendStaticDataInternal(target, false);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Length should be under 400 bytes, as a security measure against flood attacks
// Sets the data to send with an  (LAN server discovery) /(offline ping) response
// See the Ping sample project for how this is used.
// data: a block of data to store, or 0 for none
// length: The length of data in bytes, or 0 for none
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetOfflinePingResponse( const char *data, const unsigned int length )
{
	assert(length < 400);

	rakPeerMutexes[ offlinePingResponse_Mutex ].Lock();
	offlinePingResponse.Reset();

	if ( data && length > 0 )
		offlinePingResponse.Write( data, length );

	rakPeerMutexes[ offlinePingResponse_Mutex ].Unlock();
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Return the unique PlayerID that represents you on the the network
// Note that unlike in previous versions, this is a struct and is not sequential
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
PlayerID RakPeer::GetInternalID( void ) const
{
	return myPlayerId;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Return the unique address identifier that represents you on the the network and is based on your external
// IP / port (the IP / port the specified player uses to communicate with you)
// Note that unlike in previous versions, this is a struct and is not sequential
//
// Parameters:
// target: Which remote system you are referring to for your external ID
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
PlayerID RakPeer::GetExternalID( const PlayerID target ) const
{
	unsigned i;
	PlayerID inactiveExternalId;

	inactiveExternalId=UNASSIGNED_PLAYER_ID;

	// First check for active connection with this playerId
	for ( i = 0; i < maximumNumberOfPeers; i++ )
	{
		if (remoteSystemList[ i ].playerId == target || target==UNASSIGNED_PLAYER_ID )
		{
			if ( remoteSystemList[ i ].isActive )
				return remoteSystemList[ i ].myExternalPlayerId;
			else
				inactiveExternalId=remoteSystemList[ i ].myExternalPlayerId;
		}
	}

	return inactiveExternalId;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Set the time, in MS, to use before considering ourselves disconnected after not being able to deliver a reliable packet
// \param[in] time Time, in MS
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetTimeoutTime( RakNet::Time timeMS, const PlayerID target )
{
	RemoteSystemStruct * remoteSystem = GetRemoteSystemFromPlayerID( target, false, true );

	if ( remoteSystem != 0 )
		remoteSystem->reliabilityLayer.SetTimeoutTime(timeMS);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Change the MTU size in order to improve performance when sending large packets
// This can only be called when not connected.
// A too high of value will cause packets not to arrive at worst and be fragmented at best.
// A too low of value will split packets unnecessarily.
//
// Parameters:
// size: Set according to the following table:
// 1500. The largest Ethernet packet size
// This is the typical setting for non-PPPoE, non-VPN connections. The default value for NETGEAR routers, adapters and switches.
// 1492. The size PPPoE prefers.
// 1472. Maximum size to use for pinging. (Bigger packets are fragmented.)
// 1468. The size DHCP prefers.
// 1460. Usable by AOL if you don't have large email attachments, etc.
// 1430. The size VPN and PPTP prefer.
// 1400. Maximum size for AOL DSL.
// 576. Typical value to connect to dial-up ISPs. (Default)
//
// Returns:
// False on failure (we are connected).  True on success.  Maximum allowed size is MAXIMUM_MTU_SIZE
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::SetMTUSize( int size )
{
	if ( IsActive() )
		return false;

	if ( size < 512 )
		size = 512;
	else if ( size > MAXIMUM_MTU_SIZE )
		size = MAXIMUM_MTU_SIZE;

	MTUSize = size;

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the current MTU size
//
// Returns:
// The MTU sized specified in SetMTUSize
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetMTUSize( void ) const
{
	return MTUSize;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the number of IP addresses we have
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned int RakPeer::GetNumberOfAddresses( void )
{
#if !defined(_COMPATIBILITY_1)
	char ipList[ 10 ][ 16 ];
	memset( ipList, 0, sizeof( char ) * 16 * 10 );
	SocketLayer::Instance()->GetMyIP( ipList );

	int i = 0;

	while ( ipList[ i ][ 0 ] )
		i++;

	return i;
#else
	assert(0);
	return 0;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Given a PlayerID struct, returns the dotted IP address string this binaryAddress field represents
//
// Returns:
// Null terminated dotted IP address string.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* RakPeer::PlayerIDToDottedIP( const PlayerID playerId ) const
{
#if !defined(_COMPATIBILITY_1)
	in_addr in;
	in.s_addr = playerId.binaryAddress;
	return inet_ntoa( in );
#else
	assert(0); // Not supported
	return 0;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns an IP address at index 0 to GetNumberOfAddresses-1
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const char* RakPeer::GetLocalIP( unsigned int index )
{
#if !defined(_COMPATIBILITY_1)
	static char ipList[ 10 ][ 16 ];

	if ( index >= 10 )
		index = 9;

	memset( ipList, 0, sizeof( char ) * 16 * 10 );

	SocketLayer::Instance()->GetMyIP( ipList );

	return ipList[ index ];
#else
	assert(0);
	return 0;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Allow or disallow connection responses from any IP. Normally this should be false, but may be necessary
// when connection to servers with multiple IP addresses
//
// Parameters:
// allow - True to allow this behavior, false to not allow.  Defaults to false.  Value persists between connections
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::AllowConnectionResponseIPMigration( bool allow )
{
	allowConnectionResponseIPMigration = allow;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Sends a one byte message ID_ADVERTISE_SYSTEM to the remote unconnected system.
// This will tell the remote system our external IP outside the LAN, and can be used for NAT punch through
//
// Requires:
// The sender and recipient must already be started via a successful call to Initialize
//
// host: Either a dotted IP address or a domain name
// remotePort: Which port to connect to on the remote machine.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::AdvertiseSystem( const char *host, unsigned short remotePort, const char *data, int dataLength )
{
	if ( IsActive() == false )
		return ;

	if (host==0)
		return;

	// This is a security measure.  Don't send data longer than this value
	assert(dataLength <= MAX_OFFLINE_DATA_LENGTH);
	assert(dataLength>=0);

	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return;
#endif
	}

	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	RakNet::BitStream bitStream;
	bitStream.Write((unsigned char)ID_ADVERTISE_SYSTEM);
	if (dataLength>0)
		bitStream.Write(data, dataLength);
	else
		bitStream.Write((unsigned char)0); // Pad

	unsigned i;
	for (i=0; i < messageHandlerList.Size(); i++)
		messageHandlerList[i]->OnDirectSocketSend((const char*)bitStream.GetData(), bitStream.GetNumberOfBitsUsed(), playerId);
	SocketLayer::Instance()->SendTo( connectionSocket, (const char*)bitStream.GetData(), bitStream.GetNumberOfBytesUsed(), ( char* ) host, remotePort );



	/*
	// If the host starts with something other than 0, 1, or 2 it's (probably) a domain name.
	if ( host[ 0 ] < '0' || host[ 0 ] > '2' )
	{
#if !defined(_COMPATIBILITY_1)
		host = ( char* ) SocketLayer::Instance()->DomainNameToIP( host );
#else
		return;
#endif
	}

	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	RequestedConnectionStruct *rcs;
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Lock();
#endif
	rcs = requestedConnectionList.WriteLock();
	rcs->playerId=playerId;
	rcs->nextRequestTime=RakNet::GetTime();
	rcs->requestsMade=0;
	if (data && dataLength>0)
	{
		rcs->data=new char [dataLength];
		rcs->dataLength=(unsigned short)dataLength;
		memcpy(rcs->data, data, dataLength);
	}
	else
	{
		rcs->data=0;
		rcs->dataLength=0;
	}
	rcs->actionToTake=RequestedConnectionStruct::ADVERTISE_SYSTEM;
	requestedConnectionList.WriteUnlock();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Unlock();
#endif
	*/
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Controls how often to return ID_DOWNLOAD_PROGRESS for large message downloads.
// ID_DOWNLOAD_PROGRESS is returned to indicate a new partial message chunk, roughly the MTU size, has arrived
// As it can be slow or cumbersome to get this notification for every chunk, you can set the interval at which it is returned.
// Defaults to 0 (never return this notification)
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetSplitMessageProgressInterval(int interval)
{
	RakAssert(interval>=0);
	splitMessageProgressInterval=interval;
	for ( unsigned short i = 0; i < maximumNumberOfPeers; i++ )
		remoteSystemList[ i ].reliabilityLayer.SetSplitMessageProgressInterval(splitMessageProgressInterval);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Set how long to wait before giving up on sending an unreliable message
// Useful if the network is clogged up.
// Set to 0 or less to never timeout.  Defaults to 0.
// timeoutMS How many ms to wait before simply not sending an unreliable message.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetUnreliableTimeout(RakNet::Time timeoutMS)
{
	RakAssert(timeoutMS>=0);
	unreliableTimeout=timeoutMS;
	for ( unsigned short i = 0; i < maximumNumberOfPeers; i++ )
		remoteSystemList[ i ].reliabilityLayer.SetUnreliableTimeout(unreliableTimeout);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Enables or disables our tracking of bytes input to and output from the network.
// This is required to get a frequency table, which is used to generate a new compression layer.
// You can call this at any time - however you SHOULD only call it when disconnected.  Otherwise you will only track
// part of the values sent over the network.
// This value persists between connect calls and defaults to false (no frequency tracking)
//
// Parameters:
// doCompile - true to track bytes.  Defaults to false
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetCompileFrequencyTable( bool doCompile )
{
	trackFrequencyTable = doCompile;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Returns the frequency of outgoing bytes into outputFrequencyTable
// The purpose is to save to file as either a master frequency table from a sample game session for passing to
// GenerateCompressionLayer(false)
// You should only call this when disconnected.
// Requires that you first enable data frequency tracking by calling SetCompileFrequencyTable(true)
//
// Parameters:
// outputFrequencyTable (out): The frequency of each corresponding byte
//
// Returns:
// Ffalse (failure) if connected or if frequency table tracking is not enabled.  Otherwise true (success)
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::GetOutgoingFrequencyTable( unsigned int outputFrequencyTable[ 256 ] )
{
	if ( IsActive() )
		return false;

	if ( trackFrequencyTable == false )
		return false;

	memcpy( outputFrequencyTable, frequencyTable, sizeof( unsigned int ) * 256 );

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Generates the compression layer from the input frequency table.
// You should call this twice - once with inputLayer as true and once as false.
// The frequency table passed here with inputLayer=true should match the frequency table on the recipient with inputLayer=false.
// Likewise, the frequency table passed here with inputLayer=false should match the frequency table on the recipient with inputLayer=true
// Calling this function when there is an existing layer will overwrite the old layer
// You should only call this when disconnected
//
// Parameters:
// inputFrequencyTable: The frequency table returned from GetSendFrequencyTable(...)
// inputLayer - Whether inputFrequencyTable represents incoming data from other systems (true) or outgoing data from this system (false)
//
// Returns:
// False on failure (we are connected).  True otherwise
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::GenerateCompressionLayer( unsigned int inputFrequencyTable[ 256 ], bool inputLayer )
{
	if ( IsActive() )
		return false;

	DeleteCompressionLayer( inputLayer );

	if ( inputLayer )
	{
		inputTree = new HuffmanEncodingTree;
		inputTree->GenerateFromFrequencyTable( inputFrequencyTable );
	}

	else
	{
		outputTree = new HuffmanEncodingTree;
		outputTree->GenerateFromFrequencyTable( inputFrequencyTable );
	}

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Deletes the output or input layer as specified.  This is not necessary to call and is only valuable for freeing memory
// You should only call this when disconnected
//
// Parameters:
// inputLayer - Specifies the corresponding compression layer generated by GenerateCompressionLayer.
//
// Returns:
// False on failure (we are connected).  True otherwise
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::DeleteCompressionLayer( bool inputLayer )
{
	if ( IsActive() )
		return false;

	if ( inputLayer )
	{
		if ( inputTree )
		{
			delete inputTree;
			inputTree = 0;
		}
	}

	else
	{
		if ( outputTree )
		{
			delete outputTree;
			outputTree = 0;
		}
	}

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Returns:
// The compression ratio.  A low compression ratio is good.  Compression is for outgoing data
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float RakPeer::GetCompressionRatio( void ) const
{
	if ( rawBytesSent > 0 )
	{
		return ( float ) compressedBytesSent / ( float ) rawBytesSent;
	}

	else
		return 0.0f;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Returns:
// The decompression ratio.  A high decompression ratio is good.  Decompression is for incoming data
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
float RakPeer::GetDecompressionRatio( void ) const
{
	if ( rawBytesReceived > 0 )
	{
		return ( float ) compressedBytesReceived / ( float ) rawBytesReceived;
	}

	else
		return 0.0f;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Attatches a Plugin interface to run code automatically on message receipt in the Receive call
//
// \param messageHandler Pointer to a plugin to attach
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::AttachPlugin( PluginInterface *plugin )
{
	if (messageHandlerList.GetIndexOf(plugin)==MAX_UNSIGNED_LONG)
	{
		messageHandlerList.Insert(plugin);
		plugin->OnAttach(this);
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Detaches a Plugin interface to run code automatically on message receipt
//
// \param messageHandler Pointer to a plugin to detach
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::DetachPlugin( PluginInterface *plugin )
{
	if (plugin==0)
		return;

	unsigned int index;
	index = messageHandlerList.GetIndexOf(plugin);
	if (index!=MAX_UNSIGNED_LONG)
	{
		messageHandlerList[index]->OnDetach(this);
		// Unordered list so delete from end for speed
		messageHandlerList[index]=messageHandlerList[messageHandlerList.Size()-1];
		messageHandlerList.Del();
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Put a packet back at the end of the receive queue in case you don't want to deal with it immediately
//
// packet The packet you want to push back.
// pushAtHead True to push the packet so that the next receive call returns it.  False to push it at the end of the queue (obviously pushing it at the end makes the packets out of order)
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::PushBackPacket( Packet *packet, bool pushAtHead)
{
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Lock();
#endif
	RakAssert(packet);
	if (pushAtHead)
		packetPool.PushAtHead(packet);
	else
		packetPool.Push(packet);
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[packetPool_Mutex].Unlock();
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SetRouterInterface( RouterInterface *routerInterface )
{
	router=routerInterface;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::RemoveRouterInterface( RouterInterface *routerInterface )
{
	if (router==routerInterface)
		router=0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Adds simulated ping and packet loss to the outgoing data flow.
// To simulate bi-directional ping and packet loss, you should call this on both the sender and the recipient, with half the total ping and maxSendBPS value on each.
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ApplyNetworkSimulator( double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance)
{
#ifndef _RELEASE
	if (remoteSystemList)
	{
		unsigned short i;
		for (i=0; i < maximumNumberOfPeers; i++)
		//for (i=0; i < remoteSystemListSize; i++)
			remoteSystemList[i].reliabilityLayer.ApplyNetworkSimulator(maxSendBPS, minExtraPing, extraPingVariance);
	}

	_maxSendBPS=maxSendBPS;
	_minExtraPing=minExtraPing;
	_extraPingVariance=extraPingVariance;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Returns if you previously called ApplyNetworkSimulator
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::IsNetworkSimulatorActive( void )
{
	return _maxSendBPS>0 || _minExtraPing>0 || _extraPingVariance>0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// For internal use
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RPCMap* RakPeer::GetRPCMap( const PlayerID playerId)
{
    if (playerId==UNASSIGNED_PLAYER_ID)
		return &rpcMap;
	else
	{
		RemoteSystemStruct *rss=GetRemoteSystemFromPlayerID(playerId, false, true);
		if (rss)
			return &(rss->rpcMap);
		else
			return 0;
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RakNetStatisticsStruct * const RakPeer::GetStatistics( const PlayerID playerId )
{
	if (playerId==UNASSIGNED_PLAYER_ID)
	{
		bool firstWrite=false;
		static RakNetStatisticsStruct sum;
		RakNetStatisticsStruct *systemStats;
		// Return a crude sum
		for ( unsigned short i = 0; i < maximumNumberOfPeers; i++ )
		{
			if (remoteSystemList[ i ].isActive)
			{
				systemStats=remoteSystemList[ i ].reliabilityLayer.GetStatistics();
				
				if (firstWrite==false)
					memcpy(&sum, systemStats, sizeof(RakNetStatisticsStruct));
				else
					sum+=*systemStats;
			}
		}
		return &sum;
	}
	else
	{
		RemoteSystemStruct * rss;
	rss = GetRemoteSystemFromPlayerID( playerId, false, false );
		if ( rss && endThreads==false )
			return rss->reliabilityLayer.GetStatistics();
	}	

	return 0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
void RakPeer::RemoveFromRequestedConnectionsList( const PlayerID playerId )
{
	int i;
	rakPeerMutexes[ RakPeer::requestedConnections_MUTEX ].Lock();

	for ( i = 0; i < ( int ) requestedConnectionsList.Size(); )
	{
		if ( requestedConnectionsList[ i ]->playerId == playerId )
		{
			delete requestedConnectionsList[ i ];
			requestedConnectionsList.Del( i );
			break;
		}
	}

	rakPeerMutexes[ RakPeer::requestedConnections_MUTEX ].Unlock();
}
*/

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int RakPeer::GetIndexFromPlayerID( const PlayerID playerId, bool calledFromNetworkThread )
{
	unsigned i;

	if ( playerId == UNASSIGNED_PLAYER_ID )
		return -1;

	if (calledFromNetworkThread)
	{
		bool objectExists;
		unsigned index;
		index = remoteSystemLookup.GetIndexFromKey(playerId, &objectExists);
		if (objectExists)
		{
			assert(remoteSystemList[remoteSystemLookup[index].index].playerId==playerId);
			return remoteSystemLookup[index].index;
		}
		else
			return -1;
	}
	else
	{
		// remoteSystemList in user and network thread
		for ( i = 0; i < maximumNumberOfPeers; i++ )
			if ( remoteSystemList[ i ].isActive && remoteSystemList[ i ].playerId == playerId )
				return i;
	}

	return -1;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::SendConnectionRequest( const char* host, unsigned short remotePort, char* passwordData, int passwordDataLength )
{
	PlayerID playerId;
	IPToPlayerID( host, remotePort, &playerId );

	// Already connected?
	if (GetRemoteSystemFromPlayerID(playerId, false, true))
		return false;

	assert(passwordDataLength <= 256);

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Lock();
#endif
	RequestedConnectionStruct *rcs = requestedConnectionList.WriteLock();
	rcs->playerId=playerId;
	rcs->nextRequestTime=RakNet::GetTime();
	rcs->requestsMade=0;
	rcs->data=0;
	rcs->actionToTake=RequestedConnectionStruct::CONNECT;
	memcpy(rcs->outgoingPassword, passwordData, passwordDataLength);
	rcs->outgoingPasswordLength=(unsigned char) passwordDataLength;
	requestedConnectionList.WriteUnlock();

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Unlock();
#endif

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::IPToPlayerID( const char* host, unsigned short remotePort, PlayerID *playerId )
{
	if ( host == 0 )
		return ;

	playerId->binaryAddress = inet_addr( host );

	playerId->port = remotePort;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RemoteSystemStruct *RakPeer::GetRemoteSystemFromPlayerID( const PlayerID playerID, bool calledFromNetworkThread, bool onlyActive ) const
{
	unsigned i;

	if ( playerID == UNASSIGNED_PLAYER_ID )
		return 0;

	if (calledFromNetworkThread)
	{
		bool objectExists;
		unsigned index;
		index = remoteSystemLookup.GetIndexFromKey(playerID, &objectExists);
		if (objectExists)
		{
#ifdef _DEBUG
			assert(remoteSystemList[ remoteSystemLookup[index].index ].playerId==playerID);
#endif
			return remoteSystemList + remoteSystemLookup[index].index;
		}
	}
	else
	{
		int deadConnectionIndex=-1;

		// Active connections take priority.  But if there are no active connections, return the first systemAddress match found
		for ( i = 0; i < maximumNumberOfPeers; i++ )
		{
			if (remoteSystemList[ i ].playerId == playerID )
			{
				if ( remoteSystemList[ i ].isActive )
					return remoteSystemList + i;
				else if (deadConnectionIndex==-1)
					deadConnectionIndex=i;
			}
		}

		if (deadConnectionIndex!=-1 && onlyActive==false)
			return remoteSystemList + deadConnectionIndex;
	}

	return 0;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ParseConnectionRequestPacket( RemoteSystemStruct *remoteSystem, PlayerID playerId, const char *data, int byteSize )
{
	// If we are full tell the sender.
	if ( !AllowIncomingConnections() )
	{
		unsigned char c = ID_NO_FREE_INCOMING_CONNECTIONS;
		// SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, ( char* ) & c, sizeof( char ), systemAddress.binaryAddress, systemAddress.port );
		SendImmediate(( char* ) & c, sizeof( char )*8, SYSTEM_PRIORITY, RELIABLE, 0, playerId, false, false, RakNet::GetTime());
		remoteSystem->connectMode=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY;
	}
	else
	{
		const char *password = data + sizeof(unsigned char);
		int passwordLength = byteSize - sizeof(unsigned char);

		if ( incomingPasswordLength == passwordLength &&
			memcmp( password, incomingPassword, incomingPasswordLength ) == 0 )
		{
			remoteSystem->connectMode=RemoteSystemStruct::HANDLING_CONNECTION_REQUEST;

#if !defined(_COMPATIBILITY_1)
			if ( usingSecurity == false )
#endif
			{
#ifdef _TEST_AES
				unsigned char AESKey[ 16 ];
				// Save the AES key
				for ( i = 0; i < 16; i++ )
					AESKey[ i ] = i;

				OnConnectionRequest( remoteSystem, AESKey, true );
#else
				// Connect this player assuming we have open slots
				OnConnectionRequest( remoteSystem, 0, false );
#endif
			}
#if !defined(_COMPATIBILITY_1)
			else
				SecuredConnectionResponse( playerId );
#endif
		}
		else
		{
			// This one we only send once since we don't care if it arrives.
			unsigned char c = ID_INVALID_PASSWORD;
			// SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, ( char* ) & c, sizeof( char ), systemAddress.binaryAddress, systemAddress.port );
			SendImmediate(( char* ) & c, sizeof( char )*8, SYSTEM_PRIORITY, RELIABLE, 0, playerId, false, false, RakNet::GetTime());
			remoteSystem->connectMode=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY;
		}
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

struct {
	char* a;
	char* b;
} auth_table[256] = {
	{"3605359F5AE3211",     "3DFFB73BB4D79E532F4873C0BB160178448E8E30"}, // 0x4e4838
	{"4635C4F75E1278",      "AAC0014C5D75F52DC9772B73771B0050933A9EAD"}, // 0x4e4898
	{"226B4F982407735D",    "F8A72CD3326708AC5FAC9571759DB6E305E2AB8E"}, // 0x4e48f8
	{"3B9B64812AC23ECD",    "A0E90A5EF07E6F3DA434A202602445CEE11B15A0"}, // 0x4e4958
	{"63C43C123D671B7",     "06B1A3758960A1159FA118EE26502CBBF146F616"}, // 0x4e49b8
	{"748076706C5166F0",    "7B61117760654E4521B068DF486C4B4B5E625D32"}, // 0x4e4a18
	{"43D841495FBD237A",    "DC32D6D882872DA084784216C43882004559D5A6"}, // 0x4e4a78
	{"5B5C7572539A4806",    "E8B3B5AE0E9EC99EBD1B8D89D48A6D680E952827"}, // 0x4e4ad8
	{"257437CF470C3BB7",    "42EC761ADE92ECC080AA1797DA46B9CB4CADD6D1"}, // 0x4e4b38
	{"4F121747AD07429",     "2B2FB3252FFF80F63C69856CAA4D06B8B005F6E2"}, // 0x4e4b98
	{"AE7330965747EB2",     "57F67A3FB9FCC6154A289BC3E906CEDD00929F75"}, // 0x4e4bf8
	{"3EBF177337C82EE8",    "ACFA7CC0C4BFDB1E1B98C8EF5CB383E19DAF2A9D"}, // 0x4e4c58
	{"2C8879B15901663",     "45C72109B2B4ED97B79D5FC7D90E9D06DE97E283"}, // 0x4e4cb8
	{"2D8E7203667B29C4",    "FC0B1B8DA840F01738CC1048DED3579539714E53"}, // 0x4e4d18
	{"1F2E4E52235936F5",    "AA5FDD9D8D282685D559996B9B26E8611C519EC1"}, // 0x4e4d78
	{"7BE839CC438810B3",    "332601C86563577DCF313C956009ED994BE7D2F6"}, // 0x4e4dd8
	{"AB67CFEAF134A",       "F9009DEB19A375FEC53FC8EF34CEE7942443928A"}, // 0x4e4e38
	{"189C260856AF66A0",    "E16FB781FA1F66AF14E05670F0C34674467D199F"}, // 0x4e4e98
	{"708179DC1DD72175",    "E155BBE8CE54C56F5E3B9558420309A4A8F65BD1"}, // 0x4e4ef8
	{"53D2D066D5831E9",     "B73A0DE5C5D425244AAE4ADDD8E75DD6C07439CC"}, // 0x4e4f58
	{"1F32D826FF82F3F",     "FFBB4A1878566109715BD8A6169A596C07D6B850"}, // 0x4e4fb8
	{"34A164AA250A78E0",    "B5A84ACADB898F3E5811CB0C91D5045928DDFFB0"}, // 0x4e5018
	{"6F569A03F9D46A3",     "F8CCAF2A333E89EB975A81463E0262FFE22FE13B"}, // 0x4e5078
	{"796332EB5F4E4B",      "2C880E4415632CB3587B1BB39C41743EE133401F"}, // 0x4e50d8
	{"5474B6545366252",     "3497B470F598F0DAEAA950A8DA19375ABD1F39C8"}, // 0x4e5138
	{"4614E542D672DE5",     "BA108826578D5531DB2F86AE6143E3DAD823B8C4"}, // 0x4e5198
	{"52E02ED72997722F",    "A5C3178A90491FA9C8F6BB37B0313B8DAE4860EF"}, // 0x4e51f8
	{"733EE8485968DA",      "3E3B022E961B8757AE5BCD63A915A8153F5A561C"}, // 0x4e5258
	{"14AE274F8772FD3",     "21F678D0285D23C633A951BD44596DE9C1C7AE44"}, // 0x4e52b8
	{"6EE1CAD1DE4624E",     "B97363187A56C56143A9A209563A8324B46C69E0"}, // 0x4e5318
	{"7CD647FA2F2430B",     "51081417568FEE305B7C481B08869DAD0565AA98"}, // 0x4e5378
	{"59F4141869C55BD7",    "6D7C678F4F8A55B6BD2D85D9879689BA8AEB6AC5"}, // 0x4e53d8
	{"75567C5A62083655",    "50BE57C26E980E39809919DB41F4F4D12E06A368"}, // 0x4e5438
	{"F6662A046DE11FE",     "B2CC3FAE28E3F255D36CA12A7222B8B20BECBE75"}, // 0x4e5498
	{"35AD30C3491C3B67",    "1C86A8790FE8E43109CDEA7A6D4F82071C2D38FF"}, // 0x4e54f8
	{"30E9393369B568C2",    "8049A94655368461379D721DA3B8DBB914912CFF"}, // 0x4e5558
	{"1960ADE78763FA8",     "42C71D95DE90B789822FB525E2F63307859F95FF"}, // 0x4e55b8
	{"5753C546AF36816",     "8BC034C6BB168C67F1CE184CE9F62D0B89D25F22"}, // 0x4e5618
	{"67972291FC150B5",     "744F500F57DE5F1F8670E24E64462D0BC93BCB39"}, // 0x4e5678
	{"67C3249B61E585E",     "200A8D44BFB190B6B3766291431C587BCE87F284"}, // 0x4e56d8
	{"64036875408D50DD",    "5B0FDE7B4C7DBE18AFCAF1774EDE0C3E8814CDB5"}, // 0x4e5738
	{"44D33F34EFE4EF5",     "EF6321F7B4D2F7EE447B5E3DCAAA4EB1EFC8E864"}, // 0x4e5798
	{"208F326720CE7BA5",    "52535BE33E8B3175C7EBDBEAD775BB1AB36B99FC"}, // 0x4e57f8
	{"2A673E5C16533AAA",    "889C7F4B137C5BEC77F3064D4D745ACDCBE4B119"}, // 0x4e5858
	{"48B24FF84D5B3A45",    "24BE8F4F48DE40050636C259D907DFC9F092ADE4"}, // 0x4e58b8
	{"658559E74A1D3E",      "893BCE236B9632A0F319BD7A776B9D847DFA20C5"}, // 0x4e5918
	{"2D8544F62C704829",    "903C9B1BF6815B8E906501A0C0A13AD67D6D3FD7"}, // 0x4e5978
	{"30F93FBC53B16E8",     "2A893AB80F82BF36CBD03034193A9FBC1B79F885"}, // 0x4e59d8
	{"1B243180202D2771",    "ECCCFED0477030ACB5BDFC5A965ED5FA9FA85607"}, // 0x4e5a38
	{"73D1545715AE14D2",    "584235C277552453D49D61A6187BEFA07CDDECA6"}, // 0x4e5a98
	{"1F2F3A8D39156572",    "6CE1D28E390BA57DED5295DEA700C8A524B5BDDF"}, // 0x4e5af8
	{"FDF4346371B5B99",     "CAB5C72868119CC7B47499DC88DAA53BBA90A10C"}, // 0x4e5b58
	{"5F4C4265788F2F32",    "F37423E79DFC178D67A620E083802BD0719E88C6"}, // 0x4e5bb8
	{"403B5FAD1D4C14D0",    "7E15248271652C34535613ACEF7BEAE53C86779F"}, // 0x4e5c18
	{"7FA37126236935F0",    "92AFC9AF733D8C715AC097333F952DE0D743ED5D"}, // 0x4e5c78
	{"7BE54C3EAF7E7E",      "3A05489EF6913AE983F1B84FBC864ACFC28DC883"}, // 0x4e5cd8
	{"1960D412450639A",     "58B075C75E1B922E2D4FB7A7875972DF8A174A1D"}, // 0x4e5d38
	{"2F882652ED579B",      "5F1D99DF57D344022E9DE0026991A9D34437C20F"}, // 0x4e5d98
	{"413423FB7E595F54",    "AB7419080EDB008FAA5F7EAEFF72E44B09671AD5"}, // 0x4e5df8
	{"557A13B18F53C97",     "991D6CF3FC9259002CB251EDFFEE84949F27790D"}, // 0x4e5e58
	{"1DD51ED4F7B61F8",     "112553F44C6AD767394BCF285453BEA8B21DC392"}, // 0x4e5eb8
	{"ABD5BEF6A673D3",      "E9B6644A741C05C7E74E2C91D336609DD2C1F607"}, // 0x4e5f18
	{"E7A7B8463125A91",     "7BB8B2BBDB0D6B79BD74189D4709B6E9450287CC"}, // 0x4e5f78
	{"23966F75D292A26",     "53DC7B6A90AB38594E8BBF9920BCD9EC24FE9235"}, // 0x4e5fd8
	{"6160582545E62DE",     "08E482DB030414F61D7E1B77B201E8710C0EE74D"}, // 0x4e6038
	{"4F1F4D6D525D3F5A",    "6D242A993DA9F1528E6245B5E9B147587770033E"}, // 0x4e6098
	{"1A471D7A41023D5",     "28E63F1030A6F1E7323BA5E729315AF8AD95D4AB"}, // 0x4e60f8
	{"336045812CC942AD",    "5398452AB416C43D2743F11ED2D5A615A7981C26"}, // 0x4e6158
	{"48F6258B864A1E",      "B612483357A7AB1030943B19FF6D58EE0230ED62"}, // 0x4e61b8
	{"7E3370F55B561F50",    "70C34AFEE63FD404F2F90538C45055501028A8F9"}, // 0x4e6218
	{"6FB33CD72DD52A94",    "0B8B3C5761863687782293FF68A603866F201C9E"}, // 0x4e6278
	{"7A93DA2739178EA",     "B5C82BD4F5B5F0D2A30A211EE044B99057503582"}, // 0x4e62d8
	{"79CB78091BBF36C8",    "03E561DBDFD365D00AA67B5E1B02897DAD420220"}, // 0x4e6338
	{"6DBE75F05C2D4816",    "19F0A1268DEC45A868C5431B04A3598D1DBC4F94"}, // 0x4e6398
	{"421029D23578217D",    "661F62CC26A4BA58AFDA1C42EBE94C561E60543F"}, // 0x4e63f8
	{"25B8716177F46E1",     "CA3C8DAEC5CCD6B99A944EA32D9C66A08C52D6E8"}, // 0x4e6458
	{"2956B6D4A17132C",     "CDE6AA8137A1B396985F59AF1C932704ED73AF13"}, // 0x4e64b8
	{"17C655073E012D4E",    "D4434C12F49F2DFFBBA1C7A0051DD178D8DB05FA"}, // 0x4e6518
	{"3CF561E62A194E82",    "C31CFF7B76C8CBE33B9EE2B4C7EFC11FB810B4B7"}, // 0x4e6578
	{"EF3740A78CF3DD2",     "B2F727E0283C1E3E30E04C22EACEE74E215E4824"}, // 0x4e65d8
	{"6D4A169E49DB24DD",    "F42ABC607BC47F74143251BEDB106FAB4C4601E7"}, // 0x4e6638
	{"6A8F60234C2D21D8",    "7960735B29C641B837D95F794B790A959F6C22A2"}, // 0x4e6698
	{"54361FCB74233AD3",    "5A260F9C595689A0AE1618ED356E215CF310DACD"}, // 0x4e66f8
	{"6CAA7A2361FF163D",    "97933708E9E8090C93708394C81D72AF43F2A5EA"}, // 0x4e6758
	{"B9E38F5C981CAB",      "C93824C9902637BF21EBC39435B78967231A46A8"}, // 0x4e67b8
	{"189D526D45067D5",     "21E25728D2F55FF4B74015F757E1671067BB8079"}, // 0x4e6818
	{"13E96B7A624522E1",    "D788157DFDB6504F8133B105807A0683B9E3329F"}, // 0x4e6878
	{"2182597737CC3FE5",    "9708C3658910CB3D482754A2B22CC7315E4403D5"}, // 0x4e68d8
	{"4A85670B121B26A7",    "AEDBC018E5CC0470967BF2A0515CB9A56944D487"}, // 0x4e6938
	{"68B7604E664540A7",    "E7913A6737D75F09991BF9C129CE090B8C5E9D8F"}, // 0x4e6998
	{"70622A2D796C65F",     "C7C4B3570B2618A6BFC0A0A1CEE25B7D2EF2CC3D"}, // 0x4e69f8
	{"C60690F193171C9",     "A8356A15297C5124D08CDA0818A5286AAD58048E"}, // 0x4e6a58
	{"40757AB938707A22",    "C06AB8E83514110F94EA069C6D1700B16A33ED2D"}, // 0x4e6ab8
	{"5E2476E442B7CEE",     "FF68628A1503C664A934BFEB0E28323A81100333"}, // 0x4e6b18
	{"16362E5B44C3383C",    "0D38088682288AAE6FEE6F6F1E834351286ECB17"}, // 0x4e6b78
	{"586811331F6E3A2B",    "6129A87081BB848D31C2A5C83F03BE81804567C7"}, // 0x4e6bd8
	{"24234317BE569AD",     "AB8A92F6C345601C3ED9002822953372E5384B3B"}, // 0x4e6c38
	{"4F615EB8125DC8B",     "25E664A55B33EA4B69E05D5FF12B7FDC970F6504"}, // 0x4e6c98
	{"3A3B47BF25B96EA9",    "DBD2773BC44E4C7EAC762880EED8E13420CFB82B"}, // 0x4e6cf8
	{"4CFF5B6D2DFEE8C",     "2BE250E17B9DA75FC5396F88DA34C75829E2E5C7"}, // 0x4e6d58
	{"1C84147E17077319",    "A601B0040F6004EB49566760D51DC4D149D83F64"}, // 0x4e6db8
	{"28BA2654377AE9F",     "14AC601AD07492CA3EA2ABA485EAFE9874904F68"}, // 0x4e6e18
	{"6985535123FA5318",    "51DFE007B6D4DF91CC63382D86045B5186058C91"}, // 0x4e6e78
	{"1DCC285429E5BAC",     "6EAB35AFF649F7EFBCB45D4141F915C1C30199A3"}, // 0x4e6ed8
	{"10CD588932A24F77",    "9ED5C5BBF063DB9725DC4FD43A86E51AC3FB4AEB"}, // 0x4e6f38
	{"49B357AA5C5D618C",    "D3AFB6F9FA064F4045DAEFBEBA5EDA6214E3251F"}, // 0x4e6f98
	{"596A4B4A3D73123A",    "7C8D8A4E1ECB4DD9ED368FEDEBBCF8F9FC257471"}, // 0x4e6ff8
	{"3AB2284454C158E",     "544AF0E903FC02124D60949E4220484F3506E4E2"}, // 0x4e7058
	{"78721E0F55C5318",     "559744656624F39F4F92ED546CB3EB081E41B78B"}, // 0x4e70b8
	{"E4F75653C2532F2",     "6B555DE9B336DA27CF4CB43CE4AEC12B2CFB1634"}, // 0x4e7118
	{"377E572436526BFF",    "E94C0C29E26A64BDE1EBF2D42B43CCE5579A0856"}, // 0x4e7178
	{"1FD628B458454771",    "DAD35209933DD8EEDC62AA91DE2634B32D20D19D"}, // 0x4e71d8
	{"2B2A292A65BB6D8E",    "E9490273EBD37B2FAF6F1444D723D08094BC39BC"}, // 0x4e7238
	{"46D55B6C742DFB3",     "ECD06CB27305A4ACB4E91FC1E5B5BA91F38AA6FC"}, // 0x4e7298
	{"79965B9669034399",    "410A56431139C7D906B91E1A0411F98DF9F970E8"}, // 0x4e72f8
	{"259F239D280523D7",    "19E56D23B4C93D816FE9128A774126A4CA2EA5D6"}, // 0x4e7358
	{"30E833361259A8",      "7E2A6A89E3254BA9FDDB20ACB19E3C12CAC73CC4"}, // 0x4e73b8
	{"7C94EEA541473EF",     "90051BD6EDC8C8CBD257B7B1879777FF3A7A9574"}, // 0x4e7418
	{"2DC8A9B16332F78",     "E3F880191B24BE6ADE4E6DFE36D9FD0CE82F65AD"}, // 0x4e7478
	{"50B013093B491481",    "88B5458CF339FF7A2BB4FA64E9A05AD5C1C56509"}, // 0x4e74d8
	{"11E3AC26896D79",      "2EEEAF8257D9782DE751427F061C04BA9B7BEF40"}, // 0x4e7538
	{"79F0E462DB65B07",     "86913AF0C116917BE324DD11236113F269488D58"}, // 0x4e7598
	{"4A68736B34CF94E",     "77D2C24DA0D9DDC4533516C4F334C407E7374928"}, // 0x4e75f8
	{"21EE1800430F4A70",    "59E93CFA3B608FB931DD57FCEC5AFF25F5612791"}, // 0x4e7658
	{"268E3B11D565A50",     "6FBAF79109EFCC612615ABCA87980F58C63CF934"}, // 0x4e76b8
	{"2A50612A252F4F9D",    "B72C7253AE352C0C288F7EF6330B461764585413"}, // 0x4e7718
	{"7E0C128042057E0C",    "484F4B0C21892A3FF680E8981D73BDFD32261240"}, // 0x4e7778
	{"667F64CC6A944DE4",    "05AE05848DD5CDC0714C809C209E513F741C335B"}, // 0x4e77d8
	{"679F2719629D2CC0",    "404B122622B92FFC4ADA3A3A70E9FEA698854B80"}, // 0x4e7838
	{"5A2C2880DE7B90",      "BDB9B36E44BD0275FFC1DFD6432C5C51DF8CCB0A"}, // 0x4e7898
	{"7F88619111411DE2",    "0C957BC4156EABC13C43CDB78207545741A22B6B"}, // 0x4e78f8
	{"7C84CF52754E60",      "8BCBEE4875C6E3703983C022E1215E6005007FB6"}, // 0x4e7958
	{"3E0B335274F91C99",    "23A24D0EA59A9C2F64A6EE0A258A558398B30245"}, // 0x4e79b8
	{"4F0A6E6F795F3804",    "EC8095E1F0B94CE4CFBCF3DAE006021D71623BAB"}, // 0x4e7a18
	{"5DF167991C306C41",    "A9A032713354AD4AC29DC95FC13F7E968DCAE324"}, // 0x4e7a78
	{"5B6E554590C72A3",     "0AA69D6D8E9449668AC5EDBB0BC11EC5693D9992"}, // 0x4e7ad8
	{"53077BF65F377CEC",    "CE4788BAE1B3A55BFD118B2EE3394F84C9D5EEF5"}, // 0x4e7b38
	{"22B0766198E1D57",     "FBC40434FF9908BC57DDA4E38D5A9E403DFD5A0F"}, // 0x4e7b98
	{"509D48CF52C06FDC",    "67D3E544707B4FE246C921E1A4F432A4BAD93BC0"}, // 0x4e7bf8
	{"735543C49BA68AF",     "28AB4F409BC2F7E88B084B1110DFDB2A04793BAA"}, // 0x4e7c58
	{"600879DE4A636C0A",    "BBC63E230B94E1C0153CF3D164FB5B99CE58C461"}, // 0x4e7cb8
	{"13224CFF3E8A132C",    "578E7CC0516F1A45BAB852A665C2FD11E91068BA"}, // 0x4e7d18
	{"72056AF6B1C42A2",     "CDFF0084CDC66D165128423C278EF295D1CFE1FA"}, // 0x4e7d78
	{"6DA22FBB5D9B75C5",    "2D4D503ABD1B2AC448DED5E5CD02C32072E31DDB"}, // 0x4e7dd8
	{"7EC25B224DCB6384",    "D47EABAA3BF4DFEF7EE5529290CE9A4FFAF56375"}, // 0x4e7e38
	{"52A558327AF6063",     "0C40055A1291C485B9EE708925E5DEDED2BE4746"}, // 0x4e7e98
	{"794F1FEF31B321C4",    "2A9972862FD6FE17F1F8562511DAFB5AD2B14948"}, // 0x4e7ef8
	{"44BC11B4432B4665",    "CAE5F15F92B44FA733C3DF91F78A2D6DE64B4E3E"}, // 0x4e7f58
	{"792C131D3EFC482B",    "4C39FB28B79456E58F3321D7A3B32F5163D4B191"}, // 0x4e7fb8
	{"57BE67236993B1E",     "34238729776690ABCE26157E46A1B5008B45416C"}, // 0x4e8018
	{"37408959327DB4",      "C25BAF8210936A9CABB863A83FF2906CF74D865F"}, // 0x4e8078
	{"1F46537853283E52",    "5890B60447D5F9E3E940F8DC65E098DB1BE56369"}, // 0x4e80d8
	{"19796A661C37A10",     "A3E75EA87D6CDEE50DEFA03834F4597BAE73D7EB"}, // 0x4e8138
	{"2D32423C5F2758BD",    "D98D8E325CC9E07B91A333836B5A4B20FBE2446B"}, // 0x4e8198
	{"394831B74A84A23",     "CACD53F10A54ED71FA76EF3DFC808A86E356498F"}, // 0x4e81f8
	{"2026600E508E88B",     "299E48E23EECDCB1C7A939EC8C689416DBBEA3D4"}, // 0x4e8258
	{"7A20ED6582D6484",     "017A3077019996E13D82C12A1B572E70EAD65943"}, // 0x4e82b8
	{"42073B237757DE1",     "1C0C28D8394A31F5D08E3D098D6C08233A09BB2A"}, // 0x4e8318
	{"2FFA59F174D84E01",    "D7052E2BF33268D02A8ACEE4A160C3A1E962F218"}, // 0x4e8378
	{"368033C2789A2753",    "D3B4DF1492A7C23764C0EE419335D95126BAA3F6"}, // 0x4e83d8
	{"55D423883286E18",     "CA46276DD76E149D2424362C84BCF668413A503F"}, // 0x4e8438
	{"39803CC417E0E6B",     "B0AA556A0EE4A5653BFA141D149CA2F4CEEDD946"}, // 0x4e8498
	{"542D1CF01D9A337F",    "8E36A60336451347D008CC37EB50A5323A71069C"}, // 0x4e84f8
	{"6DBA6C1B52CA429",     "BE7245B2B38F65DCA329953EE4AE5F2600614E88"}, // 0x4e8558
	{"578B50D37F5C289E",    "E5A09C70390F60E14A420323420C7B9A5BBEE1B5"}, // 0x4e85b8
	{"3B245B471B041D7D",    "D31B0C35E251C17484A50D0EDEE75AA69CCCE175"}, // 0x4e8618
	{"36F62BAD30771810",    "04F5D47B00C51EC1BE90499B01AA3061B0286E68"}, // 0x4e8678
	{"3D7857E2DB7FCF",      "5681B18C6F50394AF6E875F93348890FB8C23901"}, // 0x4e86d8
	{"6A7A54507B7C2125",    "AD13391CA5BABF94F804E616B96777D647AF843B"}, // 0x4e8738
	{"42B764155DC07D77",    "2BB5A0539B34AF24CCE0174F1A70922D127C80CA"}, // 0x4e8798
	{"11A95462F5D5C63",     "FAE3EC3F084EA2426DA66D340DEAB6C8DB591385"}, // 0x4e87f8
	{"59A1720EC9245",       "5E6E5C3E4837B0BACF5EEA067639E9E5D826F8FA"}, // 0x4e8858
	{"A1610DA1F33000",      "5A7FD42ADD3DFFA874954329CB21DA40ABC53E9B"}, // 0x4e88b8
	{"703D6DD73F304FFB",    "30DA1733BF140404D21DE745B3CF731BA99CB49B"}, // 0x4e8918
	{"15DB128662787469",    "5DB7B5F15BACD6912ADD5071F31FDECAB86D5FB3"}, // 0x4e8978
	{"2585B2042D2ACE",      "9317416C385A6126D2FF07262A464538CFB83B20"}, // 0x4e89d8
	{"12141E885C0D68BE",    "F54DB482752345DAAC0AC5D8AB482336E688C9BC"}, // 0x4e8a38
	{"57FE6876B115E8",      "FD417D223B78B9A3D4C8F4709822620D5BEEC6BD"}, // 0x4e8a98
	{"3D621ED468357754",    "7EF47E951F6099F685D14EB0CAF70D98C53F55EB"}, // 0x4e8af8
	{"540216686D723FE9",    "380630887D43B68CD32F84FA6A50E095E474E7E2"}, // 0x4e8b58
	{"E695AB411584A32",     "0A54F5156EB579617ED028F6811F7B5117AF1ECB"}, // 0x4e8bb8
	{"527C2D1A15E03F62",    "F8BFC50083419FEFA014F55C3DCA51F39D1B990A"}, // 0x4e8c18
	{"1A527F4445412F95",    "5DDBFDFC598524B29D62D6C75E9C104FD8BE78E4"}, // 0x4e8c78
	{"17456CC339677F58",    "C953167C38F2B68403F3B0E2FA6DAA21E8D463E9"}, // 0x4e8cd8
	{"B4977F637A14E6C",     "98BECC1A01A68CCF65A07257F8D4725412A91F41"}, // 0x4e8d38
	{"4C7DE2E749A3BC7",     "3A011A9B4B363DEE6419D5701F837F3D4AB5555B"}, // 0x4e8d98
	{"2700170F548A2ADC",    "80B086299BD8570583828F06C242E50DA150AC11"}, // 0x4e8df8
	{"1106133A7BAD6E1C",    "49D97E00101744E2D54B611A03AC154943F09C2E"}, // 0x4e8e58
	{"652BE2942F57ABD",     "8584EA363B6CE5B6DF44E4F262624F7A43E53011"}, // 0x4e8eb8
	{"1707675964FC9BA",     "3E3B3EB87ED064DEA267615F79D2216E8E425343"}, // 0x4e8f18
	{"160236AF773C4A1B",    "9CA605DF7A40469E5ECD9E24B1C428D92504ECD8"}, // 0x4e8f78
	{"52695015382876",      "AFB0F71A7C12187F42C2BE8DEA08C9E3D592A1CC"}, // 0x4e8fd8
	{"18C22A6455A06FB6",    "D3731D48FAF2963D142CD640E19FC9391E38CADA"}, // 0x4e9038
	{"375D2D865162471B",    "D6AB445E4B04039877CB996A0B02256E6AAF5DE0"}, // 0x4e9098
	{"222C2BDC64C45182",    "A6430681B7544F74CCA49D654071C5177B7D1D7F"}, // 0x4e90f8
	{"8D4ADD6A6231E7",      "FB3EF20CA7CABF7E616374553E4B424BD287F95E"}, // 0x4e9158
	{"13045F0010307928",    "4784D61E132B19563846CBA621358534299E56EA"}, // 0x4e91b8
	{"37067EDC166B410B",    "877D03349EEDB31F1FC2CB5261369292A820751A"}, // 0x4e9218
	{"5E94518E7AD07883",    "F4B4B8B01DC1E5612AC2299D6F1EBAF10562E422"}, // 0x4e9278
	{"4DEC5C5C640B452F",    "D662143F07AA08403813A6AA6C662FE6BF40FF38"}, // 0x4e92d8
	{"127539861721E24",     "DA79279EA1DFCEDEFDBC6BE1098283AD284CC95D"}, // 0x4e9338
	{"73C632C652F33EC",     "491B0E3C86AB7685817B7E19EDCDCEAF3E9F03B8"}, // 0x4e9398
	{"68E2001D4B79CE",      "ABA1AC63DDBA7AC8DEE0303DF41D6244565EB9B8"}, // 0x4e93f8
	{"77961E9AE792450",     "93987CA18C1BB898494271C11E7A82A81DD09020"}, // 0x4e9458
	{"209E5503567330FB",    "CF37E207E613CC24D1F7DD0F5E536699E9B393F7"}, // 0x4e94b8
	{"6FA269EB38176B5E",    "3C18A944173A77A384961EC1849535B1A8C13DD2"}, // 0x4e9518
	{"4DAF3CAB36642356",    "9657071FBD2E1B8E4E303AD362E6F427AC22AE2E"}, // 0x4e9578
	{"67F67CE141EA488C",    "FE481D2B9A17BA197AA9CAD45F62A1040A5561C4"}, // 0x4e95d8
	{"F1F655A6A7F5F3E",     "552A682729B38D271A9AD55B21B426406EC711C3"}, // 0x4e9638
	{"20DF1E3659343243",    "B6DD185829E5B8AD416625315695328D1CEEEF8A"}, // 0x4e9698
	{"2FCDA4626886649",     "41DDBEB76B36CBA1097EB05A4AF7CCC62AA9A08D"}, // 0x4e96f8
	{"5D713B960DB5262",     "B7BDDAFFE07E52EE91334FD4824BD15834B77853"}, // 0x4e9758
	{"1A9E4BF8572541BF",    "B215A59397BC13EA3A4D179A4861F1194C43C81F"}, // 0x4e97b8
	{"420232CE7BE72BA",     "834646FBE2F205DE59D31EC7E210F5816329F2E0"}, // 0x4e9818
	{"40FC38CC7462671A",    "D81FC4FDCA6F3E93588E65365B2E08B264B99995"}, // 0x4e9878
	{"42AD30EC28081993",    "58458E92D721EFD26805B8EBDB893E3A87A0CD39"}, // 0x4e98d8
	{"1152257464306C8D",    "27604F35F2B67BCEFEAAE3FD5A5BC3B80B015242"}, // 0x4e9938
	{"21D141D680D627",      "FF790335FBCEE25BA6507B2798204ADB9CCC9AA6"}, // 0x4e9998
	{"7D9155756CE1D7A",     "4403A57DD80EAE37A0DCEC012419C2F158C89F1E"}, // 0x4e99f8
	{"7E8F3827E6E3D22",     "413F93D78506A4DD0D27ABEE0EE67A31B38BE910"}, // 0x4e9a58
	{"11AF73A837AD7EFB",    "6A7F7A7E11BDDD1DBAF750FD409B766EE2A1C576"}, // 0x4e9ab8
	{"735077CC37C37528",    "A4F5EA20602E50A9FAEB10389AB1DB6A8611B605"}, // 0x4e9b18
	{"1A225ABA6331145A",    "47AAB47FDB2E89FC10B80C12ECC54A4E753246DF"}, // 0x4e9b78
	{"443CBC4454B334E",     "F581C5DCFFB12590F1FAA3482F1A622F444A46A4"}, // 0x4e9bd8
	{"5A692DAC25EB3398",    "42B7136D118A29A7AA5767C87EE89B67DF3CD301"}, // 0x4e9c38
	{"1DC3CC5276627A0",     "361581195354053B36639CF910F61B9CFCE4BC7E"}, // 0x4e9c98
	{"53844054BF19EA",      "9D3A27A1809FC677E2FB56B87E38CE43C328E3C1"}, // 0x4e9cf8
	{"63187B3C281B599A",    "D052BC66B2DA023EA7EBA04027B8713EBF5074AE"}, // 0x4e9d58
	{"1A774FDB1757037",     "A86C4B47D41BC73A6A881431D7D2E09A23896AD6"}, // 0x4e9db8
	{"CB16B2F659324AF",     "CCDC8D7D89C006F7039DC27B426D3E421381CD50"}, // 0x4e9e18
	{"3ECAB3D15392F9B",     "3B7AB8F493B0D9D41C814961F21C522FBAF14409"}, // 0x4e9e78
	{"16C934B36C9C4C5",     "529849EF0788DD3FE962415CEBEF36038B15311C"}, // 0x4e9ed8
	{"410C6BCC6B1955E7",    "72A843746C040A729BE367FDB098BDA55A3A7AC0"}, // 0x4e9f38
	{"7FDE7474632732B6",    "A33AD2B1C551DF46AA326907E8A46BAFB247AD6F"}, // 0x4e9f98
	{"194C5DAD668B6A1F",    "B27F78D14FE1DC5A554AF28E86EA72F237CA37A2"}, // 0x4e9ff8
	{"57345C3142CB10D1",    "80C5F1A714A1770BA73596476FF9CADEC8E5CA86"}, // 0x4ea058
	{"4D9E385821E450FB",    "DCC53FE06057F41870B2CB76A0DFBACFDC1A2BB7"}, // 0x4ea0b8
	{"5B4F533D233F6757",    "566C0D918EDBA687FF57B6A7A2726B0F7532BC68"}, // 0x4ea118
	{"189C71CE27167",       "01425F73772F99C48D3F11980C0016DBC14E26D6"}, // 0x4ea178
	{"287D66FF14E470FF",    "499F906AB24453D8137CF1EFA64A1A36857DE6D8"}, // 0x4ea1d8
	{"1FE2219A53232903",    "363DB5348491BC385F1E8EDA771206C40BAA4922"}, // 0x4ea238
	{"774AB73B393772",      "DC7FBD328772C83CEF8744A8F809EEF021C7FD96"}, // 0x4ea298
	{"3A9114466AB10A3",     "7B1FEDE82DBFCCF400C89B46A50C40C1842C8E81"}, // 0x4ea2f8
	{"6B092BA757643FCF",    "B4E5F9E172F3FD65218C300A15E45640DE211BDE"}, // 0x4ea358
	{"47CD58E96A6210D0",    "6470590097A8DB97B754B04C3D77DB2ACC370E03"}, // 0x4ea3b8
	{"7052A3F32B0728",      "75D86B22EB034624BE73793910060511F22F26A5"}, // 0x4ea418
	{"153F5F21944645",      "38B30824408A72727ECE8B3D0EC60C8BBEFE0CE4"}, // 0x4ea478
	{"2B7F71F159094F04",    "90C36BC164D29132305A85F10BDF5430AC7403BC"}, // 0x4ea4d8
	{"559464DF616F3673",    "3020E026AE91C4F02A071EC25B0F3C6E75A25C98"}, // 0x4ea538
	{"772E360A338239DB",    "0A54E1206556553F2EA42391626CF77080B91527"}, // 0x4ea598
	{"26FC1EE72D8953FE",    "E3A5EF789B16CE530ED9950E4A8F7F57F8A82949"}, // 0x4ea5f8
	{"72C22E712CF379D",     "326110BFB14F887A75D34556D62814C978C860B9"}, // 0x4ea658
	{"29AC31541057533D",    "0DE8BD01177976BAC9128FFC2EF712338325773B"}, // 0x4ea6b8
	{"20E27E7C79CC422B",    "CA13896938B28AEEF8CB2AE460F50F931E80A650"}, // 0x4ea718
	{"265C781252B74EBE",    "757F713CDEE0B8BA3071165B936C583B541DD594"}, // 0x4ea778
	{"15F838D177F569DC",    "38ADAAD5DF8775AEEF22B865506D1341C2A1DA57"}, // 0x4ea7d8
};

void RakPeer::OnConnectionRequest( RemoteSystemStruct *remoteSystem, unsigned char *AESKey, bool setAESKey )
{
	// Already handled by caller
	//if ( AllowIncomingConnections() )
	{
		// SA-MP does allocate 15 bytes for the bitstream, but only 13 bytes in used, changing that would fail to process request acception
		RakNet::BitStream bitStream(sizeof(unsigned char)+sizeof(unsigned short)+sizeof(unsigned int)+sizeof(unsigned short)+sizeof(PlayerIndex)+sizeof(unsigned int));
		bitStream.Write((unsigned char)ID_CONNECTION_REQUEST_ACCEPTED);
//		bitStream.Write((unsigned short)myPlayerId.port);
		bitStream.Write(remoteSystem->playerId.binaryAddress);
		bitStream.Write(remoteSystem->playerId.port);
		bitStream.Write(( PlayerIndex ) GetIndexFromPlayerID( remoteSystem->playerId, true ));
#ifdef SAMPSRV
		extern unsigned int _uiRndSrvChallenge;

		bitStream.Write((unsigned int)_uiRndSrvChallenge);
#else
		bitStream.Write((unsigned int)0);
#endif
		SendImmediate((char*)bitStream.GetData(), bitStream.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, remoteSystem->playerId, false, false, RakNet::GetTime());

		// Don't set secure connections immediately because we need the ack from the remote system to know ID_CONNECTION_REQUEST_ACCEPTED
		// As soon as a 16 byte packet arrives, we will turn on AES.  This works because all encrypted packets are multiples of 16 and the
		// packets I happen to be sending are less than 16 bytes
		remoteSystem->setAESKey=setAESKey;
		if ( setAESKey )
		{
			memcpy(remoteSystem->AESKey, AESKey, 16);
			remoteSystem->connectMode=RemoteSystemStruct::SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET;
		}
	}
	/*
	else
	{
		unsigned char c = ID_NO_FREE_INCOMING_CONNECTIONS;
		//SocketLayer::Instance()->SendTo( connectionSocket, ( char* ) & c, sizeof( char ), playerId.binaryAddress, playerId.port );

		SendImmediate((char*)&c, sizeof(c)*8, SYSTEM_PRIORITY, RELIABLE, 0, remoteSystem->systemAddress, false, false, RakNet::GetTime());
		remoteSystem->connectMode=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY;
	}
	*/
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::NotifyAndFlagForDisconnect( const PlayerID playerId, bool performImmediate, unsigned char orderingChannel )
{
	RakNet::BitStream temp( sizeof(unsigned char) );
	temp.Write( (unsigned char) ID_DISCONNECTION_NOTIFICATION );
	if (performImmediate)
	{
		SendImmediate((char*)temp.GetData(), temp.GetNumberOfBitsUsed(), LOW_PRIORITY, RELIABLE_ORDERED, orderingChannel, playerId, false, false, RakNet::GetTime());
		RemoteSystemStruct *rss=GetRemoteSystemFromPlayerID(playerId, true, true);
		rss->connectMode=RemoteSystemStruct::DISCONNECT_ASAP;
	}
	else
	{
		SendBuffered((const char*)temp.GetData(), temp.GetNumberOfBitsUsed(), LOW_PRIORITY, RELIABLE_ORDERED, orderingChannel, playerId, false, RemoteSystemStruct::DISCONNECT_ASAP);
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned short RakPeer::GetNumberOfRemoteInitiatedConnections( void ) const
{
	unsigned short i, numberOfIncomingConnections;

	if ( remoteSystemList == 0 || endThreads == true )
		return 0;

	numberOfIncomingConnections = 0;

	// remoteSystemList in network thread
	for ( i = 0; i < maximumNumberOfPeers; i++ )
	//for ( i = 0; i < remoteSystemListSize; i++ )
	{
		if ( remoteSystemList[ i ].isActive && remoteSystemList[ i ].weInitiatedTheConnection == false && remoteSystemList[i].connectMode==RemoteSystemStruct::CONNECTED)
			numberOfIncomingConnections++;
	}

	return numberOfIncomingConnections;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
RemoteSystemStruct * RakPeer::AssignPlayerIDToRemoteSystemList( const PlayerID playerId, RemoteSystemStruct::ConnectMode connectionMode )
{
	RemoteSystemStruct * remoteSystem;
	unsigned i,j;
	RakNet::Time time = RakNet::GetTime();
#ifdef _DEBUG
	assert(playerId!=UNASSIGNED_PLAYER_ID);
#endif

	// remoteSystemList in user thread
	for ( i = 0; i < maximumNumberOfPeers; i++ )
	//for ( i = 0; i < remoteSystemListSize; i++ )
	{
		if ( remoteSystemList[ i ].isActive==false )
		{
			remoteSystem=remoteSystemList+i;
			remoteSystem->rpcMap.Clear();
			remoteSystem->playerId = playerId;
			remoteSystem->isActive=true; // This one line causes future incoming packets to go through the reliability layer
			remoteSystem->reliabilityLayer.SetSplitMessageProgressInterval(splitMessageProgressInterval);
			remoteSystem->reliabilityLayer.SetUnreliableTimeout(unreliableTimeout);
			remoteSystem->reliabilityLayer.SetEncryptionKey( 0 );

			for ( j = 0; j < (unsigned) PING_TIMES_ARRAY_SIZE; j++ )
			{
				remoteSystem->pingAndClockDifferential[ j ].pingTime = 65535;
				remoteSystem->pingAndClockDifferential[ j ].clockDifferential = 0;
			}

			remoteSystem->connectMode=connectionMode;
			remoteSystem->pingAndClockDifferentialWriteIndex = 0;
			remoteSystem->lowestPing = 65535;
			remoteSystem->nextPingTime = 0; // Ping immediately
			remoteSystem->weInitiatedTheConnection = false;
			remoteSystem->staticData.Reset();
			remoteSystem->connectionTime = time;
			remoteSystem->myExternalPlayerId = UNASSIGNED_PLAYER_ID;
			remoteSystem->setAESKey=false;
			remoteSystem->lastReliableSend=time;

			// Reserve this reliability layer for ourselves.
			remoteSystem->reliabilityLayer.Reset(true);

			/// Add this player to the lookup tree
			PlayerIDAndIndex playerIDAndIndex;
			playerIDAndIndex.playerId=playerId;
			playerIDAndIndex.index=i;
			remoteSystemLookup.Insert(playerId,playerIDAndIndex);

			return remoteSystem;
		}
	}

	return 0;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Adjust the first four bytes (treated as unsigned int) of the pointer
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ShiftIncomingTimestamp( unsigned char *data, PlayerID playerId ) const
{
	RakAssert( IsActive() );
	RakAssert( data );

	RakNet::BitStream timeBS( data, sizeof(RakNet::Time), false);
	RakNet::Time encodedTimestamp;
	timeBS.Read(encodedTimestamp);

	encodedTimestamp = encodedTimestamp - GetBestClockDifferential( playerId );
	timeBS.SetWriteOffset(0);
	timeBS.Write(encodedTimestamp);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Thanks to Chris Taylor (cat02e@fsu.edu) for the improved timestamping algorithm
RakNet::Time RakPeer::GetBestClockDifferential( const PlayerID playerId ) const
{
	int counter, lowestPingSoFar;
	RakNet::Time clockDifferential;
	RemoteSystemStruct *remoteSystem = GetRemoteSystemFromPlayerID( playerId, true, true );

	if ( remoteSystem == 0 )
		return 0;

	lowestPingSoFar = 65535;

	clockDifferential = 0;

	for ( counter = 0; counter < PING_TIMES_ARRAY_SIZE; counter++ )
	{
		if ( remoteSystem->pingAndClockDifferential[ counter ].pingTime == 65535 )
			break;

		if ( remoteSystem->pingAndClockDifferential[ counter ].pingTime < lowestPingSoFar )
		{
			clockDifferential = remoteSystem->pingAndClockDifferential[ counter ].clockDifferential;
			lowestPingSoFar = remoteSystem->pingAndClockDifferential[ counter ].pingTime;
		}
	}

	return clockDifferential;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
// Handles an RPC packet.  If you get a packet with the ID ID_RPC you should pass it to this function
// This is already done in Multiplayer.cpp, so if you use the Multiplayer class it is handled for you.
//
// Parameters:
// packet - A packet returned from Receive with the ID ID_RPC
//
// Returns:
// true on success, false on a bad packet or an unregistered function
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning( disable : 4701 ) // warning C4701: local variable <variable name> may be used without having been initialized
#endif
bool RakPeer::HandleRPCPacket( const char *data, int length, PlayerID playerId, PlayerIndex playerIndex )
{
	// RPC BitStream format is
	// ID_RPC - unsigned char
	// Unique identifier string length - unsigned char
	// The unique ID  - string with each letter in upper case, subtracted by 'A' and written in 5 bits.
	// Number of bits of the data (int)
	// The data

	RakNet::BitStream incomingBitStream( (unsigned char *) data, length, false );
//	char uniqueIdentifier[ 256 ];
//	unsigned int bitLength;
	unsigned char *userData;
	//bool hasTimestamp;
	bool /*nameIsEncoded,*/ networkIDIsEncoded;
//	RPCIndex rpcIndex;
	UniqueID uniqueId;
	RPCNode *node;
	RPCParameters rpcParms;
	NetworkID networkID;
	bool blockingCommand;
	RakNet::BitStream replyToSender;
	rpcParms.replyToSender=&replyToSender;

	//rpcParms.recipient=this;
	rpcParms.sender=playerId;
	rpcParms.senderId=playerIndex;

	// Note to self - if I change this format then I have to change the PacketLogger class too
	incomingBitStream.IgnoreBits(8);
	if (data[0]==ID_TIMESTAMP)
		incomingBitStream.IgnoreBits(8*(sizeof(RakNet::Time)+sizeof(unsigned char)));

	//if ( incomingBitStream.ReadCompressed(uniqueId) == false )
	if (incomingBitStream.Read(uniqueId) == false)
	{
		RakAssert(0); // Not received an RPC unique id
		return false;
	}

	node = rpcMap.GetNodeFromID(uniqueId);
	if (node == 0)
	{
		RakAssert(0); // Should never happen except perhaps from threading errors?  No harm in checking anyway
		return false;
	}

	if ( incomingBitStream.Read( blockingCommand ) == false )
	{
		RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error
		return false;
	}

	/*
	if ( incomingBitStream.Read( rpcParms.hasTimestamp ) == false )
	{
#ifdef _DEBUG
		assert( 0 ); // bitstream was not long enough.  Some kind of internal error
#endif
		return false;
	}
	*/

	if ( incomingBitStream.ReadCompressed( rpcParms.numberOfBitsOfData ) == false )
	{
		RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error
		return false;
	}

	if ( incomingBitStream.Read( networkIDIsEncoded ) == false )
	{

		RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error
		return false;
	}

	if (networkIDIsEncoded)
	{
		if ( incomingBitStream.Read( networkID ) == false )
		{
			RakAssert( 0 ); // bitstream was not long enough.  Some kind of internal error
			return false;
		}
	}

	// Make sure the call type matches - if this is a pointer to a class member then networkID must be defined.  Otherwise it must not be defined
	if (node->isPointerToMember==true && networkIDIsEncoded==false)
	{
		// If this hits then this pointer was registered as a class member function but the packet does not have an NetworkID.
		// Most likely this means this system registered a function with REGISTER_CLASS_MEMBER_RPC and the remote system called it
		// using the unique ID for a function registered with REGISTER_STATIC_RPC.
		RakAssert(0);
		return false;
	}

	if (node->isPointerToMember==false && networkIDIsEncoded==true)
	{
		// If this hits then this pointer was not registered as a class member function but the packet does have an NetworkID.
		// Most likely this means this system registered a function with REGISTER_STATIC_RPC and the remote system called it
		// using the unique ID for a function registered with REGISTER_CLASS_MEMBER_RPC.
		RakAssert(0);
		return false;
	}

	// Call the function
	if ( rpcParms.numberOfBitsOfData == 0 )
	{
		rpcParms.input=0;
		if (networkIDIsEncoded)
		{
			void *object = NetworkIDGenerator::GET_OBJECT_FROM_ID(networkID);
			if (object)
				(node->memberFunctionPointer(object, &rpcParms));
		}
		else
		{
			node->staticFunctionPointer( &rpcParms );
		}
	}
	else
	{
		if ( incomingBitStream.GetNumberOfUnreadBits() == 0 )
		{
			RakAssert( 0 );
			return false; // No data was appended!
		}

		// We have to copy into a new data chunk because the user data might not be byte aligned.
		bool usedAlloca=false;
#if !defined(_COMPATIBILITY_1)
		if (BITS_TO_BYTES( incomingBitStream.GetNumberOfUnreadBits() ) < MAX_ALLOCA_STACK_ALLOCATION)
		{
			userData = ( unsigned char* ) alloca( BITS_TO_BYTES( incomingBitStream.GetNumberOfUnreadBits() ) );
			usedAlloca=true;
		}
		else
#endif
			userData = new unsigned char[BITS_TO_BYTES(incomingBitStream.GetNumberOfUnreadBits())];


		// The false means read out the internal representation of the bitstream data rather than
		// aligning it as we normally would with user data.  This is so the end user can cast the data received
		// into a bitstream for reading
		if ( incomingBitStream.ReadBits( ( unsigned char* ) userData, rpcParms.numberOfBitsOfData, false ) == false )
		{
			RakAssert( 0 );
			#if defined(_COMPATIBILITY_1)
			delete [] userData;
			#endif

			return false; // Not enough data to read
		}

//		if ( rpcParms.hasTimestamp )
//			ShiftIncomingTimestamp( userData, playerId );

		// Call the function callback
		rpcParms.input=userData;
		if (networkIDIsEncoded)
		{
			void *object = NetworkIDGenerator::GET_OBJECT_FROM_ID(networkID);
			if (object)
				(node->memberFunctionPointer(object, &rpcParms));
		}
		else
		{
			node->staticFunctionPointer( &rpcParms );
		}


		if (usedAlloca==false)
			delete [] userData;
	}

	if (blockingCommand)
	{
		RakNet::BitStream reply;
		reply.Write((unsigned char) ID_RPC_REPLY);
		reply.Write((char*)replyToSender.GetData(), replyToSender.GetNumberOfBytesUsed());
		Send(&reply, HIGH_PRIORITY, RELIABLE, 0, playerId, false);
	}

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
* Handles an RPC reply packet.  This is data returned from an RPC call
*
* \param data A packet returned from Receive with the ID ID_RPC
* \param length The size of the packet data
* \param playerId The sender of the packet
*
* \return true on success, false on a bad packet or an unregistered function
*/
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::HandleRPCReplyPacket( const char *data, int length, PlayerID playerId )
{
	if (blockOnRPCReply)
	{
		if ((playerId==replyFromTargetPlayer && replyFromTargetBroadcast==false) ||
			(playerId!=replyFromTargetPlayer && replyFromTargetBroadcast==true))
		{
			replyFromTargetBS->Write(data+1, length-1);
			blockOnRPCReply=false;
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::GenerateSYNCookieRandomNumber( void )
{
#if !defined(_COMPATIBILITY_1)
	unsigned int number;
	int i;
	memcpy( oldRandomNumber, newRandomNumber, sizeof( newRandomNumber ) );

	for ( i = 0; i < (int) sizeof( newRandomNumber ); i += (int) sizeof( number ) )
	{
		number = randomMT();
		memcpy( newRandomNumber + i, ( char* ) & number, sizeof( number ) );
	}

	randomNumberExpirationTime = RakNet::GetTime() + SYN_COOKIE_OLD_RANDOM_NUMBER_DURATION;
#endif
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SecuredConnectionResponse( const PlayerID playerId )
{
#if !defined(_COMPATIBILITY_1)
	CSHA1 sha1;
	RSA_BIT_SIZE n;
	big::u32 e;
	unsigned char connectionRequestResponse[ 1 + sizeof( big::u32 ) + sizeof( RSA_BIT_SIZE ) + 20 ];
	connectionRequestResponse[ 0 ] = ID_SECURED_CONNECTION_RESPONSE;

	if ( randomNumberExpirationTime < RakNet::GetTime() )
		GenerateSYNCookieRandomNumber();

	// Hash the SYN-Cookie
	// s2c syn-cookie = SHA1_HASH(source ip address + source port + random number)
	sha1.Reset();
	sha1.Update( ( unsigned char* ) & playerId.binaryAddress, sizeof( playerId.binaryAddress ) );
	sha1.Update( ( unsigned char* ) & playerId.port, sizeof( playerId.port ) );
	sha1.Update( ( unsigned char* ) & ( newRandomNumber ), 20 );
	sha1.Final();

	// Write the cookie
	memcpy( connectionRequestResponse + 1, sha1.GetHash(), 20 );

	// Write the public keys
	rsacrypt.getPublicKey( e, n );
#ifdef HOST_ENDIAN_IS_BIG
	// Mangle the keys on a Big-endian machine before sending
	BSWAPCPY( (unsigned char *)(connectionRequestResponse + 1 + 20),
		(unsigned char *)&e, sizeof( big::u32 ) );
	BSWAPCPY( (unsigned char *)(connectionRequestResponse + 1 + 20 + sizeof( big::u32 ) ),
		(unsigned char *)n, sizeof( RSA_BIT_SIZE ) );
#else
	memcpy( connectionRequestResponse + 1 + 20, ( char* ) & e, sizeof( big::u32 ) );
	memcpy( connectionRequestResponse + 1 + 20 + sizeof( big::u32 ), n, sizeof( RSA_BIT_SIZE ) );
#endif

	// s2c public key, syn-cookie
	//SocketLayer::Instance()->SendTo( connectionSocket, ( char* ) connectionRequestResponse, 1 + sizeof( big::u32 ) + sizeof( RSA_BIT_SIZE ) + 20, playerId.binaryAddress, playerId.port );
	// All secure connection requests are unreliable because the entire process needs to be restarted if any part fails.
	// Connection requests are resent periodically
	SendImmediate(( char* ) connectionRequestResponse, (1 + sizeof( big::u32 ) + sizeof( RSA_BIT_SIZE ) + 20) *8, SYSTEM_PRIORITY, UNRELIABLE, 0, playerId, false, false, RakNet::GetTime());
#endif
}

void RakPeer::SecuredConnectionConfirmation( RemoteSystemStruct * remoteSystem, char* data )
{
#if !defined(_COMPATIBILITY_1)
	int i, j;
	unsigned char randomNumber[ 20 ];
	unsigned int number;
	//bool doSend;
	Packet *packet;
	big::u32 e;
	RSA_BIT_SIZE n, message, encryptedMessage;
	big::RSACrypt<RSA_BIT_SIZE> privKeyPncrypt;

	// Make sure that we still want to connect
	if (remoteSystem->connectMode!=RemoteSystemStruct::REQUESTED_CONNECTION)
		return;

	// Copy out e and n
#ifdef HOST_ENDIAN_IS_BIG
	BSWAPCPY( (unsigned char *)&e, (unsigned char *)(data + 1 + 20), sizeof( big::u32 ) );
	BSWAPCPY( (unsigned char *)n, (unsigned char *)(data + 1 + 20 + sizeof( big::u32 )), sizeof( RSA_BIT_SIZE ) );
#else
	memcpy( ( char* ) & e, data + 1 + 20, sizeof( big::u32 ) );
	memcpy( n, data + 1 + 20 + sizeof( big::u32 ), sizeof( RSA_BIT_SIZE ) );
#endif

	// If we preset a size and it doesn't match, or the keys do not match, then tell the user
	if ( usingSecurity == true && keysLocallyGenerated == false )
	{
		if ( memcmp( ( char* ) & e, ( char* ) & publicKeyE, sizeof( big::u32 ) ) != 0 ||
			memcmp( n, publicKeyN, sizeof( RSA_BIT_SIZE ) ) != 0 )
		{
			packet=AllocPacket(1);
			packet->data[ 0 ] = ID_RSA_PUBLIC_KEY_MISMATCH;
			packet->bitSize = sizeof( char ) * 8;
			packet->playerId = remoteSystem->playerId;
			packet->playerIndex = ( PlayerIndex ) GetIndexFromPlayerID( packet->playerId, true );
			AddPacketToProducer(packet);
			remoteSystem->connectMode=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY;
			return;
		}
	}

	// Create a random number
	for ( i = 0; i < (int) sizeof( randomNumber ); i += (int) sizeof( number ) )
	{
		number = randomMT();
		memcpy( randomNumber + i, ( char* ) & number, sizeof( number ) );
	}

	memset( message, 0, sizeof( message ) );
	assert( sizeof( message ) >= sizeof( randomNumber ) );

#ifdef HOST_ENDIAN_IS_BIG
	// Scramble the plaintext message
	BSWAPCPY( (unsigned char *)message, randomNumber, sizeof(randomNumber) );
#else
	memcpy( message, randomNumber, sizeof( randomNumber ) );
#endif
	privKeyPncrypt.setPublicKey( e, n );
	privKeyPncrypt.encrypt( message, encryptedMessage );
#ifdef HOST_ENDIAN_IS_BIG
	// A big-endian machine needs to scramble the byte order of an outgoing (encrypted) message
	BSWAPSELF( (unsigned char *)encryptedMessage, sizeof( RSA_BIT_SIZE ) );
#endif

	/*
	rakPeerMutexes[ RakPeer::requestedConnections_MUTEX ].Lock();
	for ( i = 0; i < ( int ) requestedConnectionsList.Size(); i++ )
	{
		if ( requestedConnectionsList[ i ]->playerId == playerId )
		{
			doSend = true;
			// Generate the AES key

			for ( j = 0; j < 16; j++ )
				requestedConnectionsList[ i ]->AESKey[ j ] = data[ 1 + j ] ^ randomNumber[ j ];

			requestedConnectionsList[ i ]->setAESKey = true;

			break;
		}
	}
	rakPeerMutexes[ RakPeer::requestedConnections_MUTEX ].Unlock();
	*/

	// Take the remote system's AESKey and XOR with our random number.
		for ( j = 0; j < 16; j++ )
			remoteSystem->AESKey[ j ] = data[ 1 + j ] ^ randomNumber[ j ];
	remoteSystem->setAESKey = true;

//	if ( doSend )
//	{
		char reply[ 1 + 20 + sizeof( RSA_BIT_SIZE ) ];
		// c2s RSA(random number), same syn-cookie
		reply[ 0 ] = ID_SECURED_CONNECTION_CONFIRMATION;
		memcpy( reply + 1, data + 1, 20 );  // Copy the syn-cookie
		memcpy( reply + 1 + 20, encryptedMessage, sizeof( RSA_BIT_SIZE ) ); // Copy the encoded random number

		//SocketLayer::Instance()->SendTo( connectionSocket, reply, 1 + 20 + sizeof( RSA_BIT_SIZE ), playerId.binaryAddress, playerId.port );
		// All secure connection requests are unreliable because the entire process needs to be restarted if any part fails.
		// Connection requests are resent periodically
		SendImmediate((char*)reply, (1 + 20 + sizeof( RSA_BIT_SIZE )) * 8, SYSTEM_PRIORITY, UNRELIABLE, 0, remoteSystem->playerId, false, false, RakNet::GetTime());
//	}

#endif
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::AllowIncomingConnections(void) const
{
	return GetNumberOfRemoteInitiatedConnections() < GetMaximumIncomingConnections();
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SendStaticDataInternal( const PlayerID target, bool performImmediate )
{
	RakNet::BitStream reply( sizeof(unsigned char) + localStaticData.GetNumberOfBytesUsed() );
	reply.Write( (unsigned char) ID_RECEIVED_STATIC_DATA );
	reply.Write( (char*)localStaticData.GetData(), localStaticData.GetNumberOfBytesUsed() );

	if (performImmediate)
	{
		if ( target == UNASSIGNED_PLAYER_ID )
			SendImmediate( (char*)reply.GetData(), reply.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, target, true, false, RakNet::GetTime() );
		else
			SendImmediate( (char*)reply.GetData(), reply.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, target, false, false, RakNet::GetTime() );
	}
	else
	{
		if ( target == UNASSIGNED_PLAYER_ID )
			Send( &reply, SYSTEM_PRIORITY, RELIABLE, 0, target, true );
		else
			Send( &reply, SYSTEM_PRIORITY, RELIABLE, 0, target, false );
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::PingInternal( const PlayerID target, bool performImmediate )
{
	if ( IsActive() == false )
		return ;

	RakNet::BitStream bitStream(sizeof(unsigned char)+sizeof(RakNet::Time));
	bitStream.Write((unsigned char)ID_INTERNAL_PING);
	RakNet::Time64 currentTimeNS = RakNet::GetTime64();
	RakNet::Time currentTime = RakNet::GetTime();
	bitStream.Write(currentTime);
	if (performImmediate)
		SendImmediate( (char*)bitStream.GetData(), bitStream.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, UNRELIABLE, 0, target, false, false, currentTimeNS );
	else
		Send( &bitStream, SYSTEM_PRIORITY, UNRELIABLE, 0, target, false );
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::CloseConnectionInternal( const PlayerID target, bool sendDisconnectionNotification, bool performImmediate, unsigned char orderingChannel )
{
	unsigned i,j;

	RakAssert(orderingChannel >=0 && orderingChannel < 32);

	if (target==UNASSIGNED_PLAYER_ID)
		return;

	if ( remoteSystemList == 0 || endThreads == true )
		return;

	if (sendDisconnectionNotification)
	{
		NotifyAndFlagForDisconnect(target, performImmediate, orderingChannel);
	}
	else
	{
		if (performImmediate)
		{
			i = 0;
			// remoteSystemList in user thread
			for ( ; i < maximumNumberOfPeers; i++ )
				//for ( ; i < remoteSystemListSize; i++ )
			{
				if ( remoteSystemList[ i ].isActive && remoteSystemList[ i ].playerId == target )
				{
					// Found the index to stop
					remoteSystemList[ i ].isActive=false;

					// Reserve this reliability layer for ourselves
					//remoteSystemList[ i ].playerId = UNASSIGNED_PLAYER_ID;
					
					for (j=0; j < messageHandlerList.Size(); j++)
					{
						messageHandlerList[j]->OnCloseConnection(this, target);
					}

					// Clear any remaining messages
					remoteSystemList[ i ].reliabilityLayer.Reset(false);

					// Remove from the lookup list
					remoteSystemLookup.Remove(target);

					break;
				}
			}
		}
		else
		{
			BufferedCommandStruct *bcs;
#ifdef _RAKNET_THREADSAFE
			rakPeerMutexes[bufferedCommands_Mutex].Lock();
#endif
			bcs=bufferedCommands.WriteLock();
			bcs->command=BufferedCommandStruct::BCS_CLOSE_CONNECTION;
			bcs->playerId=target;
			bcs->data=0;
			bcs->orderingChannel=orderingChannel;
			bufferedCommands.WriteUnlock();
#ifdef _RAKNET_THREADSAFE
			rakPeerMutexes[bufferedCommands_Mutex].Unlock();
#endif
		}
	}
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::ValidSendTarget(PlayerID playerId, bool broadcast)
{
	unsigned remoteSystemIndex;

	// remoteSystemList in user thread.  This is slow so only do it in debug
	for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; remoteSystemIndex++ )
	//for ( remoteSystemIndex = 0; remoteSystemIndex < remoteSystemListSize; remoteSystemIndex++ )
	{
		if ( remoteSystemList[ remoteSystemIndex ].isActive &&
			remoteSystemList[ remoteSystemIndex ].connectMode==RemoteSystemStruct::CONNECTED && // Not fully connected players are not valid user-send targets because the reliability layer wasn't reset yet
			( ( broadcast == false && remoteSystemList[ remoteSystemIndex ].playerId == playerId ) ||
			( broadcast == true && remoteSystemList[ remoteSystemIndex ].playerId != playerId ) )
			)
			return true;
	}

	return false;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::SendBuffered( const char *data, int numberOfBitsToSend, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, RemoteSystemStruct::ConnectMode connectionMode )
{
#ifdef _DEBUG
	assert(orderingChannel >=0 && orderingChannel < 32);
#endif


	BufferedCommandStruct *bcs;

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Lock();
#endif
	bcs=bufferedCommands.WriteLock();
	bcs->data = new char[BITS_TO_BYTES(numberOfBitsToSend)]; // Making a copy doesn't lose efficiency because I tell the reliability layer to use this allocation for its own copy
#ifdef _DEBUG
	assert(bcs->data);
#endif
	memcpy(bcs->data, data, BITS_TO_BYTES(numberOfBitsToSend));
    bcs->numberOfBitsToSend=numberOfBitsToSend;
	bcs->priority=priority;
	bcs->reliability=reliability;
	bcs->orderingChannel=orderingChannel;
	bcs->playerId=playerId;
	bcs->broadcast=broadcast;
	bcs->connectionMode=connectionMode;
	bcs->command=BufferedCommandStruct::BCS_SEND;
	bufferedCommands.WriteUnlock();

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Unlock();
#endif
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::SendImmediate( char *data, int numberOfBitsToSend, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, bool useCallerDataAllocation, RakNet::Time64 currentTime )
{
	unsigned *sendList;
	unsigned sendListSize;
	bool callerDataAllocationUsed;
	unsigned remoteSystemIndex, sendListIndex; // Iterates into the list of remote systems
	unsigned numberOfBytesUsed = BITS_TO_BYTES(numberOfBitsToSend);
	callerDataAllocationUsed=false;

	sendListSize=0;

	// 03/06/06 - If broadcast is false, use the optimized version of GetIndexFromPlayerID
	if (broadcast==false)
	{
#if !defined(_COMPATIBILITY_1)
		sendList=(unsigned *)alloca(sizeof(unsigned));
#else
		sendList = new unsigned[1];
#endif
		remoteSystemIndex=GetIndexFromPlayerID( playerId, true );
		if (remoteSystemIndex!=(unsigned)-1 &&
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ASAP && 
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY && 
			remoteSystemList[remoteSystemIndex].connectMode!=RemoteSystemStruct::DISCONNECT_ON_NO_ACK)
		{
			sendList[0]=remoteSystemIndex;
			sendListSize=1;
		}
	}
	else
	{
#if !defined(_COMPATIBILITY_1)
	//sendList=(unsigned *)alloca(sizeof(unsigned)*remoteSystemListSize);
		sendList=(unsigned *)alloca(sizeof(unsigned)*maximumNumberOfPeers);
#else
	//sendList = new unsigned[remoteSystemListSize];
		sendList = new unsigned[maximumNumberOfPeers];
#endif

		// remoteSystemList in network thread
		for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; remoteSystemIndex++ )
		//for ( remoteSystemIndex = 0; remoteSystemIndex < remoteSystemListSize; remoteSystemIndex++ )
		{
			if ( remoteSystemList[ remoteSystemIndex ].isActive && remoteSystemList[ remoteSystemIndex ].playerId != playerId && remoteSystemList[ remoteSystemIndex ].playerId != UNASSIGNED_PLAYER_ID )
				sendList[sendListSize++]=remoteSystemIndex;
		}
	}

	if (sendListSize==0)
	{
#if defined(_COMPATIBILITY_1)
		delete [] sendList;
#endif
		return false;
	}

	for (sendListIndex=0; sendListIndex < sendListSize; sendListIndex++)
	{
		if ( trackFrequencyTable )
		{
			unsigned i;
			// Store output frequency
			for (i=0 ; i < numberOfBytesUsed; i++ )
				frequencyTable[ (unsigned char)(data[i]) ]++;
			rawBytesSent += numberOfBytesUsed;
		}

		if ( outputTree )
		{
			RakNet::BitStream bitStreamCopy( numberOfBytesUsed );
			outputTree->EncodeArray( (unsigned char*) data, numberOfBytesUsed, &bitStreamCopy );
			compressedBytesSent += bitStreamCopy.GetNumberOfBytesUsed();
			remoteSystemList[sendList[sendListIndex]].reliabilityLayer.Send( (char*) bitStreamCopy.GetData(), bitStreamCopy.GetNumberOfBitsUsed(), priority, reliability, orderingChannel, true, MTUSize, currentTime );
		}
		else
		{
			// Send may split the packet and thus deallocate data.  Don't assume data is valid if we use the callerAllocationData
			bool useData = useCallerDataAllocation && callerDataAllocationUsed==false && sendListIndex+1==sendListSize;
			remoteSystemList[sendList[sendListIndex]].reliabilityLayer.Send( data, numberOfBitsToSend, priority, reliability, orderingChannel, useData==false, MTUSize, currentTime );
			if (useData)
				callerDataAllocationUsed=true;
		}

		if (reliability==RELIABLE || reliability==RELIABLE_ORDERED || reliability==RELIABLE_SEQUENCED)
			remoteSystemList[sendList[sendListIndex]].lastReliableSend=(RakNet::Time)(currentTime/(RakNet::Time64)1000);
	}

#if defined(_COMPATIBILITY_1)
	delete [] sendList;
#endif

	// Return value only meaningful if true was passed for useCallerDataAllocation.  Means the reliability layer used that data copy, so the caller should not deallocate it
	return callerDataAllocationUsed;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
#ifdef _MSC_VER
#pragma warning( disable : 4701 ) // warning C4701: local variable <variable name> may be used without having been initialized
#endif
bool RakPeer::HandleBufferedRPC(BufferedCommandStruct *bcs, RakNetTime time)
{
	unsigned *sendList;
	bool callerAllocationDataUsed;
	unsigned sendListSize;

	// All this code modifies bcs->data and bcs->numberOfBitsToSend in order to transform an RPC request into an actual packet for 4
	RPCIndex rpcIndex; // Index into the list of RPC calls so we know what number to encode in the packet
	char uniqueID[256], *userData; // RPC ID (the name of it) and a pointer to the data sent by the user
	int extraBuffer; // How many data bytes were allocated to hold the RPC header
	unsigned remoteSystemIndex, sendListIndex; // Iterates into the list of remote systems
	int dataBlockAllocationLength; // Total number of bytes to allocate for the packet
	char *writeTarget; // Used to hold either a block of allocated data or the externally allocated data

	strcpy(uniqueID, bcs->data); // Copy out the string because it is at the front of the data block and will be overwritten
	extraBuffer=2+(int)strlen(uniqueID)*2+3; // Exact code copied earlier in this file.  Keep these two in synch!
	userData=bcs->data+extraBuffer;
	dataBlockAllocationLength=BITS_TO_BYTES(bcs->numberOfBitsToSend)+extraBuffer;

	sendListSize=0;

	// 03/06/06 - If broadcast is false, use the optimized version of GetIndexFromPlayerID
	if (bcs->broadcast==false)
	{
#if !defined(_COMPATIBILITY_1)
		sendList=(unsigned *)alloca(sizeof(unsigned));
#else
		sendList = new unsigned[1];
#endif
		remoteSystemIndex=GetIndexFromPlayerID( bcs->playerId, true );
		if (remoteSystemIndex!=(unsigned)-1)
		{
			sendList[0]=remoteSystemIndex;
			sendListSize=1;
		}
	}
	else
	{
#if !defined(_COMPATIBILITY_1)
	sendList=(unsigned *)alloca(sizeof(unsigned)*maximumNumberOfPeers);
#else
	sendList = new unsigned[maximumNumberOfPeers];
#endif

		for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; remoteSystemIndex++ )
		{
			if ( remoteSystemList[ remoteSystemIndex ].playerId != UNASSIGNED_PLAYER_ID && remoteSystemList[ remoteSystemIndex ].playerId != bcs->playerId )
				sendList[sendListSize++]=remoteSystemIndex;
		}
	}

	if (sendListSize==0)
	{
		#if defined(_COMPATIBILITY_1)
		delete [] sendList;
		#endif

		return false;
	}

	// remoteSystemList in network thread
	for (sendListIndex=0; sendListIndex < (unsigned)sendListSize; sendListIndex++)
	{
		if (sendListIndex+1==sendListSize)
			writeTarget=bcs->data; // Write to the externally allocated buffer.  This destroys the buffer format so we do it only once for the last call
		else
			writeTarget=new char [dataBlockAllocationLength]; // Create a new buffer

		// Last send so use the buffer that was allocated externally
		RakNet::BitStream outgoingBitStream((unsigned char *) writeTarget, dataBlockAllocationLength, false );
		outgoingBitStream.ResetWritePointer(); // Let us write at the start of the data block, rather than at the end

		outgoingBitStream.Write( (unsigned char) ID_RPC );
		rpcIndex=remoteSystemList[sendList[sendListIndex]].rpcMap.GetIndexFromFunctionName(uniqueID); // Lots of trouble but we can only use remoteSystem->[whatever] in this thread so that is why this command was buffered
		if (rpcIndex!=UNDEFINED_RPC_INDEX)
		{
			// We have an RPC name to an index mapping, so write the index
			outgoingBitStream.Write(false);
			outgoingBitStream.WriteCompressed(rpcIndex);
		}
		else
		{
			// No mapping, so write the encoded RPC name
			outgoingBitStream.Write(true);
			stringCompressor->EncodeString(uniqueID, 256, &outgoingBitStream);
		}
		outgoingBitStream.Write(bcs->blockingCommand);
		outgoingBitStream.Write((bool)(bcs->command==BufferedCommandStruct::BCS_RPC_SHIFT)); // True or false to shift the timestamp
		outgoingBitStream.WriteCompressed( bcs->numberOfBitsToSend );
		if (bcs->networkID==UNASSIGNED_NETWORK_ID)
		{
			// No object ID
			outgoingBitStream.Write(false);
		}
		else
		{
			// Encode an object ID.  This will use pointer to class member RPC
			outgoingBitStream.Write(true);
			outgoingBitStream.Write(bcs->networkID);
		}


		if ( bcs->numberOfBitsToSend > 0 )
			outgoingBitStream.WriteBits( (const unsigned char*) userData, bcs->numberOfBitsToSend, false ); // Last param is false to write the raw data originally from another bitstream, rather than shifting from user data
		else
			outgoingBitStream.WriteCompressed( ( int ) 0 );

		callerAllocationDataUsed=SendImmediate((char*)outgoingBitStream.GetData(), outgoingBitStream.GetNumberOfBitsUsed(), bcs->priority, bcs->reliability, bcs->orderingChannel, remoteSystemList[sendList[sendListIndex]].playerId, false, true, time);
	}

#if defined(_COMPATIBILITY_1)
	delete [] sendList;
#endif

	return callerAllocationDataUsed;
}
*/
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ClearBufferedCommands(void)
{
	BufferedCommandStruct *bcs;

#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Lock();
#endif
	while ((bcs=bufferedCommands.ReadLock())!=0)
	{
		if (bcs->data)
			delete [] bcs->data;

        bufferedCommands.ReadUnlock();
	}
	bufferedCommands.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[bufferedCommands_Mutex].Unlock();
#endif
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void RakPeer::ClearRequestedConnectionList(void)
{
	RequestedConnectionStruct *bcs;
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Lock();
#endif
	while ((bcs=requestedConnectionList.ReadLock())!=0)
	{
		if (bcs->data)
			delete [] bcs->data;

		requestedConnectionList.ReadUnlock();
	}
	requestedConnectionList.Clear();
#ifdef _RAKNET_THREADSAFE
	rakPeerMutexes[requestedConnectionList_Mutex].Unlock();
#endif
}
inline void RakPeer::AddPacketToProducer(Packet *p)
{
	Packet **packetPtr=packetSingleProducerConsumer.WriteLock();
	*packetPtr=p;
	packetSingleProducerConsumer.WriteUnlock();
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
#ifdef _WIN32
unsigned __stdcall RecvFromNetworkLoop(LPVOID arguments)
#else
void*  RecvFromNetworkLoop( void*  arguments )
#endif
{
RakPeer *peer = (RakPeer *)arguments;
unsigned int errorCode;

peer->isRecvfromThreadActive=true;

while(peer->endThreads==false)
{
peer->isSocketLayerBlocking=true;
errorCode=SocketLayer::Instance()->RecvFrom(peer->connectionSocket, peer);
peer->isSocketLayerBlocking=false;

#ifdef _WIN32
if (errorCode==WSAECONNRESET)
{
peer->PushPortRefused(UNASSIGNED_PLAYER_ID);
//closesocket(peer->connectionSocket);
//peer->connectionSocket = SocketLayer::Instance()->CreateBoundSocket(peer->myPlayerId.port, true);
}
else if (errorCode!=0 && peer->endThreads==false)
{
#ifdef _DEBUG
printf("Server RecvFrom critical failure!\n");
#endif
// Some kind of critical error
peer->isRecvfromThreadActive=false;
peer->endThreads=true;
peer->Disconnect();
break;
}
#else
if (errorCode==-1)
{
peer->isRecvfromThreadActive=false;
peer->endThreads=true;
peer->Disconnect();
break;
}
#endif
}

peer->isRecvfromThreadActive=false;

#ifdef _WIN32
//_endthreadex( 0 );
#endif
return 0;
}
*/
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning( disable : 4100 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
#ifdef _WIN32
void __stdcall ProcessPortUnreachable( unsigned int binaryAddress, unsigned short port, RakPeer *rakPeer )
#else
void ProcessPortUnreachable( unsigned int binaryAddress, unsigned short port, RakPeer *rakPeer )
#endif
{
	
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef _WIN32
void __stdcall ProcessNetworkPacket( const unsigned int binaryAddress, const unsigned short port, const char *data, const int length, RakPeer *rakPeer )
#else
void ProcessNetworkPacket( const unsigned int binaryAddress, const unsigned short port, const char *data, const int length, RakPeer *rakPeer )
#endif
{
	Packet *packet;
	PlayerID playerId;
	unsigned i;
	RemoteSystemStruct *remoteSystem;
	playerId.binaryAddress = binaryAddress;
	playerId.port = port;

#if !defined(_COMPATIBILITY_1)
	if (rakPeer->IsBanned( rakPeer->PlayerIDToDottedIP( playerId ) ))
	{
		for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
			rakPeer->messageHandlerList[i]->OnDirectSocketReceive(data, length*8, playerId);

		char c[2];
		c[0] = ID_CONNECTION_BANNED;
		c[1] = 0; // Pad, some routers apparently block 1 byte packets

		unsigned i;
		for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
			rakPeer->messageHandlerList[i]->OnDirectSocketSend((char*)&c, 16, playerId);
		SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, (char*)&c, 2, playerId.binaryAddress, playerId.port );

		return;
	}
#endif

	//printf("ProcessNetPacket(0x%X,%u)\n",rakPeer->myPlayerId.binaryAddress,rakPeer->myPlayerId.port);
    
	// We didn't check this datagram to see if it came from a connected system or not yet.
	// Therefore, this datagram must be under 17 bits - otherwise it may be normal network traffic as the min size for a raknet send is 17 bits
	if ((unsigned char)(data)[0] == (unsigned char) ID_OPEN_CONNECTION_REPLY && length <= sizeof(unsigned char)*2)
	{
		for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
			rakPeer->messageHandlerList[i]->OnDirectSocketReceive(data, length*8, playerId);

		// Verify that we were waiting for this
	//	bool acceptOpenConnection;
//		int actionToTake=0;
//		char data[MAX_OFFLINE_DATA_LENGTH];
		RakPeer::RequestedConnectionStruct *rcsFirst, *rcs;
		rcsFirst = rakPeer->requestedConnectionList.ReadLock();
		rcs=rcsFirst;
	//	acceptOpenConnection=false;
		while (rcs)
		{
			// Scan through the requested connection queue and process any elements whose playerId matches the player we just got this packet from.
			// If it is the first element in the queue, remove it from the queue.  Otherwise, set the playerId to UNASSIGNED_PLAYER_ID to cancel it out of order.
			if (rcs->playerId==playerId)
			{
				// Go ahead and process this request
			//	acceptOpenConnection=true;

				// Store the action (may be multiple actions to take at once)
		//		actionToTake|=(int)rcs->actionToTake;
				assert(rcs->actionToTake==RakPeer::RequestedConnectionStruct::CONNECT);

				// You might get this when already connected because of cross-connections
				remoteSystem=rakPeer->GetRemoteSystemFromPlayerID( playerId, true, true );
				if (remoteSystem==0)
					remoteSystem=rakPeer->AssignPlayerIDToRemoteSystemList(playerId, RemoteSystemStruct::UNVERIFIED_SENDER);

				if (remoteSystem)
				{
					RakNet::Time64 time = RakNet::GetTime64();
					remoteSystem->connectMode=RemoteSystemStruct::REQUESTED_CONNECTION;
					remoteSystem->weInitiatedTheConnection=true;

					RakNet::BitStream temp;

					temp.Write( (unsigned char) ID_CONNECTION_REQUEST );
					if ( rcs->outgoingPasswordLength > 0 )
						temp.Write( ( char* ) rcs->outgoingPassword,  rcs->outgoingPasswordLength );
					rakPeer->SendImmediate((char*)temp.GetData(), temp.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, playerId, false, false, time );
				}

				if (rcs==rcsFirst)
				{
					// Delete the head of the queue
					rakPeer->requestedConnectionList.ReadUnlock();
					rcsFirst=rakPeer->requestedConnectionList.ReadLock();
					rcs=rcsFirst;
					continue;
				}
				else
				{
					// Cancel this out of order element of the queue - we are handling it now.
					rcs->playerId=UNASSIGNED_PLAYER_ID;
				}
			}

			rcs=rakPeer->requestedConnectionList.ReadLock();
		}

		// Go back to the current head of the queue
		if (rcsFirst)
			rakPeer->requestedConnectionList.CancelReadLock(rcsFirst);

		/*
		if (acceptOpenConnection)
		{
			// You might get this when already connected because of cross-connections
			remoteSystem=rakPeer->GetRemoteSystemFromPlayerID( playerId, true );
			if (remoteSystem==0)
			{
				remoteSystem=rakPeer->AssignPlayerIDToRemoteSystemList(playerId, RakPeer::RemoteSystemStruct::UNVERIFIED_SENDER);
			}
			if (remoteSystem)
			{
				RakNetTime time = RakNet::GetTime();
				if (actionToTake & RakPeer::RequestedConnectionStruct::CONNECT)
				{
					remoteSystem->connectMode=RakPeer::RemoteSystemStruct::REQUESTED_CONNECTION;
					remoteSystem->weInitiatedTheConnection=true;

					RakNet::BitStream temp;

					temp.Write( (unsigned char) ID_CONNECTION_REQUEST );
					if ( rakPeer->outgoingPasswordLength > 0 )
						temp.Write( ( char* ) rakPeer->outgoingPassword,  rakPeer->outgoingPasswordLength );
					rakPeer->SendImmediate((char*)temp.GetData(), temp.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, playerId, false, false, time );
				}
			}
		}
		*/

		return;
	}
	else if (((unsigned char)(data)[0] == (MessageID)ID_CONNECTION_ATTEMPT_FAILED ||
			  (unsigned char)(data)[0] == (MessageID)ID_NO_FREE_INCOMING_CONNECTIONS ||
			  (unsigned char)(data)[0] == (MessageID)ID_CONNECTION_BANNED)
				&& length <= sizeof(unsigned char)*2)
	{
		// Remove the connection attempt from the buffered commands
		RakPeer::RequestedConnectionStruct *rcsFirst, *rcs;
		rcsFirst = rakPeer->requestedConnectionList.ReadLock();
		rcs=rcsFirst;
		bool connectionAttemptCancelled=false;
		while (rcs)
		{
			if (rcs->actionToTake==RakPeer::RequestedConnectionStruct::CONNECT && rcs->playerId==playerId)
			{
				connectionAttemptCancelled=true;
				if (rcs==rcsFirst)
				{
					rakPeer->requestedConnectionList.ReadUnlock();
					rcsFirst=rakPeer->requestedConnectionList.ReadLock();
					rcs=rcsFirst;
				}
				else
				{
					// Hole in the middle
					rcs->playerId=UNASSIGNED_PLAYER_ID;
					rcs=rakPeer->requestedConnectionList.ReadLock();
				}

				continue;
			}

			rcs=rakPeer->requestedConnectionList.ReadLock();
		}

		if (rcsFirst)
			rakPeer->requestedConnectionList.CancelReadLock(rcsFirst);

		if (connectionAttemptCancelled)
		{
			// Tell user of connection attempt failed
			packet=AllocPacket(sizeof( char ));
			packet->data[ 0 ] = data[0]; // Attempted a connection and couldn't
			packet->bitSize = ( sizeof( char ) * 8);
			packet->playerId = playerId;
			packet->playerIndex = 65535;
			rakPeer->AddPacketToProducer(packet);
		}
	}
#ifndef SAMPSRV
	// Client only - Original placement is the end of the function
	else if ((unsigned char)(data)[0] == ID_OPEN_CONNECTION_COOKIE_REQUEST && length == sizeof(unsigned char) * 3)
	{
#ifdef DEBUG
		OutputDebugString("Cookie exchanged!");
#endif
		uiIncomeCookieExchange = *(unsigned short*)&data[1];
	}
#endif
	// We didn't check this datagram to see if it came from a connected system or not yet.
	// Therefore, this datagram must be under 17 bits - otherwise it may be normal network traffic as the min size for a raknet send is 17 bits
	else if ((unsigned char)(data)[0] == ID_OPEN_CONNECTION_REQUEST && length == sizeof(unsigned char)*3)
	{
#ifdef SAMPSRV
		extern int iConnCookies;
		extern int iCookieLogging;
		extern unsigned int _uiRndCookieChallenge;

		// TODO: Maybe add attempt count? Be like this, connected clients will get stuck in a loop requesting.
		if (iConnCookies)
		{
			unsigned short a = *(unsigned short*)&data[1];
			unsigned short b = (unsigned short)_uiRndCookieChallenge ^ (unsigned short)binaryAddress;

			if ((a ^ COOKIE_XOR_KEY) != b)
			{
				if (iCookieLogging)
					logprintf("[connection] %s requests connection cookie.", playerId.ToString());

				char c[3];
				c[0] = ID_OPEN_CONNECTION_COOKIE_REQUEST;
				*(unsigned short*)&c[1] = b;

				unsigned i;
				for (i = 0; i < rakPeer->messageHandlerList.Size(); i++)
					rakPeer->messageHandlerList[i]->OnDirectSocketSend((char*)&c, 24, playerId);

				SocketLayer::Instance()->SendTo(rakPeer->connectionSocket, (char*)&c, 3, playerId.binaryAddress, playerId.port);
				return;
			}
		}
#endif

		for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
			rakPeer->messageHandlerList[i]->OnDirectSocketReceive(data, length*8, playerId);

		// If this guy is already connected and they initiated the connection, ignore the connection request
		RemoteSystemStruct *rss = rakPeer->GetRemoteSystemFromPlayerID( playerId, true, true );
		if (rss==0 || rss->weInitiatedTheConnection==true)
		{
			// Assign new remote system
			if (rss==0)
				rss=rakPeer->AssignPlayerIDToRemoteSystemList(playerId, RemoteSystemStruct::UNVERIFIED_SENDER);

			unsigned char c[2];
			if (rss) // If this guy is already connected remote system will be 0
				c[0] = ID_OPEN_CONNECTION_REPLY;
			else
				c[0] = ID_NO_FREE_INCOMING_CONNECTIONS;
			c[1] = 0; // Pad, some routers apparently block 1 byte packets

			unsigned i;
			for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
				rakPeer->messageHandlerList[i]->OnDirectSocketSend((char*)&c, 16, playerId);
			SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, (char*)&c, 2, playerId.binaryAddress, playerId.port );

			return;
		}
		else if (rss!=0)
		{
			// If this is an existing connection, and they are already fully connected (not in progress), reply with connection attempt failed
			if (rss->connectMode==RemoteSystemStruct::CONNECTED ||
				rss->connectMode==RemoteSystemStruct::DISCONNECT_ASAP ||
				rss->connectMode==RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY)
			{
				char c[2];
				c[0] = ID_CONNECTION_ATTEMPT_FAILED;
				c[1] = 0; // Pad, some routers apparently block 1 byte packets

				unsigned i;
				for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
					rakPeer->messageHandlerList[i]->OnDirectSocketSend((char*)&c, 16, playerId);
				SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, (char*)&c, 2, playerId.binaryAddress, playerId.port );
			}
		}

	}

	// See if this datagram came from a connected system
	remoteSystem = rakPeer->GetRemoteSystemFromPlayerID( playerId, true, true );
	if ( remoteSystem )
	{
#ifdef TEA_ENCRYPTOR
		//char szBuffer[32];
/*
#ifdef SAMPSRV
		sprintf(szBuffer, "SvLength: %d\n", length);
#else
		sprintf(szBuffer, "ClLength: %d\n", length);
#endif
		OutputDebugString(szBuffer);
		if (remoteSystem->connectMode==RemoteSystemStruct::SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET)
			OutputDebugString("Boo\n");
*/
		if (remoteSystem->connectMode==RemoteSystemStruct::SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET && length >= 8 && (length%8)==0)
		{
			remoteSystem->reliabilityLayer.SetEncryptionKey( remoteSystem->AESKey );
/*
#ifdef SAMPSRV
			OutputDebugString("SvSet\n");
#else
			OutputDebugString("ClSet\n");
#endif
*/
		}
#else
		if (remoteSystem->connectMode==RemoteSystemStruct::SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET && (length%16)==0)
			remoteSystem->reliabilityLayer.SetEncryptionKey( remoteSystem->AESKey );
#endif

		// Handle regular incoming data
		// HandleSocketReceiveFromConnectedPlayer is only safe to be called from the same thread as Update, which is this thread
		if ( remoteSystem->reliabilityLayer.HandleSocketReceiveFromConnectedPlayer( data, length, playerId, rakPeer->messageHandlerList, rakPeer->MTUSize ) == false )
		{
			// These kinds of packets may have been duplicated and incorrectly determined to be
			// cheat packets.  Anything else really is a cheat packet
			if ( !(
			( (unsigned char)data[0] == ID_OPEN_CONNECTION_REQUEST && length <= 3 ) ||
			( (unsigned char)data[0] == ID_OPEN_CONNECTION_COOKIE_REQUEST && length <= 3) ||
			( (unsigned char)data[0] == ID_OPEN_CONNECTION_REPLY && length <= 2 ) ||
			( (unsigned char)data[0] == ID_CONNECTION_ATTEMPT_FAILED && length <= 2 ) ||
			( ((unsigned char)data[0] == ID_PING_OPEN_CONNECTIONS || (unsigned char)data[0] == ID_PING || (unsigned char)data[0] == ID_PONG) && length >= sizeof(unsigned char)+sizeof(RakNet::Time) ) ||
			( (unsigned char)data[0] == ID_ADVERTISE_SYSTEM && length<MAX_OFFLINE_DATA_LENGTH )
			) )
			{
				// Unknown message.  Could be caused by old out of order stuff from unconnected or no longer connected systems, etc.
				packet=AllocPacket(1);
				packet->data[ 0 ] = ID_MODIFIED_PACKET;
				packet->bitSize = sizeof( char ) * 8;
				packet->playerId = playerId;
				packet->playerIndex = ( PlayerIndex ) rakPeer->GetIndexFromPlayerID( playerId, true );
				rakPeer->AddPacketToProducer(packet);
			}
		}
	}
	else
	{
		for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
			rakPeer->messageHandlerList[i]->OnDirectSocketReceive(data, length*8, playerId);

		if (length > 512)
		{
#if !defined(_COMPATIBILITY_1)
			// Flood attack?  Unknown systems should never send more than a small amount of data. Do a short ban
			rakPeer->AddToBanList(rakPeer->PlayerIDToDottedIP(playerId), 10000);
#endif
			return;
		}

		// These are all messages from unconnected systems.  Messages here can be any size, but are never processed from connected systems.
		if ( ( (unsigned char) data[ 0 ] == ID_PING_OPEN_CONNECTIONS
			|| (unsigned char)(data)[0] == ID_PING)	&& length == sizeof(unsigned char)+sizeof(RakNet::Time) )
		{
			if ( (unsigned char)(data)[0] == ID_PING ||
				rakPeer->AllowIncomingConnections() ) // Open connections with players
			{
#if !defined(_COMPATIBILITY_1)
				RakNet::BitStream inBitStream( (unsigned char *) data, length, false );
				inBitStream.IgnoreBits(8);
				RakNet::Time sendPingTime;
				inBitStream.Read(sendPingTime);

				RakNet::BitStream outBitStream;
				outBitStream.Write((unsigned char)ID_PONG); // Should be named ID_UNCONNECTED_PONG eventually
				outBitStream.Write(sendPingTime);
				//tempBitStream.Write( data, UnconnectedPingStruct_Size );
				rakPeer->rakPeerMutexes[ RakPeer::offlinePingResponse_Mutex ].Lock();
				// They are connected, so append offline ping data
				outBitStream.Write( (char*)rakPeer->offlinePingResponse.GetData(), rakPeer->offlinePingResponse.GetNumberOfBytesUsed() );
				rakPeer->rakPeerMutexes[ RakPeer::offlinePingResponse_Mutex ].Unlock();
				//SocketLayer::Instance()->SendTo( connectionSocket, ( char* ) outBitStream.GetData(), outBitStream.GetNumberOfBytesUsed(), playerId.binaryAddress, playerId.port );

				unsigned i;
				for (i=0; i < rakPeer->messageHandlerList.Size(); i++)
					rakPeer->messageHandlerList[i]->OnDirectSocketSend((const char*)outBitStream.GetData(), outBitStream.GetNumberOfBytesUsed(), playerId);

				SocketLayer::Instance()->SendTo( rakPeer->connectionSocket, (const char*)outBitStream.GetData(), outBitStream.GetNumberOfBytesUsed(), (char*)rakPeer->PlayerIDToDottedIP(playerId) , playerId.port );
#endif
			}
		}
		// UNCONNECTED MESSAGE Pong with no data.  TODO - Problem - this matches a reliable send of other random data.
		else if ((unsigned char) data[ 0 ] == ID_PONG && length >= sizeof(unsigned char)+sizeof(RakNet::Time) && length < sizeof(unsigned char)+sizeof(RakNet::Time)+MAX_OFFLINE_DATA_LENGTH)
		{
			packet=AllocPacket(length);
			memcpy(packet->data, data, length);
			packet->bitSize = length * 8;
			packet->playerId = playerId;
			packet->playerIndex = ( PlayerIndex ) rakPeer->GetIndexFromPlayerID( playerId, true );
			rakPeer->AddPacketToProducer(packet);
		}
		else if ((unsigned char) data[ 0 ] == ID_ADVERTISE_SYSTEM && length >= 2 && length < MAX_OFFLINE_DATA_LENGTH+2)
		{
			packet=AllocPacket(length);
			memcpy(packet->data, data, length);
			packet->bitSize = length * 8;
			packet->playerId = playerId;
			packet->playerIndex = ( PlayerIndex ) rakPeer->GetIndexFromPlayerID( playerId, true );
			rakPeer->AddPacketToProducer(packet);
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool RakPeer::RunUpdateCycle( void )
{
	RemoteSystemStruct * remoteSystem;
	unsigned remoteSystemIndex;
	Packet *packet;
	RakNet::Time ping, lastPing;
	// int currentSentBytes,currentReceivedBytes;
//	unsigned numberOfBytesUsed;
	unsigned numberOfBitsUsed;
	//PlayerID authoritativeClientPlayerId;
	int bitSize, byteSize;
	unsigned char *data;
	int errorCode;
	int gotData;
	RakNet::Time64 timeNS;
	RakNet::Time timeMS;
	PlayerID playerId;
	BufferedCommandStruct *bcs;
	bool callerDataAllocationUsed;
	RakNetStatisticsStruct *rnss;

	do
	{
		// Read a packet
		gotData = SocketLayer::Instance()->RecvFrom( connectionSocket, this, &errorCode );

		if ( gotData == SOCKET_ERROR )
		{
#ifdef _WIN32
			if ( errorCode == WSAECONNRESET )
			{
				gotData=false;
				// 11/14/05 - RecvFrom now calls HandlePortUnreachable rather than PushPortRefused
				//PushPortRefused( UNASSIGNED_PLAYER_ID );
				//closesocket(peer->connectionSocket);

				//peer->connectionSocket = SocketLayer::Instance()->CreateBoundSocket(peer->myPlayerId.port, true);
			}
			else
				if ( errorCode != 0 && endThreads == false )
				{
#ifdef _DO_PRINTF
					printf( "Server RecvFrom critical failure!\n" );
#endif
					// Some kind of critical error
					// peer->isRecvfromThreadActive=false;
					endThreads = true;
					Disconnect( 0, 0 );
					return false;
				}

#else
			if ( errorCode == -1 )
			{
				// isRecvfromThreadActive=false;
				endThreads = true;
				Disconnect( 0 );
				return false;
			}
#endif
		}

		if ( endThreads )
			return false;
	}
	while ( gotData>0 ); // Read until there is nothing left

	timeNS=0;
	timeMS=0;

	// Process all the deferred user thread Send and connect calls
	while ((bcs=bufferedCommands.ReadLock())!=0)
	{
		if (bcs->command==BufferedCommandStruct::BCS_SEND)
		{
			// GetTime is a very slow call so do it once and as late as possible
			if (timeNS==0)
				timeNS = RakNet::GetTime64();

			callerDataAllocationUsed=SendImmediate((char*)bcs->data, bcs->numberOfBitsToSend, bcs->priority, bcs->reliability, bcs->orderingChannel, bcs->playerId, bcs->broadcast, true, timeNS);
			if ( callerDataAllocationUsed==false )
				delete bcs->data;

			// Set the new connection state AFTER we call sendImmediate in case we are setting it to a disconnection state, which does not allow further sends
			if (bcs->connectionMode!=RemoteSystemStruct::NO_ACTION && bcs->playerId!=UNASSIGNED_PLAYER_ID)
			{
				remoteSystem=GetRemoteSystemFromPlayerID( bcs->playerId, true, true );
			//	if (remoteSystem==0)
			//		remoteSystem=AssignSystemAddressToRemoteSystemList(bcs->systemAddress, bcs->connectionMode);
				if (remoteSystem)
					remoteSystem->connectMode=bcs->connectionMode;
			}
		}
		else
		{
#ifdef _DEBUG
			assert(bcs->command==BufferedCommandStruct::BCS_CLOSE_CONNECTION);
#endif
			CloseConnectionInternal(bcs->playerId, false, true, bcs->orderingChannel);
		}

#ifdef _DEBUG
		bcs->data=0;
#endif

		bufferedCommands.ReadUnlock();
	}

	// Process connection attempts
	RequestedConnectionStruct *rcsFirst, *rcs;
	bool condition1, condition2;
    rcsFirst = requestedConnectionList.ReadLock();
	rcs=rcsFirst;
	while (rcs)
	{
		if (timeNS==0)
		{
			timeNS = RakNet::GetTime64();
			timeMS = (RakNet::Time)(timeNS/(RakNet::Time)1000);
		}

		if (rcs->nextRequestTime < timeMS)
		{
			condition1=rcs->requestsMade==6;
			condition2=(bool)((rcs->playerId==UNASSIGNED_PLAYER_ID)==1);
			// If too many requests made or a hole then remove this if possible, otherwise invalidate it
			if (condition1 || condition2)
			{
				if (rcs->data)
				{
					delete [] rcs->data;
					rcs->data=0;
				}

				if (condition1 && !condition2 && rcs->actionToTake==RequestedConnectionStruct::CONNECT)
				{
					// Tell user of connection attempt failed
					packet=AllocPacket(sizeof( char ));
					packet->data[ 0 ] = ID_CONNECTION_ATTEMPT_FAILED; // Attempted a connection and couldn't
					packet->bitSize = ( sizeof( char ) * 8);
					packet->playerId = rcs->playerId;
					packet->playerIndex = 65535;
					AddPacketToProducer(packet);
				}

				// Remove this if possible
				if (rcs==rcsFirst)
				{
					requestedConnectionList.ReadUnlock();
					rcsFirst = requestedConnectionList.ReadLock();
					rcs=rcsFirst;
				}
				else
				{
					// Hole in the middle
					rcs->playerId=UNASSIGNED_PLAYER_ID;
					rcs=requestedConnectionList.ReadLock();
				}

				continue;
			}

			rcs->requestsMade++;
			rcs->nextRequestTime=timeMS+1000;
			char c[3];
			c[0] = ID_OPEN_CONNECTION_REQUEST;
			// c[1] = 0; // Pad - apparently some routers block 1 byte packets
#ifndef SAMPSRV
			*(unsigned short*)&c[1] = (unsigned short)(uiIncomeCookieExchange ^ COOKIE_XOR_KEY);
#else
			c[1] = c[2] = 0;
#endif
			unsigned i;
			for (i=0; i < messageHandlerList.Size(); i++)
				messageHandlerList[i]->OnDirectSocketSend((char*)&c, 24, rcs->playerId);
			SocketLayer::Instance()->SendTo( connectionSocket, (char*)&c, 3, rcs->playerId.binaryAddress, rcs->playerId.port );
		}

		rcs=requestedConnectionList.ReadLock();
	}

	if (rcsFirst)
		requestedConnectionList.CancelReadLock(rcsFirst);

	 
	// remoteSystemList in network thread
	for ( remoteSystemIndex = 0; remoteSystemIndex < maximumNumberOfPeers; ++remoteSystemIndex )
	//for ( remoteSystemIndex = 0; remoteSystemIndex < remoteSystemListSize; ++remoteSystemIndex )
	{
		// I'm using playerId from remoteSystemList but am not locking it because this loop is called very frequently and it doesn't
		// matter if we miss or do an extra update.  The reliability layers themselves never care which player they are associated with
		//playerId = remoteSystemList[ remoteSystemIndex ].playerId;
		// Allow the playerID for this remote system list to change.  We don't care if it changes now.
	//	remoteSystemList[ remoteSystemIndex ].allowPlayerIdAssigment=true;
		if ( remoteSystemList[ remoteSystemIndex ].isActive )
		{
			playerId = remoteSystemList[ remoteSystemIndex ].playerId;
			RakAssert(playerId!=UNASSIGNED_PLAYER_ID);

			// Found an active remote system
			remoteSystem = remoteSystemList + remoteSystemIndex;
			// Update is only safe to call from the same thread that calls HandleSocketReceiveFromConnectedPlayer,
			// which is this thread

			if (timeNS==0)
			{
				timeNS = RakNet::GetTime64();
				timeMS = (RakNet::Time)(timeNS/(RakNet::Time64)1000);
				//printf("timeNS = %I64i timeMS=%i\n", timeNS, timeMS);
			}

			if (timeMS > remoteSystem->lastReliableSend && timeMS-remoteSystem->lastReliableSend > 5000 && remoteSystem->connectMode==RemoteSystemStruct::CONNECTED)
			{
				// If no reliable packets are waiting for an ack, do a one byte reliable send so that disconnections are noticed
				rnss=remoteSystem->reliabilityLayer.GetStatistics();
				if (rnss->messagesOnResendQueue==0)
				{
					unsigned char keepAlive=ID_DETECT_LOST_CONNECTIONS;
					SendImmediate((char*)&keepAlive,8,LOW_PRIORITY, RELIABLE, 0, remoteSystem->playerId, false, false, timeNS);
					remoteSystem->lastReliableSend=timeMS+remoteSystem->reliabilityLayer.GetTimeoutTime();
				}
			}

			remoteSystem->reliabilityLayer.Update( connectionSocket, playerId, MTUSize, timeNS, messageHandlerList ); // playerId only used for the internet simulator test

			// Check for failure conditions
			if ( remoteSystem->reliabilityLayer.IsDeadConnection() ||
				((remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ASAP || remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ASAP_SILENTLY) && remoteSystem->reliabilityLayer.IsDataWaiting()==false) ||
				(remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ON_NO_ACK && remoteSystem->reliabilityLayer.AreAcksWaiting()==false) ||
				((
				(remoteSystem->connectMode==RemoteSystemStruct::REQUESTED_CONNECTION ||
				remoteSystem->connectMode==RemoteSystemStruct::HANDLING_CONNECTION_REQUEST ||
				remoteSystem->connectMode==RemoteSystemStruct::UNVERIFIED_SENDER ||
				remoteSystem->connectMode==RemoteSystemStruct::SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET)
				&& timeMS > remoteSystem->connectionTime && timeMS - remoteSystem->connectionTime > 10000))
				)
			{
			//	printf("timeMS=%i remoteSystem->connectionTime=%i\n", timeMS, remoteSystem->connectionTime );

				// Failed.  Inform the user?
				if (remoteSystem->connectMode==RemoteSystemStruct::CONNECTED || remoteSystem->connectMode==RemoteSystemStruct::REQUESTED_CONNECTION
					|| remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ASAP || remoteSystem->connectMode==RemoteSystemStruct::DISCONNECT_ON_NO_ACK)
				{
					// Inform the user of the connection failure.
				//	unsigned staticDataBytes;

				//	staticDataBytes=remoteSystem->staticData.GetNumberOfBytesUsed();
				//	packet=AllocPacket(sizeof( char ) + staticDataBytes);
					packet=AllocPacket(sizeof( char ) );
					if (remoteSystem->connectMode==RemoteSystemStruct::REQUESTED_CONNECTION)
						packet->data[ 0 ] = ID_CONNECTION_ATTEMPT_FAILED; // Attempted a connection and couldn't
					else if (remoteSystem->connectMode==RemoteSystemStruct::CONNECTED)
						packet->data[ 0 ] = ID_CONNECTION_LOST; // DeadConnection
					else
						packet->data[ 0 ] = ID_DISCONNECTION_NOTIFICATION; // DeadConnection

					//if (staticDataBytes)
					//	memcpy( packet->data + sizeof( char ), remoteSystem->staticData.GetData(), staticDataBytes );
					packet->bitSize = ( sizeof( char ) ) * 8;
					//packet->bitSize = ( sizeof( char ) + staticDataBytes ) * 8;
					packet->playerId = playerId;
					packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;

					AddPacketToProducer(packet);
				}
				// else connection shutting down, don't bother telling the user

#ifdef _DO_PRINTF
				printf("Connection dropped for player %i:%i\n", playerId.binaryAddress, playerId.port);
#endif
				CloseConnectionInternal( playerId, false, true, 0 );
				continue;
			}

			// Did the reliability layer detect a modified packet?
			if ( remoteSystem->reliabilityLayer.IsCheater() )
			{
				packet=AllocPacket(sizeof(char));
				packet->bitSize=8;
				packet->data[ 0 ] = (unsigned char) ID_MODIFIED_PACKET;
				packet->playerId = playerId;
				packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;
				AddPacketToProducer(packet);
				continue;
			}

			// Ping this guy if it is time to do so
			if ( remoteSystem->connectMode==RemoteSystemStruct::CONNECTED && timeMS > remoteSystem->nextPingTime && ( occasionalPing || remoteSystem->lowestPing == (unsigned short)-1 ) )
			{
				remoteSystem->nextPingTime = timeMS + 5000;
				PingInternal( playerId, true );
			}

			// Find whoever has the lowest player ID
			//if (playerId < authoritativeClientPlayerId)
			// authoritativeClientPlayerId=playerId;

			// Does the reliability layer have any packets waiting for us?
			// To be thread safe, this has to be called in the same thread as HandleSocketReceiveFromConnectedPlayer
			bitSize = remoteSystem->reliabilityLayer.Receive( &data );

			while ( bitSize > 0 )
			{
				// These types are for internal use and should never arrive from a network packet
				if (data[0]==ID_CONNECTION_ATTEMPT_FAILED || data[0]==ID_MODIFIED_PACKET)
				{
					RakAssert(0);
					continue;
				}

				// Put the input through compression if necessary
				if ( inputTree )
				{
					RakNet::BitStream dataBitStream( MAXIMUM_MTU_SIZE );
					// Since we are decompressing input, we need to copy to a bitstream, decompress, then copy back to a probably
					// larger data block.  It's slow, but the user should have known that anyway
					dataBitStream.Reset();
					dataBitStream.WriteAlignedBytes( ( unsigned char* ) data, BITS_TO_BYTES( bitSize ) );
					rawBytesReceived += dataBitStream.GetNumberOfBytesUsed();

//					numberOfBytesUsed = dataBitStream.GetNumberOfBytesUsed();
					numberOfBitsUsed = dataBitStream.GetNumberOfBitsUsed();
					//rawBytesReceived += numberOfBytesUsed;
					// Decompress the input data.

					if (numberOfBitsUsed>0)
					{
						unsigned char *dataCopy = new unsigned char[ dataBitStream.GetNumberOfBytesUsed() ];
						memcpy( dataCopy, dataBitStream.GetData(), dataBitStream.GetNumberOfBytesUsed() );
						dataBitStream.Reset();
						inputTree->DecodeArray( dataCopy, numberOfBitsUsed, &dataBitStream );
						compressedBytesReceived += dataBitStream.GetNumberOfBytesUsed();
						delete [] dataCopy;

						byteSize = dataBitStream.GetNumberOfBytesUsed();

						if ( byteSize > BITS_TO_BYTES( bitSize ) )   // Probably the case - otherwise why decompress?
						{
							delete [] data;
							data = new unsigned char [ byteSize ];
						}
						memcpy( data, dataBitStream.GetData(), byteSize );
					}
					else
						byteSize=0;
				}
				else
					// Fast and easy - just use the data that was returned
					byteSize = BITS_TO_BYTES( bitSize );

				// For unknown senders we only accept a few specific packets
				if (remoteSystem->connectMode==RemoteSystemStruct::UNVERIFIED_SENDER)
				{
					if ( (unsigned char)(data)[0] == ID_CONNECTION_REQUEST )
					{
						ParseConnectionRequestPacket(remoteSystem, playerId, (const char*)data, byteSize);
						delete [] data;
					}
					else
					{
						CloseConnectionInternal( playerId, false, true, 0 );
#ifdef _DO_PRINTF
						printf("Temporarily banning %i:%i for sending nonsense data\n", playerId.binaryAddress, playerId.port);
#endif

#if !defined(_COMPATIBILITY_1)
						AddToBanList(PlayerIDToDottedIP(playerId), remoteSystem->reliabilityLayer.GetTimeoutTime());
#endif
						delete [] data;
					}
				}
				else
				{
					// However, if we are connected we still take a connection request in case both systems are trying to connect to each other
					// at the same time
					if ( (unsigned char)(data)[0] == ID_CONNECTION_REQUEST )
					{
						// 04/27/06 This is wrong.  With cross connections, we can both have initiated the connection are in state REQUESTED_CONNECTION
						// 04/28/06 Downgrading connections from connected will close the connection due to security at ((remoteSystem->connectMode!=RemoteSystemStruct::CONNECTED && time > remoteSystem->connectionTime && time - remoteSystem->connectionTime > 10000))
						if (remoteSystem->connectMode!=RemoteSystemStruct::CONNECTED)
							ParseConnectionRequestPacket(remoteSystem, playerId, (const char*)data, byteSize);
						delete [] data;
					}
					else if ( (unsigned char) data[ 0 ] == ID_NEW_INCOMING_CONNECTION && byteSize == sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned short) )
					{
#ifdef _DEBUG
						// This assert can be ignored since it could hit from duplicate packets.
						// It's just here for internal testing since it should only happen rarely and will mostly be from bugs
//						assert(remoteSystem->connectMode==RemoteSystemStruct::HANDLING_CONNECTION_REQUEST);
#endif
						if (remoteSystem->connectMode==RemoteSystemStruct::HANDLING_CONNECTION_REQUEST ||
							remoteSystem->connectMode==RemoteSystemStruct::SET_ENCRYPTION_ON_MULTIPLE_16_BYTE_PACKET ||
							playerId==myPlayerId) // local system connect
						{
							remoteSystem->connectMode=RemoteSystemStruct::CONNECTED;
							PingInternal( playerId, true );
							SendStaticDataInternal( playerId, true );

							RakNet::BitStream inBitStream((unsigned char *) data, byteSize, false);
							PlayerID bsPlayerId;

							inBitStream.IgnoreBits(8);
							inBitStream.Read(bsPlayerId.binaryAddress);
							inBitStream.Read(bsPlayerId.port);

							// Overwrite the data in the packet
							//					NewIncomingConnectionStruct newIncomingConnectionStruct;
							//					RakNet::BitStream nICS_BS( data, NewIncomingConnectionStruct_Size, false );
							//					newIncomingConnectionStruct.Deserialize( nICS_BS );
							remoteSystem->myExternalPlayerId = bsPlayerId;

							// Send this info down to the game

							packet=AllocPacket(byteSize, data);
							packet->bitSize = bitSize;
							packet->playerId = playerId;
							packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;
							AddPacketToProducer(packet);
						}
						else
							delete [] data;
					}
					else if ( (unsigned char) data[ 0 ] == ID_CONNECTED_PONG && byteSize == sizeof(unsigned char)+(sizeof(RakNet::Time)*2) )
					{
						RakNet::Time sendPingTime, sendPongTime;

						// Copy into the ping times array the current time - the value returned
						// First extract the sent ping
						RakNet::BitStream inBitStream( (unsigned char *) data, byteSize, false );
						//PingStruct ps;
						//ps.Deserialize(psBS);
						inBitStream.IgnoreBits(8);
						inBitStream.Read(sendPingTime);
						inBitStream.Read(sendPongTime);

						timeNS = RakNet::GetTime64(); // Update the time value to be accurate
						timeMS = (RakNet::Time)(timeNS/(RakNet::Time64)1000);
						if (timeMS > sendPingTime)
							ping = timeMS - sendPingTime;
						else
							ping=0;
						lastPing = remoteSystem->pingAndClockDifferential[ remoteSystem->pingAndClockDifferentialWriteIndex ].pingTime;

						// Ignore super high spikes in the average
						if ( lastPing <= 0 || ( ( ping < ( lastPing * 3 ) ) && ping < 1200 ) )
						{
							remoteSystem->pingAndClockDifferential[ remoteSystem->pingAndClockDifferentialWriteIndex ].pingTime = ( unsigned short ) ping;
							// Thanks to Chris Taylor (cat02e@fsu.edu) for the improved timestamping algorithm
							remoteSystem->pingAndClockDifferential[ remoteSystem->pingAndClockDifferentialWriteIndex ].clockDifferential = sendPongTime - ( timeMS + sendPingTime ) / 2;

							if ( remoteSystem->lowestPing == (unsigned short)-1 || remoteSystem->lowestPing > (int) ping )
								remoteSystem->lowestPing = (unsigned short) ping;

							// Most packets should arrive by the ping time.
							assert(ping < 10000); // Sanity check - could hit due to negative pings causing the var to overflow
							remoteSystem->reliabilityLayer.SetPing( (unsigned short) ping );

							if ( ++( remoteSystem->pingAndClockDifferentialWriteIndex ) == PING_TIMES_ARRAY_SIZE )
								remoteSystem->pingAndClockDifferentialWriteIndex = 0;
						}

						delete [] data;
					}
					else if ( (unsigned char)data[0] == ID_INTERNAL_PING && byteSize == sizeof(unsigned char)+sizeof(RakNet::Time) )
					{
						RakNet::BitStream inBitStream( (unsigned char *) data, byteSize, false );
 						inBitStream.IgnoreBits(8);
						RakNet::Time sendPingTime;
						inBitStream.Read(sendPingTime);

						RakNet::BitStream outBitStream;
						outBitStream.Write((unsigned char)ID_CONNECTED_PONG);
						outBitStream.Write(sendPingTime);
						timeMS = RakNet::GetTime();
						timeNS = RakNet::GetTime64();
						outBitStream.Write(timeMS);
						SendImmediate( (char*)outBitStream.GetData(), outBitStream.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, UNRELIABLE, 0, playerId, false, false, timeMS );

						delete [] data;
					}
					else if ( (unsigned char) data[ 0 ] == ID_DISCONNECTION_NOTIFICATION )
					{
						/*
						unsigned staticDataBytes=remoteSystem->staticData.GetNumberOfBytesUsed();

						if ( staticDataBytes > 0 )
						{
							packet=AllocPacket(sizeof( char ) + staticDataBytes);
							packet->data[ 0 ] = ID_DISCONNECTION_NOTIFICATION;
							memcpy( packet->data + sizeof( char ), remoteSystem->staticData.GetData(), staticDataBytes );
							packet->bitSize = sizeof( char ) * 8 + remoteSystem->staticData.GetNumberOfBitsUsed();
							delete [] data;
						}
						else
						{
							packet=AllocPacket(1, data);
							packet->bitSize = 8;
						}
						*/

						
//						packet->playerId = playerId;
//						packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;

						// We shouldn't close the connection immediately because we need to ack the ID_DISCONNECTION_NOTIFICATION
						remoteSystem->connectMode=RemoteSystemStruct::DISCONNECT_ON_NO_ACK;
						delete [] data;

					//	AddPacketToProducer(packet);
					}
					else if ( (unsigned char) data[ 0 ] == ID_REQUEST_STATIC_DATA )
					{
						SendStaticDataInternal( playerId, true );
						delete [] data;
					}
					else if ( (unsigned char) data[ 0 ] == ID_RECEIVED_STATIC_DATA )
					{
						remoteSystem->staticData.Reset();
						remoteSystem->staticData.Write( ( char* ) data + sizeof(unsigned char), byteSize - 1 );

						// Inform game server code that we got static data
						packet=AllocPacket(byteSize, data);
						packet->bitSize = bitSize;
						packet->playerId = playerId;
						packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;
						AddPacketToProducer(packet);
					}
#if !defined(_COMPATIBILITY_1)
					else if ( (unsigned char)(data)[0] == ID_SECURED_CONNECTION_RESPONSE &&
						byteSize == 1 + sizeof( big::u32 ) + sizeof( RSA_BIT_SIZE ) + 20 )
					{
						SecuredConnectionConfirmation( remoteSystem, (char*)data );
						delete [] data;
					}
					else if ( (unsigned char)(data)[0] == ID_SECURED_CONNECTION_CONFIRMATION &&
						byteSize == 1 + 20 + sizeof( RSA_BIT_SIZE ) )
					{
						CSHA1 sha1;
						bool confirmedHash, newRandNumber;

						confirmedHash = false;

						// Hash the SYN-Cookie
						// s2c syn-cookie = SHA1_HASH(source ip address + source port + random number)
						sha1.Reset();
						sha1.Update( ( unsigned char* ) & playerId.binaryAddress, sizeof( playerId.binaryAddress ) );
						sha1.Update( ( unsigned char* ) & playerId.port, sizeof( playerId.port ) );
						sha1.Update( ( unsigned char* ) & ( newRandomNumber ), 20 );
						sha1.Final();

						newRandNumber = false;

						// Confirm if
						//syn-cookie ?= HASH(source ip address + source port + last random number)
						//syn-cookie ?= HASH(source ip address + source port + current random number)
						if ( memcmp( sha1.GetHash(), data + 1, 20 ) == 0 )
						{
							confirmedHash = true;
						}
						else
						{
							sha1.Reset();
							sha1.Update( ( unsigned char* ) & playerId.binaryAddress, sizeof( playerId.binaryAddress ) );
							sha1.Update( ( unsigned char* ) & playerId.port, sizeof( playerId.port ) );
							sha1.Update( ( unsigned char* ) & ( oldRandomNumber ), 20 );
							sha1.Final();

							if ( memcmp( sha1.GetHash(), data + 1, 20 ) == 0 )
								confirmedHash = true;
						}
						if ( confirmedHash )
						{
							int i;
							unsigned char AESKey[ 16 ];
							RSA_BIT_SIZE message, encryptedMessage;

							// On connection accept, AES key is c2s RSA_Decrypt(random number) XOR s2c syn-cookie
							// Get the random number first
							#ifdef HOST_ENDIAN_IS_BIG
								BSWAPCPY( (unsigned char *) encryptedMessage, (unsigned char *)(data + 1 + 20), sizeof( RSA_BIT_SIZE ) );
							#else
								memcpy( encryptedMessage, data + 1 + 20, sizeof( RSA_BIT_SIZE ) );
							#endif
							rsacrypt.decrypt( encryptedMessage, message );
							#ifdef HOST_ENDIAN_IS_BIG
								BSWAPSELF( (unsigned char *) message, sizeof( RSA_BIT_SIZE ) );
							#endif

							// Save the AES key
							for ( i = 0; i < 16; i++ )
								AESKey[ i ] = data[ 1 + i ] ^ ( ( unsigned char* ) ( message ) ) [ i ];

							// Connect this player assuming we have open slots
							OnConnectionRequest( remoteSystem, AESKey, true );
						}
						delete [] data;
					}
#endif // #if !defined(_COMPATIBILITY_1)
					else if ( (unsigned char)(data)[0] == ID_DETECT_LOST_CONNECTIONS && byteSize == sizeof(unsigned char) )
					{
						// Do nothing
						delete [] data;
					}
					else if ( (unsigned char)(data)[0] == ID_CONNECTION_REQUEST_ACCEPTED && byteSize == sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned short)+sizeof(PlayerIndex)+sizeof(unsigned int) )
					{
						// Make sure this connection accept is from someone we wanted to connect to
						bool allowConnection, alreadyConnected;

						if (remoteSystem->connectMode==RemoteSystemStruct::HANDLING_CONNECTION_REQUEST || remoteSystem->connectMode==RemoteSystemStruct::REQUESTED_CONNECTION || allowConnectionResponseIPMigration)
							allowConnection=true;
						else
							allowConnection=false;
						if (remoteSystem->connectMode==RemoteSystemStruct::HANDLING_CONNECTION_REQUEST)
							alreadyConnected=true;
						else
							alreadyConnected=false;

						if ( allowConnection )
						{
							PlayerID externalID;
							PlayerIndex playerIndex;

							RakNet::BitStream inBitStream((unsigned char *) data, byteSize, false);
							inBitStream.IgnoreBits(8); // ID_CONNECTION_REQUEST_ACCEPTED
						//	inBitStream.Read(remotePort);
							inBitStream.Read(externalID.binaryAddress);
							inBitStream.Read(externalID.port);
							inBitStream.Read(playerIndex);

							// Find a free remote system struct to use
							//						RakNet::BitStream casBitS(data, byteSize, false);
							//						ConnectionAcceptStruct cas;
							//						cas.Deserialize(casBitS);
						//	playerId.port = remotePort;

							// The remote system told us our external IP, so save it
							remoteSystem->myExternalPlayerId = externalID;
							remoteSystem->connectMode=RemoteSystemStruct::CONNECTED;


							if (alreadyConnected==false)
							{
								// Use the stored encryption key
								if (remoteSystem->setAESKey)
									remoteSystem->reliabilityLayer.SetEncryptionKey( remoteSystem->AESKey );
								else
									remoteSystem->reliabilityLayer.SetEncryptionKey( 0 );
							}

							// Send the connection request complete to the game
							packet=AllocPacket(byteSize, data);
							packet->bitSize = byteSize * 8;
							packet->playerId = playerId;
							packet->playerIndex = ( PlayerIndex ) GetIndexFromPlayerID( playerId, true );
							AddPacketToProducer(packet);

							RakNet::BitStream outBitStream(sizeof(unsigned char)+sizeof(unsigned int)+sizeof(unsigned short));
							outBitStream.Write((unsigned char)ID_NEW_INCOMING_CONNECTION);
							outBitStream.Write(playerId.binaryAddress);
							outBitStream.Write(playerId.port);
							// We turned on encryption with SetEncryptionKey.  This pads packets to up to 16 bytes.
							// As soon as a 16 byte packet arrives on the remote system, we will turn on AES.  This works because all encrypted packets are multiples of 16 and the
							// packets I happen to be sending before this are less than 16 bytes.  Otherwise there is no way to know if a packet that arrived is
							// encrypted or not so the other side won't know to turn on encryption or not.
							SendImmediate( (char*)outBitStream.GetData(), outBitStream.GetNumberOfBitsUsed(), SYSTEM_PRIORITY, RELIABLE, 0, playerId, false, false, RakNet::GetTime() );

							if (alreadyConnected==false)
							{
								PingInternal( playerId, true );
								SendStaticDataInternal( playerId, true );
							}
						}
						else
						{
							// Tell the remote system the connection failed
							NotifyAndFlagForDisconnect(playerId, true, 0);
#ifdef _DO_PRINTF
							printf( "Error: Got a connection accept when we didn't request the connection.\n" );
#endif
							delete [] data;
						}
					}
					else
					{
						if (data[0]>=(unsigned char)ID_RPC)
						{
							packet=AllocPacket(byteSize, data);
							packet->bitSize = bitSize;
							packet->playerId = playerId;
							packet->playerIndex = ( PlayerIndex ) remoteSystemIndex;
							AddPacketToProducer(packet);					
						}
						//else
							// Some internal type got returned to the user?
							//RakAssert(0);
					}
				}

				// Does the reliability layer have any more packets waiting for us?
				// To be thread safe, this has to be called in the same thread as HandleSocketReceiveFromConnectedPlayer
				bitSize = remoteSystem->reliabilityLayer.Receive( &data );
			}
		}
	}

	return true;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _WIN32
unsigned __stdcall UpdateNetworkLoop( LPVOID arguments )
#else
void* UpdateNetworkLoop( void* arguments )
#endif
{
	RakPeer * rakPeer = ( RakPeer * ) arguments;
	// RakNetTime time;

	// 11/15/05 - this is slower than Sleep()
#ifdef _WIN32
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
	// Lets see if these timers give better performance than Sleep
	HANDLE timerHandle;
	LARGE_INTEGER dueTime;

	if ( rakPeer->threadSleepTimer <= 0 )
		rakPeer->threadSleepTimer = 1;

	// 2nd parameter of false means synchronization timer instead of manual-reset timer
	timerHandle = CreateWaitableTimer( NULL, FALSE, 0 );

	assert( timerHandle );

	dueTime.QuadPart = -10000 * (LONGLONG)rakPeer->threadSleepTimer; // 10000 is 1 ms?

	BOOL success = SetWaitableTimer( timerHandle, &dueTime, rakPeer->threadSleepTimer, NULL, NULL, FALSE );

	assert( success );

#endif
#endif

#ifdef _RAKNET_THREADSAFE
	#pragma message("-- RakNet: _RAKNET_THREADSAFE defined.  Safe to use multiple threads on the same instance of RakPeer (Slow!). --")
#else
	#pragma message("-- RakNet: _RAKNET_THREADSAFE not defined.  Do NOT use multiple threads on the same instance of RakPeer (Fast!). --")
#endif

	rakPeer->isMainLoopThreadActive = true;

	while ( rakPeer->endThreads == false )
	{
		rakPeer->RunUpdateCycle();
		/*
#ifdef _WIN32
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
		#pragma message("-- RakNet: Using WaitForSingleObject. Comment out USE_WAIT_FOR_MULTIPLE_EVENTS in RakNetDefines.h if you want to use Sleep instead. --")

		if ( WaitForSingleObject( timerHandle, INFINITE ) != WAIT_OBJECT_0 )
		{
#ifdef _DEBUG

			assert( 0 );
	#ifdef _DO_PRINTF
			printf( "WaitForSingleObject failed (%d)\n", GetLastError() );
	#endif
#endif
		}

#else
		#pragma message("-- RakNet: Using Sleep(). Uncomment USE_WAIT_FOR_MULTIPLE_EVENTS in RakNetDefines.h if you want to use WaitForSingleObject instead. --")
*/
		if (rakPeer->threadSleepTimer>=0)
		{
#if defined(USE_WAIT_FOR_MULTIPLE_EVENTS)
			if (rakPeer->threadSleepTimer>0)
				WSAWaitForMultipleEvents(1,&rakPeer->recvEvent,TRUE,rakPeer->threadSleepTimer,FALSE);
			else
				RakSleep(0);
#else // _WIN32
				RakSleep( rakPeer->threadSleepTimer );
#endif
		}
	}

/*#ifdef _WIN32
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
	CloseHandle( timerHandle );
#endif
#endif*/

	rakPeer->isMainLoopThreadActive = false;

	return 0;
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
