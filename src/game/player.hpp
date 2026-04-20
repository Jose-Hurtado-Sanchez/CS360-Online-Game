#include "../shared/packets.hpp"
#include <enet/enet.h>

class Player 
{
    private: 
        float x;
        float health; 
        uint32_t id; // used to track who is who server host is ALWAYS p1
        ENetPeer* peer; // need this because it is were data will be communicated back and forth from server
        ActionType currentAction = ActionType::NONE;

    public:
        // Getters ----------------------------------------------------------------------
        float getX() 
        {
            return x; 
        }
        float getHealth()  
        { 
            return health; 
        }
        uint32_t getId()  
        { 
            return id; 
        }
        ENetPeer* getPeer()  
        { 
            return peer; 
        }
        ActionType getCurrentAction()
        {
            return currentAction;
        }
        // Getters ----------------------------------------------------------------------


        // Setters ------------------------------------------------------------------------
        void setX(float newX) 
        {
             x = newX; 
        }
        void setHealth(float newHealth) 
        { 
            health = newHealth; 
        }
        void setId(uint32_t newId) 
        { 
            id = newId; 
        }
        void setPeer(ENetPeer* newPeer) 
        { 
            peer = newPeer; 
        }
        // Setters ------------------------------------------------------------------------

        void takeDamage()
        {
            health -= 2; 
            if (health < 0) 
            {
                health = 0; //if health is going to be negative set it to 0 instead
            }
        }

        void startGame() 
        {
            health = 100; //Reset health for new round
            
            //Determine x position based on whether the player is the host
            if (id == 1) {
                // Host player starts on the left
                x = 100.0;
            } else {
                //Non-host player starts on the right
                x = 300.0;
            }
        }

        void applyInput(InputPacket input, ENetPeer* senderPeer, Player* opponent) 
        {
            bool opponentBlocked = false;
            if (senderPeer != peer) {
                return;
            }

            // Update player state based on movement
            switch (input.move) 
            {
                case MoveType::LEFT:
                    x -= 1.0f; // Move left
                    break;
                case MoveType::RIGHT:
                    x += 1.0f; // Move right
                    break;
                case MoveType::NONE:
                    // No movement
                    break;
            }

            // Keep track of this player's most recent action for the server to broadcast.
            currentAction = input.action;

            switch (input.action) 
            {
                case ActionType::LEFT_ATTACK:
                    if (opponent)
                    {
                        if(input.action == ActionType::LEFT_ATTACK && opponent->getCurrentAction() == ActionType::LEFT_BLOCK) 
                        {
                            opponentBlocked = true;
                        }

                        if (!opponentBlocked)
                        {
                            opponent->takeDamage();
                        }
                    }
                case ActionType::RIGHT_ATTACK:
                    if (opponent)
                    {   
                        if(input.action == ActionType::RIGHT_ATTACK && opponent->getCurrentAction() == ActionType::RIGHT_BLOCK)
                        {
                            opponentBlocked = true;
                        }

                        if (!opponentBlocked)
                        {
                            opponent->takeDamage();
                        }
                    }
                    break;

                //other actions does not affect it so just worry about the attacks they already check blocking 
                case ActionType::LEFT_BLOCK:
                    break;
                case ActionType::RIGHT_BLOCK:
                    break;
                case ActionType::NONE:
                    break;
                default:
                    // No action performed
                    break;
            }
        }


        


};