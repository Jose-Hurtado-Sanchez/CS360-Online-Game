To build in vscode 

BEFORE COMPILING:
ADD the IP of the person who is running the server and make sure you are not on WSU wifi 
Both machines have to be on the same wifi perferably have both players in the same room 
Have eNet and SFML downloaded on your computer for this program to run


OVERALL NOTES:
Player Who starts server is p1 and therefore gets id 1 and id 2 goes to p2



Current push notes:
-Used Copilot to autocomplete work in main_server.hpp and player.hpp (few lines at at time) to finish then made edits to better fit goal 
    -useful in assisting autocomplete in functions onRecieve and sendGameState especially
-Realized gamestate packet needs to communicate move and attack for both players so added that in the struct
-Created animations to be used in the game for different attacks and also defensive positions (AI images was used) 
-Uploading the client to connect with the server and used copilot to help complete the client part of the server-client system
-uploading new animations that align with our previous images and did 5 images for attack and 5 for defensive positions
-Used Copilot to autocomplete work in client.hpp and client.cpp (few lines at at time) to finish then made edits to better fit goal 
-Also had generative ai to help me complete the sendInput and handlePacket functions to make sure that it works with the server
-Updated the client.cpp and client.hpp to work with the varaiables within the server files 
    -Updated the comments on the client files 
-Used AI to build makefile using prompt "make the make file" along with the name of all the cpp files 
-Client and Server works to make a connection and the window pops up for both players 
-Player 1 will be on the left side and Player 2 will be on the right side

TO-DO:
-Player class apply input function still needs implementation eventually to contain the calls for animation changes 
-SFML implementation 
-Game logic 
-Testing and Debugging


Ensure a working tasks.json and c_cpp_properties.json configuration in .vscode

Once that is done then Cmd Shft B to build (On Mac) Probs going to be Crl Shft B on Windows

then run ./game -- will become ./server and ./client eventually 



