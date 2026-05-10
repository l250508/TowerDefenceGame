#pragma once
#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>
#include <iostream>
using namespace std;

const int MAX_ENEMIES = 60;
const int MAX_WP = 30;
const int MAX_BASES = 6;

extern sf::Vector2f waypoints[MAX_WP];
extern int          waypointCount;

struct Base
{
    float x, y;
    int   hp;
    int   maxHp;
    int   damageToPlayer;   // player hp is lost when this base is destroyed
    bool  destroyed;
    float attackFlashTimer; // flashes red when being attacked

    Base() {
        x = y = 0;
        hp = maxHp = 60;
        damageToPlayer = 5;
        destroyed = false;
        attackFlashTimer = 0.f;
    }
};

extern Base bases[MAX_BASES];
extern int  baseCount;

void addBase(float x, float y, int hp, int dmgToPlayer);    //adds bases
void clearBases();

void drawBases(sf::RenderWindow& win);

class Enemy {
private:
    float x, y;
    float speed;
    int   hp, maxHp;
    int   wpIndex;
    int   id;
    int   reward;
    bool  reachedEnd;
    bool  dead;

    float flashTimer;       //animation timers
    float deathTimer;
    bool  isDying;
    float deathScale;
    float bobTimer;

    static int nextId;

    void drawHpBar(sf::RenderWindow& win, float barWidth, float yAbove);

protected:
    void moveAlongPath(float dt);
    void drawBody(sf::RenderWindow& win, float size, sf::Color col);
    void drawBar(sf::RenderWindow& win, float barWidth, float yAbove);
    void updateAnimations(float dt);

    void setHp(int h);
    void setSpeed(float s);
    void setReward(int r);
    void setPos(float px, float py);

    bool  getIsDying() { return isDying; }
    float getDeathScale() { return deathScale; }
    float getFlashTimer() { return flashTimer; }
    float getBobTimer() { return bobTimer; }
    int   getWpIndex() { return wpIndex; }
    float getSpeed() { return speed; }

public:
    Enemy();
    virtual ~Enemy() {}

    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& w) = 0;
    virtual string getTypeName() = 0;

    void takeDamage(int dmg);

    bool isAlive();
    bool hasReachedEnd();
    bool isDead();
    bool isDeathAnimDone();

    float getX();
    float getY();
    int   getId();
    int   getReward();
    int   getHp();
    int   getMaxHp();
};

void clearPath();
void addWaypoint(float x, float y);
sf::Vector2f getStartPoint();
void drawPath(sf::RenderWindow& win);

void registerTowerTarget(float x, float y, int* hp, bool* destroyed);
void clearTowerTargets();

//0=Normal  1=Faster  2=Tank  3=TowerAttacker
Enemy* createEnemy(int type, float sx, float sy, float sm, float hm);

class AdaptiveDifficulty {
private:
    float hpMult;
    float speedMult;
    int   wavesSinceIncrease;

public:
    AdaptiveDifficulty();
    float getHpMult();
    float getSpeedMult();
    void  recordResult(int killed, int leaked, int waveNum);
    void  reset();
};

template <typename DiffType>
struct WaveData
{
    int      waveNumber;
    int      enemyTypes[MAX_ENEMIES];
    int      enemyCount;
    float    spawnDelay;
    DiffType hpMult;
    DiffType speedMult;
};

class WaveGenerator {
public:
    WaveGenerator();
    WaveData<float> generate(int waveNum, int level,
        float hpMult, float speedMult);
};

class WaveManager {
private:
    WaveGenerator   generator;
    WaveData<float> activeWave;
    float           spawnTimer;
    int             spawnIndex;
    int             killsThisWave;
    int             leaksThisWave;

    Enemy* enemies[MAX_ENEMIES];
    int    enemyCount;

    void spawnOne();
    void clearAllEnemies();

public:
    int  currentWave;
    int  totalWaves;
    bool waveRunning;
    bool allDone;
    int  currentLevel;

    AdaptiveDifficulty difficulty;

    WaveManager();
    ~WaveManager();

    void setLevel(int lvl);

    void startNextWave();
    int  update(float dt, int& playerHealth);   // returns coins
    bool isWaveComplete();
    void reset();

    int  getEnemiesInRange(float tx, float ty, float range,
        Enemy* out[], int maxOut);
    void damageEnemy(int enemyId, int dmg);

    void drawAllEnemies(sf::RenderWindow& win);
    int  getRemainingEnemies();
    int  getEnemiesLeftToSpawn();
};

#endif