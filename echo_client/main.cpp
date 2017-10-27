#include <bits/stdc++.h>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>


int main(int argc,char * argv[])
{
    if(argc!=3)
    {
        std::cout<<"Usage: <address> <port>\n";
        return 0;
    }
    sf::IpAddress ip(argv[1]);
    int port = atoi(argv[2]);

    sf::TcpSocket server;
    sf::Socket::Status status = server.connect(ip,port);

    if(status != sf::Socket::Done)
    {
        std::cout<<"Can't connect to "<<ip<<' '<<port<<'\n';
    }
    else
    {
        while(status == sf::Socket::Done)
        {
            std::string message;
            std::getline(std::cin,message);
            if(message=="/exit")
                status = sf::Socket::Disconnected;
            else
            {
                server.send(message.c_str(),message.size()+1);
                char buffer[1024];
                std::size_t received=0;
                server.receive(buffer,sizeof(buffer),received);
                std::cout<<buffer<<'\n';
            }
        }
        std::cout<<"Disconnected form server.\n";
    }
    return 0;
}
