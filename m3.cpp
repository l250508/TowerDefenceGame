#include "Enemy.h"
#include <stdexcept>
using namespace std;


int Enemy::nextId = 0;
sf::Vector2f waypoints[MAX_WP];
int          waypointCount = 0;

void clearPath(){
    waypointCount = 0;
}

void addWaypoint(float x, float y){
    if (waypointCount < MAX_WP)
    {
        waypoints[waypointCount].x = x;
        waypoints[waypointCount].y = y;
        waypointCount++;
    }
}

sf::Vector2f getStartPoint(){
    if (waypointCount == 0) return sf::Vector2f(0, 0);
    return waypoints[0];
}

void drawPath(sf::RenderWindow& win)
{
    for (int i = 0; i + 1 < waypointCount; i++)
    {
        float x1 = waypoints[i].x, y1 = waypoints[i].y;
        float x2 = waypoints[i + 1].x, y2 = waypoints[i + 1].y;
        float dx = x2 - x1, dy = y2 - y1;
        float len = sqrt(dx * dx + dy * dy);
        float angle = atan2(dy, dx) * 180.f / 3.14159f;

        sf::RectangleShape seg(sf::Vector2f(len, 36));
        seg.setOrigin(0, 18);
        seg.setPosition(x1, y1);
        seg.setRotation(angle);
        seg.setFillColor(sf::Color(150, 110, 60));
        win.draw(seg);
    }
    for (int i = 0; i < waypointCount; i++)
    {
        sf::CircleShape dot(7.f);
        dot.setOrigin(7.f, 7.f);
        dot.setPosition(waypoints[i].x, waypoints[i].y);
        dot.setFillColor(sf::Color::White);
        win.draw(dot);
    }
}


//small houses

Base bases[MAX_BASES];
int  baseCount = 0;

void addBase(float x, float y, int hp, int dmgToPlayer)
{
    if (baseCount >= MAX_BASES) return;
    bases[baseCount].x = x;
    bases[baseCount].y = y;
    bases[baseCount].hp = hp;
    bases[baseCount].maxHp = hp;
    bases[baseCount].damageToPlayer = dmgToPlayer;
    bases[baseCount].destroyed = false;
    bases[baseCount].attackFlashTimer = 0.f;
    baseCount++;
}

void clearBases()
{
    baseCount = 0;
}

void drawBases(sf::RenderWindow& win)
{
    for (int i = 0; i < baseCount; i++)
    {
        if (bases[i].destroyed) continue;

        float bx = bases[i].x;
        float by = bases[i].y;

        //drawing of the house
        sf::Color bodyCol = (bases[i].attackFlashTimer > 0.f)
            ? sf::Color(255, 80, 80)  
            : sf::Color(160, 100, 50);

        sf::RectangleShape body(sf::Vector2f(32, 28));
        body.setOrigin(16, 14);
        body.setPosition(bx, by);
        body.setFillColor(bodyCol);
        body.setOutlineThickness(2.f);
        body.setOutlineColor(sf::Color(80, 50, 20));
        win.draw(body);

        sf::ConvexShape roof;
        roof.setPointCount(3);
        roof.setPoint(0, sf::Vector2f(bx - 18, by - 14));
        roof.setPoint(1, sf::Vector2f(bx + 18, by - 14));
        roof.setPoint(2, sf::Vector2f(bx, by - 30));
        roof.setFillColor(sf::Color(140, 40, 40));
        roof.setOutlineThickness(1.5f);
        roof.setOutlineColor(sf::Color(80, 20, 20));
        win.draw(roof);


        sf::RectangleShape door(sf::Vector2f(8, 12));
        door.setOrigin(4, 0);
        door.setPosition(bx, by);
        door.setFillColor(sf::Color(60, 35, 15));
        win.draw(door);

        // HP bar of house
        float pct = (float)bases[i].hp / (float)bases[i].maxHp;

        sf::RectangleShape hpBg(sf::Vector2f(36, 5));
        hpBg.setFillColor(sf::Color(60, 60, 60));
        hpBg.setPosition(bx - 18, by - 38);
        win.draw(hpBg);

        sf::Color barCol = sf::Color::Green;
        if (pct < 0.5f)  barCol = sf::Color::Yellow;
        if (pct < 0.25f) barCol = sf::Color(255, 80, 0);

        sf::RectangleShape hpBar(sf::Vector2f(36 * pct, 5));
        hpBar.setFillColor(barCol);
        hpBar.setPosition(bx - 18, by - 38);
        win.draw(hpBar);


        if (bases[i].attackFlashTimer > 0.f)
            bases[i].attackFlashTimer -= 0.016f; // 1 frame at 60fps
    }
}
//enemy class for all enemies

