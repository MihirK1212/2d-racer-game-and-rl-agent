#ifndef HUD_H
#define HUD_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "race_manager.h"
#include "track_progress.h"

class HUD {
    sf::Font font;
    unsigned int screenWidth;
    unsigned int screenHeight;

    sf::Color car0Color = sf::Color::Red;
    sf::Color car1Color = sf::Color(80, 140, 255);

    void drawText(sf::RenderWindow& window, const std::string& str,
                  unsigned int size, sf::Color color,
                  float x, float y, bool centerX = false) const;

public:
    HUD(const std::string& fontPath, unsigned int screenWidth, unsigned int screenHeight);

    void draw(sf::RenderWindow& window,
              const RaceManager& raceManager,
              const TrackProgress& trackProgress,
              int numCars);
};

#endif
