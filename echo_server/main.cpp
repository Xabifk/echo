#include <bits/stdc++.h>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

void echo(sf::TcpSocket& client)
{
    while(client.getRemotePort() != 0)
    {
        char buffer[1024];
        std::size_t received=0;
        client.receive(buffer,sizeof(buffer),received);
        std::string message(buffer);
        client.send(message.c_str(),message.size()+1);
    }
}

int main(int argc,char * argv[])
{
    if(argc!=2)
    {
        std::cout<<"Usage: <port>\n";
        return 0;
    }
    int port=atoi(argv[1]);
    sf::TcpListener listener;
    bool running = false;
    if(listener.listen(port)==sf::Socket::Status::Done)
    {
        running = true;
        std::cout<<"Server running on port "<<listener.getLocalPort()<<'\n';
    }

    sf::TcpSocket client;
    while(running)
    {
        if(listener.accept(client) == sf::Socket::Done)
        {
            std::cout<<"New connection at: "<<client.getRemoteAddress()<<"\n";
            echo(client);
        }
    }
    return 0;
}
