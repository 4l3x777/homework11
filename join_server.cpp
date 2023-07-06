#include <iostream>
#include "join_server.h"

//echo INSERT A 0 lean | nc localhost 9000
//echo INSERT A 1 sweater | nc localhost 9000
//echo INSERT A 2 frank | nc localhost 9000
//echo INSERT A 3 violation | nc localhost 9000
//echo INSERT A 4 quality | nc localhost 9000
//echo INSERT A 5 precision | nc localhost 9000

//echo INSERT B 3 proposal | nc localhost 9000
//echo INSERT B 4 example | nc localhost 9000
//echo INSERT B 5 lake | nc localhost 9000
//echo INSERT B 6 flour | nc localhost 9000
//echo INSERT B 7 wonder | nc localhost 9000
//echo INSERT B 8 selection | nc localhost 9000

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: join_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        join_server::server server(io_context, std::atoi(argv[1]));

        io_context.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    return 0;
}