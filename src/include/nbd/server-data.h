#ifndef NBD_SERVER_DATA_H
#define NBD_SERVER_DATA_H

/** Per-export flags: */
#define F_READONLY 1      /**< flag to tell us a file is readonly */
#define F_MULTIFILE 2	  /**< flag to tell us a file is exported using -m */
#define F_COPYONWRITE 4	  /**< flag to tell us a file is exported using
			    copyonwrite */
#define F_AUTOREADONLY 8  /**< flag to tell us a file is set to autoreadonly */
#define F_SPARSE 16	  /**< flag to tell us copyronwrite should use a sparse file */
#define F_SDP 32	  /**< flag to tell us the export should be done using the Socket Direct Protocol for RDMA */
#define F_SYNC 64	  /**< Whether to fsync() after a write */
#define F_FLUSH 128	  /**< Whether server wants FLUSH to be sent by the client */
#define F_FUA 256	  /**< Whether server wants FUA to be sent by the client */
#define F_ROTATIONAL 512  /**< Whether server wants the client to implement the elevator algorithm */
#define F_TEMPORARY 1024  /**< Whether the backing file is temporary and should be created then unlinked */
#define F_TRIM 2048       /**< Whether server wants TRIM (discard) to be sent by the client */
#define F_FIXED 4096	  /**< Client supports fixed new-style protocol (and can thus send us extra options */

/**
 * Types of virtualization
 **/
typedef enum {
	VIRT_NONE=0,	/**< No virtualization */
	VIRT_IPLIT,	/**< Literal IP address as part of the filename */
	VIRT_IPHASH,	/**< Replacing all dots in an ip address by a / before
			     doing the same as in IPLIT */
	VIRT_CIDR,	/**< Every subnet in its own directory */
} VIRT_STYLE;

/**
 * Variables associated with a server.
 **/
typedef struct {
	gchar* exportname;    /**< (unprocessed) filename of the file we're exporting */
	off_t expected_size; /**< size of the exported file as it was told to
			       us through configuration */
	gchar* listenaddr;   /**< The IP address we're listening on */
	unsigned int port;   /**< port we're exporting this file at */
	char* authname;      /**< filename of the authorization file */
	int flags;           /**< flags associated with this exported file */
	int socket;	     /**< The socket of this server. */
	int socket_family;   /**< family of the socket */
	VIRT_STYLE virtstyle;/**< The style of virtualization, if any */
	uint8_t cidrlen;     /**< The length of the mask when we use
				  CIDR-style virtualization */
	gchar* prerun;	     /**< command to be ran after connecting a client,
				  but before starting to serve */
	gchar* postrun;	     /**< command that will be ran after the client
				  disconnects */
	gchar* servename;    /**< name of the export as selected by nbd-client */
	int max_connections; /**< maximum number of opened connections */
	gchar* transactionlog;/**< filename for transaction log */
} SERVER;

#endif
