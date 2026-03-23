#include "hud.h"
#include <stdexcept>

HUD::HUD(const std::string& fontPath, unsigned int screenWidth, unsigned int screenHeight)
    : font(fontPath), screenWidth(screenWidth), screenHeight(screenHeight)
{
}

void HUD::drawText(sf::RenderWindow& window, const std::string& str,
                   unsigned int size, sf::Color color,
                   float x, float y, bool centerX) const
{
    sf::Text text(font, str, size);
    text.setFillColor(color);

    if (centerX) {
        sf::FloatRect bounds = text.getLocalBounds();
        x = (static_cast<float>(screenWidth) - bounds.size.x) / 2.0f;
    }

    text.setPosition({x, y});
    window.draw(text);
}

void HUD::draw(sf::RenderWindow& window,
               const RaceManager& raceManager,
               const TrackProgress& trackProgress,
               int numCars)
{
    RaceState state = raceManager.getState();
    int totalLaps = raceManager.getTotalLaps();

    // --- Per-car info (laps + position) ---
    if (state == RaceState::RACING || state == RaceState::FINISHED) {
        int leaderIdx = raceManager.getLeaderIndex();

        for (int i = 0; i < numCars; ++i) {
            const CarProgress& cp = trackProgress.getCarProgress(i);
            int displayLap = std::min(cp.lapsCompleted + 1, totalLaps);

            sf::Color color = (i == 0) ? car0Color : car1Color;
            std::string label = (i == 0) ? "P1 (You)" : "P2 (RL)";
            std::string posStr = (i == leaderIdx) ? "1st" : "2nd";

            float xBase = (i == 0) ? 10.0f : static_cast<float>(screenWidth) - 200.0f;
            float yBase = 10.0f;

            drawText(window, label, 16, color, xBase, yBase);
            drawText(window, "Lap " + std::to_string(displayLap) + "/" + std::to_string(totalLaps),
                     20, color, xBase, yBase + 22.0f);
            drawText(window, posStr, 24, color, xBase, yBase + 48.0f);

            if (cp.goingWrongWay && state == RaceState::RACING) {
                float wrongWayY = (i == 0)
                    ? static_cast<float>(screenHeight) / 2.0f - 60.0f
                    : static_cast<float>(screenHeight) / 2.0f - 20.0f;
                drawText(window, "WRONG WAY!", 28, color, 0, wrongWayY, true);
            }
        }
    }

    // --- Waiting prompt ---
    if (state == RaceState::WAITING) {
        drawText(window, "Press SPACE to start", 32, sf::Color::White,
                 0, static_cast<float>(screenHeight) / 2.0f - 20.0f, true);
    }

    // --- Winner ---
    if (state == RaceState::FINISHED) {
        int winner = raceManager.getWinnerIndex();
        std::string winnerLabel = (winner == 0) ? "You Win!" : "RL Wins!";
        sf::Color winColor = (winner == 0) ? car0Color : car1Color;

        drawText(window, winnerLabel, 64, winColor,
                 0, static_cast<float>(screenHeight) / 2.0f - 80.0f, true);
        drawText(window, "Press SPACE to restart", 24, sf::Color::White,
                 0, static_cast<float>(screenHeight) / 2.0f, true);
    }
}
