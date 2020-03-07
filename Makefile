LDFLAGS =-l GL -l GLU -l glut `pkg-config --libs opencv4 --cflags opencv4` 

all:
	g++ -O2 main.cpp -o dcm2opengl $(LDFLAGS)

clean:
	rm dcm2opengl