Enemy::Enemy()
{
    x = y = 0;
    hp = maxHp = 1;
    speed = 60.f;
    wpIndex = 0;
    dead = false;
    reachedEnd = false;
    reward = 10;
    id = nextId++;

    flashTimer = 0.f;
    deathTimer = 0.f;
    isDying = false;
    deathScale = 1.f;
    bobTimer = 0.f;
}

void Enemy::setHp(int h) { hp = maxHp = h; }
void Enemy::setSpeed(float s) { speed = s; }
void Enemy::setReward(int r) { reward = r; }
void Enemy::setPos(float px, float py) { x = px; y = py; }

float Enemy::getX() { return x; }
float Enemy::getY() { return y; }
int   Enemy::getId() { return id; }
int   Enemy::getReward() { return reward; }
int   Enemy::getHp() { return hp; }
int   Enemy::getMaxHp() { return maxHp; }

bool Enemy::isAlive() { return !dead && !reachedEnd && !isDying; }
bool Enemy::hasReachedEnd() { return reachedEnd && !isDying; }
bool Enemy::isDead() { return dead; }

bool Enemy::isDeathAnimDone()
{
    if (!isDying) return false;
    return deathTimer >= 0.45f;
}

void Enemy::takeDamage(int dmg)
{
    if (isDying) return;
    hp -= dmg;
    flashTimer = 0.12f;
    if (hp <= 0)
    {
        hp = 0;
        dead = true;
        isDying = true;
        deathTimer = 0.f;
    }
}

void Enemy::updateAnimations(float dt)
{
    bobTimer += dt * 3.f;
    if (flashTimer > 0.f) flashTimer -= dt;

    if (isDying)
    {
        deathTimer += dt;
        deathScale = 1.f - (deathTimer / 0.45f);
        if (deathScale < 0.f) deathScale = 0.f;
    }
}

void Enemy::moveAlongPath(float dt)
{
    if (isDying) return;

    if (wpIndex >= waypointCount)
    {
        reachedEnd = true;
        return;
    }

    float tx = waypoints[wpIndex].x;
    float ty = waypoints[wpIndex].y;
    float dx = tx - x;
    float dy = ty - y;
    float dist = sqrt(dx * dx + dy * dy);

    if (dist < 5.f)
    {
        wpIndex++;
        if (wpIndex >= waypointCount)
            reachedEnd = true;
    }
    else
    {
        x += (dx / dist) * speed * dt;
        y += (dy / dist) * speed * dt;
    }
}

void Enemy::drawHpBar(sf::RenderWindow& win, float barWidth, float yAbove)
{
    if (isDying) return;

    float pct = (float)hp / (float)maxHp;

    sf::RectangleShape bg(sf::Vector2f(barWidth, 5));
    bg.setFillColor(sf::Color(60, 60, 60));
    bg.setPosition(x - barWidth / 2.f, y - yAbove);
    win.draw(bg);

    sf::Color col = sf::Color::Green;
    if (pct < 0.5f)  col = sf::Color::Yellow;
    if (pct < 0.25f) col = sf::Color(255, 80, 0);

    sf::RectangleShape bar(sf::Vector2f(barWidth * pct, 5));
    bar.setFillColor(col);
    bar.setPosition(x - barWidth / 2.f, y - yAbove);
    win.draw(bar);
}

