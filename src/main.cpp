#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <string.h>
#include "SymbolMono18pt7b.h"

// PINOS DISPLAY
#define TFT_CS                      10
#define TFT_RST                     8  // define reset pin, or set to -1 and connect to Arduino RESET pin
#define TFT_DC                      9  // define data/command pin

// PINOS BOTÕES
#define BUTTON_SELECT_PIN           2 //botão para selecionar
#define BUTTON_CONFIRM_PIN          3 //botão para confirmar

// DEFINIÇÕES DE ALINHAMENTO
#define FIRST_LINE_MENU             10
#define LINE_HEIGHT_MENU            38

// TELAS
#define NUMBER_SCREENS              4
#define SCREEN_MENU                 0
#define SCREEN_PLAY                 1
#define SCREEN_RANKING              2
#define SCREEN_CREDITS              3
#define SCREEN_SCORE                4
#define SCREEN_GAME_OVER            5

#define MOVEMENT_DISTANCE_LEFT      8
#define MOVEMENT_DISTANCE_RIGHT     -8
#define MOVIMENT_DISTANCE_SHOT      10
#define MOVEMENT_RIGHT_LIMIT        215
#define MOVEMENT_LEFT_LIMIT         0

#define PLAYER_SPAWN_Y              195

#define DELAY_UPDATE_PROJECTILE     20
#define DELAY_SHOT_PROJECTILE       250
#define DELAY_SPAWN_ENEMY           1500
#define DELAY_UPDATE_ENEMY          50
#define DELAY_PLAYER_MOVEMENT       18

// CORES
#define BASE_TEXT_COLOR             0xFFE0
#define BACKGROUND_COLOR            0x0000
#define ACTOR_COLOR                 0xFFFF
#define ACTOR_DAMAGE_COLOR          0xF800
#define GAME_OVER_COLOR             0x7BEF

// MAXIMO RANKING E MAXIMO NOME DO JOGADOR
#define MAX_RANKING_ENTRIES         3
#define MAX_NAME_PLAYER             4
#define MAX_PLAYER_PROJECTILES      3
#define MAX_ENEMIES                 3
#define MAX_PLAYER_LIVES            3

#define DEATH_STAR_RADIUS           20

void showMainMenu();
void showLogo();
void hidePreviousSelector();
void showMenuSelector();
void updateMenuSelector();
void selectMenuOption();
void initGame();
void showRanking();
void sortRanking();
void showCredits();
void backMainMenu();
void movePlayerShip(int8_t x);
void renderPlayerShip();
void hidePlayerShip();

void addPoints(uint16_t points);
void clearScreen();
void formatBaseText(uint8_t size);
void showText(uint8_t x, uint8_t y, const __FlashStringHelper* text);

void renderEnemy(uint8_t enemy, uint16_t color = ACTOR_COLOR);

void updateProjectiles();
void updateRanking();
void enableEnemy(uint8_t enemy, uint8_t positionX, uint8_t positionY);
boolean shootPlayerProjectile();
void updateEnemies();
void removeEnemy(uint8_t enemy);
void applyEnemyDamage(uint8_t enemy, uint8_t damage);
void renderPlayerProjectile(uint8_t projectile);
void hideEnemy(uint8_t enemy);
void updateGameOverSelector();
void updatePlayerName(int8_t leftInput, int8_t rightInput);
uint8_t checkEnemyCollision(uint8_t x, uint8_t y);

void updateGame(int8_t leftInput, int8_t rightInput);
void hidePlayerProjectile(uint8_t projectile);
void removePlayerProjectile(uint8_t projectile);
void hidePlayerProjectile(uint8_t projectile);
void applyEnemyDamage(uint8_t enemy, uint8_t damage);
boolean isPointInCircle(uint8_t targetX, uint8_t targetY, uint8_t targetRadius, uint8_t x, uint8_t y);
boolean isPlayerPointsInScore();
void showGameOver();
int applyPlayerDamage();
void SpawnNewEnemy();
void resetGameVariables();

