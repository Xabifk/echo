/*************************************
            CHAT SERVER
*************************************/


#include <bits/stdc++.h>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

class Server
{
public:

    Server(short unsigned port)
    {
        m_port = port;

        m_listening_thread = std::make_shared<std::thread>(&Server::listen ,this);
    }

    ~Server()
    {
        m_running_listener = false;

        m_listening_thread->join();

        m_listener.close();
        m_selector.clear();
    }

private:

    short unsigned m_port = 0;

    sf::TcpListener m_listener;
    sf::SocketSelector m_selector;

    std::vector<std::shared_ptr<sf::TcpSocket>> m_clients;

    std::atomic<bool> m_running_listener;
    std::shared_ptr<std::thread> m_listening_thread = nullptr;

    bool echo(sf::TcpSocket &client)
    {
        bool connected = true;

        sf::Packet packet;

        if(client.receive(packet) == sf::Socket::Done)
        {
            if(client.send(packet) != sf::Socket::Done)
            {
                connected = false;
            }
        }
        else
        {
            connected = false;
        }
        return connected;
    }

    void printInfo()
    {
        std::cout<<'['<<m_clients.size()<<" connected client(s)]\n";
    }

    bool acceptConnection()
    {
        bool connected = true;
        std::shared_ptr<sf::TcpSocket> new_client = std::make_shared<sf::TcpSocket>();

        if(m_listener.accept(*new_client) == sf::Socket::Done) // the connection is successful
        {
            m_clients.push_back(new_client);
            m_selector.add(*new_client);
        }
        else // the connection is not successful
        {
            connected = false;
        }

        return connected;
    }

    void removeClient(std::vector<std::shared_ptr<sf::TcpSocket>>::iterator &client_it)
    {
        m_selector.remove(**client_it);
        m_clients.erase(client_it);
    }

    void listen()
    {
        m_listener.setBlocking(false);
        sf::Time timeout = sf::seconds(5.0f);

        if(m_listener.listen(m_port) == sf::Socket::Done)
        {
            std::cout<<"Listening on port "<<m_port<<"!\n";
        }
        else
        {
            std::cout<<"Could not start server on port "<<m_port<<"\n";
            exit(1);
        }

        m_selector.add(m_listener);

        m_running_listener = true;
        while(m_running_listener == true)
        {
            if(m_selector.wait(timeout)) // wait for any activity
            {
                if(m_selector.isReady(m_listener)) // if we have a new connection
                {
                    acceptConnection();
                }
                else // check clients
                {
                    for(std::vector<std::shared_ptr<sf::TcpSocket>>::iterator client_it = m_clients.begin(); client_it != m_clients.end(); client_it++)
                    {
                        sf::TcpSocket& client = **client_it;

                        if(m_selector.isReady(client)) // the client has some data
                        {
                            if(!echo(client))
                            {
                                removeClient(client_it);
                                client_it--;
                            }
                        }
                        else if(client.getRemoteAddress() == sf::IpAddress::None)
                        {
                            removeClient(client_it);
                            client_it--;
                        }
                    }
                }
            }
            else
            {
                printInfo();
            }

        }
        //end of loop
    }

};


int main(int argc,char * argv[])
{
    if(argc!=2)
    {
        std::cout<<"Usage: <port>\n";
        return 1;
    }

    short unsigned port = atoi(argv[1]);
    Server server(port);
    std::string s;
    std::cin>>s;

    return 0;
}
