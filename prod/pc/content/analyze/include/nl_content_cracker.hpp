/********************************************************************************************
 *
 * NextLabs Content Analysis File Cracking Interface
 *
 * This file describes the interface of file crackers.
 *
 *******************************************************************************************/

#ifndef __NL_CONTENT_CRACKER_HPP__
#define __NL_CONTENT_CRACKER_HPP__

#include <boost/utility.hpp>

namespace NLCA
{
  /** ContentCracker
   *
   *  \brief Abstract base for file cracker.  An object of this type may not be copied.
   */
  class ContentCracker : public boost::noncopyable
  {
    public:
      virtual ~ContentCracker(void) { /* empty */ }

      /** GetText
       *
       *  \brief Retreive text from a cracker.
       *
       *  \param buf (in/out)      Buffer.  The buffer will always be NULL terminated.
       *  \param buf_size (in/out) Size of buffer in characters provided by user.  This
       *                           value is set by GetText and indicates how many chars
       *                           were written to the callers buffer.
       *                           The size ready may be zero.  This does not indicate
       *                           an error has occurred.  A subsequent call to GetText
       *                           should be made.
       *                           The size of buf_size must not exceed NLCA::CHUNK_SIZE.
       *  \param eos (out)         End of stream.  There is no more data to read.
       *
       *  \return true on success, otherwise false.
       */
      virtual bool GetText( wchar_t* buf , unsigned long& buf_size , bool& eos ) = 0;

      /** GetText
       *
       *  \brief Read from a cracker instance such that several chunks may be stitched together into the
       *         callers buffer.  Multiple reads will be done until (a) we reach EOS (b) we fill up the
       *         buffer or (c) we read in_min_len bytes
       *
       *  \param in_buf (in)         Buffer to store reads  Will always be NULL terminated
       *  \param in_buf_len (in-out) Length of in_buf in chars.
       *  \param in_eos (out)        End-of-stream indicator when true.
       *  \param in_min_len (in)     Minimum length to make available to read.
       *
       *  \return true on successful read, otherwise false.
       */
      virtual bool GetText( _Out_ wchar_t* in_buf ,
                            _Inout_ unsigned long& in_buf_len ,
                            _Inout_ bool& in_eos ,
                            _In_ size_t in_min_len )
      {
          bool result = false;  /* default to fail */
          unsigned long total_len = 0; /* length (in chars) of the total read */
          bool prev_read_was_zero_bytes = false;

          in_min_len = min(in_min_len, in_buf_len-1);

          /* Read until end-of-stream or the caller's space is used up */
          while( in_eos == false ) 
          {
              unsigned long read_len = in_buf_len - total_len;

              result = GetText(in_buf + total_len,read_len,in_eos);

              if( result == false )
              {
                  break;  /* failed to get text */
              }
              
              /* May be between some {word,paragraph,etc.} break but not end-of-stream.  Unless we get two in a row */
              if( read_len == 0 && !prev_read_was_zero_bytes)
              {
                  prev_read_was_zero_bytes = true;
                  continue;
              }
              
              /* Update total_len with size of current string read.  The chunk read from
               * GetText may contain string termination before the indicated output size
               * from GetText.
               */
              total_len += static_cast<unsigned long>(wcslen(in_buf+total_len));
              
              /* Quit if we have enough or have run out of space or got two zero byte reads in a row*/
              if( total_len >= in_min_len || (in_buf_len - total_len) < MIN_BUF_SIZE || (prev_read_was_zero_bytes && read_len == 0))
              {
                  break;
              }

              prev_read_was_zero_bytes = (read_len == 0);
          }/* while !eos */
          
          in_buf_len = total_len; /* adjust callers size */
          
          return result;
      }/* GetText */

      /** Cancel
       *
       *  \brief Cancel (abort) text extraction.  This method is optional for derived
       *         classes.  The return value indicates that the cancel request was
       *         successful, not necessarily that the cancel has completed.  A cancel
       *         should force an end-of-file (eof) or end-of-stream (eos) such that
       *         GetText() may complete without reading until the actual termnal byte
       *         of the file.
       *
       *  \return true if cancel was successfully requested, otherwise false.
       */
      virtual bool Cancel(void) throw()
      {
          return false;
      }
	  
	  protected:
		static const int MIN_BUF_SIZE = 16;
  };/* ContentCracker */
}/* NLCA */

#endif /* __NL_CONTENT_CRACKER_HPP__ */
