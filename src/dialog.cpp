/*

dialog.cpp
----------

Copyright (C) 2016, Tomislav Karastojkovic (http://www.alepho.com).

Distributed under the FreeBSD license, see the accompanying file LICENSE or
copy at http://www.freebsd.org/copyright/freebsd-license.html.

*/


#include <string>
#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <mailio/dialog.hpp>


using std::to_string;
using std::move;
using std::istream;
using std::make_shared;
using std::shared_ptr;
using std::bind;
using std::chrono::milliseconds;
using asio::ip::tcp;
using asio::buffer;
using asio::streambuf;
using asio::io_context;
using asio::steady_timer;
using asio::ssl::context;
using std::system_error;
using std::error_code;
using boost::algorithm::trim_if;
using boost::algorithm::is_any_of;


namespace mailio
{




dialog::dialog(io_context& ios,const std::string& hostname, unsigned port, milliseconds timeout) : std::enable_shared_from_this<dialog>(),
    ios_(ios),hostname_(hostname), port_(port), socket_(make_shared<tcp::socket>(ios_)), timer_(make_shared<steady_timer>(ios_)),
    timeout_(timeout), timer_expired_(false), strmbuf_(make_shared<streambuf>()), istrm_(make_shared<istream>(strmbuf_.get()))
{
}


dialog::dialog(const dialog& other) : std::enable_shared_from_this<dialog>(),
    hostname_(move(other.hostname_)),
    port_(other.port_), 
    socket_(other.socket_),
    timer_(other.timer_),
    timeout_(other.timeout_), 
    timer_expired_(other.timer_expired_), 
    strmbuf_(other.strmbuf_), 
    istrm_(other.istrm_),
    ios_(other.ios_)
{
}


void dialog::connect()
{
    try
    {
        if (timeout_.count() == 0)
        {
            tcp::resolver res(ios_);
            asio::connect(*socket_, res.resolve(hostname_, to_string(port_)));
        }
        else
            connect_async();
    }
    catch (const system_error& exc)
    {
        throw dialog_error("Server connecting failed.", exc.code().message());
    }
}


void dialog::send(const std::string& line)
{
    if (timeout_.count() == 0)
        send_sync(*socket_, line);
    else
        send_async(*socket_, line);
}


// TODO: perhaps the implementation should be common with `receive_raw()`
std::string dialog::receive(bool raw)
{
    if (timeout_.count() == 0)
        return receive_sync(*socket_, raw);
    else
        return receive_async(*socket_, raw);
}


template<typename Socket>
void dialog::send_sync(Socket& socket, const std::string& line)
{
    try
    {
        std::string l = line + "\r\n";
        write(socket, buffer(l, l.size()));
    }
    catch (const system_error& exc)
    {
        throw dialog_error("Network sending error.", exc.code().message());
    }
}


template<typename Socket>
std::string dialog::receive_sync(Socket& socket, bool raw)
{
    try
    {
        read_until(socket, *strmbuf_, "\n");
        std::string line;
        getline(*istrm_, line, '\n');
        if (!raw)
            trim_if(line, is_any_of("\r\n"));
        return line;
    }
    catch (const system_error& exc)
    {
        throw dialog_error("Network receiving error.", exc.code().message());
    }
}


void dialog::connect_async()
{
    tcp::resolver res(ios_);
    check_timeout();

    bool has_connected{false}, connect_error{false};
    error_code errc;
    async_connect(*socket_, res.resolve(hostname_, to_string(port_)),
        [&has_connected, &connect_error, &errc](const error_code& error, const asio::ip::tcp::endpoint&)
        {
            if (!error)
                has_connected = true;
            else
                connect_error = true;
            errc = error;
        });
    wait_async(has_connected, connect_error, "Network connecting timed out.", "Network connecting failed.", errc);
}


template<typename Socket>
void dialog::send_async(Socket& socket, std::string line)
{
    check_timeout();
    std::string l = line + "\r\n";
    bool has_written{false}, send_error{false};
    error_code errc;
    async_write(socket, buffer(l, l.size()),
        [&has_written, &send_error, &errc](const error_code& error, size_t)
        {
            if (!error)
                has_written = true;
            else
                send_error = true;
            errc = error;
        });
    wait_async(has_written, send_error, "Network sending timed out.", "Network sending failed.", errc);
}


template<typename Socket>
std::string dialog::receive_async(Socket& socket, bool raw)
{
    check_timeout();
    bool has_read{false}, receive_error{false};
    std::string line;
    error_code errc;
    async_read_until(socket, *strmbuf_, "\n",
        [&has_read, &receive_error, this, &line, &errc, raw](const error_code& error, size_t)
        {
            if (!error)
            {
                getline(*istrm_, line, '\n');
                if (!raw)
                    trim_if(line, is_any_of("\r\n"));
                has_read = true;
            }
            else
                receive_error = true;
            errc = error;
        });
    wait_async(has_read, receive_error, "Network receiving timed out.", "Network receiving failed.", errc);
    return line;
}


void dialog::wait_async(const bool& has_op, const bool& op_error, const char* expired_msg, const char* op_msg, const error_code& error)
{
    do
    {
        if (timer_expired_)
            throw dialog_error(expired_msg, error.message());
        if (op_error)
            throw dialog_error(op_msg, error.message());
        ios_.run_one();
    }
    while (!has_op);
}


void dialog::check_timeout()
{
    // Expiring automatically cancels the timer, per documentation.
    timer_->expires_after(timeout_);
    timer_expired_ = false;
    timer_->async_wait(bind(&dialog::timeout_handler, shared_from_this(), std::placeholders::_1));
}


void dialog::timeout_handler(const error_code& error)
{
    if (!error)
        timer_expired_ = true;
}


dialog_ssl::dialog_ssl(io_context& ios, const std::string& hostname, unsigned port, milliseconds timeout, const ssl_options_t& options) :
    dialog(ios, hostname, port, timeout), ssl_(false), context_(make_shared<context>(options.method)),
    ssl_socket_(make_shared<asio::ssl::stream<tcp::socket&>>(*socket_, *context_))
{
}


dialog_ssl::dialog_ssl(const dialog& other, const ssl_options_t& options) : dialog(other), context_(make_shared<context>(options.method)),
    ssl_socket_(make_shared<asio::ssl::stream<tcp::socket&>>(*socket_, *context_))
{
    try
    {
        ssl_socket_->set_verify_mode(options.verify_mode);
        ssl_socket_->handshake(asio::ssl::stream_base::client);
        ssl_ = true;
    }
    catch (const system_error& exc)
    {
        // TODO: perhaps the message is confusing
        throw dialog_error("Switching to SSL failed.", exc.code().message());
    }
}


void dialog_ssl::send(const std::string& line)
{
    if (!ssl_)
    {
        dialog::send(line);
        return;
    }

    if (timeout_.count() == 0)
        send_sync(*ssl_socket_, line);
    else
        send_async(*ssl_socket_, line);
}


std::string dialog_ssl::receive(bool raw)
{
    if (!ssl_)
        return dialog::receive(raw);

    try
    {
        if (timeout_.count() == 0)
            return receive_sync(*ssl_socket_, raw);
        else
            return receive_async(*ssl_socket_, raw);
    }
    catch (const system_error& exc)
    {
        throw dialog_error("Network receiving error.", exc.code().message());
    }
}


shared_ptr<dialog_ssl> dialog_ssl::to_ssl(const shared_ptr<dialog> dlg, const dialog_ssl::ssl_options_t& options)
{
    return make_shared<dialog_ssl>(*dlg, options);
}


std::string dialog_error::details() const
{
    return details_;
}

} // namespace mailio
