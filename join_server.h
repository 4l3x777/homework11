#pragma once
#include <memory>
#include <utility>
#include <vector>
#include <cstdlib>
#include <list>

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "sql_engine.h"

namespace join_server
{

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start(std::shared_ptr<sqlite3cpp::SQLDB> sqldb)
    {
        do_read(sqldb);
    }

private:
    void do_read(std::shared_ptr<sqlite3cpp::SQLDB> sqldb)
    {
        auto self(shared_from_this());
        socket_.async_read_some(
            boost::asio::buffer(data_, max_length),
            [this, self, sqldb](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    if (length)
                    {
                        std::vector<std::string> commands;
                        boost::split(commands, std::string{data_, length - 1}, boost::is_any_of("\n "));

                        std::string message;
                        memset(data_, 0x0, max_length);
                        if (commands.size() >= 1)
                        {
                            //echo INSERT A 0 lean | nc localhost 9000
                            if (commands[0] == "INSERT" && commands.size() == 4) 
                            {
                                std::string sql_request = 
                                    commands[0] + 
                                    " INTO " + 
                                    commands[1] + 
                                    " VALUES(" + commands[2] + ",\'" + commands[3] + "\')";
                                message = sqldb->request(sql_request);
                            }
                            //echo TRUNCATE A | nc localhost 9000
                            else if (commands[0] == "TRUNCATE" && commands.size() == 2)
                            {
                                //TRUNCATE has deprecated in sqlite -> used DELETE
                                std::string sql_request = "DELETE FROM " + commands[1];
                                message = sqldb->request(sql_request);
                                if (message == "OK") sqldb->request("VACUUM");
                            }
                            //INTERSECTION
                            else if (commands[0] == "INTERSECTION" && commands.size() == 1)
                            {
                                std::string sql_request = "SELECT A.id || ',' || A.name || ',' || B.name  FROM A INNER JOIN B ON A.id = B.id";
                                message = sqldb->request(sql_request);
                            }
                            //echo SYMMETRIC_DIFFERENCE | nc localhost 9000
                            else if (commands[0] == "SYMMETRIC_DIFFERENCE" && commands.size() == 1)
                            {
                                std::string sql_request = "SELECT CASE WHEN A.id IS NOT NULL THEN A.id ELSE B.id END || ',' || CASE WHEN A.name IS NOT NULL THEN A.name ELSE '' END || ',' || CASE WHEN B.name IS NOT NULL THEN B.name ELSE '' END FROM A FULL OUTER JOIN B ON A.id = B.id WHERE A.id IS NULL OR B.id IS NULL";
                                message = sqldb->request(sql_request);
                            }
                            else 
                            {
                                message = "Unknown command!";
                            }
                        }
                        else
                        {
                            message = "Unknown command!";
                        }
                        
                        if (!message.empty()) 
                        {
                            // error message interpreter
                            if (message.find("UNIQUE constraint failed") != std::string::npos)
                            {
                                message = "ERR duplicate " + commands[2];
                            }

                            strncpy(data_, message.data(), message.size());
                            do_write(message.size());
                        }
                    }
                }
            }
        );
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(
            socket_, 
            boost::asio::buffer(data_, length),
            [this, self](boost::system::error_code ec, std::size_t) {}
        );
    }

    tcp::socket socket_;
    enum { max_length = 2048 };
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_context& io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        sqldb = std::make_shared<sqlite3cpp::SQLDB>();
        if (sqldb->initDB()) do_accept();
        else return;
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<session>(std::move(socket))->start(sqldb);
                }
                do_accept();
            }
        );
    }
    std::shared_ptr<sqlite3cpp::SQLDB> sqldb;
    tcp::acceptor acceptor_;
};

};