void Enemy::drawBody(sf::RenderWindow& win, float size, sf::Color col)
{
    if (isDying)
    {
        float s = size * deathScale;
        sf::RectangleShape body(sf::Vector2f(s, s));
        body.setOrigin(s / 2.f, s / 2.f);
        body.setPosition(x, y);
        body.setFillColor(sf::Color(255, 80, 0, (sf::Uint8)(200 * deathScale)));
        body.setRotation(deathTimer * 500.f);
        body.setOutlineThickness(1.5f);
        body.setOutlineColor(sf::Color(200, 50, 0));
        win.draw(body);

        for (int i = 0; i < 4; i++)
        {
            float angle = (i * 90.f + deathTimer * 300.f) * 3.14159f / 180.f;
            float pRadius = deathTimer * 40.f;
            sf::CircleShape p(4.f * deathScale);
            p.setOrigin(4.f * deathScale, 4.f * deathScale);
            p.setPosition(x + cos(angle) * pRadius, y + sin(angle) * pRadius);
            p.setFillColor(sf::Color(255, 160, 0, (sf::Uint8)(180 * deathScale)));
            win.draw(p);
        }
        return;
    }

    float bobOffset = sin(bobTimer) * 2.5f;
    sf::Color drawCol = (flashTimer > 0.f) ? sf::Color::White : col;

    sf::RectangleShape body(sf::Vector2f(size, size));
    body.setOrigin(size / 2.f, size / 2.f);
    body.setPosition(x, y + bobOffset);
    body.setFillColor(drawCol);
    body.setOutlineThickness(1.5f);
    body.setOutlineColor(sf::Color(20, 20, 20));
    win.draw(body);

    // eyes
    float eyeSize = size * 0.18f;
    float eyeOffset = size * 0.18f;

    sf::RectangleShape eyeL(sf::Vector2f(eyeSize, eyeSize));
    eyeL.setOrigin(eyeSize / 2.f, eyeSize / 2.f);
    eyeL.setFillColor(sf::Color::White);
    eyeL.setPosition(x - eyeOffset, y - size * 0.1f + bobOffset);
    win.draw(eyeL);

    sf::RectangleShape eyeR(sf::Vector2f(eyeSize, eyeSize));
    eyeR.setOrigin(eyeSize / 2.f, eyeSize / 2.f);
    eyeR.setFillColor(sf::Color::White);
    eyeR.setPosition(x + eyeOffset, y - size * 0.1f + bobOffset);
    win.draw(eyeR);

    // enemy's pupils look towards next waypoint
    float lookDx = 0.f, lookDy = 0.f;
    if (wpIndex < waypointCount)
    {
        lookDx = waypoints[wpIndex].x - x;
        lookDy = waypoints[wpIndex].y - y;
        float d = sqrt(lookDx * lookDx + lookDy * lookDy);
        if (d > 0.f) { lookDx /= d; lookDy /= d; }
    }
    float pupilSize = eyeSize * 0.5f;
    float pupilShift = eyeSize * 0.2f;

    sf::RectangleShape pupilL(sf::Vector2f(pupilSize, pupilSize));
    pupilL.setOrigin(pupilSize / 2.f, pupilSize / 2.f);
    pupilL.setFillColor(sf::Color(20, 20, 20));
    pupilL.setPosition(x - eyeOffset + lookDx * pupilShift,
        y - size * 0.1f + bobOffset + lookDy * pupilShift);
    win.draw(pupilL);

    sf::RectangleShape pupilR(sf::Vector2f(pupilSize, pupilSize));
    pupilR.setOrigin(pupilSize / 2.f, pupilSize / 2.f);
    pupilR.setFillColor(sf::Color(20, 20, 20));
    pupilR.setPosition(x + eyeOffset + lookDx * pupilShift,
        y - size * 0.1f + bobOffset + lookDy * pupilShift);
    win.draw(pupilR);
}

void Enemy::drawBar(sf::RenderWindow& win, float barWidth, float yAbove)
{
    drawHpBar(win, barWidth, yAbove);
}


//NORMAL ENEMY  

class NormalEnemy : public Enemy
{
public:
    NormalEnemy(float sx, float sy, float sm, float hm)
    {
        setPos(sx, sy);
        setHp((int)(100 * hm));
        setSpeed(150.f * sm);
        setReward(10);
    }

    void update(float dt) override
    {
        updateAnimations(dt);
        moveAlongPath(dt);
    }

    void draw(sf::RenderWindow& win) override
    {
        drawBody(win, 26, sf::Color(210, 50, 50));
        drawBar(win, 26, 24);
    }

