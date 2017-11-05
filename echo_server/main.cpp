#include <bits/stdc++.h>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

void echo(sf::TcpSocket& client)
{
        char buffer[1024];
        std::size_t received=0;
        client.receive(buffer,sizeof(buffer),received);
        std::string message(buffer);
        client.send(message.c_str(),message.size()+1);
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

    std::vector<sf::TcpSocket*> clients;

    sf::SocketSelector selector;

    selector.add(listener);

    sf::TcpSocket client;


    while(running)
    {
        if(selector.wait())
        {
            if(selector.isReady(listener))
            {
                sf::TcpSocket* client=new sf::TcpSocket;
                if(listener.accept(*client)==sf::Socket::Done)
                {
                    clients.push_back(client);
                    selector.add(*client);
                    std::cout<<"New client at "<<client->getRemoteAddress()<<'\n';
                }
                else
                {
                    std::cout<<"Client couldn't connect.\n";
                    delete client;
                }
            }
            else
            {
                for(std::vector<sf::TcpSocket*>::iterator it=clients.begin(); it!=clients.end(); it++)
                {
                    sf::TcpSocket& client= **it;
                    if(client.getRemotePort()==0)
                    {
                        std::cout<<"Client at "<<client.getRemoteAddress()<<" was disconnected\n";
                        selector.remove(client);
                        clients.erase(it);
                        it--;
                    }
                    else
                    {
                        if(selector.isReady(client))
                        {
                            echo(client);
                        }
                    }
                }
            }
        }
    }
    return 0;
}
