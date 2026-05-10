#include <string>
#include <SFML/Graphics.hpp>
#include <cmath>
#include "enemy.h"
using namespace std;
using namespace sf;

const int MAX_TOWERS = 50;

//tower base class
class Tower {
protected:
    float x, y;
    int damage;
    float range;
    float attackCooldown;
    float currentCooldown;
    int cost;
    int upgradeLevel;
    CircleShape shape;

    int currentHp;
    int maxHp;
    bool destroyed;

public:
    Tower(float posX, float posY) : x(posX), y(posY), currentCooldown(0), upgradeLevel(0) {
        range = 100.0f;
        attackCooldown = 1.0f;
        damage = 10;
        cost = 100;
        maxHp = 100;
        currentHp = maxHp;
        destroyed = false;
        shape.setRadius(15);
        shape.setFillColor(Color::Transparent);
        shape.setOrigin(15, 15);
    }
    virtual ~Tower() {}

    //getters
    float getX() const { return x; }
    float getY() const { return y; }
    float getRange() const { return range; }
    int getDamage() const { return damage; }
    int getCost() const { return cost; }
    float getAttackCooldown() const { return attackCooldown; }

    //hp
    int* getHpPtr() { return &currentHp; }
    bool* getDestroyedPtr() { return &destroyed; }
    int   getCurrentHp()  const { return currentHp; }
    int   getMaxHp()      const { return maxHp; }
    bool  isDestroyed()   const { return destroyed; }

    void takeDamage(int amount) {
        if (destroyed) return;
        currentHp -= amount;
        if (currentHp <= 0) { currentHp = 0; destroyed = true; }
    }

    void heal(int amount) {
        if (destroyed) return;
        currentHp += amount;
        if (currentHp > maxHp) currentHp = maxHp;
    }

    void setMaxHp(int hp) { maxHp = hp; currentHp = maxHp; }
    void setPosition(float newX, float newY) { x = newX; y = newY; }
    void setRange(float newRange) { range = newRange; }
    void setDamage(int newDamage) { damage = newDamage; }

    virtual void attack(Enemy* enemies[], int enemyCount) = 0;
    virtual string getType() const = 0;
    virtual void draw(RenderWindow& window) = 0;

    void upgrade() {
        if (canUpgrade()) {
            if (upgradeLevel == 0) damage += 10;
            else if (upgradeLevel == 1) range += 25;
            else if (upgradeLevel == 2) applySpecialUpgrade();
            upgradeLevel++;
        }
    }

    bool   canUpgrade()   const { return upgradeLevel < 3 && !destroyed; }
    int    getUpgradeCost() const { return 100 + (upgradeLevel * 50); }
    int    getUpgradeLevel() const { return upgradeLevel; }

    string getNextUpgradeName() const {
        if (upgradeLevel == 0) return "Damage +10";
        if (upgradeLevel == 1) return "Range +25";
        if (upgradeLevel == 2) return "Special Upgrade";
        return "Fully upgraded";
    }
    virtual void applySpecialUpgrade() {}

    void updateCooldown(float dt) {
        if (currentCooldown > 0) {
            currentCooldown -= dt;
            if (currentCooldown < 0) currentCooldown = 0;
        }
    }

    bool canAttack() const { return currentCooldown <= 0 && !destroyed; }

    bool isEnemyInRange(Enemy* enemy) {
        if (!enemy || destroyed) return false;
        float dx = enemy->getX() - x;
        float dy = enemy->getY() - y;
        return sqrt(dx * dx + dy * dy) <= range;
    }

    int findClosestEnemy(Enemy* enemies[], int enemyCount) {
        int   closestIndex = -1;
        float minDist = range;
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i] && enemies[i]->isAlive() && isEnemyInRange(enemies[i])) {
                float dx = enemies[i]->getX() - x;
                float dy = enemies[i]->getY() - y;
                float d = sqrt(dx * dx + dy * dy);
                if (d < minDist) { minDist = d; closestIndex = i; }
            }
        }
        return closestIndex;
    }

    void findAllEnemiesInRange(Enemy* enemies[], int enemyCount,
        Enemy* result[], int& resultCount) {
        resultCount = 0;
        for (int i = 0; i < enemyCount && resultCount < MAX_ENEMIES; i++) {
            if (enemies[i] && enemies[i]->isAlive() && isEnemyInRange(enemies[i]))
                result[resultCount++] = enemies[i];
        }
    }

    virtual string serialize() const {
        return getType() + "," + to_string(x) + "," + to_string(y) + "," +
            to_string(damage) + "," + to_string(range) + "," +
            to_string(upgradeLevel) + "," + to_string(currentHp) + "," +
            to_string(maxHp);
    }
};

