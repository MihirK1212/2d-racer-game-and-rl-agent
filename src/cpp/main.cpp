#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <ctime>
#include <optional>
#include <string>

int main()
{
    // Create large window
    sf::RenderWindow window(
        sf::VideoMode({1000, 600}),
        "Arrange The Letters - SFML 3");

    window.setFramerateLimit(60);

    // Load font (place file next to executable or adjust path)
    sf::Font font;
    if (!font.openFromFile("arial.ttf"))
        return -1;

    // Letters to display
    const std::vector<std::string> letters =
    { "I","L","O","V","E","Y","O","U","V","I","S","H","W","A" };

    std::vector<sf::Text> textObjects;
    textObjects.reserve(letters.size());

    // Random number generator
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> xDist(50, 900);
    std::uniform_int_distribution<int> yDist(50, 500);

    // Create text objects with random positions
    for (const auto& letter : letters)
    {
        sf::Text text(font, letter, 80);
        text.setFillColor(sf::Color::Green);
        text.setPosition({
            static_cast<float>(xDist(rng)),
            static_cast<float>(yDist(rng))
        });

        textObjects.push_back(text);
    }

    int selectedIndex = -1;
    std::string numberBuffer;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* keyPressed =
                event->getIf<sf::Event::KeyPressed>())
            {
                const auto key = keyPressed->code;

                // Collect numeric input (0–9)
                if (key >= sf::Keyboard::Key::Num0 &&
                    key <= sf::Keyboard::Key::Num9)
                {
                    int digit =
                        static_cast<int>(key) -
                        static_cast<int>(sf::Keyboard::Key::Num0);

                    numberBuffer += std::to_string(digit);
                }

                // Confirm selection with F
                if (key == sf::Keyboard::Key::F &&
                    !numberBuffer.empty())
                {
                    int index = std::stoi(numberBuffer) - 1;

                    if (index >= 0 &&
                        index < static_cast<int>(textObjects.size()))
                    {
                        selectedIndex = index;
                    }

                    numberBuffer.clear();
                }
            }
        }

        // Movement
        if (selectedIndex >= 0)
        {
            float moveSpeed = 4.0f;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
                textObjects[selectedIndex].move({-moveSpeed, 0.f});

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
                textObjects[selectedIndex].move({moveSpeed, 0.f});

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
                textObjects[selectedIndex].move({0.f, -moveSpeed});

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
                textObjects[selectedIndex].move({0.f, moveSpeed});
        }

        // Highlight selected letter
        for (size_t i = 0; i < textObjects.size(); ++i)
        {
            if (static_cast<int>(i) == selectedIndex)
                textObjects[i].setFillColor(sf::Color::Yellow);
            else
                textObjects[i].setFillColor(sf::Color::Green);
        }

        window.clear(sf::Color::Black);

        for (const auto& text : textObjects)
            window.draw(text);

        window.display();
    }

    return 0;
}