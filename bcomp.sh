#g++ -g main.cpp -lGLU -lGL -lglfw `sdl2-config --cflags --libs` -L/usr/include/mysql -lmysqlclient -I/usr/include/mysql -pthread -lSDL2main -lSDL2
# g++ -g main.cpp -lGLU -lGL -lglfw `sdl2-config --cflags --libs` -L/usr/include/mysql -lmysqlclient -I/usr/include/mysql -pthread -lSDL2main -lSDL2
g++ main.cpp -lGLEW -lGL -lglfw -pthread
