/*************************************
            ECHO SERVER
*************************************/


#include <bits/stdc++.h>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

class Server
{
public:

    void setPort(short unsigned port)
    {
        m_port = port;
    }

    void startServer()
    {
        if(m_listening_thread != nullptr)
        {
            std::cout<<"[Server it's already running]\n";
            return;
        }
        m_listening_thread = std::make_shared<std::thread>(&Server::listen ,this);
    }

    void stopServer()
    {
        if(m_listening_thread == nullptr)
        {
            std::cout<<"[Server is already stopped]\n";
            return;
        }

        m_running_listener = false;

        m_listening_thread->join();
        m_listening_thread = nullptr;

        m_listener.close();
        m_selector.clear();
    }

    unsigned getNrClients()
    {
        return m_clients.size();
    }

    Server()
    {}

    Server(short unsigned port)
    {
        setPort(port);

        startServer();
    }

    ~Server()
    {
        stopServer();
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
        sf::Time timeout = sf::seconds(1.0f);

        if(m_listener.listen(m_port) == sf::Socket::Done)
        {
            std::cout<<"[Listening on port "<<m_port<<"!]\n";
        }
        else
        {
            std::cout<<"[Could not start server on port "<<m_port<<"]\n";
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

        }
        //end of loop
    }

};

class Command
{
public:

    bool isRunning()
    {
        return m_running;
    }

    void execute(std::string command)
    {
        if(m_server == nullptr)
        {
            std::cerr<<"[No server linked]\n";
            return;
        }

        switch(getStringCode(command))
        {
        case eExit:
            m_running = false;
            break;
        case eStop:
            m_server->stopServer();
            break;
        case eStart:
            m_server->startServer();
            break;
        case eRestart:
            m_server->stopServer();
            std::cout<<"[Restarting...]\n";
            m_server->startServer();
            break;
        case eStatus:
            std::cout<<"["<<m_server->getNrClients()<<" connected client(s)]\n";
            break;
        default:
            std::cout<<"[Command not recognised]\n";
            break;
        }
    }

    void setServer(Server &server)
    {
        m_server.reset(&server);
    }

    Command(Server &server)
    {
        setServer(server);
    }

    Command()
    {}

    ~Command()
    {
        m_server.release();
    }

private:
    bool m_running = true;

    std::unique_ptr<Server> m_server = NULL;

    enum m_string_code
    {
        eExit, eStart, eStop, eRestart, eStatus, eUndefined
    };

    m_string_code getStringCode(std::string command)
    {
        if(command == "/exit") return eExit;
        if(command == "/start") return eStart;
        if(command == "/stop") return eStop;
        if(command == "/restart") return eRestart;
        if(command == "/status") return eStatus;
        return eUndefined;
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
    Command command(server);

    sf::Time time = sf::seconds(0.5f);
    sf::sleep(time); //race condition

    std::string com;

    while(command.isRunning())
    {
        std::cout<<'>';
        std::cin>>com;
        command.execute(com);
    }

    return 0;
}