const unsigned char playerShipBitmap[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00,
    0x00, 0x0d, 0x80, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00,
    0x00, 0x0f, 0x80, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x3f, 0xe0, 0x00,
    0x00, 0x7f, 0xf0, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x01, 0xff, 0xfc, 0x00,
    0x03, 0xdf, 0xde, 0x00, 0x03, 0x9f, 0xce, 0x00, 0x00, 0x38, 0xe0, 0x00, 0x00, 0x50, 0x50, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

typedef struct {
    uint8_t positionX;
    uint8_t positionY;
    uint32_t lastUpdate;
    boolean active;
} projectile;

typedef struct {
    uint8_t positionX;
    uint8_t positionY;
    uint32_t lastDamageUpdate;
    uint32_t lastMovementUpdate;
    uint8_t hp;
    boolean active;
} enemy;

enemy enemies[MAX_ENEMIES] = {0, 0, 0, 0, 5, false};
uint8_t totalEnemies = 0;

projectile playerProjectiles[MAX_PLAYER_PROJECTILES] = {0, 0, 0, false};
uint8_t totalPlayerProjectiles = 0;

uint8_t currentMenuOption = 1;
uint8_t currentScreen = 0;

uint8_t playerNameSelector = 0;
char playerName[MAX_NAME_PLAYER] = "A__";
uint8_t totalLives = MAX_PLAYER_LIVES;
uint16_t totalPoints = 0;
uint8_t playerPositionX = 100;
uint32_t lastPlayerMovementUpdate = 0;

uint32_t lastPlayerShotUpdate = 0;
uint32_t lastSpawnEnemyUpdate = 0;

uint32_t rankingPoints[MAX_RANKING_ENTRIES] = {0, 0, 0};
char playersNames[MAX_RANKING_ENTRIES][MAX_NAME_PLAYER] = {"\0", "\0", "\0"};

void setup(void) {
    Serial.begin(9600);

    // if the display has CS pin try with SPI_MODE0
    tft.init(240, 240, SPI_MODE2);    // Init ST7789 display 240x240 pixel
    tft.setRotation(2);

    //showLogo();
    //delay(3000);

    pinMode(BUTTON_SELECT_PIN, INPUT);
    pinMode(BUTTON_CONFIRM_PIN, INPUT);

    sortRanking();
    showMainMenu();
}

void showLogo() {
    clearScreen();
    formatBaseText(4);
    showText(40, 40, F("Attack"));
    showText(40, 80, F("on"));
    showText(40, 120, F("Death"));
    showText(40, 160, F("Star"));
}

void formatBaseText(uint8_t size)
{
    tft.setTextSize(size);
    tft.setTextColor(BASE_TEXT_COLOR);
}

void showText(uint8_t x, uint8_t y, const __FlashStringHelper* text)
{
    tft.setCursor(x, y);
    tft.print(text);    
}

void showText(uint8_t x, uint8_t y, const String text)
{
    tft.setCursor(x, y);
    tft.print(text);    
}

void renderizarSimbolo(uint8_t x, uint8_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t Size){
    tft.setFont(&SymbolMono18pt7b);
    tft.drawChar(x,y,c,color,bg,Size);
    tft.setFont();
}

void showHUD() {
    renderizarSimbolo(0, 240, GLYPH_HEART, BASE_TEXT_COLOR,0,1);
    tft.fillRect(24, 225, 75, 25, BACKGROUND_COLOR);
    formatBaseText(2);
    showText(25, 225, String(totalLives));
    showText(55, 225, String(totalPoints));
}

void showMainMenu() {
    clearScreen();


    formatBaseText(4);
    showText(36, FIRST_LINE_MENU + LINE_HEIGHT_MENU, F("Jogar"));
    showText(36, FIRST_LINE_MENU + (2 * LINE_HEIGHT_MENU), F("Ranking"));
    showText(36, FIRST_LINE_MENU + (3 * LINE_HEIGHT_MENU), F("Creditos"));
    showText(36, FIRST_LINE_MENU + (4 * LINE_HEIGHT_MENU), F("Opcoes"));

    showMenuSelector();
}

void hidePreviousSelector()
{
    tft.setTextColor(BACKGROUND_COLOR);
    showText(10, FIRST_LINE_MENU + (currentMenuOption * LINE_HEIGHT_MENU), F("\t"));   
    delay(20);
}

void showMenuSelector()
{
    tft.setTextColor(BASE_TEXT_COLOR);
    showText(10, FIRST_LINE_MENU + (currentMenuOption * LINE_HEIGHT_MENU), F("\t"));
}

void updateMenuSelector()
{
    hidePreviousSelector();
    if (currentMenuOption < NUMBER_SCREENS) {
        currentMenuOption++;
    } else {
        currentMenuOption = SCREEN_PLAY;
    }
    showMenuSelector();

    // Delay é utilizado para evitar a execução repetida
    delay(500);
}

void selectMenuOption()
{
    switch (currentMenuOption) {
        case SCREEN_PLAY: initGame();
            break;
        case SCREEN_RANKING: showRanking();
            break;
        case SCREEN_CREDITS: showCredits();
            break;
    }
    currentScreen = currentMenuOption;
}

void initGame()
{
    clearScreen();
    delay(1000);  

    resetGameVariables();

    showHUD();
    renderPlayerShip();
}

void resetGameVariables()
{
    lastPlayerShotUpdate = 0;
    lastSpawnEnemyUpdate = 0;
    totalPoints = 0;
    playerPositionX = 100;
    lastPlayerMovementUpdate = 0;

    for (uint8_t i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = false;
    }

    for (uint8_t i = 0; i < MAX_PLAYER_PROJECTILES; i++)
    {
        playerProjectiles[i].active = false;
    }

    totalEnemies = 0;

    totalPlayerProjectiles = 0;

    playerNameSelector = 0;
    strcpy(playerName, "A__");

    totalLives = MAX_PLAYER_LIVES;
}

void updateGame(int8_t leftInput, int8_t rightInput)
{
    if (leftInput == HIGH) {
        movePlayerShip(MOVEMENT_DISTANCE_RIGHT);
    }
    if (rightInput == HIGH) {
        movePlayerShip(MOVEMENT_DISTANCE_LEFT);
    }
    shootPlayerProjectile();
    updateProjectiles();

    SpawnNewEnemy();

    updateEnemies();
}

void SpawnNewEnemy()
{
    if (millis() - lastSpawnEnemyUpdate > DELAY_SPAWN_ENEMY && totalEnemies < MAX_ENEMIES) {
        lastSpawnEnemyUpdate = millis();
        for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) {
                enableEnemy(i, random(10, 180), 0);
                break;
            }
        }
    }
}

