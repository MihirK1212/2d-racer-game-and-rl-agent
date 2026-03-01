#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <ctime>
#include <optional>
#include <string>

int main()
{
    // Bigger window
    sf::RenderWindow window(sf::VideoMode({1000, 600}), "Arrange The Letters");

    // Load font (make sure arial.ttf is in your project folder)
    sf::Font font;
    if (!font.openFromFile("arial.ttf"))
        return -1;

    // Letters to display
    std::vector<std::string> letters =
    { "I","L","O","V","E","Y","O","U","V","I","S","H","W","A" };

    std::vector<sf::Text> textObjects;

    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> xDist(50, 900);
    std::uniform_int_distribution<int> yDist(50, 500);

    // Create letter objects with random positions
    for (const auto& l : letters)
    {
        sf::Text text(font, l, 80);
        text.setFillColor(sf::Color::Green);
        text.setPosition(
            sf::Vector2f(static_cast<float>(xDist(rng)),
                         static_cast<float>(yDist(rng))));
        textObjects.push_back(text);
    }

    int selectedIndex = -1;
    std::string numberBuffer = "";

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                // Capture number input
                if (keyPressed->code >= sf::Keyboard::Num0 &&
                    keyPressed->code <= sf::Keyboard::Num9)
                {
                    int digit = keyPressed->code - sf::Keyboard::Num0;
                    numberBuffer += std::to_string(digit);
                }

                // Press F to confirm selection
                if (keyPressed->code == sf::Keyboard::F && !numberBuffer.empty())
                {
                    int index = std::stoi(numberBuffer) - 1;
                    if (index >= 0 && index < textObjects.size())
                        selectedIndex = index;

                    numberBuffer.clear();
                }
            }
        }

        // Move selected letter with arrow keys
        if (selectedIndex != -1)
        {
            float moveSpeed = 2.5f;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                textObjects[selectedIndex].move({-moveSpeed, 0});
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                textObjects[selectedIndex].move({moveSpeed, 0});
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                textObjects[selectedIndex].move({0, -moveSpeed});
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                textObjects[selectedIndex].move({0, moveSpeed});
        }

        window.clear(sf::Color::Black);

        for (auto& text : textObjects)
            window.draw(text);

        window.display();
    }

    return 0;
}

// #include <SFML/Graphics.hpp>

// int main()
// {
//     sf::Window  window(sf::VideoMode({200, 200}), "SFML works!");
//     sf::CircleShape shape(100.f);
//     shape.setFillColor(sf::Color::Green);

//     while (window.isOpen())
//     {
//         while (const std::optional event = window.pollEvent())
//         {
//             if (event->is<sf::Event::Closed>())
//                 window.close();
//         }

//         window.clear();
//         window.draw(shape);
//         window.display();
//     }
// }