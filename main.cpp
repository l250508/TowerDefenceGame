#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>  
#include <iostream>
#include <fstream>
#include "enemy.h"
#include "m3.cpp"
#include "m2.cpp"
#include "m5.cpp"

using namespace sf;
using namespace std;


class Level {
private:
    int currentLevel;
    int enemiesRemaining;
public:
    Level() : currentLevel(1), enemiesRemaining(5) {}
    void enemyDefeated() { enemiesRemaining--; }
    bool levelComplete() { return enemiesRemaining <= 0; }
    void nextLevel() { currentLevel++; if (currentLevel <= 3) enemiesRemaining = currentLevel * 5; }
    bool allLevelsComplete() { return currentLevel > 3; }
    int  getLevel() { return currentLevel; }
    int  getEnemiesRemaining() { return enemiesRemaining; }
    void reset() { currentLevel = 1; enemiesRemaining = 5; }
};


class Settings {
private:
    int difficulty;
public:
    Settings() : difficulty(1) {}
    void setDifficulty(int d) { difficulty = d; }
    int  getDifficulty() { return difficulty; }
    void saveSettings() { ofstream f("settings.txt"); f << difficulty; }
};

enum GameState { MENU, NAME_INPUT, PLAYING, GAME_OVER, WIN, SETTINGS_MENU, LEVEL_TRANSITION };

const int ROWS = 10;     //constants
const int COLS = 15;
const int CELL = 40;
const int OFFSET_X = 50;
const int OFFSET_Y = 100;
const int SIDEBAR_X = OFFSET_X + COLS * CELL + 30;
const int WAVES_PER_LEVEL = 5;

pair<int, int> path[] = {      //creating the path
    {0,1},{1,1},{2,1},{3,1},
    {3,2},{3,3},{3,4},
    {2,4},{1,4},
    {1,5},{1,6},{1,7},
    {2,7},{3,7},{4,7},{5,7},
    {5,8},{5,9},{5,10},
    {6,10},{7,10},{8,10},{9,10}
};
int pathSize = sizeof(path) / sizeof(path[0]);

bool isOnPath(int row, int col) {
    for (int i = 0; i < pathSize; i++)
        if (path[i].first == row && path[i].second == col) return true;
    return false;
}

void setupWaypoints() {
    clearPath();
    for (int i = 0; i < pathSize; i++) {
        float x = OFFSET_X + path[i].second * CELL + CELL / 2;
        float y = OFFSET_Y + path[i].first * CELL + CELL / 2;
        addWaypoint(x, y);
    }
}


//create houses away from the path 
const int HOUSE_COUNT = 8;
int housePos[HOUSE_COUNT][2] = {
    {0, 0},   
    {0, 4},   
    {0, 8},   
    {2, 8},   
    {4, 0},  
    {6, 5},  
    {7, 12},  
    {9, 0}    
};


void registerHousesAsBases() {
    clearBases();
    for (int i = 0; i < HOUSE_COUNT; i++) { // skip any house that accidentally lands on path
        if (isOnPath(housePos[i][0], housePos[i][1])) continue;
        float bx = OFFSET_X + housePos[i][1] * CELL + CELL / 2.f;
        float by = OFFSET_Y + housePos[i][0] * CELL + CELL / 2.f;
        addBase(bx, by, 60, 5);   //it is of 60 hp and player loses 5 hp when destroyed
    }
}


class Button {    //creates buttons 
public:
    RectangleShape box;
    Text text;
    void setup(Font& font, string str, float x, float y, int sz = 20) {
        box.setSize(Vector2f(220, 60));
        box.setPosition(x, y);
        box.setFillColor(Color(40, 40, 40));
        box.setOutlineThickness(2);
        box.setOutlineColor(Color::Cyan);
        text.setFont(font); text.setString(str); text.setCharacterSize(sz);
        text.setFillColor(Color::White);
        FloatRect r = text.getLocalBounds();
        text.setOrigin(r.width / 2, r.height / 2);
        text.setPosition(x + 110, y + 30);
    }
    bool clicked(Event& e, RenderWindow& w) {
        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
            Vector2i m = Mouse::getPosition(w);
            return box.getGlobalBounds().contains((float)m.x, (float)m.y);
        }
        return false;
    }
    void draw(RenderWindow& w) { w.draw(box); w.draw(text); }
};