boolean shootPlayerProjectile()
{
    if (millis() - lastPlayerShotUpdate > DELAY_SHOT_PROJECTILE) {
        if (totalPlayerProjectiles < MAX_PLAYER_PROJECTILES) {
            for (uint8_t i = 0; i < MAX_PLAYER_PROJECTILES; i++) {
                if (!playerProjectiles[i].active) {
                    playerProjectiles[i].positionX = playerPositionX + 14;
                    playerProjectiles[i].positionY = PLAYER_SPAWN_Y + 5;
                    playerProjectiles[i].lastUpdate = 0;
                    playerProjectiles[i].active = true;
                    lastPlayerShotUpdate = millis();
                    renderPlayerProjectile(i);
                    return true;
                }
            }
        }
    }
    return false;
}

void renderPlayerProjectile(uint8_t projectile)
{
    tft.drawRect(playerProjectiles[projectile].positionX, playerProjectiles[projectile].positionY, 2, 5, BASE_TEXT_COLOR);
}

void hidePlayerProjectile(uint8_t projectile)
{
    tft.drawRect(playerProjectiles[projectile].positionX, playerProjectiles[projectile].positionY, 2, 5, BACKGROUND_COLOR);
}

void removePlayerProjectile(uint8_t projectile)
{
    playerProjectiles[projectile].active = false;
    hidePlayerProjectile(projectile);
}

void updateProjectiles()
{
    for(uint8_t i = 0; i < MAX_PLAYER_PROJECTILES; i++)
    {
        if (playerProjectiles[i].active && (millis() - playerProjectiles[i].lastUpdate > DELAY_UPDATE_PROJECTILE)) {
            hidePlayerProjectile(i);
            if (playerProjectiles[i].positionY - MOVIMENT_DISTANCE_SHOT > 0) {
                playerProjectiles[i].positionY -= MOVIMENT_DISTANCE_SHOT;
            } else {
                removePlayerProjectile(i);
                return;
            }
            renderPlayerProjectile(i);
            uint8_t enemy = checkEnemyCollision(playerProjectiles[i].positionX, playerProjectiles[i].positionY);
            if(enemy != MAX_ENEMIES)
            {
                addPoints(10);
                removePlayerProjectile(i);
                applyEnemyDamage(enemy, 1);
            }
            playerProjectiles[i].lastUpdate = millis();
        }
    }
}