    string getTypeName() override { return "Normal"; }
};


//FAST ENEMY 

class FastEnemy : public Enemy
{
public:
    FastEnemy(float sx, float sy, float sm, float hm)
    {
        setPos(sx, sy);
        setHp((int)(55 * hm));
        setSpeed(255.f * sm);
        setReward(15);
    }

    void update(float dt) override
    {
        updateAnimations(dt);
        moveAlongPath(dt);
    }

    void draw(sf::RenderWindow& win) override
    {
        // motion trail behind fast enemy
        if (!getIsDying())
        {
            sf::RectangleShape t1(sf::Vector2f(14, 14));
            t1.setOrigin(7, 7);
            t1.setPosition(getX() - 10.f, getY());
            t1.setFillColor(sf::Color(240, 210, 30, 80));
            win.draw(t1);

            sf::RectangleShape t2(sf::Vector2f(9, 9));
            t2.setOrigin(4.5f, 4.5f);
            t2.setPosition(getX() - 20.f, getY());
            t2.setFillColor(sf::Color(240, 210, 30, 35));
            win.draw(t2);
        }
        drawBody(win, 20, sf::Color(240, 210, 30));
        drawBar(win, 20, 18);
    }

    string getTypeName() override { return "Fast"; }
};


//TANK ENEMY

class TankEnemy : public Enemy
{
private:
    float armorFlash;

public:
    TankEnemy(float sx, float sy, float sm, float hm)
    {
        setPos(sx, sy);
        setHp((int)(400 * hm));
        setSpeed(80.f * sm);
        setReward(30);
        armorFlash = 0.f;
    }

    void update(float dt) override
    {
        updateAnimations(dt);
        moveAlongPath(dt);
        if (armorFlash > 0.f) armorFlash -= dt;
    }

    void draw(sf::RenderWindow& win) override
    {
        if (!getIsDying())
        {
            float bx = getX();
            float by = getY() + sin(getBobTimer()) * 2.5f;

            sf::Color platCol = (armorFlash > 0.f)
                ? sf::Color::White
                : sf::Color(30, 30, 160);

            for (int i = 0; i < 4; i++)
            {
                float ax = (i < 2) ? bx - 22.f : bx + 22.f;
                float ay = (i % 2 == 0) ? by - 22.f : by + 22.f;
                sf::RectangleShape plate(sf::Vector2f(11, 11));
                plate.setOrigin(5.5f, 5.5f);
                plate.setPosition(ax, ay);
                plate.setFillColor(platCol);
                plate.setOutlineThickness(1.f);
                plate.setOutlineColor(sf::Color(10, 10, 80));
                win.draw(plate);
            }
        }
        drawBody(win, 36, sf::Color(40, 60, 200));
        drawBar(win, 40, 30);
    }

    string getTypeName() override { return "Tank"; }
};

// TOWER ATTACKER ENEMY

class TowerTarget
{
public:
    float  x, y;
    int* hp;
    bool* destroyed;
};

const int MAX_TOWER_TARGETS = 50;
extern TowerTarget towerTargets[MAX_TOWER_TARGETS];
extern int         towerTargetCount;


void registerTowerTarget(float x, float y, int* hp, bool* destroyed);
void clearTowerTargets();

TowerTarget towerTargets[MAX_TOWER_TARGETS];
int         towerTargetCount = 0;

void registerTowerTarget(float x, float y, int* hp, bool* destroyed)
{
    if (towerTargetCount >= MAX_TOWER_TARGETS) return;
    towerTargets[towerTargetCount].x = x;
    towerTargets[towerTargetCount].y = y;
    towerTargets[towerTargetCount].hp = hp;
    towerTargets[towerTargetCount].destroyed = destroyed;
    towerTargetCount++;
}

void clearTowerTargets()
{
    towerTargetCount = 0;
}

class TowerAttacker : public Enemy
{
private:
    float attackRange;      
    int   attackDmg;        
    float attackCooldown;   
    float attackTimer;      
    bool  isAttacking;      
    float attackAnim;       