//cannon
class CannonTower : public Tower {
private:
    float splashRadius;
public:
    CannonTower(float posX, float posY) : Tower(posX, posY) {
        damage = 30;
        attackCooldown = 1.5f;
        range = 120.0f;
        cost = 150;
        splashRadius = 40.0f;
        setMaxHp(150);
        shape.setFillColor(Color(255, 100, 100));
    }

    string getType() const override { return "Cannon"; }
    float getSplashRadius() const { return splashRadius; }

    void attack(Enemy* enemies[], int enemyCount) override {
        if (!canAttack()) return;
        int idx = findClosestEnemy(enemies, enemyCount);
        if (idx == -1) return;

        Enemy* target = enemies[idx];
        target->takeDamage(damage);
        currentCooldown = attackCooldown;

        float tx = target->getX(), ty = target->getY();
        for (int i = 0; i < enemyCount; i++) {
            if (!enemies[i] || enemies[i] == target || !enemies[i]->isAlive()) continue;
            float dx = enemies[i]->getX() - tx;
            float dy = enemies[i]->getY() - ty;
            if (sqrt(dx * dx + dy * dy) <= splashRadius)
                enemies[i]->takeDamage(damage / 2);
        }
    }

    void applySpecialUpgrade() override {
        splashRadius += 30;
        if (splashRadius > 120) splashRadius = 120;
    }

    void draw(RenderWindow& window) override {
        // wood base
        RectangleShape base(Vector2f(34, 10));
        base.setFillColor(Color(150, 75, 0));
        base.setOrigin(17, 5);
        base.setPosition(x, y + 10);
        window.draw(base);

        RectangleShape support(Vector2f(16, 8));
        support.setFillColor(Color(160, 82, 45));
        support.setOrigin(8, 4);
        support.setPosition(x - 2, y + 2);
        window.draw(support);

        // wheel
        CircleShape wheel(8);
        wheel.setFillColor(Color(180, 100, 20));
        wheel.setOrigin(8, 8);
        wheel.setPosition(x + 8, y + 10);
        window.draw(wheel);

        CircleShape wheelCenter(4);
        wheelCenter.setFillColor(Color(255, 170, 40));
        wheelCenter.setOrigin(4, 4);
        wheelCenter.setPosition(x + 8, y + 10);
        window.draw(wheelCenter);

        // barrel
        RectangleShape barrel(Vector2f(32, 12));
        barrel.setFillColor(Color(110, 110, 110));
        barrel.setOrigin(6, 6);
        barrel.setRotation(-18);
        barrel.setPosition(x - 4, y);
        window.draw(barrel);

        RectangleShape backRim(Vector2f(4, 14));
        backRim.setFillColor(Color(70, 70, 70));
        backRim.setOrigin(2, 7);
        backRim.setRotation(-18);
        backRim.setPosition(x - 14, y + 2);
        window.draw(backRim);

        CircleShape front(7);
        front.setFillColor(Color(90, 90, 90));
        front.setOrigin(7, 7);
        front.setPosition(x + 20, y - 8);
        window.draw(front);

        CircleShape hole(2);
        hole.setFillColor(Color::Black);
        hole.setOrigin(2, 2);
        hole.setPosition(x + 24, y - 8);
        window.draw(hole);

        // pivot
        CircleShape pivot(6);
        pivot.setFillColor(Color(255, 165, 0));
        pivot.setOrigin(6, 6);
        pivot.setPosition(x - 2, y + 1);
        window.draw(pivot);

        CircleShape pivotInner(3);
        pivotInner.setFillColor(Color(255, 200, 50));
        pivotInner.setOrigin(3, 3);
        pivotInner.setPosition(x - 2, y + 1);
        window.draw(pivotInner);

        // hp bar
        if (!destroyed && currentHp < maxHp) {
            RectangleShape hpBg(Vector2f(34, 5));
            hpBg.setFillColor(Color(50, 0, 0));
            hpBg.setPosition(x - 17, y - 25);
            window.draw(hpBg);

            RectangleShape hpBar(Vector2f(34 * ((float)currentHp / maxHp), 5));
            hpBar.setFillColor(Color::Red);
            hpBar.setPosition(x - 17, y - 25);
            window.draw(hpBar);
        }
    }
};

// RAPID TOWER
class RapidTower : public Tower {
private:
    int multiShotCount;
public:
    RapidTower(float posX, float posY) : Tower(posX, posY) {
        damage = 8;
        attackCooldown = 0.4f;
        range = 100.0f;
        cost = 120;
        multiShotCount = 1;
        setMaxHp(80);
        shape.setFillColor(Color(100, 255, 100));
    }

    string getType() const override { return "Rapid"; }