void applyEnemyDamage(uint8_t enemy, uint8_t damage)
{
    if (enemies[enemy].hp - damage > 0)
    {
        enemies[enemy].hp -= damage;
        enemies[enemy].lastDamageUpdate = millis();
        renderEnemy(enemy, ACTOR_DAMAGE_COLOR);
    } else  {
        enemies[enemy].hp = 0;
        removeEnemy(enemy);
    }
}

void addPoints(uint16_t points)
{
    totalPoints += 10;
    showHUD();
}

boolean isPointInCircle(uint8_t targetX, uint8_t targetY, uint8_t targetRadius, uint8_t x, uint8_t y)
{
    return (pow(x - targetX, 2) + pow(y - targetY, 2)) < pow(targetRadius, 2);
}

uint8_t checkEnemyCollision(uint8_t x, uint8_t y)
{
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            if (isPointInCircle(enemies[i].positionX, enemies[i].positionY, DEATH_STAR_RADIUS, x, y)) {
                return i;
            }
        }
    }
    return MAX_ENEMIES;
}

void enableEnemy(uint8_t enemy, uint8_t positionX, uint8_t positionY)
{
    if (!enemies[enemy].active) {
        enemies[enemy].positionX = positionX;
        enemies[enemy].positionY = positionY;
        enemies[enemy].lastDamageUpdate = 0;
        enemies[enemy].lastMovementUpdate = 0;
        enemies[enemy].active = true;
        enemies[enemy].hp = 5;
        totalEnemies++;

        renderEnemy(enemy);
    }
}

void removeEnemy(uint8_t enemy)
{
    if (enemies[enemy].active) {
        enemies[enemy].active = false;
        hideEnemy(enemy);
        totalEnemies--;
    }
}

boolean isPlayerPointsInScore()
{
    if (totalPoints > rankingPoints[MAX_RANKING_ENTRIES - 1]) {
        return true;
    }
    return false;
}

void showGameOver()
{
    currentScreen = SCREEN_GAME_OVER;   
    formatBaseText(4);
    tft.fillRect(10, 50, 220, 120, GAME_OVER_COLOR);
    showText(20, 60, F("GameOver"));
    formatBaseText(3);
    if (isPlayerPointsInScore()) {
        currentScreen = SCREEN_SCORE;
        showText(20, 100, String(totalPoints));
        updateGameOverSelector();
    }
}

void updateGameOverSelector()
{
    formatBaseText(3);
    tft.fillRect(20, 130, 55, 25, GAME_OVER_COLOR);
    showText(20, 130, playerName);
}

void updatePlayerName(int8_t leftInput, int8_t rightInput)
{
    isPlayerPointsInScore();
    if (leftInput == HIGH) {
        if (playerName[playerNameSelector] >= 'A' && playerName[playerNameSelector] < 'Z') {
            playerName[playerNameSelector] += 1;
        } else {
            playerName[playerNameSelector] = 'A';
        }
        updateGameOverSelector();
    }
    if (rightInput == HIGH) {
        if (playerNameSelector < 2) {
            playerNameSelector++;
            playerName[playerNameSelector] = 'A';
            updateGameOverSelector();
        } else {
            updateRanking();
            backMainMenu();
        }
    }
    delay(200);
}

void updateRanking()
{
    strcpy(playersNames[MAX_RANKING_ENTRIES - 1], playerName);
    rankingPoints[MAX_RANKING_ENTRIES - 1] = totalPoints;
    sortRanking();
}

int applyPlayerDamage()
{
    if (totalLives <= 1) {
        showGameOver();
    }
    totalLives--;
    showHUD();
    return totalLives;
}

void updateEnemies()
{
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active && (millis() - enemies[i].lastMovementUpdate > DELAY_UPDATE_ENEMY)) {
            hideEnemy(i);
            if (enemies[i].positionY + 4 > 220) {
                enemies[i].positionY = 0;

                if (!applyPlayerDamage()) {
                    return;
                }
            } else {
                enemies[i].positionY += 4;
            }
            if (millis() - enemies[i].lastDamageUpdate > 150) {
                renderEnemy(i);
            } else {
                renderEnemy(i, ACTOR_DAMAGE_COLOR);
            }
            enemies[i].lastMovementUpdate = millis();
        }
    }
}

void hideEnemy(uint8_t enemy)
{
    renderEnemy(enemy, BACKGROUND_COLOR);
}

void renderEnemy(uint8_t enemy, uint16_t color)
{
    tft.fillCircle(enemies[enemy].positionX, enemies[enemy].positionY, DEATH_STAR_RADIUS, color);
}

