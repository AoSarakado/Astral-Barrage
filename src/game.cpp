#include "game.h"

#include "raymath.h"

#include <algorithm>

namespace {

constexpr double simulationStep = 1.0 / 120.0;
constexpr float laneStart = 42.0f;
constexpr int playerMaxHealth = 3;
constexpr int enemyMaxHealth = 28;
constexpr float playerSpeed = 420.0f;
constexpr float playerHitRadius = 9.0f;
constexpr float enemyHitRadius = 36.0f;
constexpr float playerBulletRadius = 5.0f;
constexpr float enemyBulletRadius = 8.0f;
constexpr float playerBulletSpeed = 820.0f;
constexpr float enemyBulletSpeed = 450.0f;
constexpr float playerFireInterval = 0.18f;
constexpr float firstVolleyDelay = 0.90f;
constexpr float enemyVolleyInterval = 1.08f;
constexpr float volleyOffsets[] = { -132.0f, 0.0f, 132.0f };
constexpr float playerHitRecovery = 0.58f;
constexpr float enemyHitFlashDuration = 0.12f;

constexpr Color playerColor{ 100, 238, 255, 255 };
constexpr Color enemyColor{ 255, 143, 76, 255 };
constexpr Color enemyCoreColor{ 255, 207, 153, 255 };
constexpr Color outlineColor{ 7, 14, 23, 255 };
constexpr Color panelColor{ 10, 20, 31, 238 };
constexpr Color mutedText{ 173, 196, 210, 255 };
constexpr Color emptyHealthColor{ 41, 57, 70, 255 };
constexpr Color laneColor{ 145, 195, 219, 140 };

void drawCenteredText(const char* text, int centerX, int y, int fontSize, Color color)
{
    DrawText(text, centerX - MeasureText(text, fontSize) / 2, y, fontSize, color);
}

} // namespace

Game::Game(int screenWidth, int screenHeight)
    : screenWidth_(screenWidth), screenHeight_(screenHeight),
      laneEnd_(screenWidth * 0.66f),
      enemyPosition_{ screenWidth * 0.88f, screenHeight * 0.60f },
      cannonPosition_{ enemyPosition_.x, enemyPosition_.y - screenHeight * 0.235f }
{
    resetRound();
}

void Game::resetRound()
{
    playerPosition_ = { screenWidth_ * 0.18f, enemyPosition_.y };
    bullets_.clear();
    playerHealth_ = playerMaxHealth;
    enemyHealth_ = enemyMaxHealth;
    playerFireCooldown_ = 0.0f;
    enemyFireCooldown_ = firstVolleyDelay;
    playerInvulnerability_ = 0.0f;
    enemyHitFlash_ = 0.0f;
    pendingSeconds_ = 0.0;
}

bool Game::roundOver() const
{
    return playerHealth_ == 0 || enemyHealth_ == 0;
}

void Game::update()
{
    if (roundOver()) {
        if (IsKeyPressed(KEY_R)) resetRound();
        return;
    }

    const bool left = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
    const bool right = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
    const int moveDirection = static_cast<int>(right) - static_cast<int>(left);
    const bool firing = IsKeyDown(KEY_SPACE);

    pendingSeconds_ += GetFrameTime();
    while (pendingSeconds_ >= simulationStep && !roundOver()) {
        simulate(static_cast<float>(simulationStep), moveDirection, firing);
        pendingSeconds_ -= simulationStep;
    }
}

void Game::simulate(float seconds, int moveDirection, bool firing)
{
    playerPosition_.x = Clamp(playerPosition_.x + moveDirection * playerSpeed * seconds,
                              laneStart, laneEnd_);
    playerInvulnerability_ = std::max(0.0f, playerInvulnerability_ - seconds);
    enemyHitFlash_ = std::max(0.0f, enemyHitFlash_ - seconds);

    playerFireCooldown_ = std::max(0.0f, playerFireCooldown_ - seconds);
    if (firing && playerFireCooldown_ == 0.0f) {
        bullets_.push_back({ { playerPosition_.x + 26.0f, playerPosition_.y },
                             { playerBulletSpeed, 0.0f }, Shooter::Player });
        playerFireCooldown_ = playerFireInterval;
    }

    enemyFireCooldown_ -= seconds;
    if (enemyFireCooldown_ <= 0.0f) {
        for (const float offset : volleyOffsets) {
            const Vector2 crossing{ playerPosition_.x + offset, playerPosition_.y };
            const Vector2 direction = Vector2Normalize(Vector2Subtract(crossing, cannonPosition_));
            bullets_.push_back({ cannonPosition_, Vector2Scale(direction, enemyBulletSpeed),
                                 Shooter::Enemy });
        }
        enemyFireCooldown_ += enemyVolleyInterval;
    }

    const Rectangle screen{ 0.0f, 0.0f, static_cast<float>(screenWidth_),
                            static_cast<float>(screenHeight_) };
    for (auto bullet = bullets_.begin(); bullet != bullets_.end();) {
        bullet->position = Vector2Add(bullet->position, Vector2Scale(bullet->velocity, seconds));
        const bool hostile = bullet->shooter == Shooter::Enemy;
        const float radius = hostile ? enemyBulletRadius : playerBulletRadius;
        const Vector2 target = hostile ? playerPosition_ : enemyPosition_;
        const float targetRadius = hostile ? playerHitRadius : enemyHitRadius;
        const bool hit = CheckCollisionCircles(bullet->position, radius, target, targetRadius);

        if (hit) {
            if (!hostile) {
                --enemyHealth_;
                enemyHitFlash_ = enemyHitFlashDuration;
            } else if (playerInvulnerability_ == 0.0f) {
                --playerHealth_;
                playerInvulnerability_ = playerHitRecovery;
            }
        }

        if (hit || !CheckCollisionCircleRec(bullet->position, radius, screen)) {
            bullet = bullets_.erase(bullet);
        } else {
            ++bullet;
        }
        if (roundOver()) return;
    }
}