    void attack(Enemy* enemies[], int enemyCount) override {
        if (!canAttack()) return;
        Enemy* inRange[MAX_ENEMIES];
        int inRangeCount = 0;
        findAllEnemiesInRange(enemies, enemyCount, inRange, inRangeCount);
        int shots = (multiShotCount < inRangeCount) ? multiShotCount : inRangeCount;
        for (int i = 0; i < shots; i++)
            if (inRange[i]) inRange[i]->takeDamage(damage);
        if (shots > 0) currentCooldown = attackCooldown;
    }

    void applySpecialUpgrade() override {
        attackCooldown -= 0.1f;
        if (attackCooldown < 0.2f) attackCooldown = 0.2f;
        multiShotCount++;
        if (multiShotCount > 3) multiShotCount = 3;
    }

    void draw(RenderWindow& window) override {
        CircleShape base(14);
        base.setFillColor(Color(60, 60, 60));
        base.setOrigin(14, 14);
        base.setPosition(x, y);
        window.draw(base);

        CircleShape body(10);
        body.setFillColor(Color(0, 200, 100));
        body.setOrigin(10, 10);
        body.setPosition(x, y);
        window.draw(body);

        for (int i = 0; i < 3; i++) {
            RectangleShape barrel(Vector2f(18, 4));
            barrel.setFillColor(Color(120, 120, 120));
            barrel.setOrigin(0, 2);
            barrel.setPosition(x, y);
            float angle = i * 120.f;
            barrel.setRotation(angle);
            window.draw(barrel);

            CircleShape tip(3);
            tip.setFillColor(Color::Yellow);
            tip.setOrigin(3, 3);
            float rad = angle * 3.14159f / 180.f;
            tip.setPosition(x + cos(rad) * 22, y + sin(rad) * 22);
            window.draw(tip);
        }

        if (!destroyed && currentHp < maxHp) {
            RectangleShape hpBg(Vector2f(28, 4));
            hpBg.setFillColor(Color(50, 0, 0));
            hpBg.setPosition(x - 14, y - 20);
            window.draw(hpBg);

            RectangleShape hpBar(Vector2f(28 * ((float)currentHp / maxHp), 4));
            hpBar.setFillColor(Color::Red);
            hpBar.setPosition(x - 14, y - 20);
            window.draw(hpBar);
        }
    }
};

//ice
class IceTower : public Tower {
private:
    float slowFactor;
    float slowDuration;
public:
    IceTower(float posX, float posY) : Tower(posX, posY) {
        damage = 5;
        attackCooldown = 0.8f;
        range = 110.0f;
        cost = 130;
        slowFactor = 0.5f;
        slowDuration = 2.0f;
        setMaxHp(120);
        shape.setFillColor(Color(100, 100, 255));
    }

    string getType() const override { return "Ice"; }

    void attack(Enemy* enemies[], int enemyCount) override {
        if (!canAttack()) return;
        Enemy* inRange[MAX_ENEMIES];
        int inRangeCount = 0;
        findAllEnemiesInRange(enemies, enemyCount, inRange, inRangeCount);
        for (int i = 0; i < inRangeCount; i++)
            if (inRange[i]) inRange[i]->takeDamage(damage);
        if (inRangeCount > 0) currentCooldown = attackCooldown;
    }

    void applySpecialUpgrade() override {
        slowFactor -= 0.15f;
        if (slowFactor < 0.2f) slowFactor = 0.2f;
        slowDuration += 1.0f;
        if (slowDuration > 5.0f) slowDuration = 5.0f;
    }

    void draw(RenderWindow& window) override {
        CircleShape aura(18);
        aura.setFillColor(Color(180, 240, 255, 80));
        aura.setOrigin(18, 18);
        aura.setPosition(x, y);
        window.draw(aura);

        ConvexShape crystal;
        crystal.setPointCount(6);
        crystal.setPoint(0, Vector2f(x, y - 18));
        crystal.setPoint(1, Vector2f(x + 6, y - 6));
        crystal.setPoint(2, Vector2f(x + 10, y + 8));
        crystal.setPoint(3, Vector2f(x, y + 14));
        crystal.setPoint(4, Vector2f(x - 10, y + 8));
        crystal.setPoint(5, Vector2f(x - 6, y - 6));
        crystal.setFillColor(Color(140, 220, 255));
        window.draw(crystal);

        for (int i = 0; i < 3; i++) {
            float angle = i * 120.f * 3.14159f / 180.f;
            float dx = cos(angle), dy = sin(angle);
            ConvexShape shard;
            shard.setPointCount(3);
            shard.setPoint(0, Vector2f(x, y));
            shard.setPoint(1, Vector2f(x + dx * 14 + dy * 4, y + dy * 14 - dx * 4));
            shard.setPoint(2, Vector2f(x + dx * 20, y + dy * 20));
            shard.setFillColor(Color(210, 250, 255, 180));
            window.draw(shard);
        }

        CircleShape core(3);
        core.setFillColor(Color::White);
        core.setOrigin(3, 3);
        core.setPosition(x, y);
        window.draw(core);

        if (!destroyed && currentHp < maxHp) {
            RectangleShape hpBg(Vector2f(32, 4));
            hpBg.setFillColor(Color(50, 0, 0));
            hpBg.setPosition(x - 16, y - 24);
            window.draw(hpBg);

            RectangleShape hpBar(Vector2f(32 * ((float)currentHp / maxHp), 4));
            hpBar.setFillColor(Color::Red);
            hpBar.setPosition(x - 16, y - 24);
            window.draw(hpBar);
        }
    }
};

