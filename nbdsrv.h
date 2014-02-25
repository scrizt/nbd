#ifndef NBDSRV_H
#define NBDSRV_H

#include "lfs.h"

#include <glib.h>
#include <stdbool.h>
#include <stdint.h>

#include <sys/socket.h>
#include <sys/types.h>

/* Structures */

/**
 * Types of virtuatlization
 **/
typedef enum {
	VIRT_NONE=0,	/**< No virtualization */
	VIRT_IPLIT,	/**< Literal IP address as part of the filename */
	VIRT_IPHASH,	/**< Replacing all dots in an ip address by a / before
			     doing the same as in IPLIT */
	VIRT_CIDR,	/**< Every subnet in its own directory */
} VIRT_STYLE;

typedef struct nbd_backend NBD_BACKEND;
typedef struct nbd_client CLIENT;

/**
  * A callback function
  *
  * @param be the backend that triggered this callback
  * @param cl the client for which we're running this callback
  * @param commdata data we read from the socket. May be NULL in case this
  * callback was not called to (help) handle a read request
  * @param userdata data which was passed along when this callback was registered
  */
typedef void(*nbd_callback)(NBD_BACKEND* be, CLIENT* cl, void* commdata, void* userdata);
/**
  * Function pointer to register an expectation for data
  *
  * @param callback the callback function to call when the expected data has arrived
  * @param be the backend which expects the data
  * @param len the expected length
  * @param userdata data to be passed on to the callback function
  */
typedef void(*nbd_expectfunc)(nbd_callback callback, NBD_BACKEND* be, size_t len, void* userdata);
/**
  * A function which performs raw I/O to a file, using a buffer
  *
  * @param fd the file to read from or write to
  * @param buf the memory buffer
  * @param len (expected) size of the data to read or write
  */
typedef void(*nbd_raw_iofunc)(int fd, void* buf, size_t len);
/**
  * A function to perform "fast" I/O.
  *
  * @param be the backend related to this call
  * @param socket the network socket to read from or write to
  * @param offset the offset in the backend file where we should start reading or writing
  * @param len the length of the data to read or write
  * @param finalize a function to call when the I/O operation has completed
  * @param userdata data to pass along to the finalize function
  */
typedef void(*nbd_iofunc)(NBD_BACKEND* be, int socket, off_t offset, size_t len, nbd_callback finalize, void* userdata);
/**
  * A function to initialize a backend
  *
  * @todo do we need this? Probably don't.
  */
typedef bool(*nbd_initfunc)(NBD_BACKEND*, int socket, CLIENT* client);

typedef struct _backend_template {
	nbd_initfunc init;	   /**< initializer. */
	nbd_initfunc deinit;	   /**< deinitializer. */
} NBD_BACKEND_TEMPLATE;

struct nbd_backend {
	struct nbd_backend* next;/**< The next in case of a stack of
				     backends. May be NULL. */
	int cur_file;		/**< The file that this backend should
				     read from or write to this one time */
	int net;		/**< The socket that this backend should
				     read from or write to this one time */
	void* data;		/**< Opaque data for this particular
				     backend. */
	CLIENT* client;		/**< The client to which this backend belongs */
	nbd_iofunc copy_to_file;   /**< function to copy from a socket to a
					file. Backend should attempt to use the
					most efficient manner possible to do
					so. */
	nbd_iofunc copy_to_socket; /**< function to copy from a file to a
					socket. Backend should attempt to use
					the most efficient manner possible to
					do so. */
	nbd_iofunc trim;	   /**< function to trim data in the backend */
	nbd_raw_iofunc send_data;  /**< function to enqueue data for writing to a
					socket.  May be called by the
					copy_to_socket function for a buffered
					backend, but is mainly meant for
					writing headers and negotiation. */
	nbd_expectfunc expect_data;/**< function to register a callback for
					available data. May be called by the
					copy_to_file function for a buffered
					backend, but is mainly meant for
					reading headers and negotiation. */
	nbd_callback read_ready;   /**< called when the socket can be
					read from. This callback should
					assume that the socket is in
					non-blocking mode, and should
					read as much data as it can in
					one go, parse that if possible,
					and then return. */
	nbd_callback write_ready;  /**< called when the socket can be
					written to. This callback should
					assume that the socket is in
					non-blocking mode, and should
					write as much data as it can in
					one go, and then return. */
	void(*flush)();		   /**< called when the backend storage should
					be flushed. */
};

/**
 * Variables associated with a server.
 **/
typedef struct {
	gchar* exportname;     /**< (unprocessed) filename of the file we're exporting */
	off_t expected_size;   /**< size of the exported file as it was told to
			       us through configuration */
	gchar* listenaddr;     /**< The IP address we're listening on */
	unsigned int port;     /**< port we're exporting this file at */
	char* authname;        /**< filename of the authorization file */
	int flags;             /**< flags associated with this exported file */
	int socket;	       /**< The socket of this server. */
	int socket_family;     /**< family of the socket */
	VIRT_STYLE virtstyle;  /**< The style of virtualization, if any */
	uint8_t cidrlen;       /**< The length of the mask when we use
				  CIDR-style virtualization */
	gchar* prerun;	       /**< command to be ran after connecting a client,
				  but before starting to serve */
	gchar* postrun;	       /**< command that will be ran after the client
				  disconnects */
	gchar* servename;      /**< name of the export as selected by nbd-client */
	int max_connections;   /**< maximum number of opened connections */
	gchar* transactionlog; /**< filename for transaction log */
} SERVER;

