#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <string.h>
#include "SymbolMono18pt7b.h"

// PINOS DISPLAY
#define TFT_CS                  10
#define TFT_RST                 8  // define reset pin, or set to -1 and connect to Arduino RESET pin
#define TFT_DC                  9  // define data/command pin

// PINOS BOTÕES
#define BOTAOSELECIONAR_PIN     2 //botão para selecionar
#define BOTAOCONFIRMAR_PIN      3 //botão para confirmar

// DEFINIÇÕES DE ALINHAMENTO
#define INICIO_LINHA_MENU       10
#define TAMANHO_LINHA_MENU      38

// TELAS
#define NUMERO_TELAS            4
#define TELA_MENU               0
#define TELA_JOGAR              1
#define TELA_RANKING            2
#define TELA_CREDITOS           3
#define TELA_SCORE              4
#define TELA_GAME_OVER          5

#define MOVIMENTO_NAVE_ESQ      8
#define MOVIMENTO_NAVE_DIR      -8
#define MOVIMENTO_TIRO          10

#define ATRASO_ATUALIZACAO_TIRO 20
#define ATRASO_DISPARO          250

// CORES
#define COR_TEXTO_BASE          0xFFE0
#define COR_FUNDO               0x00

// MAXIMO RANKING E MAXIMO NOME DO JOGADOR
#define MAXIMO_RANKING          3
#define MAXIMO_NOME_JOGADOR     4
#define MAXIMO_TIROS            3
#define MAXIMO_INIMIGOS         3
#define MAXIMO_VIDAS            1

void imprimirMenu();
void ImprimirLogo();
void ocultarSeletorAnterior();
void mostrarSeletorMenu();
void atualizarSeletorMenu();
void selecionarOpcaoMenu();
void lerJogador();
void iniciarJogo();
void imprimirRanking();
void ordenarRanking();
void creditos();
void sair();
void voltarMenu();
void moverNave(int8_t x);
void renderizarNave();
void ocultarNave();

void somarPontos(uint16_t pontos);
void limparTela();
void formatarTextoBase(uint8_t tamanho);
void imprimirTexto(uint8_t x, uint8_t y, const __FlashStringHelper* texto);

void renderizarInimigo(uint8_t inimigo, uint16_t cor = ST77XX_WHITE);

void atualizaDisparos();
void atualizarRanking();
void ativarInimigo(uint8_t inimigo, uint8_t posicaoX, uint8_t posicaoY);
boolean dispararTiro();
void atualizarDisparos();
void atualizaInimigos();
void renderizarTiro(uint8_t tiro);
void ocultarInimigo(uint8_t inimigo);
void atualizarSeletorGameOver();
void atualizarNomeJogador(int8_t esquerda, int8_t direita);
void atualizarDisparos();
uint8_t checarColisaoInimigos(uint8_t x, uint8_t y);


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
    boolean ativo;
} inimigo;

inimigo inimigos[MAXIMO_INIMIGOS] = {0, 0, 0, 0, false};
uint8_t totalInimigos = 0;

tiro tiros[MAXIMO_TIROS] = {0, 0, 0, false};
uint8_t totalTiros = 0;

uint8_t menuSelecionado = 1;
uint8_t telaAtual = 0;

uint8_t seletorNome = 0;
char nomeJogador[MAXIMO_NOME_JOGADOR] = "A__";
uint8_t totalVidas = MAXIMO_VIDAS;
uint16_t totalPontos = 0;
uint8_t posicaoNave = 100;
uint32_t delayNave = 0;

uint32_t delayTiro = 0;

uint32_t pontosRanking[MAXIMO_RANKING] = {0, 0, 0};
char jogadores[MAXIMO_RANKING][MAXIMO_NOME_JOGADOR] = {"\0", "\0", "\0"};

void setup(void) {
    Serial.begin(9600);

    // if the display has CS pin try with SPI_MODE0
    tft.init(240, 240, SPI_MODE2);    // Init ST7789 display 240x240 pixel
    tft.setRotation(2);

    //ImprimirLogo();
    //delay(3000);

    pinMode(BOTAOSELECIONAR_PIN, INPUT);
    pinMode(BOTAOCONFIRMAR_PIN, INPUT);

    ordenarRanking();
    imprimirMenu();
}