    // finds the nearest tower or base within attackRange
    bool findNearestTarget(float& targetX, float& targetY)
    {
        float nearest = attackRange;
        bool  found = false;

        // check towers first
        for (int i = 0; i < towerTargetCount; i++)
        {
            if (towerTargets[i].destroyed && *towerTargets[i].destroyed) continue;
            if (towerTargets[i].hp && *towerTargets[i].hp <= 0)          continue;

            float dx = towerTargets[i].x - getX();
            float dy = towerTargets[i].y - getY();
            float dist = sqrt(dx * dx + dy * dy);

            if (dist < nearest)
            {
                nearest = dist;
                targetX = towerTargets[i].x;
                targetY = towerTargets[i].y;
                found = true;
            }
        }

        // also check bases
        for (int i = 0; i < baseCount; i++)
        {
            if (bases[i].destroyed) continue;

            float dx = bases[i].x - getX();
            float dy = bases[i].y - getY();
            float dist = sqrt(dx * dx + dy * dy);

            if (dist < nearest)
            {
                nearest = dist;
                targetX = bases[i].x;
                targetY = bases[i].y;
                found = true;
            }
        }

        return found;
    }


    void hitTarget(float tx, float ty, int& playerHealth)
    {
        attackAnim = 0.15f;  

        // try towers first
        for (int i = 0; i < towerTargetCount; i++)
        {
            if (towerTargets[i].destroyed && *towerTargets[i].destroyed) continue;
            float dx = towerTargets[i].x - tx;
            float dy = towerTargets[i].y - ty;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < 5.f && towerTargets[i].hp)
            {
                *towerTargets[i].hp -= attackDmg;
                if (*towerTargets[i].hp <= 0)
                {
                    *towerTargets[i].hp = 0;
                    if (towerTargets[i].destroyed)
                        *towerTargets[i].destroyed = true;
                    cout << "[TowerAttacker] Tower destroyed!\n";
                }
                return;
            }
        }

        // try bases
        for (int i = 0; i < baseCount; i++)
        {
            if (bases[i].destroyed) continue;
            float dx = bases[i].x - tx;
            float dy = bases[i].y - ty;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < 5.f)
            {
                bases[i].hp -= attackDmg;
                bases[i].attackFlashTimer = 0.2f;
                if (bases[i].hp <= 0)
                {
                    bases[i].hp = 0;
                    bases[i].destroyed = true;
                    // damage player when base falls
                    playerHealth -= bases[i].damageToPlayer;
                    cout << "[TowerAttacker] Base destroyed! Player loses "
                        << bases[i].damageToPlayer << " hp\n";
                }
                return;
            }
        }
    }

public:
    // playerHealth ref is stored so hitTarget can reduce it
    int* playerHealthPtr;

    TowerAttacker(float sx, float sy, float sm, float hm)
    {
        setPos(sx, sy);
        setHp((int)(200 * hm));
        setSpeed(60.f * sm);
        setReward(100);

        attackRange = 55.f;    
        attackDmg = 30;       
        attackCooldown = 1.2f;    
        attackTimer = 0.f;
        isAttacking = false;
        attackAnim = 0.1f;
        playerHealthPtr = nullptr;
    }


    void updateWithHealth(float dt, int& playerHealth)
    {
        updateAnimations(dt);
        if (attackAnim > 0.f) attackAnim -= dt;

        float tx = 0.f, ty = 0.f;
        if (findNearestTarget(tx, ty))
        {
            // stop and attack
            isAttacking = true;
            attackTimer -= dt;
            if (attackTimer <= 0.f)
            {
                attackTimer = attackCooldown;
                hitTarget(tx, ty, playerHealth);
            }
        }
        else
        {
            isAttacking = false;
            moveAlongPath(dt);
        }
    }

    // standard update called when no health ptr available
    void update(float dt) override
    {
        updateAnimations(dt);
        if (attackAnim > 0.f) attackAnim -= dt;

        float tx = 0.f, ty = 0.f;
        if (findNearestTarget(tx, ty))
        {
            isAttacking = true;
            attackTimer -= dt;
            if (attackTimer <= 0.f)
            {
                attackTimer = attackCooldown;
                int dummy = 999;
                hitTarget(tx, ty, dummy);
            }
        }
        else
        {
            isAttacking = false;
            moveAlongPath(dt);
        }
    }

    void draw(sf::RenderWindow& win) override
    {
        if (!getIsDying())
        {
            float bx = getX();
            float by = getY() + sin(getBobTimer()) * 2.5f;

            // weapon spike that extends during attack animation
            float spikeLen = isAttacking ? (14.f + attackAnim * 20.f) : 8.f;
            sf::RectangleShape spike(sf::Vector2f(spikeLen, 5));
            spike.setOrigin(0, 2.5f);
            spike.setPosition(bx + 13, by);
            spike.setFillColor(sf::Color(200, 200, 50));
            spike.setOutlineThickness(1.f);
            spike.setOutlineColor(sf::Color(100, 100, 0));
            win.draw(spike);

            if (isAttacking)
            {
                sf::RectangleShape brow(sf::Vector2f(14, 3));
                brow.setOrigin(7, 1.5f);
                brow.setPosition(bx, by - 15);
                brow.setFillColor(sf::Color(180, 30, 30));
                brow.setRotation(-10.f);
                win.draw(brow);
            }
        }

        drawBody(win, 28, sf::Color(220, 90, 20));
        drawBar(win, 30, 26);
    }

    string getTypeName() override { return "TowerAttacker"; }
};


