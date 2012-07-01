# SVN Version: $Rev: 158 $

# C++ compiler
CXX := g++

# Standard flags for C++ 
CXXFLAGS := -Wall

# Standard preprocessor flags (common for CC and CXX) 
CPPFLAGS := 

# Standard linker flags 
LDFLAGS := 

# Location and arguments of wx-config script 
WX_CONFIG := wx-config-2.8

# C++ flags to use with wxWidgets code 
WX_CXXFLAGS := `$(WX_CONFIG) --cxxflags`

# Doxygen flags
DOXYFLAGS := 

### Variables: ###

CLIENT_CXXFLAGS =  -I. $(WX_CXXFLAGS) $(CPPFLAGS) $(CXXFLAGS)
CLIENT_OBJECTS =  \
  client.o \
  cl_config.o \
  gameWindow.o \
  MenuWindow.o \
  LobbyWindow.o \
  LoginWindow.o \
  replayWindow.o \
  client_connect.o \
  md5.o \
  HelpWindow.o
CLIENT_SRCS = \
  client.cpp \
  cl_config.cpp \
  gameWindow.cpp \
  MenuWindow.cpp \
  LobbyWindow.cpp \
  LoginWindow.cpp \
  replayWindow.cpp \
  client_connect.cpp \
  md5.cpp \
  HelpWindow.cpp
CLIENT_HEAD = \
  client.h \
  cl_board.h \
  cl_pieces.h \
  cl_config.h \
  gameWindow.h \
  MenuWindow.h \
  LobbyWindow.h \
  LoginWindow.h \
  replayWindow.h \
  client_connect.h \
  server_handler.h \
  md5.h \
  cl_receiver.h \
  HelpWindow.h

GAMELOGIC_CXXFLAGS =  -I. $(CPPFLAGS) $(CXXFLAGS)
GAMELOGIC_OBJECTS =  \
  cl_board.o \
  cl_pieces.o
GAMELOGIC_SRCS = \
  cl_board.cpp \
  cl_pieces.cpp
GAMELOGIC_HEAD = \
  cl_board.h \
  cl_pieces.h

SERVER_CXXFLAGS =  -I. $(WX_CXXFLAGS) $(CPPFLAGS) $(CXXFLAGS)
SERVER_OBJECTS =  \
	server.o \
	server_handler.o
SERVER_SRCS =  \
	server.cpp \
	server_handler.cpp
SERVER_HEAD =  \
	server.h \
	server_handler.h

TINYXML_CXXFLAGS = -I. $(CPPFLAGS) $(CXXFLAGS) -DTIXML_USE_TICPP -DTIXML_USE_STL
TINYXML_SRCS = \
  tinyxml.cpp \
  tinystr.cpp \
  tinyxmlerror.cpp \
  tinyxmlparser.cpp \
  ticpp.cpp
TINYXML_HEAD = \
  tinyxml.h \
  tinystr.h \
  ticpprc.h \
  ticpp.h
TINYXML_OBJ = \
  tinyxml.o \
  tinystr.o \
  tinyxmlerror.o \
  tinyxmlparser.o \
  ticpp.o

### Targets: ###

# It's possible that doc won't be needed
all: tinyxml gamelogic dama2008 damaserver2008 doc

clean: 
	rm -f ./*.o
	rm -f ./*.d
	rm -f ./*.gch
	rm -f ./*.*~
	rm -f ./*.log
	rm -f ./dama2008
	rm -f ./damaserver2008
	rm -rf ./../doc/html

doc: $(CLIENT_SRCS) $(CLIENT_HEAD) $(SERVER_SRCS) $(SERVER_HEAD) Doxyfile
	doxygen $(DOXYFLAGS)

tinyxml: $(TINYXML_SRCS) $(TINYXML_HEAD)
	$(CXX) -c $(TINYXML_CXXFLAGS) $(TINYXML_SRCS)

gamelogic: $(GAMELOGIC_SRCS) $(GAMELOGIC_HEAD)
	$(CXX) -c $(GAMELOGIC_CXXFLAGS) $(GAMELOGIC_SRCS)

$(SERVER_OBJECTS): $(SERVER_SRCS) $(SERVER_HEAD)
	$(CXX) -c $(SERVER_CXXFLAGS) $(SERVER_SRCS)

$(CLIENT_OBJECTS): $(CLIENT_SRCS) $(CLIENT_HEAD)
	$(CXX) -c $(CLIENT_CXXFLAGS) $(CLIENT_SRCS)

dama2008: $(CLIENT_OBJECTS)
	$(CXX) -o $@ $(CLIENT_OBJECTS) $(GAMELOGIC_OBJECTS) $(TINYXML_OBJ) $(LDFLAGS)  `$(WX_CONFIG) --libs core,net,base,html,adv`

damaserver2008: $(SERVER_OBJECTS)
	$(CXX) -o $@ $(SERVER_OBJECTS) $(GAMELOGIC_OBJECTS) $(TINYXML_OBJ) $(LDFLAGS)  `$(WX_CONFIG) --libs net,base`

.PHONY: all clean doc