/**
  * Variables associated with a client connection
  */
struct nbd_client {
	uint64_t exportsize;    /**< size of the file we're exporting */
	char *clientname;    /**< peer, in human-readable format */
	struct sockaddr_storage clientaddr; /**< peer, in binary format, network byte order */
	char *exportname;    /**< (processed) filename of the file we're exporting */
	GArray *export;    /**< array of FILE_INFO of exported files;
			       array size is always 1 unless we're
			       doing the multiple file option */
	int net;	     /**< The actual client socket */
	SERVER *server;	     /**< The server this client is getting data from */
	char* difffilename;  /**< filename of the copy-on-write file, if any */
	int difffile;	     /**< filedescriptor of copyonwrite file. @todo
			       shouldn't this be an array too? (cfr export) Or
			       make -m and -c mutually exclusive */
	uint32_t difffilelen;     /**< number of pages in difffile */
	uint32_t *difmap;	     /**< see comment on the global difmap for this one */
	gboolean modern;     /**< client was negotiated using modern negotiation protocol */
	int transactionlogfd;/**< fd for transaction log */
	int clientfeats;     /**< Features supported by this client */
	bool want_write;     /**< true if we want to write to the socket. */
	bool active;         /**< true if the client connection is still active. */
	NBD_BACKEND* backend;/**< The first backend */
};

/* Constants and macros */

/**
 * Error domain common for all NBD server errors.
 **/
#define NBDS_ERR g_quark_from_static_string("server-error-quark")

/**
 * NBD server error codes.
 **/
typedef enum {
        NBDS_ERR_CFILE_NOTFOUND,          /**< The configuration file is not found */
        NBDS_ERR_CFILE_MISSING_GENERIC,   /**< The (required) group "generic" is missing */
        NBDS_ERR_CFILE_KEY_MISSING,       /**< A (required) key is missing */
        NBDS_ERR_CFILE_VALUE_INVALID,     /**< A value is syntactically invalid */
        NBDS_ERR_CFILE_VALUE_UNSUPPORTED, /**< A value is not supported in this build */
        NBDS_ERR_CFILE_NO_EXPORTS,        /**< A config file was specified that does not
                                               define any exports */
        NBDS_ERR_CFILE_INCORRECT_PORT,    /**< The reserved port was specified for an
                                               old-style export. */
        NBDS_ERR_CFILE_DIR_UNKNOWN,       /**< A directory requested does not exist*/
        NBDS_ERR_CFILE_READDIR_ERR,       /**< Error occurred during readdir() */
        NBDS_ERR_SO_LINGER,               /**< Failed to set SO_LINGER to a socket */
        NBDS_ERR_SO_REUSEADDR,            /**< Failed to set SO_REUSEADDR to a socket */
        NBDS_ERR_SO_KEEPALIVE,            /**< Failed to set SO_KEEPALIVE to a socket */
        NBDS_ERR_GAI,                     /**< Failed to get address info */
        NBDS_ERR_SOCKET,                  /**< Failed to create a socket */
        NBDS_ERR_BIND,                    /**< Failed to bind an address to socket */
        NBDS_ERR_LISTEN,                  /**< Failed to start listening on a socket */
        NBDS_ERR_SYS,                     /**< Underlying system call or library error */
} NBDS_ERRS;

/**
  * Logging macros.
  *
  * @todo remove this. We should use g_log in all cases, and use the
  * logging mangler to redirect to syslog if and when necessary.
  */
#ifdef ISSERVER
#define msg(prio, ...) syslog(prio, __VA_ARGS__)
#else
#define msg(prio, ...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, __VA_ARGS__)
#endif
#define MY_NAME "nbd_server"

/* Functions */

/**
  * Check whether a given address matches a given netmask.
  *
  * @param mask the address or netmask to check against, in ASCII representation
  * @param addr the address to check, in network byte order
  * @param af the address family of the passed address (AF_INET or AF_INET6)
  *
  * @return true if the address matches the mask, false otherwise; in case of
  * failure to parse netmask, returns false with err set appropriately.
  * @todo decide what to do with v6-mapped IPv4 addresses.
  */
bool address_matches(const char* mask, const void* addr, int af, GError** err);

/**
  * Gets a byte to allow for address masking.
  *
  * @param masklen the length of the requested mask.
  * @return if the length of the mask is 8 or longer, 0xFF. Otherwise, a byte
  * with `masklen' number of leading bits set to 1, everything else set to 0.
  */
uint8_t getmaskbyte(int masklen) G_GNUC_PURE;

/**
 * Check whether a client is allowed to connect. Works with an authorization
 * file which contains one line per machine or network, with CIDR-style
 * netmasks.
 *
 * @param opts The client who's trying to connect.
 * @return 0 - authorization refused, 1 - OK
 **/
int authorized_client(CLIENT *opts);

/**
 * duplicate server
 * @param s the old server we want to duplicate
 * @return new duplicated server
 **/
SERVER* dup_serve(const SERVER *const s);

/**
 * append new server to array
 * @param s server
 * @param a server array
 * @return 0 success, -1 error
 */
int append_serve(const SERVER *const s, GArray *const a);

/**
 * Detect the size of a file.
 *
 * @param fhandle An open filedescriptor
 * @return the size of the file, or UINT64_MAX if detection was
 * impossible.
 **/
uint64_t size_autodetect(int fhandle);
#endif //NBDSRV_H
