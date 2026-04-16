CXX = clang++
CXXFLAGS = -std=c++17 -I/opt/homebrew/include -I./src
LDFLAGS = -L/opt/homebrew/lib -lsfml-graphics -lsfml-window -lsfml-system -lenet

# Source files
CLIENT_SRC = src/game/client.cpp
SERVER_SRC = src/server/main_server.cpp

# If you later add player.cpp, uncomment this:
# COMMON_SRC = src/game/player.cpp

# Output binaries
CLIENT_BIN = client
SERVER_BIN = server

# Default build (build both)
all: $(CLIENT_BIN) $(SERVER_BIN)

# Build client
$(CLIENT_BIN):
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) -o $(CLIENT_BIN) $(LDFLAGS)

# Build server
$(SERVER_BIN):
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) -o $(SERVER_BIN) $(LDFLAGS)

# Clean
clean:
	rm -f $(CLIENT_BIN) $(SERVER_BIN)