//coins system
class CoinSystem {
private:
    int totalCoins, spent, earned, totalKills, score;
    const int killreward = 50;
public:
    CoinSystem() : totalCoins(500), spent(0), earned(0), totalKills(0), score(0) {}

    void addCoins(int amount) { totalCoins += amount; earned += amount; updateScore(); }
    void setCoins(int amount) { totalCoins = amount;  updateScore(); }
    void setScore(int s) { score = s; }

    bool deductCoins(int amount) {
        if (totalCoins < amount) return false;
        totalCoins -= amount; spent += amount; updateScore(); return true;
    }

    void resetCoins() { totalCoins = 500; spent = 0; earned = 0; totalKills = 0; score = 0; }

    bool canAffordTower(Tower* t) { return totalCoins >= t->getCost(); }
    bool purchaseTower(Tower* t) { return canAffordTower(t) ? deductCoins(t->getCost()) : false; }
    bool canAffordUpgrade(Tower* t) { return t->canUpgrade() && totalCoins >= t->getUpgradeCost(); }

    bool purchaseUpgrade(Tower* t) {
        if (!canAffordUpgrade(t)) return false;
        deductCoins(t->getUpgradeCost()); t->upgrade(); return true;
    }

    void addKill() { totalKills++; addCoins(killreward); updateScore(); }

    int getCoins() const { return totalCoins; }
    int getKills() const { return totalKills; }
    int getScore() const { return score; }
    int getSpent() const { return spent; }
    int getTotalEarned() const { return earned; }

    void updateScore() { score = earned + (totalKills * 10); }

    string serialize() const {
        return to_string(totalCoins) + "," + to_string(spent) + "," +
            to_string(earned) + "," + to_string(totalKills) + "," + to_string(score);
    }
};

//combat system
class CombatSystem {
private:
    Tower* towers[MAX_TOWERS];
    int    towerCount;

public:
    CombatSystem() : towerCount(0) {
        for (int i = 0; i < MAX_TOWERS; i++) towers[i] = nullptr;
    }
    ~CombatSystem() { clearAll(); }

    Tower* getTower(int i)    const { return towers[i]; }
    int    getTowerCount()    const { return towerCount; }

    void addTower(Tower* t) {
        if (towerCount < MAX_TOWERS) towers[towerCount++] = t;
    }

    void deleteTower(int index) {
        if (index < 0 || index >= towerCount) return;
        delete towers[index];
        for (int i = index; i < towerCount - 1; i++) towers[i] = towers[i + 1];
        towers[--towerCount] = nullptr;
    }

    void removeDestroyedTowers() {
        bool anyRemoved = false;
        for (int i = towerCount - 1; i >= 0; i--) {
            if (towers[i] && towers[i]->isDestroyed()) {
                deleteTower(i);
                anyRemoved = true;
            }
        }
        if (anyRemoved) {
            clearTowerTargets();
            for (int i = 0; i < towerCount; i++) {
                if (towers[i] && !towers[i]->isDestroyed())
                    registerTowerTarget(towers[i]->getX(), towers[i]->getY(),
                        towers[i]->getHpPtr(),
                        towers[i]->getDestroyedPtr());
            }
        }
    }

    int getTowerIndexAt(float px, float py, float radius = 20.f) {
        for (int i = 0; i < towerCount; i++) {
            if (!towers[i] || towers[i]->isDestroyed()) continue;
            float dx = towers[i]->getX() - px;
            float dy = towers[i]->getY() - py;
            if (sqrt(dx * dx + dy * dy) <= radius) return i;
        }
        return -1;
    }

    void updateCooldowns(float dt) {
        for (int i = 0; i < towerCount; i++)
            if (towers[i]) towers[i]->updateCooldown(dt);
    }

    void clearAll() {
        for (int i = 0; i < towerCount; i++) { delete towers[i]; towers[i] = nullptr; }
        towerCount = 0;
    }
};