// type: 0=Normal  1=Fast  2=Tank  3=TowerAttacker

Enemy* createEnemy(int type, float sx, float sy, float sm, float hm)
{
    if (type == 0) return new NormalEnemy(sx, sy, sm, hm);
    if (type == 1) return new FastEnemy(sx, sy, sm, hm);
    if (type == 2) return new TankEnemy(sx, sy, sm, hm);
    if (type == 3) return new TowerAttacker(sx, sy, sm, hm);
    throw invalid_argument("createEnemy: unknown type");
}


//ADAPTIVE DIFFICULTY
AdaptiveDifficulty::AdaptiveDifficulty()
{
    hpMult = 1.0f;
    speedMult = 1.0f;
    wavesSinceIncrease = 0;
}

float AdaptiveDifficulty::getHpMult() { return hpMult; }
float AdaptiveDifficulty::getSpeedMult() { return speedMult; }

void AdaptiveDifficulty::recordResult(int killed, int leaked, int waveNum)
{
    // base increase every wave - game always gets harder
    hpMult += 0.08f;
    speedMult += 0.04f;

    int total = killed + leaked;
    if (total > 0)
    {
        float killRate = (float)killed / (float)total;

        if (killRate >= 0.70f)
        {
            hpMult += 0.10f;
            speedMult += 0.06f;
            wavesSinceIncrease = 0;
        }
        else if (killRate < 0.40f)
        {
            hpMult -= 0.05f;
            speedMult -= 0.03f;
            wavesSinceIncrease++;
        }

        if (wavesSinceIncrease >= 2)
        {
            hpMult -= 0.04f;
            speedMult -= 0.02f;
            wavesSinceIncrease = 0;
        }
    }

    // never easier than wave 1
    if (hpMult < 1.0f) hpMult = 1.0f;
    if (speedMult < 1.0f) speedMult = 1.0f;
    if (hpMult > 3.0f) hpMult = 3.0f;
    if (speedMult > 2.5f) speedMult = 2.5f;

    cout << "[Difficulty] Wave " << waveNum
        << " HP x" << hpMult
        << " Spd x" << speedMult << endl;
}

void AdaptiveDifficulty::reset()
{
    hpMult = 1.0f;
    speedMult = 1.0f;
    wavesSinceIncrease = 0;
}


// WAVE GENERATOR

WaveGenerator::WaveGenerator()
{
    srand((unsigned int)time(nullptr));
}