void Game::draw() const
{
    const float laneY = playerPosition_.y;
    DrawLineEx({ laneStart, laneY }, { laneEnd_, laneY }, 3.0f, outlineColor);
    DrawLineEx({ laneStart, laneY }, { laneEnd_, laneY }, 1.0f, laneColor);
    DrawLineEx({ laneStart, laneY - 10.0f }, { laneStart, laneY + 10.0f }, 2.0f, laneColor);
    DrawLineEx({ laneEnd_, laneY - 10.0f }, { laneEnd_, laneY + 10.0f }, 2.0f, laneColor);

    const float playerX = playerPosition_.x;
    DrawTriangle({ playerX + 28.0f, laneY }, { playerX - 18.0f, laneY - 18.0f },
                 { playerX - 18.0f, laneY + 18.0f }, outlineColor);
    DrawTriangle({ playerX + 22.0f, laneY }, { playerX - 14.0f, laneY - 12.0f },
                 { playerX - 14.0f, laneY + 12.0f }, playerColor);
    DrawCircleV(playerPosition_, playerHitRadius + 2.0f, outlineColor);
    DrawCircleV(playerPosition_, playerHitRadius,
                playerInvulnerability_ > 0.0f ? enemyCoreColor : RAYWHITE);

    DrawLineEx(cannonPosition_, enemyPosition_, 13.0f, outlineColor);
    DrawLineEx(cannonPosition_, enemyPosition_, 3.0f, enemyColor);
    DrawCircleV(cannonPosition_, 15.0f, outlineColor);
    DrawCircleV(cannonPosition_, 10.0f, enemyFireCooldown_ < 0.20f ? enemyCoreColor : enemyColor);
    DrawCircleV(enemyPosition_, enemyHitRadius + 3.0f, outlineColor);
    DrawCircleV(enemyPosition_, enemyHitRadius, enemyHitFlash_ > 0.0f ? RAYWHITE : enemyColor);
    DrawCircleV(enemyPosition_, enemyHitRadius - 6.0f, outlineColor);
    DrawCircleV(enemyPosition_, 13.0f, enemyHealth_ > 0 ? enemyCoreColor : emptyHealthColor);

    for (const Bullet& bullet : bullets_) {
        if (bullet.shooter == Shooter::Enemy) {
            DrawCircleV(bullet.position, enemyBulletRadius + 2.0f, outlineColor);
            DrawCircleV(bullet.position, enemyBulletRadius, enemyColor);
            DrawCircleV(bullet.position, 3.0f, enemyCoreColor);
        } else {
            const Vector2 tail{ bullet.position.x - 17.0f, bullet.position.y };
            DrawLineEx(tail, bullet.position, 9.0f, outlineColor);
            DrawCircleV(bullet.position, playerBulletRadius + 1.0f, outlineColor);
            DrawLineEx(tail, bullet.position, 5.0f, playerColor);
            DrawCircleV(bullet.position, playerBulletRadius, playerColor);
        }
    }

    DrawRectangle(32, 28, screenWidth_ - 64, 72, panelColor);
    DrawText("Astral Barrage", 50, 49, 30, RAYWHITE);

    const int playerHudX = screenWidth_ - 488;
    DrawText("PILOT", playerHudX, 45, 16, mutedText);
    for (int segment = 0; segment < playerMaxHealth; ++segment) {
        DrawRectangle(playerHudX + segment * 38, 68, 32, 12,
                      segment < playerHealth_ ? playerColor : emptyHealthColor);
    }

    const int enemyHudX = screenWidth_ - 278;
    constexpr int enemyBarWidth = 226;
    DrawText("ENEMY CORE", enemyHudX, 45, 16, mutedText);
    DrawRectangle(enemyHudX, 68, enemyBarWidth, 12, emptyHealthColor);
    DrawRectangle(enemyHudX, 68, enemyBarWidth * enemyHealth_ / enemyMaxHealth, 12, enemyColor);

    const char* controls = "A / D  or  LEFT / RIGHT   Move     SPACE   Fire";
    const int footerY = screenHeight_ - 64;
    DrawRectangle(32, footerY, MeasureText(controls, 18) + 32, 36, panelColor);
    DrawText(controls, 48, footerY + 9, 18, RAYWHITE);

    if (roundOver()) {
        const bool won = enemyHealth_ == 0;
        const int centerX = screenWidth_ / 2;
        const int cardY = screenHeight_ / 2 - 67;
        DrawRectangle(0, 0, screenWidth_, screenHeight_, Color{ 5, 10, 16, 148 });
        DrawRectangle(centerX - 174, cardY, 348, 134, panelColor);
        DrawRectangle(centerX - 174, cardY, 348, 3, won ? playerColor : enemyColor);
        drawCenteredText(won ? "Victory" : "Defeat", centerX, cardY + 23, 42, RAYWHITE);
        drawCenteredText("R   Restart", centerX, cardY + 89, 20, mutedText);
    }
}
