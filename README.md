# Description
This is a small C++ library that I forked from https://github.com/karastojko/mailio
# Main changes
1. Removed Imap and Pop3 related files
2. Reduced Boost dependencies. Specifically,
  1. using std::regex instead of boost::regex
  2. using standalone asio (https://think-async.com/Asio/) instead of boost::asio. Reverting to boost::asio should be straigthforward.
3. asio::io_context was declared as a static variable in dialog.cpp. It is now passed as a parameter. The main motivation was to use it a web server developed around asio.
# Example
The following example is adapted from Karastojko to use io_context as a parameter
```CPP
message msg;
msg.from(mail_address("mailio library", "mailio@gmail.com"));
msg.add_recipient(mail_address("mailio library", "mailio@gmail.com"));
msg.subject("smtp simple message");
msg.content("Hello, World!");
asio::io_context io_service;
smtp conn(io_service, "smtp.gmail.com", 587);
conn.authenticate("mailio@gmail.com", "mailiopass", smtp::auth_method_t::LOGIN);
conn.submit(msg);
```
The examples in the example folder compiles but the values such as server name, user name and password needs to be fixed
# Build Instructions
Asio header files must be added to the include folder
Boost header files are also needed for date-time functions.
When gcc implements some of the missing chrono functions I can remove the need for Boost.
Openssl, both dev and runtime need to be ----------