WaveData<float> WaveGenerator::generate(int waveNum, int level,
    float hpMult, float speedMult)
{
    if (waveNum < 1)
        throw invalid_argument("Wave number must be 1 or higher");

    WaveData<float> w;
    w.waveNumber = waveNum;
    w.hpMult = hpMult;
    w.speedMult = speedMult;
    w.enemyCount = 0;

    w.spawnDelay = 1.8f - (waveNum * 0.18f);
    if (w.spawnDelay < 0.35f) w.spawnDelay = 0.35f;

    int count = 6 + waveNum * 3;
    if (count > MAX_ENEMIES) count = MAX_ENEMIES;

    // how many TowerAttackers to mix in (1 or 2 per wave from wave 2)
    int attackerSlots = (waveNum >= 2) ? 2 : 0;
    if (waveNum >= 4) attackerSlots = 3;

    int attackersPlaced = 0;

    for (int i = 0; i < count; i++)
    {
        int roll = rand() % 100;
        int type = 0;

        // decide if this slot should be a TowerAttacker

        bool placeAttacker = false;
        if (attackersPlaced < attackerSlots)
        {
            int slotsLeft = count - i;
            int attackersLeft = attackerSlots - attackersPlaced;


            if (slotsLeft <= attackersLeft)
                placeAttacker = true;
            else if (rand() % slotsLeft < attackersLeft)
                placeAttacker = true;
        }

        if (placeAttacker)
        {
            type = 3;  
            attackersPlaced++;
        }
        else if (level == 1)
        {
            type = (waveNum <= 2) ? 0 : ((roll < 65) ? 0 : 1);
        }
        else if (level == 2)
        {
            if (roll < 45) type = 0;
            else if (roll < 80) type = 1;
            else                type = 2;
        }
        else
        {
            if (roll < 30) type = 0;
            else if (roll < 60) type = 1;
            else                type = 2;
        }

        w.enemyTypes[w.enemyCount++] = type;
    }


    if (level >= 2)
    {
        int fastCount = 0;
        for (int i = 0; i < w.enemyCount; i++)
            if (w.enemyTypes[i] == 1) fastCount++;
        if (fastCount < 2)
        {
            int replaced = 0;
            for (int i = 0; i < w.enemyCount && replaced < 2; i++)
                if (w.enemyTypes[i] == 0)
                {
                    w.enemyTypes[i] = 1; replaced++;
                }
        }
    }

    if (level >= 3)
    {
        int tankCount = 0;
        for (int i = 0; i < w.enemyCount; i++)
            if (w.enemyTypes[i] == 2) tankCount++;
        if (tankCount < 2)
        {
            int replaced = 0;
            for (int i = 0; i < w.enemyCount && replaced < 2; i++)
                if (w.enemyTypes[i] == 0)
                {
                    w.enemyTypes[i] = 2; replaced++;
                }
        }
    }

    cout << "[WaveGen] Wave " << waveNum << " Lv" << level
        << " Count:" << w.enemyCount
        << " Attackers:" << attackersPlaced
        << " Delay:" << w.spawnDelay << endl;

    return w;
}

// WAVE MANAGER

WaveManager::WaveManager()
{
    currentWave = 0;
    totalWaves = 15;
    waveRunning = false;
    allDone = false;
    currentLevel = 1;
    spawnTimer = 0.f;
    spawnIndex = 0;
    killsThisWave = 0;
    leaksThisWave = 0;
    enemyCount = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i] = nullptr;
    activeWave.enemyCount = 0;
}

WaveManager::~WaveManager() { clearAllEnemies(); }

void WaveManager::clearAllEnemies()
{
    for (int i = 0; i < enemyCount; i++)
    {
        if (enemies[i]) { delete enemies[i]; enemies[i] = nullptr; }
    }
    enemyCount = 0;
}

void WaveManager::setLevel(int lvl)
{
    currentLevel = lvl;
    currentWave = 0;
    allDone = false;
    waveRunning = false;
    difficulty.reset();
    clearAllEnemies();
}

void WaveManager::reset()
{
    clearAllEnemies();
    currentWave = 0;
    waveRunning = false;
    allDone = false;
    spawnTimer = 0.f;
    spawnIndex = 0;
    killsThisWave = 0;
    leaksThisWave = 0;
    difficulty.reset();
    activeWave.enemyCount = 0;
}

