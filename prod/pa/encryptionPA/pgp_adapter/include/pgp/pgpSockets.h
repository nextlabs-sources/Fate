/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpSockets_h	/* [ */
#define Included_pgpSockets_h

#include <stdio.h>

#include "pgpOptionList.h"
#include "pgpTLS.h"
#include "pgpErrors.h"

#if PGP_UNIX || PGP_SYMBIAN
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>		/* Needed for struct timeval */
#if PGP_UNIX_LINUX
# include <sys/ioctl.h>		/* Need FIONREAD */
#elif PGP_UNIX_SOLARIS || PGP_UNIX_DARWIN
# include <sys/filio.h>
#elif PGP_UNIX_AIX
#include <sys/ioctl.h>
#endif /* ! PGP_UNIX_LINUX */
# include <netinet/in.h>
# include <netdb.h>
#endif

#if PGP_WIN32
# include <winsock.h>
#endif

PGP_BEGIN_C_DECLARATIONS

#if PGP_UNIX || PGP_SYMBIAN
typedef int							PGPSocketRef;
#else
typedef SOCKET						PGPSocketRef;
#endif


/*
 * Unix and Windows share the same Berkeley socket interface. This isn't
 * the most efficient Windows implmentation of TCP/IP, but it is
 * compatible with UNIX berkeley sockets, making cross-platform possible.
 *
 * Trying to write cross-platform win32 TCP/IP code using all the fancy
 * dancy Win32 network functions would be nearly impossible IMHO
 *
 * The Mac doesn't have the berkeley stuff, so we roll our own for all
 * of the structures.
 *
 * Start with Unix and Win32
 */
#if PGP_UNIX || PGP_WIN32 || PGP_SYMBIAN

# define kInvalidPGPSocketRef		((PGPSocketRef) (~0))

 typedef struct hostent		PGPHostEntry;
 typedef struct protoent	PGPProtocolEntry;
 typedef struct servent		PGPServiceEntry;
 typedef struct sockaddr_in	PGPSocketAddressInternet;
 typedef struct sockaddr	PGPSocketAddress;
 typedef struct in_addr		PGPInternetAddress;
 typedef fd_set 			PGPSocketSet;
 typedef struct timeval		PGPSocketsTimeValue;

# define PGPSOCKETSET_CLEAR(socketRef, set) FD_CLR((int) (socketRef), (set))
#ifdef PGP_WIN32
	# define PGPSOCKETSET_SET(socketRef, set)   FD_SET((UINT_PTR) (socketRef), (set))
#else
	# define PGPSOCKETSET_SET(socketRef, set)   FD_SET((int) (socketRef), (set))
#endif
# define PGPSOCKETSET_ZERO(set)				FD_ZERO((set))
# define PGPSOCKETSET_ISSET(socketRef, set) FD_ISSET((int) (socketRef), (set))

/* Select types and defines */
#ifndef PGPSOCKETSET_SIZE
#  ifdef FD_SETSIZE
#    define PGPSOCKETSET_SIZE FD_SETSIZE
#  else
#    define PGPSOCKETSET_SIZE 64
#  endif
#endif

 /* Address families */
 enum {
	kPGPAddressFamilyUnspecified	=	AF_UNSPEC,
	kPGPAddressFamilyInternet		=	AF_INET
 };

 /* Protocol families */
 enum {
	kPGPProtocolFamilyInternet	=	PF_INET
 };

 /* Types */
 enum {
	kPGPSocketTypeStream			=	SOCK_STREAM,
	kPGPSocketTypeDatagram			=	SOCK_DGRAM
 };

#if ! PGP_SYMBIAN
 /* Commands for PGPIOControlSocket */
 enum {
	kPGPSocketCommandGetUnreadData	=	FIONREAD
 };
#endif

 /* Levels for PGPGetSocketOptions and PGPSetSocketOptions */
 enum {
	kPGPSocketOptionLevelSocket	=	SOL_SOCKET
 };

#if ! PGP_SYMBIAN
 /* Options for PGPGetSocketOptions and PGPSetSocketOptions */
 enum {
	kPGPSocketOptionAcceptingConnections	=	SO_ACCEPTCONN,
	kPGPSocketOptionType					=	SO_TYPE
 };
