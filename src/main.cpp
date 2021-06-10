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
#define MOVIMENTO_TIRO              10

#define DELAY_UPDATE_PROJECTILE     20
#define DELAY_SHOT_PROJECTILE       250

// CORES
#define BASE_TEXT_COLOR             0xFFE0
#define BACKGROUND_COLOR            0x00

// MAXIMO RANKING E MAXIMO NOME DO JOGADOR
#define MAX_RANKING_ENTRIES         3
#define MAX_NAME_PLAYER             4
#define MAX_PLAYER_PROJECTILES      3
#define MAX_ENEMIES                 3
#define MAX_PLAYER_LIVES            1

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

void addPoints(uint16_t pontos);
void clearScreen();
void formatBaseText(uint8_t tamanho);
void showText(uint8_t x, uint8_t y, const __FlashStringHelper* texto);

void renderEnemy(uint8_t inimigo, uint16_t cor = ST77XX_WHITE);

void updateProjectiles();
void updateRanking();
void enableEnemy(uint8_t inimigo, uint8_t positionX, uint8_t positionY);
boolean shootPlayerProjectile();
void updateEnemies();
void removeEnemy(uint8_t inimigo);
void applyEnemyDamage(uint8_t inimigo, uint8_t dano);
void renderProjectile(uint8_t tiro);
void hideEnemy(uint8_t inimigo);
void updateGameOverSelector();
void updatePlayerName(int8_t esquerda, int8_t direita);
uint8_t checkEnemyCollision(uint8_t x, uint8_t y);


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
} tiro;

typedef struct {
    uint8_t positionX;
    uint8_t positionY;
    uint32_t lastDamageUpdate;
    uint32_t lastMovementUpdate;
    uint8_t hp;
    boolean active;
} inimigo;

inimigo enemies[MAX_ENEMIES] = {0, 0, 0, 0, 5, false};
uint8_t totalEnemies = 0;

tiro playerProjectiles[MAX_PLAYER_PROJECTILES] = {0, 0, 0, false};
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

void formatBaseText(uint8_t tamanho)
{
    tft.setTextSize(tamanho);
    tft.setTextColor(BASE_TEXT_COLOR);
}

void showText(uint8_t x, uint8_t y, const __FlashStringHelper* texto)
{
    tft.setCursor(x, y);
    tft.print(texto);    
}

void showText(uint8_t x, uint8_t y, const String texto)
{
    tft.setCursor(x, y);
    tft.print(texto);    
}

void renderizarSimbolo(uint8_t x, uint8_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t Size){
    tft.setFont(&SymbolMono18pt7b);
    tft.drawChar(x,y,c,color,bg,Size);
    tft.setFont();
}

void exibirHUD() {
    renderizarSimbolo(0, 20, GLYPH_HEART, BASE_TEXT_COLOR,0,1);
    tft.fillRect(24, 0, 75, 25, BACKGROUND_COLOR);
    formatBaseText(2);
    showText(25, 5, String(totalLives));
    showText(55, 5, String(totalPoints));
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
    totalPoints = 0;
    playerNameSelector = 0;
    strcpy(playerName, "A__");
    totalLives = MAX_PLAYER_LIVES;

    exibirHUD();
    renderPlayerShip();

    enableEnemy(0, 120, 35);
}

void atualizaJogo(int8_t esquerda, int8_t direita)
{
    if (esquerda == HIGH) {
        movePlayerShip(MOVEMENT_DISTANCE_RIGHT);
    }
    if (direita == HIGH) {
        movePlayerShip(MOVEMENT_DISTANCE_LEFT);
    }
    shootPlayerProjectile();
    updateProjectiles();
    updateEnemies();
}