class SettingsIcon {
public:
    CircleShape icon; Text gearText; bool hovered;
    SettingsIcon() : hovered(false) {}
    void setup(Font& font, float x, float y) {
        icon.setRadius(20); icon.setPosition(x, y);
        icon.setFillColor(Color(60, 60, 80));
        icon.setOutlineThickness(2); icon.setOutlineColor(Color::Cyan);
        gearText.setFont(font); gearText.setString("?");
        gearText.setCharacterSize(28); gearText.setFillColor(Color::White);
        FloatRect r = gearText.getLocalBounds();
        gearText.setOrigin(r.width / 2, r.height / 2 + 2);
        gearText.setPosition(x + 20, y + 20);
    }
    bool clicked(Event& e, RenderWindow& w) {
        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
            Vector2i m = Mouse::getPosition(w);
            return icon.getGlobalBounds().contains((float)m.x, (float)m.y);
        }
        return false;
    }
    void update(RenderWindow& w) {
        Vector2i m = Mouse::getPosition(w);
        hovered = icon.getGlobalBounds().contains((float)m.x, (float)m.y);
        icon.setOutlineColor(hovered ? Color::Yellow : Color::Cyan);
        icon.setFillColor(hovered ? Color(80, 80, 100) : Color(60, 60, 80));
        gearText.setFillColor(hovered ? Color::Yellow : Color::White);
    }
    void draw(RenderWindow& w) { w.draw(icon); w.draw(gearText); }
};


class TowerButton {
public:
    RectangleShape box; Text text; string towerType; int cost;
    void setup(Font& font, string type, int tc, float x, float y) {
        towerType = type; cost = tc;
        box.setSize(Vector2f(130, 50)); box.setPosition(x, y);
        box.setFillColor(Color(50, 50, 80));
        box.setOutlineThickness(2); box.setOutlineColor(Color::Yellow);
        text.setFont(font); text.setString(type + "\n$" + to_string(tc));
        text.setCharacterSize(14); text.setFillColor(Color::White);
        FloatRect r = text.getLocalBounds();
        text.setOrigin(r.width / 2, r.height / 2);
        text.setPosition(x + 65, y + 25);
    }
    bool contains(float mx, float my) { return box.getGlobalBounds().contains(mx, my); }
    void draw(RenderWindow& w) { w.draw(box); w.draw(text); }
};


class UpgradeButton {
public:
    RectangleShape box;
    Text upgradeText, costText, levelText;
    int  towerIndex;
    bool visible;
    UpgradeButton() : towerIndex(-1), visible(false) {}

    void setup(Font& font, float x, float y) {
        box.setSize(Vector2f(200, 90)); box.setPosition(x, y);
        box.setFillColor(Color(40, 40, 60));
        box.setOutlineThickness(2); box.setOutlineColor(Color(255, 200, 0));
        upgradeText.setFont(font); upgradeText.setCharacterSize(14); upgradeText.setFillColor(Color::White);
        costText.setFont(font);    costText.setCharacterSize(12);    costText.setFillColor(Color::Yellow);
        levelText.setFont(font);   levelText.setCharacterSize(12);   levelText.setFillColor(Color::Cyan);
        visible = false;
    }

    void updateForTower(Tower* tower, int index, int coins) {
        if (!tower) { visible = false; return; }
        towerIndex = index;
        visible = tower->canUpgrade();
        if (!visible) return;
        upgradeText.setString("Upgrade: " + tower->getNextUpgradeName());
        costText.setString("Cost: $" + to_string(tower->getUpgradeCost()));
        levelText.setString("Level: " + to_string(tower->getUpgradeLevel()) + "/3");
        upgradeText.setPosition(box.getPosition().x + 10, box.getPosition().y + 10);
        costText.setPosition(box.getPosition().x + 10, box.getPosition().y + 35);
        levelText.setPosition(box.getPosition().x + 10, box.getPosition().y + 60);
        bool canAfford = (coins >= tower->getUpgradeCost());
        box.setOutlineColor(canAfford ? Color(0, 255, 0) : Color(255, 100, 100));
        costText.setFillColor(canAfford ? Color(0, 255, 0) : Color(255, 100, 100));
    }

    bool contains(float mx, float my) { return visible && box.getGlobalBounds().contains(mx, my); }
    void draw(RenderWindow& w) {
        if (!visible) return;
        w.draw(box); w.draw(upgradeText); w.draw(costText); w.draw(levelText);
    }
};


void resetGame(CoinSystem& economy, CombatSystem& combat, WaveManager& waveMgr,
    Level& level, int& health, int& currentWave, bool& waveStarted,
    RectangleShape grid[ROWS][COLS], int& selectedTowerIndex,
    UpgradeButton& upgradeBtn)
{
    health = 100; currentWave = 1; waveStarted = false;
    selectedTowerIndex = -1; upgradeBtn.visible = false;

    economy.resetCoins();
    combat.clearAll();
    level.reset();

    clearTowerTargets();   // clear attacker targets before new game

    waveMgr.setLevel(1);
    waveMgr.currentWave = 0;
    waveMgr.allDone = false;
    waveMgr.waveRunning = false;

    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            grid[i][j].setFillColor(Color::Black);

    for (int i = 0; i < pathSize; i++)
        grid[path[i].first][path[i].second].setFillColor(Color(120, 120, 120));

    registerHousesAsBases();
}