void ImprimirLogo() {
    limparTela();
    formatarTextoBase(4);
    imprimirTexto(40, 40, F("Attack"));
    imprimirTexto(40, 80, F("on"));
    imprimirTexto(40, 120, F("Death"));
    imprimirTexto(40, 160, F("Star"));
}

void formatarTextoBase(uint8_t tamanho)
{
    tft.setTextSize(tamanho);
    tft.setTextColor(COR_TEXTO_BASE);
}

void imprimirTexto(uint8_t x, uint8_t y, const __FlashStringHelper* texto)
{
    tft.setCursor(x, y);
    tft.print(texto);    
}

void imprimirTexto(uint8_t x, uint8_t y, const String texto)
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
    renderizarSimbolo(0, 20, GLYPH_HEART, COR_TEXTO_BASE,0,1);
    tft.fillRect(24, 0, 75, 25, COR_FUNDO);
    formatarTextoBase(2);
    imprimirTexto(25, 5, String(totalVidas));
    imprimirTexto(55, 5, String(totalPontos));
}

void imprimirMenu() {
    limparTela();


    formatarTextoBase(4);
    imprimirTexto(36, INICIO_LINHA_MENU + TAMANHO_LINHA_MENU, F("Jogar"));
    imprimirTexto(36, INICIO_LINHA_MENU + (2 * TAMANHO_LINHA_MENU), F("Ranking"));
    imprimirTexto(36, INICIO_LINHA_MENU + (3 * TAMANHO_LINHA_MENU), F("Creditos"));
    imprimirTexto(36, INICIO_LINHA_MENU + (4 * TAMANHO_LINHA_MENU), F("Opcoes"));

    mostrarSeletorMenu();
}

void ocultarSeletorAnterior()
{
    tft.setTextColor(COR_FUNDO);
    imprimirTexto(10, INICIO_LINHA_MENU + (menuSelecionado * TAMANHO_LINHA_MENU), F("\t"));   
    delay(20);
}

void mostrarSeletorMenu()
{
    tft.setTextColor(COR_TEXTO_BASE);
    imprimirTexto(10, INICIO_LINHA_MENU + (menuSelecionado * TAMANHO_LINHA_MENU), F("\t"));
}

void atualizarSeletorMenu()
{
    ocultarSeletorAnterior();
    if (menuSelecionado < NUMERO_TELAS) {
        menuSelecionado++;
    } else {
        menuSelecionado = TELA_JOGAR;
    }
    mostrarSeletorMenu();

    // Delay é utilizado para evitar a execução repetida
    delay(500);
}

void selecionarOpcaoMenu()
{
    switch (menuSelecionado) {
        case TELA_JOGAR: iniciarJogo();
            break;
        case TELA_RANKING: imprimirRanking();
            break;
        case TELA_CREDITOS: creditos();
            break;
    }
    telaAtual = menuSelecionado;
}

void iniciarJogo()
{
    limparTela();
    delay(1000);  
    totalPontos = 0;
    seletorNome = 0;
    strcpy(nomeJogador, "A__");
    totalVidas = MAXIMO_VIDAS;

    exibirHUD();
    renderizarNave();

    ativarInimigo(0, 120, 35);
}

void atualizaJogo(int8_t esquerda, int8_t direita)
{
    if (esquerda == HIGH) {
        moverNave(MOVIMENTO_NAVE_DIR);
    }
    if (direita == HIGH) {
        moverNave(MOVIMENTO_NAVE_ESQ);
    }
    dispararTiro();
    atualizaDisparos();
    atualizaInimigos();
}

boolean dispararTiro()
{
    if (millis() - delayTiro > ATRASO_DISPARO) {
        if (totalTiros < MAXIMO_TIROS) {
            for (uint8_t i = 0; i < MAXIMO_TIROS; i++) {
                if (!tiros[i].ativo) {
                    tiros[i].posicaoX = posicaoNave + 14;
                    tiros[i].posicaoY = 205;
                    tiros[i].ultimaAtualizacao = 0;
                    tiros[i].ativo = true;
                    delayTiro = millis();
                    renderizarTiro(i);
                    return true;
                }
            }
        }
    }
    return false;
}

void renderizarTiro(uint8_t tiro)
{
    tft.drawRect(tiros[tiro].posicaoX, tiros[tiro].posicaoY, 2, 5, COR_TEXTO_BASE);
}

