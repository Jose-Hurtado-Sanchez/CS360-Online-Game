#include "main_server.hpp"

int main()
{



    if (enet_initialize() != 0)
    {
        // enet initalization failed
        std::cout<< "ENET FAILED TO INITALIZE";
        return 1;

    }

    // typical server declarations
    ENetAddress address;
    ENetHost* server; // host
    ENetEvent event; // will be used to check for events later



    address.host = ENET_HOST_ANY; 
    address.port = 7777; // picked port 7777 because yt tutorial but needs to be same as CLIENT -- CHECK LATER
    server = enet_host_create(&address,1,1,0,0); // 1 client can connect, 2 channels one reliable and one unreliable 0 for both bandwith limiatation cause we watn to go as fast as possible


    if (server == NULL)
    {
        std::cout<< "Sever was not created";
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
                    printf("Client Connected:");
                    onConnect(event.peer); // does something with the peer that connected 
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    printf("Server Recvied :");
                    onRecieve(event.peer,event.packet); // does something with the packet when recieved 
                    enet_packet_destroy(event.packet); //destroy old unsused packet
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("Server disconected :");
                    onDisconnect(event.peer);
                    break;
                    
                case ENET_EVENT_TYPE_NONE:
                    printf("No Event :");
                    break;

            }
        }
    }

    

    //SERVER GAME LOOP ------------------------------------------------------------------------------------
    return 0;
   
    
    atexit(enet_deinitialize); // once main returns anything deinitialize server

}