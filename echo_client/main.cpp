/*************************************
            ECHO CLIENT
*************************************/

#include <bits/stdc++.h>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

class Server
{
public:
    void setIpAddress(sf::IpAddress ipaddress)
    {
        m_ipaddress = ipaddress;
    }

    void setPort(short unsigned port)
    {
        m_port = port;
    }

    void setIpPort(sf::IpAddress ipaddress, short unsigned port)
    {
        setIpAddress(ipaddress);
        setPort(port);
    }

    void connect()
    {
        if(m_selector_thread != nullptr)
        {
            std::cout<<"[You are already connected]\n";
            return;
        }

        if(m_server.connect(m_ipaddress,m_port) == sf::Socket::Done)
        {
            std::cout<<"[Connected to server "<<m_ipaddress<<':'<<m_port<<"]\n";
        }
        else
        {
            std::cout<<"[Couldn't connect to server "<<m_ipaddress<<':'<<m_port<<"]\n";
            exit(1);
        }

        m_selector.add(m_server);

        m_selector_thread = std::make_shared<std::thread>(&Server::listen, this);
    }

    void connect(sf::IpAddress ipaddress, short unsigned port)
    {
        setIpPort(ipaddress, port);

        if(m_selector_thread != nullptr)
            disconnect();

        connect();
    }

    void disconnect()
    {
        if(m_selector_thread == nullptr)
        {
            std::cout<<"[You are not connected]\n";
            return;
        }

        std::cout<<"[Disconnecting...]\n";

        m_running_listener = false;
        m_selector_thread->join();
        m_selector_thread = nullptr;
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

    Server(sf::IpAddress ipaddress, short unsigned port)
    {
        setIpPort(ipaddress,port);

        connect();
    }

    ~Server()
    {
        disconnect();
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
        sf::Time timeout = sf::seconds(1.0f);
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

class Command
{
public:
    bool isRunning()
    {
        return m_running;
    }

    bool isCommand(std::string command)
    {
        if(command[0] == '/')
            return true;
        else
            return false;
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
        case eDisconnect:
            m_server->disconnect();
            break;
        case eConnect:
            m_server->connect();
            break;
        case eRestart:
            m_server->disconnect();
            m_server->connect();
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

    std::unique_ptr<Server> m_server = nullptr;

    enum m_string_code
    {
        eExit, eDisconnect, eConnect, eRestart, eUndefined
    };

    m_string_code getStringCode(std::string command)
    {
        if(command == "/exit") return eExit;
        if(command == "/disconnect") return eDisconnect;
        if(command == "/connect") return eConnect;
        if(command == "/restart") return eRestart;
        return eUndefined;
    }


};
int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        std::cout<<"Usage: <address> <port>\n";
        return 0;
    }
    sf::IpAddress ipaddress(argv[1]);
    short unsigned port = atoi(argv[2]);

    Server server(ipaddress, port);
    Command command(server);

    std::string message;

    while(command.isRunning())
    {
        getline(std::cin, message);

        if(command.isCommand(message))
            command.execute(message);
        else
            server.send(message);
    }

    return 0;
}