void saveCurrentGame(CoinSystem& economy, CombatSystem& combat, WaveManager& waveMgr,
    int health, const string& playerName, int currentWave,
    RectangleShape grid[ROWS][COLS])
{
    SaveData sd;
    sd.health = health; sd.coins = economy.getCoins(); sd.score = economy.getScore();
    sd.playerName = playerName; sd.currentWave = currentWave;
    sd.towerCount = combat.getTowerCount();

    for (int i = 0; i < combat.getTowerCount() && i < 50; i++) {
        Tower* t = combat.getTower(i);
        if (!t) continue;
        sd.towerX[i] = t->getX(); sd.towerY[i] = t->getY();
        sd.towerTypes[i] = t->getType();
        sd.towerUpgradeLevels[i] = t->getUpgradeLevel();
        sd.towerDamages[i] = t->getDamage();
        sd.towerRanges[i] = t->getRange();
    }
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            sd.towerGrid[i][j] = (grid[i][j].getFillColor() == Color::Transparent);

    sd.enemiesRemaining = waveMgr.getRemainingEnemies();
    sd.enemiesSpawned = waveMgr.getEnemiesLeftToSpawn();

    SaveLoad saver;
    saver.saveGame(sd, "savegame.txt");
}

void drawBaseTower(RenderWindow& window) {
    float baseX = OFFSET_X + path[pathSize - 1].second * CELL - 10;
    float baseY = OFFSET_Y + path[pathSize - 1].first * CELL - 35;

    RectangleShape towerBody(Vector2f(60, 90));
    towerBody.setPosition(baseX, baseY);
    towerBody.setFillColor(Color(110, 110, 110));
    towerBody.setOutlineThickness(3); towerBody.setOutlineColor(Color::Black);
    window.draw(towerBody);

    RectangleShape leftWall(Vector2f(18, 70));
    leftWall.setPosition(baseX - 12, baseY + 20);
    leftWall.setFillColor(Color(90, 90, 90));
    leftWall.setOutlineThickness(2); leftWall.setOutlineColor(Color::Black);
    window.draw(leftWall);

    RectangleShape rightWall(Vector2f(18, 70));
    rightWall.setPosition(baseX + 54, baseY + 20);
    rightWall.setFillColor(Color(90, 90, 90));
    rightWall.setOutlineThickness(2); rightWall.setOutlineColor(Color::Black);
    window.draw(rightWall);

    ConvexShape roof; roof.setPointCount(3);
    roof.setPoint(0, Vector2f(baseX - 10, baseY));
    roof.setPoint(1, Vector2f(baseX + 30, baseY - 40));
    roof.setPoint(2, Vector2f(baseX + 70, baseY));
    roof.setFillColor(Color(120, 40, 140));
    roof.setOutlineThickness(2); roof.setOutlineColor(Color::Black);
    window.draw(roof);

    RectangleShape door(Vector2f(18, 28));
    door.setPosition(baseX + 21, baseY + 62);
    door.setFillColor(Color(80, 40, 0));
    door.setOutlineThickness(2); door.setOutlineColor(Color::Black);
    window.draw(door);

    CircleShape doorTop(9);
    doorTop.setPosition(baseX + 21, baseY + 53);
    doorTop.setFillColor(Color(80, 40, 0));
    doorTop.setOutlineThickness(2); doorTop.setOutlineColor(Color::Black);
    window.draw(doorTop);

    RectangleShape w1(Vector2f(10, 14)); w1.setPosition(baseX + 10, baseY + 28);
    w1.setFillColor(Color::Yellow); w1.setOutlineThickness(1); w1.setOutlineColor(Color::Black);
    window.draw(w1);

    RectangleShape w2(Vector2f(10, 14)); w2.setPosition(baseX + 40, baseY + 28);
    w2.setFillColor(Color::Yellow); w2.setOutlineThickness(1); w2.setOutlineColor(Color::Black);
    window.draw(w2);

    RectangleShape pole(Vector2f(3, 30));
    pole.setPosition(baseX + 30, baseY - 55);
    pole.setFillColor(Color::Black); window.draw(pole);

    ConvexShape flag; flag.setPointCount(3);
    flag.setPoint(0, Vector2f(baseX + 33, baseY - 55));
    flag.setPoint(1, Vector2f(baseX + 55, baseY - 48));
    flag.setPoint(2, Vector2f(baseX + 33, baseY - 40));
    flag.setFillColor(Color::Red); window.draw(flag);

    for (int i = 0; i < 6; i++) {
        RectangleShape line(Vector2f(60, 1));
        line.setPosition(baseX, baseY + 12 + i * 12);
        line.setFillColor(Color(70, 70, 70)); window.draw(line);
    }
}

