/*************************************
            ECHO CLIENT
*************************************/

#include <bits/stdc++.h>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

class Server
{
public:
    Server(sf::IpAddress ipaddress, short unsigned port)
    {
        if(m_server.connect(ipaddress,port) == sf::Socket::Done)
        {
            std::cout<<"[Connected to server "<<ipaddress<<':'<<port<<"]\n";
        }
        else
        {
            std::cout<<"[Couldn't connect to server "<<ipaddress<<':'<<port<<"]\n";
            exit(1);
        }

        m_selector.add(m_server);

        m_selector_thread = std::make_shared<std::thread>(&Server::listen, this);
    }

    ~Server()
    {
        std::cout<<"[Disconnecting...]\n";

        m_running_listener = false;
        m_selector_thread->join();
        m_server.disconnect();
    }

    void send(const std::string message)
    {
        sf::Packet packet;

        packet<<message;
        if(m_server.send(packet) != sf::Socket::Done)
        {
            errorDisconnect();
            exit(1);
        }
    }



private:
    sf::IpAddress m_ipaddress;
    short unsigned m_port;

    sf::TcpSocket m_server;
    sf::SocketSelector m_selector;

    std::atomic<bool> m_running_listener;
    std::shared_ptr<std::thread> m_selector_thread = nullptr;

    void errorDisconnect()
    {
        std::cout<<"[Connection lost. Disconnecting]\n";
        m_server.disconnect();
    }

    void receive()
    {
        sf::Packet packet;
        std::string message;

        if(m_server.receive(packet) != sf::Socket::Done)
        {
            errorDisconnect();
            exit(1);
        }

        packet>>message;
        std::cout<<message<<'\n';
    }

    void listen()
    {
        sf::Time timeout = sf::seconds(5.0f);
        m_running_listener = true;
        while(m_running_listener)
        {
            if(m_selector.wait(timeout))
            {
                if(m_selector.isReady(m_server))
                {
                    receive();
                }
            }
        }
    }

};
int main(int argc,char * argv[])
{
    if(argc!=3)
    {
        std::cout<<"Usage: <address> <port>\n";
        return 0;
    }
    sf::IpAddress ipaddress(argv[1]);
    short unsigned port = atoi(argv[2]);

    Server server(ipaddress, port);

    std::string message;
    bool running = true;

    while(running)
    {
        std::cin>>message;

        if(message == "/exit")
        {
            running = false;
            break;
        }

        server.send(message);
    }

    return 0;
}