void WaveManager::startNextWave()
{
    if (allDone) return;

    if (waveRunning && (killsThisWave + leaksThisWave) > 0)
        difficulty.recordResult(killsThisWave, leaksThisWave, currentWave);

    clearAllEnemies();
    currentWave++;

    if (currentWave > totalWaves)
    {
        allDone = true;
        waveRunning = false;
        return;
    }

    try
    {
        activeWave = generator.generate(currentWave, currentLevel,
            difficulty.getHpMult(),
            difficulty.getSpeedMult());
    }
    catch (exception& e)
    {
        cout << "Wave error: " << e.what() << endl;
        return;
    }

    spawnIndex = 0;
    spawnTimer = 0.f;
    waveRunning = true;
    killsThisWave = 0;
    leaksThisWave = 0;
}

void WaveManager::spawnOne()
{
    if (spawnIndex >= activeWave.enemyCount) return;
    if (enemyCount >= MAX_ENEMIES)           return;
    if (waypointCount == 0)                  return;

    int   type = activeWave.enemyTypes[spawnIndex++];
    float sx = getStartPoint().x;
    float sy = getStartPoint().y;

    try
    {
        enemies[enemyCount] = createEnemy(type, sx, sy,
            activeWave.speedMult,
            activeWave.hpMult);
        enemyCount++;
    }
    catch (exception& e) { cout << "Spawn error: " << e.what() << endl; }
}

int WaveManager::update(float dt, int& playerHealth)
{
    if (!waveRunning) return 0;

    // update all enemies

    for (int i = 0; i < enemyCount; i++)
    {
        TowerAttacker* ta = dynamic_cast<TowerAttacker*>(enemies[i]);
        if (ta)
            ta->updateWithHealth(dt, playerHealth);
        else
            enemies[i]->update(dt);
    }

    if (spawnIndex < activeWave.enemyCount)
    {
        spawnTimer += dt;
        if (spawnTimer >= activeWave.spawnDelay)
        {
            spawnTimer = 0.f;
            spawnOne();
        }
    }

    // clean up dead or leaked enemy
    int i = 0;
    int coinsEarned = 0;
    int newLeaks = 0;

    while (i < enemyCount)
    {
        bool remove = false;

        if (enemies[i]->isDead() && enemies[i]->isDeathAnimDone())
        {
            coinsEarned += enemies[i]->getReward() * 5;
            killsThisWave++;
            remove = true;
        }
        else if (enemies[i]->hasReachedEnd())
        {
            
            newLeaks++;
            leaksThisWave++;
            remove = true;
        }

        if (remove)
        {
            delete enemies[i];
            enemies[i] = nullptr;
            for (int j = i; j < enemyCount - 1; j++)
                enemies[j] = enemies[j + 1];
            enemies[enemyCount - 1] = nullptr;
            enemyCount--;
        }
        else i++;
    }


    playerHealth -= newLeaks * 10;

    return coinsEarned;
}

bool WaveManager::isWaveComplete()
{
    if (!waveRunning) return false;

    bool spawnDone = (spawnIndex >= activeWave.enemyCount);

    int stillPresent = 0;
    for (int i = 0; i < enemyCount; i++)
        if (!enemies[i]->isDeathAnimDone())
            stillPresent++;

    return spawnDone && (stillPresent == 0);
}

int WaveManager::getEnemiesInRange(float tx, float ty, float range,
    Enemy* out[], int maxOut)
{
    int found = 0;
    for (int i = 0; i < enemyCount && found < maxOut; i++)
    {
        if (!enemies[i]->isAlive()) continue;
        float dx = enemies[i]->getX() - tx;
        float dy = enemies[i]->getY() - ty;
        if (sqrt(dx * dx + dy * dy) <= range)
            out[found++] = enemies[i];
    }
    return found;
}

void WaveManager::damageEnemy(int enemyId, int dmg)
{
    for (int i = 0; i < enemyCount; i++)
        if (enemies[i]->getId() == enemyId && enemies[i]->isAlive())
        {
            enemies[i]->takeDamage(dmg);
            return;
        }
}

void WaveManager::drawAllEnemies(sf::RenderWindow& win)
{
    for (int i = 0; i < enemyCount; i++)
        enemies[i]->draw(win);
}

int WaveManager::getRemainingEnemies()
{
    int count = 0;
    for (int i = 0; i < enemyCount; i++)
        if (enemies[i]->isAlive()) count++;
    return count;
}

int WaveManager::getEnemiesLeftToSpawn()
{
    return activeWave.enemyCount - spawnIndex;
}