void ocultarTiro(uint8_t tiro)
{
    tft.drawRect(tiros[tiro].posicaoX, tiros[tiro].posicaoY, 2, 5, COR_FUNDO);
}

void removerTiro(uint8_t tiro)
{
    tiros[tiro].ativo = false;
    ocultarTiro(tiro);
}

void atualizaDisparos()
{
    for(uint8_t i = 0; i < MAXIMO_TIROS; i++)
    {
        if (tiros[i].ativo && (millis() - tiros[i].ultimaAtualizacao > ATRASO_ATUALIZACAO_TIRO)) {
            ocultarTiro(i);
            if (tiros[i].posicaoY - MOVIMENTO_TIRO > 0) {
                tiros[i].posicaoY -= MOVIMENTO_TIRO;
            } else {
                removerTiro(i);
                return;
            }
            renderizarTiro(i);
            uint8_t inimigo = checarColisaoInimigos(tiros[i].posicaoX, tiros[i].posicaoY);
            if(inimigo != MAXIMO_INIMIGOS)
            {
                somarPontos(10);
                removerTiro(i);
                inimigos[inimigo].ultimoDano = millis();
                renderizarInimigo(inimigo, ST77XX_RED);
            }
            tiros[i].ultimaAtualizacao = millis();
        }
    }
}

void somarPontos(uint16_t pontos)
{
    totalPontos += 10;
    exibirHUD();
}

boolean checarColisaoCirculo(uint8_t alvoX, uint8_t alvoY, uint8_t raioAlvo, uint8_t x, uint8_t y)
{
    return (pow(x - alvoX, 2) + pow(y - alvoY, 2)) < pow(raioAlvo, 2);
}

uint8_t checarColisaoInimigos(uint8_t x, uint8_t y)
{
    for (uint8_t i = 0; i < MAXIMO_INIMIGOS; i++) {
        if (inimigos[i].ativo) {
            if (checarColisaoCirculo(inimigos[i].posicaoX, inimigos[i].posicaoY, 25, x, y)) {
                return i;
            }
        }
    }
    return MAXIMO_INIMIGOS;
}

void ativarInimigo(uint8_t inimigo, uint8_t posicaoX, uint8_t posicaoY)
{
    if (!inimigos[inimigo].ativo) {
        inimigos[inimigo].posicaoX = posicaoX;
        inimigos[inimigo].posicaoY = posicaoY;
        inimigos[inimigo].ultimoDano = 0;
        inimigos[inimigo].ultimoMovimento = 0;
        inimigos[inimigo].ativo = true;

        renderizarInimigo(inimigo);
    }
}

boolean checarRanking()
{
    if (totalPontos > pontosRanking[MAXIMO_RANKING - 1]) {
        return true;
    }
    return false;
}

void imprimirTelaGameOver()
{
    formatarTextoBase(4);
    imprimirTexto(20, 60, F("GameOver"));
    formatarTextoBase(3);
    if (checarRanking()) {
        telaAtual = TELA_SCORE;
        imprimirTexto(20, 100, String(totalPontos));
        atualizarSeletorGameOver();
    } else {
        telaAtual = TELA_GAME_OVER;
    }
}

void atualizarSeletorGameOver()
{
    formatarTextoBase(3);
    tft.fillRect(20, 130, 55, 25, COR_FUNDO);
    imprimirTexto(20, 130, nomeJogador);
}

void atualizarNomeJogador(int8_t esquerda, int8_t direita)
{
    checarRanking();
    if (esquerda == HIGH) {
        if (nomeJogador[seletorNome] >= 'A' && nomeJogador[seletorNome] < 'Z') {
            nomeJogador[seletorNome] += 1;
        } else {
            nomeJogador[seletorNome] = 'A';
        }
        atualizarSeletorGameOver();
    }
    if (direita == HIGH) {
        if (seletorNome < 2) {
            seletorNome++;
            nomeJogador[seletorNome] = 'A';
            atualizarSeletorGameOver();
        } else {
            atualizarRanking();
            voltarMenu();
        }
    }
    delay(200);
}

void atualizarRanking()
{
    strcpy(jogadores[MAXIMO_RANKING - 1], nomeJogador);
    pontosRanking[MAXIMO_RANKING - 1] = totalPontos;
    ordenarRanking();
}

void perderVida()
{
    if (totalVidas <= 1) {
        imprimirTelaGameOver();
    }
    totalVidas--;
    exibirHUD();
}

