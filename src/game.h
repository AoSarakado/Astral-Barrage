#pragma once

#include "raylib.h"

#include <vector>

class Game {
public:
    Game(int screenWidth, int screenHeight);

    void update();
    void draw() const;

private:
    enum class Shooter { Player, Enemy };

    struct Bullet {
        Vector2 position;
        Vector2 velocity;
        Shooter shooter;
    };

    void resetRound();
    void simulate(float seconds, int moveDirection, bool firing);
    bool roundOver() const;

    int screenWidth_;
    int screenHeight_;
    float laneEnd_;
    Vector2 enemyPosition_;
    Vector2 cannonPosition_;
    Vector2 playerPosition_;
    std::vector<Bullet> bullets_;
    int playerHealth_;
    int enemyHealth_;
    float playerFireCooldown_;
    float enemyFireCooldown_;
    float playerInvulnerability_;
    float enemyHitFlash_;
    double pendingSeconds_;
};