boolean shootPlayerProjectile()
{
    if (millis() - lastPlayerShotUpdate > DELAY_SHOT_PROJECTILE) {
        if (totalPlayerProjectiles < MAX_PLAYER_PROJECTILES) {
            for (uint8_t i = 0; i < MAX_PLAYER_PROJECTILES; i++) {
                if (!playerProjectiles[i].active) {
                    playerProjectiles[i].positionX = playerPositionX + 14;
                    playerProjectiles[i].positionY = 205;
                    playerProjectiles[i].lastUpdate = 0;
                    playerProjectiles[i].active = true;
                    lastPlayerShotUpdate = millis();
                    renderProjectile(i);
                    return true;
                }
            }
        }
    }
    return false;
}

void renderProjectile(uint8_t tiro)
{
    tft.drawRect(playerProjectiles[tiro].positionX, playerProjectiles[tiro].positionY, 2, 5, BASE_TEXT_COLOR);
}

void ocultarTiro(uint8_t tiro)
{
    tft.drawRect(playerProjectiles[tiro].positionX, playerProjectiles[tiro].positionY, 2, 5, BACKGROUND_COLOR);
}

void removerTiro(uint8_t tiro)
{
    playerProjectiles[tiro].active = false;
    ocultarTiro(tiro);
}

void updateProjectiles()
{
    for(uint8_t i = 0; i < MAX_PLAYER_PROJECTILES; i++)
    {
        if (playerProjectiles[i].active && (millis() - playerProjectiles[i].lastUpdate > DELAY_UPDATE_PROJECTILE)) {
            ocultarTiro(i);
            if (playerProjectiles[i].positionY - MOVIMENTO_TIRO > 0) {
                playerProjectiles[i].positionY -= MOVIMENTO_TIRO;
            } else {
                removerTiro(i);
                return;
            }
            renderProjectile(i);
            uint8_t inimigo = checkEnemyCollision(playerProjectiles[i].positionX, playerProjectiles[i].positionY);
            if(inimigo != MAX_ENEMIES)
            {
                addPoints(10);
                removerTiro(i);
                applyEnemyDamage(inimigo, 1);
            }
            playerProjectiles[i].lastUpdate = millis();
        }
    }
}

void applyEnemyDamage(uint8_t inimigo, uint8_t dano)
{
    if (enemies[inimigo].hp - dano > 0)
    {
        enemies[inimigo].hp -= dano;
        enemies[inimigo].lastDamageUpdate = millis();
        renderEnemy(inimigo, ST77XX_RED);
    } else  {
        enemies[inimigo].hp = 0;
        removeEnemy(inimigo);
    }
}

void addPoints(uint16_t pontos)
{
    totalPoints += 10;
    exibirHUD();
}

boolean checarColisaoCirculo(uint8_t alvoX, uint8_t alvoY, uint8_t raioAlvo, uint8_t x, uint8_t y)
{
    return (pow(x - alvoX, 2) + pow(y - alvoY, 2)) < pow(raioAlvo, 2);
}

uint8_t checkEnemyCollision(uint8_t x, uint8_t y)
{
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            if (checarColisaoCirculo(enemies[i].positionX, enemies[i].positionY, 25, x, y)) {
                return i;
            }
        }
    }
    return MAX_ENEMIES;
}

void enableEnemy(uint8_t inimigo, uint8_t positionX, uint8_t positionY)
{
    if (!enemies[inimigo].active) {
        enemies[inimigo].positionX = positionX;
        enemies[inimigo].positionY = positionY;
        enemies[inimigo].lastDamageUpdate = 0;
        enemies[inimigo].lastMovementUpdate = 0;
        enemies[inimigo].active = true;
        enemies[inimigo].hp = 5;

        renderEnemy(inimigo);
    }
}

void removeEnemy(uint8_t inimigo)
{
    enemies[inimigo].active = false;
    hideEnemy(inimigo);
}

boolean checarRanking()
{
    if (totalPoints > rankingPoints[MAX_RANKING_ENTRIES - 1]) {
        return true;
    }
    return false;
}

void imprimirTelaGameOver()
{
    formatBaseText(4);
    showText(20, 60, F("GameOver"));
    formatBaseText(3);
    if (checarRanking()) {
        currentScreen = SCREEN_SCORE;
        showText(20, 100, String(totalPoints));
        updateGameOverSelector();
    } else {
        currentScreen = SCREEN_GAME_OVER;
    }
}