void atualizaInimigos()
{
    for (uint8_t i = 0; i < MAXIMO_INIMIGOS; i++) {
        if (inimigos[i].ativo && (millis() - inimigos[i].ultimoMovimento > 50)) {
            ocultarInimigo(i);
            if (inimigos[i].posicaoY + 4 > 220) {
                inimigos[i].posicaoY = 0;

                perderVida();
            } else {
                inimigos[i].posicaoY += 4;
            }
            if (millis() - inimigos[i].ultimoDano > 150) {
                renderizarInimigo(i);
            } else {
                renderizarInimigo(i, ST77XX_RED);
            }
            inimigos[i].ultimoMovimento = millis();
        }
    }
}

void ocultarInimigo(uint8_t inimigo)
{
    renderizarInimigo(inimigo, COR_FUNDO);
}

void renderizarInimigo(uint8_t inimigo, uint16_t cor)
{
    tft.fillCircle(inimigos[inimigo].posicaoX, inimigos[inimigo].posicaoY, 25, cor);
}

void creditos()
{
    limparTela();
    formatarTextoBase(3);

    imprimirTexto(36, INICIO_LINHA_MENU + TAMANHO_LINHA_MENU, F("Creditos"));

    imprimirTexto(36, INICIO_LINHA_MENU + (2 * TAMANHO_LINHA_MENU), F("Gabriel"));
    imprimirTexto(36, INICIO_LINHA_MENU + (3 * TAMANHO_LINHA_MENU), F("Helcio"));
    imprimirTexto(36, INICIO_LINHA_MENU + (4 * TAMANHO_LINHA_MENU), F("Jean"));
    imprimirTexto(36, INICIO_LINHA_MENU + (5 * TAMANHO_LINHA_MENU), F("Kellwyn"));
    imprimirTexto(36, INICIO_LINHA_MENU + (6 * TAMANHO_LINHA_MENU), F("Rafael"));
    imprimirTexto(36, INICIO_LINHA_MENU + (7 * TAMANHO_LINHA_MENU), F("Rennan"));
}

void limparTela()
{
    tft.fillScreen(COR_FUNDO); 
}

void imprimirRanking()
{
    limparTela();
    formatarTextoBase(3);
    imprimirTexto(36, INICIO_LINHA_MENU + TAMANHO_LINHA_MENU, F("Ranking"));

    for (int i = 0; i < 3; i++) {
        tft.setCursor(36, INICIO_LINHA_MENU + ((i + 2) * TAMANHO_LINHA_MENU));
        tft.print(jogadores[i]);
        tft.print (": ");
        tft.print(pontosRanking[i]);
    }
}

void moverNave(int8_t x)
{
    if (((x > 0 && posicaoNave < 215) ||
        (x < 0 && posicaoNave > 0)) && (millis() - delayNave > 18)) {
        ocultarNave();
        
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
        renderizarNave();
    }
}

void renderizarNave()
{
    tft.drawBitmap(posicaoNave, 200, nave, 32, 32, ST77XX_WHITE);
}

void ocultarNave()
{
    tft.drawBitmap(posicaoNave, 200, nave, 32, 32, COR_FUNDO);
}

void ordenarRanking()
{
    uint8_t i, j;
    uint32_t troca;
    char trocaNome[MAXIMO_NOME_JOGADOR];
    for (i = 0; i < MAXIMO_RANKING - 1; i++) {
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

void voltarMenu()
{
    telaAtual = TELA_MENU;
    imprimirMenu();
}

void loop() {
    int8_t selecionaEstado = digitalRead(BOTAOSELECIONAR_PIN);
    int8_t confirmaEstado = digitalRead(BOTAOCONFIRMAR_PIN);
    if (telaAtual == TELA_JOGAR) {
        atualizaJogo(selecionaEstado, confirmaEstado);
    } else {
        if (telaAtual == TELA_MENU) {
            if (selecionaEstado == HIGH) {
                atualizarSeletorMenu();
            }
            if (confirmaEstado == HIGH) {
                selecionarOpcaoMenu();
            }
        } else {
            if (telaAtual == TELA_SCORE) {
                atualizarNomeJogador(selecionaEstado, confirmaEstado);
            } else {
                if (selecionaEstado == HIGH || confirmaEstado == HIGH) {
                    voltarMenu();
                }
            }
        }
    }
}