#endif

 /* Protocols */
 enum {
	kPGPTCPProtocol	=	IPPROTO_TCP,
	kPGPUDPProtocol	=	IPPROTO_UDP
 };

 /* Send flags */
 enum {
	kPGPSendFlagNone		=	0
 };

 /* Receive flags */
 enum {
	kPGPReceiveFlagNone		=	0
 };

 /* Internet Addresses */
 enum {
 	kPGPInternetAddressAny = INADDR_ANY
 };

#endif /* PGP_UNIX || PGP_WIN32 */

/*
 * Onto the Mac, where we need to create our own versions of the various
 * structures.
 */
#if PGP_MACINTOSH

# define	kInvalidPGPSocketRef		((PGPSocketRef) NULL)

 typedef struct PGPInternetAddress {
	union {
		struct {
			PGPByte		s_b1;
			PGPByte		s_b2;
			PGPByte		s_b3;
			PGPByte		s_b4;
		}					S_un_b;
		struct {
			PGPUInt16	s_w1;
			PGPUInt16	s_w2;
		}					S_un_w;
		PGPUInt32		S_addr;
	} S_un;
# define s_addr	S_un.S_addr
 } PGPInternetAddress;

 typedef struct PGPSocketAddressInternet {
	PGPInt16			sin_family;
	PGPUInt16			sin_port;
	PGPInternetAddress	sin_addr;
	PGPByte				sin_zero[8];
 } PGPSocketAddressInternet;

 typedef struct PGPSocketAddress {
	PGPUInt16	sa_family;
	PGPByte		sa_data[14];
 } PGPSocketAddress;

 typedef struct PGPHostEntry {
	char *		h_name;
	char **		unused;
	PGPInt16	h_addrtype;
	PGPInt16	h_length;
	char **		h_addr_list;
# define h_addr	h_addr_list[0]
 } PGPHostEntry;

 typedef struct PGPProtocolEntry {
	char *		p_name;
	char **		p_aliases;
	PGPInt16	p_proto;
 } PGPProtocolEntry;

 typedef struct PGPServiceEntry {
	char *		s_name;
	char **		s_aliases;
	PGPUInt16	s_port;
	char *		s_proto;
 } PGPServiceEntry;

 /* Select types and defines */
# ifndef PGPSOCKETSET_SETSIZE
#  define PGPSOCKETSET_SETSIZE	64
# endif

 typedef struct PGPSocketSet {
	PGPUInt16		fd_count;
	PGPSocketRef	fd_array[PGPSOCKETSET_SETSIZE];
 } PGPSocketSet;

# define PGPSOCKETSET_CLEAR(socketRef, set)	do {						\
    PGPUInt16	__i;													\
	for (__i = 0; __i < ((PGPSocketSet * (set))->fd_count; __i++) {		\
		if (((PGPSocketSet *) (set))->fd_array[__i] == socketRef) {		\
			while (__i < (((PGPSocketSet *) (set))->fd_count - 1)) {	\
				((PGPSocketSet *) (set))->fd_array[__i] =				\
					((PGPSocketSet *) (set))->fd_array[__i + 1];		\
				__i++;													\
			}															\
			((PGPSocketSet *) (set))->fd_count--;						\
			break;														\
		}																\
	}																	\
 } while (0)
										
# define PGPSOCKETSET_SET(socketRef, set)	do {					\
    if (((PGPSocketSet *) (set))->fd_count < PGPSOCKETSET_SETSIZE) {	\
	    ((PGPSocketSet *) (set))->fd_array[((PGPSocketSet *)			\
	    (set))->fd_count++] = (socketRef);								\
    }																	\
 } while (0)

# define PGPSOCKETSET_ZERO(set)	(((PGPSocketSet *) (set))->fd_count = 0)

 PGPInt32 __PGPSocketsIsSet(PGPSocketRef, PGPSocketSet *);
	
