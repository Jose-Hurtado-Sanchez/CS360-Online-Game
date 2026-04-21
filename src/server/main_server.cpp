#include "main_server.hpp"

int main()
{



    if (enet_initialize() != 0)
    {
        // enet initalization failed
       std::cout<<"ENET server initalization not successful"<<std::endl;
        return 1;

    }
    else
    {
       std::cout<<"ENET server initalization successful"<<std::endl;
    }

    // typical server declarations
    ENetAddress address;
    ENetHost* server; // host
    ENetEvent event; // will be used to check for events later



    address.host = ENET_HOST_ANY; 
    address.port = 7777; // picked port 7777 because yt tutorial but needs to be same as CLIENT -- CHECK LATER
    server = enet_host_create(&address,2,1,65536,65536); // 2 client can connect, 1 channel that is reliable, 64*1024 bytes 


    if (server == NULL)
    {
        std::cout<< "Sever was not created" << std::endl;
        return 1;
    }

    //SERVER GAME LOOP -------------------------------------------------------------------------------------
 
    while(true)
    { 
        while(enet_host_service(server,&event,10) >0) // while there is an event
        {
            switch(event.type) 
            {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Client Connected:" << std::endl;
                    onConnect(event.peer); // does something with the peer that connected 
                    break;

                case ENET_EVENT_TYPE_RECEIVE: // a input packet was recieved from a client
                    std::cout << "Server Received :" << std::endl;
                    onRecieve(event.peer,event.packet); // does something with the packet when recieved 
                    enet_packet_destroy(event.packet); //destroy old unsused packet
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Server disconected :" << std::endl;
                    onDisconnect(event.peer);
                    break;
                    
                case ENET_EVENT_TYPE_NONE:
                    std::cout<<"No Event :"<<std::endl;
                    break;

            }
        }
    }

    

    //SERVER GAME LOOP ------------------------------------------------------------------------------------
    return 0;
   
    
    atexit(enet_deinitialize); // once main returns anything deinitialize server

}