int main() {
    RenderWindow window(VideoMode(1200, 700), "Tower Defense");
    window.setFramerateLimit(60);

    sf::Music music;
    if (music.openFromFile("bgmusic.mpeg")) {
        music.setLoop(true);
        music.setVolume(50);  //volume 0 to 100
        music.play();
    }


    GameState   state = MENU;
    Level       level;
    Settings    settings;
    CoinSystem  economy;
    CombatSystem combat;
    WaveManager  waveMgr;
    HighScoreManager highScores;
    SaveLoad    saveLoad;
    PauseManager pauseMgr;

    int    health = 100;
    string playerName = "";
    string selectedTower = "Cannon";
    Clock  clock;
    float  deltaTime = 0;
    bool   waveStarted = false;
    int    currentWave = 1;
    float  transitionTimer = 0;
    int    nextWaveNumber = 1;
    int    selectedTowerIndex = -1;
    float  saveConfirmTimer = 0;

    UpgradeButton upgradeBtn;
    SettingsIcon  settingsIcon;
    Text saveConfirmText;

    setupWaypoints();    // set path
    registerHousesAsBases(); 

    Font font;
    if (!font.loadFromFile("ARCADE_N.ttf"))
        cout << "Font not loaded!\n";

    //UI 
    Text title("TOWER DEFENSE", font, 50);
    title.setFillColor(Color::Yellow);
    { FloatRect r = title.getLocalBounds(); title.setOrigin(r.width / 2, r.height / 2); }
    title.setPosition(600, 80);

    Button startBtn, loadBtn, exitBtn;
    startBtn.setup(font, "START NEW GAME", 490, 200, 14);
    loadBtn.setup(font, "LOAD GAME", 490, 280);
    exitBtn.setup(font, "EXIT", 490, 360);

    Button restartBtn, saveBtn, resumeBtn, settingsExitBtn;
    restartBtn.setup(font, "RESTART", 490, 180);
    saveBtn.setup(font, "SAVE GAME", 490, 260);
    resumeBtn.setup(font, "RESUME", 490, 340);
    settingsExitBtn.setup(font, "EXIT TO MENU", 490, 420, 16);

    settingsIcon.setup(font, (float)window.getSize().x - 50, 20);

    saveConfirmText.setFont(font);
    saveConfirmText.setCharacterSize(16);
    saveConfirmText.setFillColor(Color::Green);
    saveConfirmText.setPosition((float)SIDEBAR_X, 420);

    TowerButton cannonBtn, rapidBtn, iceBtn;
    cannonBtn.setup(font, "Cannon", 150, (float)SIDEBAR_X, 410);
    rapidBtn.setup(font, "Rapid", 120, (float)SIDEBAR_X, 470);
    iceBtn.setup(font, "Ice", 130, (float)SIDEBAR_X, 530);

    upgradeBtn.setup(font, (float)SIDEBAR_X, 600);

    HighScoreEntry topScores[3];
    int scoreCount = highScores.getTopScores(topScores, 3);

    Text highScoreTitle("HIGH SCORES", font, 24);
    highScoreTitle.setFillColor(Color::Yellow);
    {
        FloatRect r = highScoreTitle.getLocalBounds();
        highScoreTitle.setOrigin(r.width / 2, r.height / 2);
        highScoreTitle.setPosition(600, 480);
    }

    Text scoreTexts[3];
    for (int i = 0; i < 3; i++) {
        scoreTexts[i].setFont(font); scoreTexts[i].setCharacterSize(18);
        scoreTexts[i].setFillColor(Color::White);
        scoreTexts[i].setPosition(470, 520 + i * 30);
    }

    Text inputText("", font, 30); inputText.setFillColor(Color::White);
    Text prompt("ENTER YOUR NAME:", font, 30); prompt.setFillColor(Color::Cyan);
    { FloatRect r = prompt.getLocalBounds(); prompt.setOrigin(r.width / 2, r.height / 2); prompt.setPosition(600, 250); }

    //grid 
    RectangleShape grid[ROWS][COLS];
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++) {
            grid[i][j].setSize(Vector2f(CELL - 2, CELL - 2));
            grid[i][j].setPosition(OFFSET_X + j * CELL, OFFSET_Y + i * CELL);
            grid[i][j].setFillColor(Color::Black);
            grid[i][j].setOutlineThickness(1);
            grid[i][j].setOutlineColor(Color(80, 80, 80));
        }
    for (int i = 0; i < pathSize; i++)
        grid[path[i].first][path[i].second].setFillColor(Color(0, 180, 0));

    //houses shapes
    RectangleShape houses[HOUSE_COUNT];
    RectangleShape houseRoofs[HOUSE_COUNT];
    RectangleShape chimneys[HOUSE_COUNT];

    for (int i = 0; i < HOUSE_COUNT; i++) {
        int r = housePos[i][0], c = housePos[i][1];
        houses[i].setSize(Vector2f(26, 26));
        houses[i].setPosition(OFFSET_X + c * CELL + 7, OFFSET_Y + r * CELL + 10);
        houses[i].setFillColor(Color(181, 101, 29));
        houses[i].setOutlineThickness(2); houses[i].setOutlineColor(Color::Black);

        houseRoofs[i].setSize(Vector2f(34, 10));
        houseRoofs[i].setPosition(OFFSET_X + c * CELL + 3, OFFSET_Y + r * CELL + 2);
        houseRoofs[i].setFillColor(Color(170, 0, 0));

        chimneys[i].setSize(Vector2f(6, 14));
        chimneys[i].setPosition(OFFSET_X + c * CELL + 25, OFFSET_Y + r * CELL);
        chimneys[i].setFillColor(Color(80, 80, 80));
    }

    //hud shapes
    RectangleShape healthBarBg(Vector2f((float)(COLS * CELL), 25));
    healthBarBg.setPosition((float)OFFSET_X, 50);
    healthBarBg.setFillColor(Color(50, 0, 0));
    healthBarBg.setOutlineThickness(2); healthBarBg.setOutlineColor(Color::Red);

    RectangleShape healthBar(Vector2f((float)(COLS * CELL), 25));
    healthBar.setPosition((float)OFFSET_X, 50);
    healthBar.setFillColor(Color::Red);

    Text healthText("HP: 100", font, 18); healthText.setFillColor(Color::White);

    Text moneyText("", font, 20);       moneyText.setFillColor(Color::Yellow);
    Text scoreDisplay("", font, 20);    scoreDisplay.setFillColor(Color::White);
    Text waveText("", font, 20);        waveText.setFillColor(Color::Cyan);
    Text levelDisplayText("", font, 20); levelDisplayText.setFillColor(Color::Green);
    Text enemyText("", font, 20);       enemyText.setFillColor(Color::Red);
    Text towerSelect("Selected: Cannon", font, 16);
    Text upgradePrompt("Right-click tower to upgrade", font, 14);
    Text selectedTowerInfo("", font, 14);

    moneyText.setPosition((float)SIDEBAR_X, 120);
    scoreDisplay.setPosition((float)SIDEBAR_X, 160);
    waveText.setPosition((float)SIDEBAR_X, 200);
    levelDisplayText.setPosition((float)SIDEBAR_X, 240);
    enemyText.setPosition((float)SIDEBAR_X, 280);
    towerSelect.setPosition((float)SIDEBAR_X, 350);
    upgradePrompt.setPosition((float)SIDEBAR_X, 370);
    selectedTowerInfo.setPosition((float)SIDEBAR_X, 390);

    Text overText("GAME OVER", font, 50); overText.setFillColor(Color::Red);
    { FloatRect r = overText.getLocalBounds(); overText.setOrigin(r.width / 2, r.height / 2); overText.setPosition(600, 250); }

    Text winText("YOU WIN!", font, 50); winText.setFillColor(Color::Green);
    { FloatRect r = winText.getLocalBounds(); winText.setOrigin(r.width / 2, r.height / 2); winText.setPosition(600, 250); }

    Text settingsTitle("SETTINGS", font, 45); settingsTitle.setFillColor(Color::Cyan);
    { FloatRect r = settingsTitle.getLocalBounds(); settingsTitle.setOrigin(r.width / 2, r.height / 2); settingsTitle.setPosition(600, 100); }

    Text transitionText("", font, 45); transitionText.setFillColor(Color::Yellow);
    transitionText.setPosition(600, 300);

    Text instruction("Press ENTER to start", font, 20); instruction.setFillColor(Color(150, 150, 150));
    { FloatRect r = instruction.getLocalBounds(); instruction.setOrigin(r.width / 2, r.height / 2); instruction.setPosition(600, 350); }

    Text restartPrompt("Press R to Restart", font, 24); restartPrompt.setFillColor(Color::Yellow);
    { FloatRect r = restartPrompt.getLocalBounds(); restartPrompt.setOrigin(r.width / 2, r.height / 2); restartPrompt.setPosition(600, 400); }

    //main loop
    while (window.isOpen()) {
        deltaTime = clock.restart().asSeconds();
        if (deltaTime > 0.033f) deltaTime = 0.033f;

        if (saveConfirmTimer > 0) {
            saveConfirmTimer -= deltaTime;
            if (saveConfirmTimer <= 0) saveConfirmText.setString("");
        }

        if (state == LEVEL_TRANSITION) {
            transitionTimer -= deltaTime;
            if (transitionTimer <= 0) {
                state = PLAYING;
                waveMgr.startNextWave();
                waveStarted = true;
            }
        }

        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
                window.close();

            if ((state == GAME_OVER || state == WIN) &&
                event.type == Event::KeyPressed && event.key.code == Keyboard::R) {
                resetGame(economy, combat, waveMgr, level, health, currentWave,
                    waveStarted, grid, selectedTowerIndex, upgradeBtn);
                waveStarted = true;
                waveMgr.startNextWave();
                state = PLAYING;
            }

            if (state == MENU) {
                if (startBtn.clicked(event, window)) { state = NAME_INPUT; playerName = ""; }

                if (loadBtn.clicked(event, window)) {
                    SaveData ld;
                    if (saveLoad.loadGame(ld, "savegame.txt")) {
                        health = ld.health; playerName = ld.playerName;
                        economy.setCoins(ld.coins); currentWave = ld.currentWave;
                        combat.clearAll(); clearTowerTargets();
                        for (int i = 0; i < ld.towerCount && i < 50; i++) {
                            Tower* t = nullptr;
                            if (ld.towerTypes[i] == "Cannon") t = new CannonTower(ld.towerX[i], ld.towerY[i]);
                            else if (ld.towerTypes[i] == "Rapid")  t = new RapidTower(ld.towerX[i], ld.towerY[i]);
                            else if (ld.towerTypes[i] == "Ice")    t = new IceTower(ld.towerX[i], ld.towerY[i]);
                            if (t) {
                                for (int u = 0; u < ld.towerUpgradeLevels[i]; u++) t->upgrade();
                                combat.addTower(t);
                                registerTowerTarget(t->getX(), t->getY(),
                                    t->getHpPtr(), t->getDestroyedPtr());
                            }
                        }
                        for (int i = 0; i < ROWS; i++)
                            for (int j = 0; j < COLS; j++) {
                                if (ld.towerGrid[i][j]) grid[i][j].setFillColor(Color::Transparent);
                                else grid[i][j].setFillColor(isOnPath(i, j) ? Color(0, 180, 0) : Color::Black);
                            }
                        registerHousesAsBases();
                        waveMgr.setLevel(currentWave);
                        waveMgr.startNextWave();
                        waveStarted = true; state = PLAYING;
                    }
                }
                if (exitBtn.clicked(event, window)) window.close();
            }
            else if (state == NAME_INPUT) {
                if (event.type == Event::TextEntered) {
                    if (event.text.unicode == '\b' && !playerName.empty()) playerName.pop_back();
                    else if ((event.text.unicode == '\r' || event.text.unicode == '\n') && !playerName.empty()) {
                        resetGame(economy, combat, waveMgr, level, health, currentWave,
                            waveStarted, grid, selectedTowerIndex, upgradeBtn);
                        state = LEVEL_TRANSITION; transitionTimer = 2.f;
                        transitionText.setString("WAVE 1");
                        nextWaveNumber = 1; waveMgr.currentWave = 0;
                    }
                    else if (event.text.unicode < 128 && playerName.length() < 20)
                        playerName += (char)event.text.unicode;
                }
            }
            else if (state == SETTINGS_MENU) {
                if (restartBtn.clicked(event, window)) {
                    resetGame(economy, combat, waveMgr, level, health, currentWave,
                        waveStarted, grid, selectedTowerIndex, upgradeBtn);
                    state = LEVEL_TRANSITION; transitionTimer = 2.f;
                    transitionText.setString("RESTARTING...");
                }
                else if (saveBtn.clicked(event, window)) {
                    saveCurrentGame(economy, combat, waveMgr, health, playerName, currentWave, grid);
                    saveConfirmTimer = 2.f; saveConfirmText.setString("Game Saved!");
                }
                else if (resumeBtn.clicked(event, window))       state = PLAYING;
                else if (settingsExitBtn.clicked(event, window)) state = MENU;
            }
            else if (state == PLAYING && !pauseMgr.isPaused()) {
                settingsIcon.update(window);
                if (settingsIcon.clicked(event, window)) state = SETTINGS_MENU;

                float mx = (float)Mouse::getPosition(window).x;
                float my = (float)Mouse::getPosition(window).y;

                if (cannonBtn.contains(mx, my) && event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
                {
                    selectedTower = "Cannon"; towerSelect.setString("Selected: Cannon ($150)");
                }
                if (rapidBtn.contains(mx, my) && event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
                {
                    selectedTower = "Rapid";  towerSelect.setString("Selected: Rapid ($120)");
                }
                if (iceBtn.contains(mx, my) && event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
                {
                    selectedTower = "Ice";    towerSelect.setString("Selected: Ice ($130)");
                }

                if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                    int col = ((int)event.mouseButton.x - OFFSET_X) / CELL;
                    int row = ((int)event.mouseButton.y - OFFSET_Y) / CELL;

                    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
                        bool occupied = (grid[row][col].getFillColor() == Color::Transparent);
                        if (!isOnPath(row, col) && !occupied) {
                            float tx = OFFSET_X + col * CELL + CELL / 2.f;
                            float ty = OFFSET_Y + row * CELL + CELL / 2.f;
                            Tower* newTower = nullptr;
                            int    cost = 0;
                            if (selectedTower == "Cannon") { newTower = new CannonTower(tx, ty); cost = 150; }
                            else if (selectedTower == "Rapid") { newTower = new RapidTower(tx, ty); cost = 120; }
                            else if (selectedTower == "Ice") { newTower = new IceTower(tx, ty); cost = 130; }

                            if (newTower && economy.getCoins() >= cost) {
                                economy.deductCoins(cost);
                                combat.addTower(newTower);
                                grid[row][col].setFillColor(Color::Transparent);
                               
                                registerTowerTarget(newTower->getX(), newTower->getY(),    // register so TowerAttacker enemies can find and hit it
                                    newTower->getHpPtr(),
                                    newTower->getDestroyedPtr());
                            }
                            else { delete newTower; }
                        }
                    }

                    if (upgradeBtn.contains(mx, my) &&
                        selectedTowerIndex >= 0 && selectedTowerIndex < combat.getTowerCount()) {
                        Tower* t = combat.getTower(selectedTowerIndex);
                        if (t && economy.purchaseUpgrade(t))
                            upgradeBtn.updateForTower(t, selectedTowerIndex, economy.getCoins());
                    }
                }

                if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
                    int col = ((int)event.mouseButton.x - OFFSET_X) / CELL;
                    int row = ((int)event.mouseButton.y - OFFSET_Y) / CELL;
                    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
                        bool found = false;
                        for (int i = 0; i < combat.getTowerCount(); i++) {
                            Tower* t = combat.getTower(i);
                            if (!t) continue;
                            int tc = ((int)t->getX() - OFFSET_X) / CELL;
                            int tr = ((int)t->getY() - OFFSET_Y) / CELL;
                            if (tr == row && tc == col) {
                                selectedTowerIndex = i; found = true;
                                upgradeBtn.updateForTower(t, i, economy.getCoins());
                                selectedTowerInfo.setString("Selected: " + t->getType() +
                                    " (Lvl " + to_string(t->getUpgradeLevel()) + ")");
                                break;
                            }
                        }
                        if (!found) { selectedTowerIndex = -1; upgradeBtn.visible = false; selectedTowerInfo.setString(""); }
                    }
                }
                if (event.type == Event::KeyPressed && event.key.code == Keyboard::P)
                    pauseMgr.togglePause();
            }
        }

        //update
        if (state == PLAYING && !pauseMgr.isPaused()) {
            if (waveStarted && !waveMgr.allDone) {
                int earned = waveMgr.update(deltaTime, health);
                if (earned > 0) economy.addCoins(earned);

                if (waveMgr.isWaveComplete() && waveMgr.waveRunning) {

                    waveMgr.waveRunning = false;
                    currentWave++;

                    if ((currentWave - 1) % WAVES_PER_LEVEL == 0) {
                        level.nextLevel();
                        transitionText.setString("LEVEL " + to_string(level.getLevel()));
                    }
                    else {
                        int displayWave = ((currentWave - 1) % WAVES_PER_LEVEL) + 1;
                        transitionText.setString("WAVE " + to_string(displayWave));
                    }

                    state = LEVEL_TRANSITION;
                    transitionTimer = 2.f;

                    FloatRect r = transitionText.getLocalBounds();
                    transitionText.setOrigin(r.width / 2, r.height / 2);
                }
            }

            combat.updateCooldowns(deltaTime);
            combat.removeDestroyedTowers();   // removes dead towers

            for (int i = 0; i < combat.getTowerCount(); i++) {
                Tower* t = combat.getTower(i);
                if (t && t->canAttack()) {
                    Enemy* inRange[100]; int count = 0;
                    count = waveMgr.getEnemiesInRange(t->getX(), t->getY(), t->getRange(), inRange, 100);
                    if (count > 0) t->attack(inRange, count);
                }
            }

            if (selectedTowerIndex >= 0) {
                Tower* t = combat.getTower(selectedTowerIndex);
                if (t) upgradeBtn.updateForTower(t, selectedTowerIndex, economy.getCoins());
                else { selectedTowerIndex = -1; upgradeBtn.visible = false; }
            }

            // health cannot be negative 
            if (health < 0) health = 0;

            if (health <= 0) { state = GAME_OVER; highScores.addScore(playerName, economy.getScore()); }
            if (waveMgr.allDone && health > 0) { state = WIN; highScores.addScore(playerName, economy.getScore()); }

            float hpRatio = (float)health / 100.f;
            if (hpRatio > 1.f) hpRatio = 1.f;
            if (hpRatio < 0.f) hpRatio = 0.f;
            healthBar.setSize(Vector2f(hpRatio * COLS * CELL, 25));
            healthText.setString("HP: " + to_string(health));
            {
                FloatRect b = healthText.getLocalBounds();
                healthText.setPosition(OFFSET_X + (COLS * CELL) / 2 - b.width / 2, 65 - 12);
            }

            moneyText.setString("Gold: $" + to_string(economy.getCoins()));
            scoreDisplay.setString("Score: " + to_string(economy.getScore()));
            int displayWave = ((currentWave - 1) % WAVES_PER_LEVEL) + 1;
            waveText.setString("Wave: " + to_string(displayWave));

            levelDisplayText.setString("Level: " + to_string(level.getLevel()));
            enemyText.setString("Enemies: " + to_string(waveMgr.getRemainingEnemies()));
        }

        //draw
        window.clear(Color(20, 20, 25));

        if (state == MENU) {
            window.draw(title); window.draw(highScoreTitle);
            for (int i = 0; i < scoreCount; i++) {
                scoreTexts[i].setString(to_string(i + 1) + ". " + topScores[i].name + " - " + to_string(topScores[i].score));
                window.draw(scoreTexts[i]);
            }
            startBtn.draw(window); loadBtn.draw(window); exitBtn.draw(window);
        }
        else if (state == NAME_INPUT) {
            inputText.setString(playerName + "_");
            inputText.setPosition(600 - inputText.getLocalBounds().width / 2, 300);
            window.draw(prompt); window.draw(inputText); window.draw(instruction);
        }
        else if (state == SETTINGS_MENU) {
            window.draw(settingsTitle);
            restartBtn.draw(window); saveBtn.draw(window);
            resumeBtn.draw(window);  settingsExitBtn.draw(window);
            window.draw(saveConfirmText);
        }
        else if (state == LEVEL_TRANSITION || state == PLAYING) {
            drawPath(window);
            window.draw(healthBarBg); window.draw(healthBar); window.draw(healthText);
            for (int i = 0; i < ROWS; i++) for (int j = 0; j < COLS; j++) window.draw(grid[i][j]);

            // Draw houses
            for (int i = 0; i < HOUSE_COUNT; i++) {
                bool destroyed = (i < baseCount && bases[i].destroyed);
                if (!destroyed) {
                    window.draw(houses[i]);
                    window.draw(houseRoofs[i]);
                    window.draw(chimneys[i]);
                }
            }
            // draws the hp bars and red flash when attacked
            drawBases(window);

            drawBaseTower(window);

            for (int i = 0; i < combat.getTowerCount(); i++) {
                Tower* t = combat.getTower(i);
                if (!t) continue;
                t->draw(window);
                if (i == selectedTowerIndex) {
                    CircleShape hl(20);
                    hl.setPosition(t->getX(), t->getY()); hl.setOrigin(20, 20);
                    hl.setFillColor(Color::Transparent);
                    hl.setOutlineThickness(3); hl.setOutlineColor(Color(255, 200, 0));
                    window.draw(hl);
                }
            }

            waveMgr.drawAllEnemies(window);
            settingsIcon.draw(window);

            window.draw(moneyText); window.draw(scoreDisplay); window.draw(waveText);
            window.draw(levelDisplayText); window.draw(enemyText);
            window.draw(towerSelect); window.draw(upgradePrompt); window.draw(selectedTowerInfo);
            cannonBtn.draw(window); rapidBtn.draw(window); iceBtn.draw(window);
            upgradeBtn.draw(window);

            if (state == LEVEL_TRANSITION) {
                RectangleShape ov(Vector2f(1200, 700)); ov.setFillColor(Color(0, 0, 0, 180));
                window.draw(ov); window.draw(transitionText);
            }
            if (pauseMgr.isPaused() && state == PLAYING) {
                Text pt("PAUSED", font, 50); pt.setFillColor(Color::Yellow);
                FloatRect r = pt.getLocalBounds(); pt.setOrigin(r.width / 2, r.height / 2);
                pt.setPosition(600, 300); window.draw(pt);
            }
        }
        else if (state == GAME_OVER || state == WIN) {
            window.draw(state == GAME_OVER ? overText : winText);
            Text fs("Final Score: " + to_string(economy.getScore()), font, 28);
            fs.setFillColor(Color::White);
            FloatRect r = fs.getLocalBounds(); fs.setOrigin(r.width / 2, r.height / 2);
            fs.setPosition(600, 320); window.draw(fs); window.draw(restartPrompt);
        }

        window.display();
    }
    return 0;
}