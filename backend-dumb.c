/**
 * The default "dumb" backend which uses read() and write() for
 * _everything_. Slow, but should work everywhere.
 **/
#include <nbdsrv.h>
#include <unistd.h>

static ssize_t dumb_copy_to_buffer(NBD_BACKEND* be G_GNUC_UNUSED, int socket, void* buf, size_t len) {
	return read(socket, buf, len);
}

static ssize_t dumb_copy_from_buffer(NBD_BACKEND* be G_GNUC_UNUSED, int socket, void* buf, size_t len) {
	return write(socket, buf, len);
}

static ssize_t dumb_copy_to_file(NBD_BACKEND* be, int socket, off_t offset, size_t len) {
	uint8_t buf[len];
	
	ssize_t rlen = dumb_copy_to_buffer(be, socket, buf, len);
	ssize_t tlen = rlen;
	if(rlen < 0) {
		return rlen;
	}
	lseek(be->cur_file, offset, SEEK_SET);
	/* Need to loop here, because to do otherwise would be to lose data.
	 * Think about it. */
	do {
		ssize_t wlen = dumb_copy_from_buffer(be, be->cur_file, buf, rlen);
		if(wlen < 0) {
			return wlen;
		}
		rlen -= wlen;
	} while(rlen > 0);

	return tlen;
}

static ssize_t dumb_copy_from_file(NBD_BACKEND* be, int socket, off_t offset, size_t len) {
	uint8_t buf[len];

	lseek(be->cur_file, offset, SEEK_SET);
	ssize_t rlen = dumb_copy_to_buffer(be, be->cur_file, buf, len);
	ssize_t tlen = rlen;
	if(rlen < 0) {
		return rlen;
	}
	/* Need to loop here, because to do otherwise would be to lose data.
	 * Think about it. */
	do {
		ssize_t wlen = dumb_copy_from_buffer(be, socket, buf, rlen);
		if(wlen < 0) {
			return wlen;
		}
		rlen -= wlen;
	} while(rlen > 0);

	return tlen;
}

void nbd_dumb_init(NBD_BACKEND* be, SERVER* srv G_GNUC_UNUSED) {
	be->copy_to_file = dumb_copy_to_file;
	be->copy_from_file = dumb_copy_from_file;
	be->copy_to_buffer = dumb_copy_to_buffer;
	be->copy_from_buffer = dumb_copy_from_buffer;
}
