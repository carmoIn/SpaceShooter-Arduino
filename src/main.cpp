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
void enableEnemy(uint8_t inimigo, uint8_t posicaoX, uint8_t posicaoY);
boolean shootPlayerProjectile();
void updateEnemies();
void removeEnemy(uint8_t inimigo);
void applyEnemyDamage(uint8_t inimigo, uint8_t dano);
void renderProjectile(uint8_t tiro);
void hideEnemy(uint8_t inimigo);
void updateGameOverSelector();
void updatePlayerName(int8_t esquerda, int8_t direita);
uint8_t checkEnemyCollision(uint8_t x, uint8_t y);


const unsigned char nave[] PROGMEM = {
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
    uint8_t posicaoX;
    uint8_t posicaoY;
    uint32_t ultimaAtualizacao;
    boolean ativo;
} tiro;

typedef struct {
    uint8_t posicaoX;
    uint8_t posicaoY;
    uint32_t ultimoDano;
    uint32_t ultimoMovimento;
    uint8_t vida;
    boolean ativo;
} inimigo;

inimigo inimigos[MAX_ENEMIES] = {0, 0, 0, 0, 5, false};
uint8_t totalInimigos = 0;

tiro tiros[MAX_PLAYER_PROJECTILES] = {0, 0, 0, false};
uint8_t totalTiros = 0;

uint8_t menuSelecionado = 1;
uint8_t telaAtual = 0;

uint8_t seletorNome = 0;
char nomeJogador[MAX_NAME_PLAYER] = "A__";
uint8_t totalVidas = MAX_PLAYER_LIVES;
uint16_t totalPontos = 0;
uint8_t posicaoNave = 100;
uint32_t delayNave = 0;

uint32_t delayTiro = 0;

uint32_t pontosRanking[MAX_RANKING_ENTRIES] = {0, 0, 0};
char jogadores[MAX_RANKING_ENTRIES][MAX_NAME_PLAYER] = {"\0", "\0", "\0"};

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
    showText(25, 5, String(totalVidas));
    showText(55, 5, String(totalPontos));
}

void showMainMenu() {
    clearScreen();


    formatBaseText(4);
    showText(36, FIRST_LINE_MENU + LINE_HEIGHT_MENU, F("Jogar"));
    showText(36, FIRST_LINE_MENU + (2 * LINE_HEIGHT_MENU), F("Ranking"));
    showText(36, FIRST_LINE_MENU + (3 * LINE_HEIGHT_MENU), F("showCredits"));
    showText(36, FIRST_LINE_MENU + (4 * LINE_HEIGHT_MENU), F("Opcoes"));

    showMenuSelector();
}

void hidePreviousSelector()
{
    tft.setTextColor(BACKGROUND_COLOR);
    showText(10, FIRST_LINE_MENU + (menuSelecionado * LINE_HEIGHT_MENU), F("\t"));   
    delay(20);
}

void showMenuSelector()
{
    tft.setTextColor(BASE_TEXT_COLOR);
    showText(10, FIRST_LINE_MENU + (menuSelecionado * LINE_HEIGHT_MENU), F("\t"));
}

void updateMenuSelector()
{
    hidePreviousSelector();
    if (menuSelecionado < NUMBER_SCREENS) {
        menuSelecionado++;
    } else {
        menuSelecionado = SCREEN_PLAY;
    }
    showMenuSelector();

    // Delay é utilizado para evitar a execução repetida
    delay(500);
}

void selectMenuOption()
{
    switch (menuSelecionado) {
        case SCREEN_PLAY: initGame();
            break;
        case SCREEN_RANKING: showRanking();
            break;
        case SCREEN_CREDITS: showCredits();
            break;
    }
    telaAtual = menuSelecionado;
}

