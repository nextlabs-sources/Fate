

#ifndef __ASYNC_PIPE_HPP__
#define __ASYNC_PIPE_HPP__

#include <string>
#include <vector>
#include <thread>

namespace NX {
namespace async_pipe {


class server
{
public:
    server();
    server(unsigned long size, unsigned long timeout=3000);
    virtual ~server();

    void listen(const std::wstring& port);
    void shutdown();
    void read(void* context);

    virtual void on_read(unsigned char* data, unsigned long* size, bool* write_response);

    static const unsigned long default_buffer_size;

    inline unsigned long buffer_size() const { return _buffer_size; }
    
private:
    static void main_worker(server* serv) ;

    HANDLE listen_to_pipe(bool* pending) ;
    void accept(HANDLE pipe, OVERLAPPED* overlap, bool pending) ;

private:
    std::wstring    _name;
    unsigned long   _buffer_size;
    unsigned long   _timeout;
    std::thread     _listen_thread;
    OVERLAPPED      _overlap;
    bool            _shutting_down;
};


class client
{
public:
    client();
    client(unsigned long buffer_size);
    virtual ~client();

    virtual bool connect(const std::wstring& name, unsigned long timeout=3000) ;
    virtual void disconnect();

    virtual bool read(std::vector<unsigned char>& data, unsigned long timeout=5000);
    virtual bool write(const std::vector<unsigned char>& data, unsigned long timeout= 5000);

    HANDLE pipe() { return _pipe; }


private:
    HANDLE          _pipe;
    unsigned long   _pipe_mode;
    OVERLAPPED      _overlap;
    std::vector<unsigned char>  _buffer;
};



}   // namespace NX::async_pipe
}   // namespace NX


#endif