# define PGPSOCKETSET_ISSET(socketRef, set)	__PGPSocketsIsSet(	\
													(socketRef),(set))
							
 typedef struct PGPSocketsTimeValue {
        PGPInt32    tv_sec;         /* seconds */
        PGPInt32    tv_usec;        /* and microseconds */
 } PGPSocketsTimeValue;

 /* Address families */
 enum {
	kPGPAddressFamilyUnspecified	=	0,
	kPGPAddressFamilyInternet		=	2
 };

 /* Protocol families */
 enum {
	kPGPProtocolFamilyInternet	=	kPGPAddressFamilyInternet
 };

 /* Types */
 enum {
	kPGPSocketTypeStream			=	1,
	kPGPSocketTypeDatagram			=	2
 };

 /* Commands for PGPIOControlSocket */
 enum {
	kPGPSocketCommandGetUnreadData	=	(0x40000000
		| (((long) sizeof(PGPUInt32) & 0x7F) << 16) | ('f' << 8) | 127)
 };

 /* Levels for PGPGetSocketOptions and PGPSetSocketOptions */
 enum {
	kPGPSocketOptionLevelSocket	=	0xFFFFFFFF
 };

 /* Options for PGPGetSocketOptions and PGPSetSocketOptions */
 enum {
	kPGPSocketOptionAcceptingConnections	=	0x00000002,
	kPGPSocketOptionType					=	0x00001008
 };

 /* Protocols */
 enum {
	kPGPTCPProtocol	=	6,
	kPGPUDPProtocol	=	17
 };

 /* Send flags */
 enum {
	kPGPSendFlagNone		=	0
 };

 /* Receive flags */
 enum {
	kPGPReceiveFlagNone		=	0
 };

 /* Internet Addresses */
 enum {
	kPGPInternetAddressAny	=	0x00000000
 };

#endif /* PGP_MACINTOSH */

/*
 * Some global things for all platforms
 */

#define PGPSocketRefIsValid(ref)	((ref) != kInvalidPGPSocketRef)

typedef struct PGPSocketsThreadStorage *	PGPSocketsThreadStorageRef;
# define kInvalidPGPSocketsThreadStorageRef				\
			((PGPSocketsThreadStorageRef) NULL)
#define PGPSocketsThreadStorageRefIsValid(ref)			\
			((ref) != kInvalidPGPSocketsThreadStorageRef)

extern PGPSocketAddressInternet	kPGPAddressAny;

/* Errors */
#define kPGPSockets_Error	-1

/* Net byte ordering macros (PGP_WORDSBIGENDIAN defined by configure) */
PGPInt32	PGPHostToNetLong(PGPInt32 x);
PGPInt16	PGPHostToNetShort(PGPInt16 x);
PGPInt32	PGPNetToHostLong(PGPInt32 x);
PGPInt16	PGPNetToHostShort(PGPInt16 x);
 
/*
 * Shared function interface (except for idle handler code)
 */

/*
 * Use the idle event handler to receive periodic idle events during
 * network calls. Usually this is used only in non-preemptive multi-tasking
 * OSes to allow yielding in threads. Pre-emptive multi-tasking systems
 * should probably not use the call as it interrupts the efficient wait state
 * of threads waiting on network calls.
 *
 * Idle event handlers need to be added on a per thread basis.
 *
 * Returning an error from the idle event handler will cause the socket
 * that is blocking to close.
 *
 */
PGPError		PGPSetSocketsIdleEventHandler(
						PGPEventHandlerProcPtr inCallback,
						PGPUserValue inUserData);
						
PGPError		PGPGetSocketsIdleEventHandler(
						PGPEventHandlerProcPtr * outCallback,
						PGPUserValue * outUserData);

/* Static storage creation */
PGPError		PGPSocketsCreateThreadStorage(
					PGPSocketsThreadStorageRef * outPreviousStorage);
PGPError		PGPSocketsDisposeThreadStorage(
					PGPSocketsThreadStorageRef inPreviousStorage);
					
/* Stack based class for saving and restoring thread storage */
#ifdef __cplusplus	/* [ */
class StPGPPreserveSocketsStorage {
public:
			StPGPPreserveSocketsStorage() : mStorage(NULL)
				{ PGPSocketsCreateThreadStorage(&mStorage); }
			~StPGPPreserveSocketsStorage()
				{ PGPSocketsDisposeThreadStorage(mStorage); }

protected:
	PGPSocketsThreadStorageRef	mStorage;
};
#endif	/* ] __cplusplus */


/* Initialization and termination */
PGPError		PGPSocketsInit(void);
void			PGPSocketsCleanup(void);

/* Socket creation and destruction */
PGPSocketRef	PGPOpenSocket(PGPInt32 inAddressFamily, PGPInt32 inSocketType,
							  PGPInt32 inSocketProtocol);
PGPInt32		PGPCloseSocket(PGPSocketRef inSocketRef);

/* Endpoint binding */
PGPInt32		PGPBindSocket(PGPSocketRef inSocketRef,
							  const PGPSocketAddress * inAddress,
							  PGPInt32 inAddressLength);
