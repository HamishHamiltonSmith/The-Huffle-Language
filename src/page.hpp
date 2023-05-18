#pragma once

#include<vector>
#include<string>
#include<iostream>
#include"globals.hpp"
#include<SFML/Graphics.hpp>

class PageObj {
    public:
    virtual void update(sf::RenderWindow* win)=0;
};

typedef sf::Vector2f v2;

//Page objects

class Text : public PageObj {
    public:
    sf::Font f;
    sf::Text text;
    std::string str;
    v2 pos;
    

    Text(std::string chars, std::string fPath, double px, double py, double size=DEFAULT_FONT_SIZE) {
        if (!f.loadFromFile(fPath)) {
            throw new RuntimeError("Text Element: Failed to find local font file", 0);
        }

        this->pos = v2(px,py);
        this->str = chars;

        text = sf::Text(chars, f, size);
        text.setFillColor(sf::Color::Black);
        text.setPosition(px, py);
    }

    void update(sf::RenderWindow* win) {
        win->draw(this->text);
    }  
};

class Rect : public PageObj {
    public:
    sf::Color colour;
    sf::RectangleShape mesh;
    
    Rect(sf::Color colour, double w, double h, double px, double py) {
        mesh = sf::RectangleShape(v2(w,h));
        mesh.setPosition(px,py);
        mesh.setFillColor(colour);
    }

    void update(sf::RenderWindow* win) {
        win->draw(this->mesh);
    }

};

class Page {
    public:
    std::vector<PageObj*> elements;
    sf::RenderWindow* page;
    
    Page() {
        this->page = new sf::RenderWindow(sf::VideoMode(1000,500), "New Page");
        this->page->display();
    }

    void addElement(PageObj* obj) {
        this->elements.push_back(obj);
    }

    void update() {
        page->clear(sf::Color::White);
        for (int x=0; x<this->elements.size(); x++) {
            elements[x]->update(this->page);
        }
        page->display();
    }
};