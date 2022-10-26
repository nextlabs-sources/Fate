
#ifndef __NLCA_FRAMEWORK_NETWORK_HPP__
#define __NLCA_FRAMEWORK_NETWORK_HPP__

/** nl_recv_sync
 *
 *  \brief Blocking recv until a specified number of bytes have been read.
 *
 *  \param s (in)          Socket.
 *  \param buf (out)       Buffer to write received data.
 *  \param len (in)        Number of bytes to read.
 *  \param flags (in)      Flags for recv().
 *  \param timeout_ms (in) Timeout (in milliseconds) for value for each recv call.  There
 *                         may be up to len recv calls, so the range of wait (blocked)
 *                         time is [0,len*tv].  Timeout of 0 means block.
 *
 *  \return See recv() for return value for general notes.  Return of 0 is graceful
 *          disconnect or timeout expired.
 */
static int nl_recv_sync( nlsocket s , char* buf , int len , int flags , int timeout_ms )
{
  assert( buf != NULL && len > 0 );
  int bytes_remaining = len;

  // Read all request bytes with successive recv's.
  for( ; bytes_remaining > 0 ; )
  {
    timeval* tv = NULL;                   // timeout for select (default is block (NULL))
    timeval curr_tv;
    curr_tv.tv_sec  = timeout_ms / 1000;           // seconds
    curr_tv.tv_usec = (timeout_ms % 1000) * 1000;  // microseconds
    if( timeout_ms > 0 )
    {
      tv = &curr_tv;                      // caller's timeout is non-zero and should be used
    }

    fd_set rfds;
    FD_ZERO(&rfds);  // empty descriptor set
    FD_SET((SOCKET)s,&rfds); // add caller's socket to descriptor set

    // Either block (in_tv == NULL) use use copy of caller's timeval
    int count = select(0,&rfds,NULL,NULL,tv);
    // If the socket is not readable fail back to the caller
    if( count != 1 )
    {
      return 0;
    }

    // attempt to read remaining bytes
    int bytes_read = recv(s,                             // caller's socket
			  buf + (len - bytes_remaining), // caller's buf to curr pos
			  bytes_remaining,               // bytes to read
			  flags);                        // caller's flags

    // gracefult disconnect or error?
    if( bytes_read <= 0 )
    {
      return bytes_read;
    }
    // decrement remaining bytes since 'bytes_read' was just received
    bytes_remaining -= bytes_read;
  }//for
  return len;
}//nl_recv_sync

#endif /* __NLCA_FRAMEWORK_NETWORK_HPP */
