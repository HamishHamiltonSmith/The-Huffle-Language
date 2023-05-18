g++ -c interpreter.cpp -std=c++20
g++ interpreter.o -lsfml-graphics -lsfml-system -lsfml-window
rm interpreter.o
