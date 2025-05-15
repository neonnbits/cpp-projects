#include <SFML/Graphics.hpp>
#include <iostream>
#include "../include/harmonograph.h"

int main(){
    sf::RenderWindow window;
    window.create(sf::VideoMode({800, 800}), "Harmonograph");
    window.setFramerateLimit(60);
    window.setPosition(sf::Vector2i(100,100));

    Pendulum pendulumX1(300, 2.0, 0.0, 0.002);
    Pendulum pendulumX2(50, 5.0, 0.0, 0.002);
    Pendulum pendulumY1(300, 3.0, M_PI/2, 0.002); 
    Pendulum pendulumY2(50, 7.0, 0.0, 0.002);

    // Generate all points in advance
    std::vector<sf::Vertex> allPoints;
    
    for (double t = 0; t < 1000; t+=0.01)
    {
        double x = pendulumX1.calculate(t) + pendulumX2.calculate(t);
        double y = pendulumY1.calculate(t) + pendulumY2.calculate(t);

        float screenX = 400 + x;
        float screenY = 400 + y;

        // Create a color gradient based on time
        float ratio = t / 100.0f;
        sf::Color color(
            128 + 127 * std::sin(ratio * 3),
            128 + 127 * std::sin(ratio * 5 + 1),
            128 + 127 * std::sin(ratio * 7 + 2),
            255
        );

        sf::Vertex vertex;
        vertex.position = sf::Vector2f(screenX, screenY);
        vertex.color = color;
        allPoints.push_back(vertex);
    }
    
    sf::VertexArray curve(sf::PrimitiveType::LineStrip);
    int currentPoint = 0;
    
    sf::Clock clock;
    
    while(window.isOpen()){
        while(std::optional event = window.pollEvent()){
            if(event->is<sf::Event::Closed>()){
                window.close();
            }
            
            if(event->is<sf::Event::KeyPressed>()) {
                curve.clear();
                currentPoint = 0;
            }
        }
        
        if (currentPoint < allPoints.size()) {
            int pointsPerFrame = 100;
            
            for (int i = 0; i < pointsPerFrame && currentPoint < allPoints.size(); i++) {
                curve.append(allPoints[currentPoint]);
                currentPoint++;
            }
        }

        window.clear(sf::Color(30, 30, 30));
        window.draw(curve);
        window.display();
    }
    
    return 0;
}