void initGame()
{
    clearScreen();
    delay(1000);  
    totalPontos = 0;
    seletorNome = 0;
    strcpy(nomeJogador, "A__");
    totalVidas = MAX_PLAYER_LIVES;

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
    if (millis() - delayTiro > DELAY_SHOT_PROJECTILE) {
        if (totalTiros < MAX_PLAYER_PROJECTILES) {
            for (uint8_t i = 0; i < MAX_PLAYER_PROJECTILES; i++) {
                if (!tiros[i].ativo) {
                    tiros[i].posicaoX = posicaoNave + 14;
                    tiros[i].posicaoY = 205;
                    tiros[i].ultimaAtualizacao = 0;
                    tiros[i].ativo = true;
                    delayTiro = millis();
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
    tft.drawRect(tiros[tiro].posicaoX, tiros[tiro].posicaoY, 2, 5, BASE_TEXT_COLOR);
}

void ocultarTiro(uint8_t tiro)
{
    tft.drawRect(tiros[tiro].posicaoX, tiros[tiro].posicaoY, 2, 5, BACKGROUND_COLOR);
}

void removerTiro(uint8_t tiro)
{
    tiros[tiro].ativo = false;
    ocultarTiro(tiro);
}

void updateProjectiles()
{
    for(uint8_t i = 0; i < MAX_PLAYER_PROJECTILES; i++)
    {
        if (tiros[i].ativo && (millis() - tiros[i].ultimaAtualizacao > DELAY_UPDATE_PROJECTILE)) {
            ocultarTiro(i);
            if (tiros[i].posicaoY - MOVIMENTO_TIRO > 0) {
                tiros[i].posicaoY -= MOVIMENTO_TIRO;
            } else {
                removerTiro(i);
                return;
            }
            renderProjectile(i);
            uint8_t inimigo = checkEnemyCollision(tiros[i].posicaoX, tiros[i].posicaoY);
            if(inimigo != MAX_ENEMIES)
            {
                addPoints(10);
                removerTiro(i);
                applyEnemyDamage(inimigo, 1);
            }
            tiros[i].ultimaAtualizacao = millis();
        }
    }
}

void applyEnemyDamage(uint8_t inimigo, uint8_t dano)
{
    if (inimigos[inimigo].vida - dano > 0)
    {
        inimigos[inimigo].vida -= dano;
        inimigos[inimigo].ultimoDano = millis();
        renderEnemy(inimigo, ST77XX_RED);
    } else  {
        inimigos[inimigo].vida = 0;
        removeEnemy(inimigo);
    }
}

void addPoints(uint16_t pontos)
{
    totalPontos += 10;
    exibirHUD();
}

boolean checarColisaoCirculo(uint8_t alvoX, uint8_t alvoY, uint8_t raioAlvo, uint8_t x, uint8_t y)
{
    return (pow(x - alvoX, 2) + pow(y - alvoY, 2)) < pow(raioAlvo, 2);
}

uint8_t checkEnemyCollision(uint8_t x, uint8_t y)
{
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (inimigos[i].ativo) {
            if (checarColisaoCirculo(inimigos[i].posicaoX, inimigos[i].posicaoY, 25, x, y)) {
                return i;
            }
        }
    }
    return MAX_ENEMIES;
}

void enableEnemy(uint8_t inimigo, uint8_t posicaoX, uint8_t posicaoY)
{
    if (!inimigos[inimigo].ativo) {
        inimigos[inimigo].posicaoX = posicaoX;
        inimigos[inimigo].posicaoY = posicaoY;
        inimigos[inimigo].ultimoDano = 0;
        inimigos[inimigo].ultimoMovimento = 0;
        inimigos[inimigo].ativo = true;
        inimigos[inimigo].vida = 5;

        renderEnemy(inimigo);
    }
}

void removeEnemy(uint8_t inimigo)
{
    inimigos[inimigo].ativo = false;
    hideEnemy(inimigo);
}

boolean checarRanking()
{
    if (totalPontos > pontosRanking[MAX_RANKING_ENTRIES - 1]) {
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
        telaAtual = SCREEN_SCORE;
        showText(20, 100, String(totalPontos));
        updateGameOverSelector();
    } else {
        telaAtual = SCREEN_GAME_OVER;
    }
}

void updateGameOverSelector()
{
    formatBaseText(3);
    tft.fillRect(20, 130, 55, 25, BACKGROUND_COLOR);
    showText(20, 130, nomeJogador);
}

void updatePlayerName(int8_t esquerda, int8_t direita)
{
    checarRanking();
    if (esquerda == HIGH) {
        if (nomeJogador[seletorNome] >= 'A' && nomeJogador[seletorNome] < 'Z') {
            nomeJogador[seletorNome] += 1;
        } else {
            nomeJogador[seletorNome] = 'A';
        }
        updateGameOverSelector();
    }
    if (direita == HIGH) {
        if (seletorNome < 2) {
            seletorNome++;
            nomeJogador[seletorNome] = 'A';
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
    strcpy(jogadores[MAX_RANKING_ENTRIES - 1], nomeJogador);
    pontosRanking[MAX_RANKING_ENTRIES - 1] = totalPontos;
    sortRanking();
}

void perderVida()
{
    if (totalVidas <= 1) {
        imprimirTelaGameOver();
    }
    totalVidas--;
    exibirHUD();
}

void updateEnemies()
{
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (inimigos[i].ativo && (millis() - inimigos[i].ultimoMovimento > 50)) {
            hideEnemy(i);
            if (inimigos[i].posicaoY + 4 > 220) {
                inimigos[i].posicaoY = 0;

                perderVida();
            } else {
                inimigos[i].posicaoY += 4;
            }
            if (millis() - inimigos[i].ultimoDano > 150) {
                renderEnemy(i);
            } else {
                renderEnemy(i, ST77XX_RED);
            }
            inimigos[i].ultimoMovimento = millis();
        }
    }
}

void hideEnemy(uint8_t inimigo)
{
    renderEnemy(inimigo, BACKGROUND_COLOR);
}

void renderEnemy(uint8_t inimigo, uint16_t cor)
{
    tft.fillCircle(inimigos[inimigo].posicaoX, inimigos[inimigo].posicaoY, 25, cor);
}

void showCredits()
{
    clearScreen();
    formatBaseText(3);

    showText(36, FIRST_LINE_MENU + LINE_HEIGHT_MENU, F("showCredits"));

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
        tft.print(jogadores[i]);
        tft.print (": ");
        tft.print(pontosRanking[i]);
    }
}

void movePlayerShip(int8_t x)
{
    if (((x > 0 && posicaoNave < 215) ||
        (x < 0 && posicaoNave > 0)) && (millis() - delayNave > 18)) {
        hidePlayerShip();
        
        if (x > 0) { // direita positivo
            if (posicaoNave > 215) {
                posicaoNave = 215;
            }
            posicaoNave += x;
        } else { // esquerda (negativo)
            if(posicaoNave + x > 0) {
                posicaoNave += x;
            } else {
                posicaoNave = 0;
            }
        }
        delayNave = millis();
        renderPlayerShip();
    }
}

void renderPlayerShip()
{
    tft.drawBitmap(posicaoNave, 200, nave, 32, 32, ST77XX_WHITE);
}

void hidePlayerShip()
{
    tft.drawBitmap(posicaoNave, 200, nave, 32, 32, BACKGROUND_COLOR);
}

void sortRanking()
{
    uint8_t i, j;
    uint32_t troca;
    char trocaNome[MAX_NAME_PLAYER];
    for (i = 0; i < MAX_RANKING_ENTRIES - 1; i++) {
        for (j = i + 1; j < 3; j++) {
            if (pontosRanking[j] > pontosRanking[i]) {
                troca = pontosRanking[j];
                pontosRanking[j] = pontosRanking[i];
                pontosRanking[i] = troca;
                strcpy(trocaNome, jogadores[j]);
                strcpy(jogadores[j], jogadores[i]);
                strcpy(jogadores[i], trocaNome);
            }
        }
    }
}

void backMainMenu()
{
    telaAtual = SCREEN_MENU;
    showMainMenu();
}

void loop() {
    int8_t selecionaEstado = digitalRead(BUTTON_SELECT_PIN);
    int8_t confirmaEstado = digitalRead(BUTTON_CONFIRM_PIN);
    if (telaAtual == SCREEN_PLAY) {
        atualizaJogo(selecionaEstado, confirmaEstado);
    } else {
        if (telaAtual == SCREEN_MENU) {
            if (selecionaEstado == HIGH) {
                updateMenuSelector();
            }
            if (confirmaEstado == HIGH) {
                selectMenuOption();
            }
        } else {
            if (telaAtual == SCREEN_SCORE) {
                updatePlayerName(selecionaEstado, confirmaEstado);
            } else {
                if (selecionaEstado == HIGH || confirmaEstado == HIGH) {
                    backMainMenu();
                }
            }
        }
    }
}