void updateGameOverSelector()
{
    formatBaseText(3);
    tft.fillRect(20, 130, 55, 25, BACKGROUND_COLOR);
    showText(20, 130, playerName);
}

void updatePlayerName(int8_t esquerda, int8_t direita)
{
    checarRanking();
    if (esquerda == HIGH) {
        if (playerName[playerNameSelector] >= 'A' && playerName[playerNameSelector] < 'Z') {
            playerName[playerNameSelector] += 1;
        } else {
            playerName[playerNameSelector] = 'A';
        }
        updateGameOverSelector();
    }
    if (direita == HIGH) {
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

void perderVida()
{
    if (totalLives <= 1) {
        imprimirTelaGameOver();
    }
    totalLives--;
    exibirHUD();
}

void updateEnemies()
{
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active && (millis() - enemies[i].lastMovementUpdate > 50)) {
            hideEnemy(i);
            if (enemies[i].positionY + 4 > 220) {
                enemies[i].positionY = 0;

                perderVida();
            } else {
                enemies[i].positionY += 4;
            }
            if (millis() - enemies[i].lastDamageUpdate > 150) {
                renderEnemy(i);
            } else {
                renderEnemy(i, ST77XX_RED);
            }
            enemies[i].lastMovementUpdate = millis();
        }
    }
}

void hideEnemy(uint8_t inimigo)
{
    renderEnemy(inimigo, BACKGROUND_COLOR);
}

void renderEnemy(uint8_t inimigo, uint16_t cor)
{
    tft.fillCircle(enemies[inimigo].positionX, enemies[inimigo].positionY, 25, cor);
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
    if (((x > 0 && playerPositionX < 215) ||
        (x < 0 && playerPositionX > 0)) && (millis() - lastPlayerMovementUpdate > 18)) {
        hidePlayerShip();
        
        if (x > 0) { // direita positivo
            if (playerPositionX > 215) {
                playerPositionX = 215;
            }
            playerPositionX += x;
        } else { // esquerda (negativo)
            if(playerPositionX + x > 0) {
                playerPositionX += x;
            } else {
                playerPositionX = 0;
            }
        }
        lastPlayerMovementUpdate = millis();
        renderPlayerShip();
    }
}

void renderPlayerShip()
{
    tft.drawBitmap(playerPositionX, 200, playerShipBitmap, 32, 32, ST77XX_WHITE);
}

void hidePlayerShip()
{
    tft.drawBitmap(playerPositionX, 200, playerShipBitmap, 32, 32, BACKGROUND_COLOR);
}

void sortRanking()
{
    uint8_t i, j;
    uint32_t troca;
    char trocaNome[MAX_NAME_PLAYER];
    for (i = 0; i < MAX_RANKING_ENTRIES - 1; i++) {
        for (j = i + 1; j < 3; j++) {
            if (rankingPoints[j] > rankingPoints[i]) {
                troca = rankingPoints[j];
                rankingPoints[j] = rankingPoints[i];
                rankingPoints[i] = troca;
                strcpy(trocaNome, playersNames[j]);
                strcpy(playersNames[j], playersNames[i]);
                strcpy(playersNames[i], trocaNome);
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
    int8_t selecionaEstado = digitalRead(BUTTON_SELECT_PIN);
    int8_t confirmaEstado = digitalRead(BUTTON_CONFIRM_PIN);
    if (currentScreen == SCREEN_PLAY) {
        atualizaJogo(selecionaEstado, confirmaEstado);
    } else {
        if (currentScreen == SCREEN_MENU) {
            if (selecionaEstado == HIGH) {
                updateMenuSelector();
            }
            if (confirmaEstado == HIGH) {
                selectMenuOption();
            }
        } else {
            if (currentScreen == SCREEN_SCORE) {
                updatePlayerName(selecionaEstado, confirmaEstado);
            } else {
                if (selecionaEstado == HIGH || confirmaEstado == HIGH) {
                    backMainMenu();
                }
            }
        }
    }
}