void showCredits()
{
    clearScreen();
    formatBaseText(3);

    showText(36, FIRST_LINE_MENU + LINE_HEIGHT_MENU, F("Creditos"));

    showText(36, FIRST_LINE_MENU + (2 * LINE_HEIGHT_MENU), F("Gabriel"));
    showText(36, FIRST_LINE_MENU + (3 * LINE_HEIGHT_MENU), F("Helcio"));
    showText(36, FIRST_LINE_MENU + (4 * LINE_HEIGHT_MENU), F("Jean"));
    showText(36, FIRST_LINE_MENU + (5 * LINE_HEIGHT_MENU), F("Kellwyn"));
    showText(36, FIRST_LINE_MENU + (6 * LINE_HEIGHT_MENU), F("Rafael"));
    showText(36, FIRST_LINE_MENU + (7 * LINE_HEIGHT_MENU), F("Rennan"));
}

void clearScreen()
{
    tft.fillScreen(BACKGROUND_COLOR); 
}

void showRanking()
{
    clearScreen();
    formatBaseText(3);
    showText(36, FIRST_LINE_MENU + LINE_HEIGHT_MENU, F("Ranking"));

    for (int i = 0; i < 3; i++) {
        tft.setCursor(36, FIRST_LINE_MENU + ((i + 2) * LINE_HEIGHT_MENU));
        tft.print(playersNames[i]);
        tft.print (": ");
        tft.print(rankingPoints[i]);
    }
}

void movePlayerShip(int8_t x)
{
    if (((x > 0 && playerPositionX < MOVEMENT_RIGHT_LIMIT) ||
        (x < 0 && playerPositionX > MOVEMENT_LEFT_LIMIT)) &&
        (millis() - lastPlayerMovementUpdate > DELAY_PLAYER_MOVEMENT)) {
        hidePlayerShip();
        
        if (x > 0) { // rightInput positivo
            if (playerPositionX > MOVEMENT_RIGHT_LIMIT) {
                playerPositionX = MOVEMENT_RIGHT_LIMIT;
            }
            playerPositionX += x;
        } else { // leftInput (negativo)
            if(playerPositionX + x > MOVEMENT_LEFT_LIMIT) {
                playerPositionX += x;
            } else {
                playerPositionX = MOVEMENT_LEFT_LIMIT;
            }
        }
        lastPlayerMovementUpdate = millis();
        renderPlayerShip();
    }
}

void renderPlayerShip()
{
    tft.drawBitmap(playerPositionX, PLAYER_SPAWN_Y, playerShipBitmap, 32, 32, ACTOR_COLOR);
}

void hidePlayerShip()
{
    tft.drawBitmap(playerPositionX, PLAYER_SPAWN_Y, playerShipBitmap, 32, 32, BACKGROUND_COLOR);
}

void sortRanking()
{
    uint8_t i, j;
    uint32_t swapPoints;
    char swapName[MAX_NAME_PLAYER];
    for (i = 0; i < MAX_RANKING_ENTRIES - 1; i++) {
        for (j = i + 1; j < 3; j++) {
            if (rankingPoints[j] > rankingPoints[i]) {
                swapPoints = rankingPoints[j];
                rankingPoints[j] = rankingPoints[i];
                rankingPoints[i] = swapPoints;
                strcpy(swapName, playersNames[j]);
                strcpy(playersNames[j], playersNames[i]);
                strcpy(playersNames[i], swapName);
            }
        }
    }
}

void backMainMenu()
{
    currentScreen = SCREEN_MENU;
    showMainMenu();
}

void loop() {
    int8_t selectInput = digitalRead(BUTTON_SELECT_PIN);
    int8_t confirmInput = digitalRead(BUTTON_CONFIRM_PIN);
    if (currentScreen == SCREEN_PLAY) {
        updateGame(selectInput, confirmInput);
    } else {
        if (currentScreen == SCREEN_MENU) {
            if (selectInput == HIGH) {
                updateMenuSelector();
            }
            if (confirmInput == HIGH) {
                selectMenuOption();
            }
        } else {
            if (currentScreen == SCREEN_SCORE) {
                updatePlayerName(selectInput, confirmInput);
            } else {
                if (selectInput == HIGH || confirmInput == HIGH) {
                    backMainMenu();
                }
            }
        }
    }
}