PGPInt32		PGPConnect(PGPSocketRef inSocketRef,
						   const PGPSocketAddress * inServerAddress,
						   PGPInt32 inAddressLength);

/* Send functions */
PGPInt32		PGPSend(PGPSocketRef inSocketRef, const void * inBuffer,
						PGPInt32 inBufferLength, PGPInt32 inFlags);
PGPInt32		PGPWrite(PGPSocketRef inSocketRef, const void * inBuffer,
						 PGPInt32 inBufferLength);
PGPInt32		PGPSendTo(PGPSocketRef inSocketRef, const void * inBuffer,
						  PGPInt32 inBufferLength, PGPInt32 inFlags,
						  PGPSocketAddress * inAddress,
						  PGPInt32 inAddressLength);

/* Receive functions */
PGPInt32		PGPReceive(PGPSocketRef inSocketRef, void * outBuffer,
						   PGPInt32 inBufferSize, PGPInt32 inFlags);
PGPInt32		PGPRead(PGPSocketRef inSocketRef, void * outBuffer,
						PGPInt32 inBufferSize);
PGPInt32		PGPReceiveFrom(PGPSocketRef inSocketRef, void * outBuffer,
							   PGPInt32 inBufferSize, PGPInt32 inFlags,
							   PGPSocketAddress * outAddress,
							   PGPInt32 * ioAddressLength);

/* Server functions */
PGPInt32		PGPListen(PGPSocketRef inSocketRef, PGPInt32 inMaxBacklog);
PGPSocketRef	PGPAccept(PGPSocketRef inSocketRef,
						  PGPSocketAddress * outAddress,
						  PGPInt32 * ioAddressLength);

/* Select */
/* Note that inNumSetCount is not used under Mac and Windows */
PGPInt32 		PGPSelect(PGPInt32 inNumSetCount,
						  PGPSocketSet * ioReadSet,
						  PGPSocketSet * ioWriteSet,
						  PGPSocketSet * ioErrorSet,
						  const PGPSocketsTimeValue * inTimeout);

/* DNS and protocol services */
PGPHostEntry *		PGPGetHostByName(const PGPChar8 * inName);
PGPHostEntry *		PGPGetHostByAddress(const PGPChar8 * inAddress,
										PGPInt32 inLength,
										PGPInt32 inType);
PGPInt32			PGPGetHostName(PGPChar8 * outName, PGPInt32 inNameLength);
PGPProtocolEntry *	PGPGetProtocolByName(const PGPChar8 * inName);
PGPProtocolEntry *	PGPGetProtocolByNumber(PGPInt32 inNumber);
PGPServiceEntry *	PGPGetServiceByName(const PGPChar8 * inName,
										const PGPChar8 * inProtocol);
PGPServiceEntry *	PGPGetServiceByPort(PGPInt32 inPort,
										const PGPChar8 * inProtocol);

/* Error reporting */
PGPError	PGPGetLastSocketsError(void);

/* Utilities */
PGPInt32	PGPGetSocketName(PGPSocketRef inSocketRef,
							 PGPSocketAddress * outName,
							 PGPInt32 * ioNameLength);
PGPInt32	PGPGetPeerName(PGPSocketRef inSocketRef,
						   PGPSocketAddress * outName,
						   PGPInt32 * ioNameLength);
PGPUInt32	PGPDottedToInternetAddress(const PGPChar8 * inAddress);
PGPChar8 *	PGPInternetAddressToDottedString(PGPInternetAddress inAddress);

/* Control and options */
PGPInt32	PGPIOControlSocket(PGPSocketRef inSocketRef,
							   PGPInt32 inCommand, PGPUInt32 * ioParam);
PGPInt32	PGPGetSocketOptions(PGPSocketRef inSocketRef, PGPInt32 inLevel,
								PGPInt32 inOptionName,
								PGPChar8 * outOptionValue,
								PGPInt32 * ioOptionLength);
PGPInt32	PGPSetSocketOptions(PGPSocketRef inSocketRef, PGPInt32 inLevel,
								PGPInt32 inOptionName,
								const PGPChar8 * inOptionValue,
								PGPInt32 inOptionLength);

/* TLS */
PGPError	PGPSocketsEstablishTLSSession(PGPSocketRef inSocketRef,
										  PGPtlsSessionRef inTLSSession);


PGP_END_C_DECLARATIONS

#endif /* Included_pgpSockets_h */
