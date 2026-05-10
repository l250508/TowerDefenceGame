#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace std;

struct SaveData {
    //basic stats
    int level = 1;
    int health = 100;
    int coins = 500;
    int score = 0;
    int towerCount = 0;
    string playerName = "";

    //wave management
    int currentWave = 1;
    int enemiesRemaining = 0;
    int enemiesSpawned = 0;
    float spawnTimer = 0;
    float spawnDelay = 1.5f;

    //tower data 
    string towerTypes[50];
    float towerX[50];
    float towerY[50];
    int towerUpgradeLevels[50];
    int towerDamages[50];
    float towerRanges[50];

    //grid state
    bool towerGrid[10][15];

    //wave enemy types that haven't spawned yet
    int remainingEnemyTypes[100];
    int remainingEnemyCount = 0;

    bool isValid() const {
        if (level >= 1 && level <= 10 &&
            health >= 0 && health <= 1000 &&
            coins >= 0 && score >= 0 &&
            towerCount >= 0 && towerCount <= 50 &&
            currentWave >= 1 && currentWave <= 20 &&
            enemiesRemaining >= 0 &&
            remainingEnemyCount >= 0) {
            return true;
        }
        return false;
    }

    void reset() {
        level = 1;
        health = 100;
        coins = 500;
        score = 0;
        towerCount = 0;
        playerName = "";
        currentWave = 1;
        enemiesRemaining = 0;
        enemiesSpawned = 0;
        spawnTimer = 0;
        spawnDelay = 1.5f;
        remainingEnemyCount = 0;

        //reset grid
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 15; j++) {
                towerGrid[i][j] = false;
            }
        }
    }
};

struct HighScoreEntry {
    string name;
    int score;

    HighScoreEntry() : name(""), score(0) {}
    HighScoreEntry(string n, int s) : name(n), score(s) {}
};

class HighScoreManager {
private:
    string filename = "highscores.txt";
    HighScoreEntry topScores[3];
    int scoreCount = 0;

public:
    HighScoreManager() {
        loadScores();
    }

    bool addScore(const string& playerName, int score) {
        if (score < 0 || playerName.empty()) {
            cerr << "Error!!! Invalid score or name" << endl;
            return false;
        }

        //create new entry
        HighScoreEntry newEntry(playerName, score);

        //load existing scores
        loadScores();

        //add new score
        HighScoreEntry allScores[100];
        int totalCount = scoreCount;
        for (int i = 0; i < scoreCount; i++) {
            allScores[i] = topScores[i];
        }
        allScores[totalCount] = newEntry;
        totalCount++;

        //sort by score 
        for (int i = 0; i < totalCount - 1; i++) {
            for (int j = i + 1; j < totalCount; j++) {
                if (allScores[i].score < allScores[j].score) {
                    HighScoreEntry temp = allScores[i];
                    allScores[i] = allScores[j];
                    allScores[j] = temp;
                }
            }
        }

        //keep top 3
        scoreCount = (totalCount < 3) ? totalCount : 3;
        for (int i = 0; i < scoreCount; i++) {
            topScores[i] = allScores[i];
        }

        //save to file
        return saveScores();
    }

    bool saveScores() {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error !!! Cannot open highscore file for writing" << endl;
            return false;
        }

        file << scoreCount << endl;
        for (int i = 0; i < scoreCount; i++) {
            file << topScores[i].name << endl;
            file << topScores[i].score << endl;
        }

        file.close();
        return true;
    }

    bool loadScores() {
        ifstream file(filename);
        if (!file.is_open()) {
            scoreCount = 0;
            return true;
        }

        file >> scoreCount;
        if (scoreCount > 3) scoreCount = 3;

        file.ignore();
        for (int i = 0; i < scoreCount; i++) {
            getline(file, topScores[i].name);
            file >> topScores[i].score;
            file.ignore();
        }

        file.close();
        return true;
    }

    int getTopScores(HighScoreEntry output[], int maxCount) {
        int count = (scoreCount < maxCount) ? scoreCount : maxCount;
        for (int i = 0; i < count; i++) {
            output[i] = topScores[i];
        }
        return count;
    }

    string getTopScoresText() {
        string result = "";
        for (int i = 0; i < scoreCount; i++) {
            result += to_string(i + 1) + ". " + topScores[i].name + " - " + to_string(topScores[i].score) + "\n";
        }
        return result;
    }

    void displayScores() {
        cout << "=== HIGH SCORES ===" << endl;
        for (int i = 0; i < scoreCount; i++) {
            cout << i + 1 << ". " << topScores[i].name << " - " << topScores[i].score << endl;
        }
    }
};

//save or  load game
class SaveLoad {
private:
    bool validateFile(const string& filename) {
        if (filename.empty()) {
            cerr << "Error !!! filename is empty" << endl;
            return false;
        }
        return true;
    }

public:
    bool saveGame(const SaveData& state, const string& filename) {
        if (!validateFile(filename)) return false;
        if (!state.isValid()) {
            cerr << "Error !!! game state is invalid" << endl;
            return false;
        }

        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error !!! Cannot open file for writing: " << filename << endl;
            return false;
        }

        //save stats
        file << state.level << endl;
        file << state.health << endl;
        file << state.coins << endl;
        file << state.score << endl;
        file << state.playerName << endl;
        file << state.towerCount << endl;
        file << state.currentWave << endl;
        file << state.enemiesRemaining << endl;
        file << state.enemiesSpawned << endl;
        file << state.spawnTimer << endl;
        file << state.spawnDelay << endl;

        // Save tower data
        for (int i = 0; i < state.towerCount; i++) {
            file << state.towerTypes[i] << endl;
            file << state.towerX[i] << endl;
            file << state.towerY[i] << endl;
            file << state.towerUpgradeLevels[i] << endl;
            file << state.towerDamages[i] << endl;
            file << state.towerRanges[i] << endl;
        }

        // Save grid state 
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 15; j++) {
                file << state.towerGrid[i][j] << " ";
            }
            file << endl;
        }

        // Save remaining enemy types
        file << state.remainingEnemyCount << endl;
        for (int i = 0; i < state.remainingEnemyCount; i++) {
            file << state.remainingEnemyTypes[i] << endl;
        }

        if (file.fail()) {
            cerr << "Error !!! Failed to write to file" << endl;
            file.close();
            return false;
        }

        file.close();
        cout << "Game saved successfully! " << state.towerCount << " towers saved." << endl;
        return true;
    }

    bool loadGame(SaveData& state, const string& filename) {
        if (!validateFile(filename)) return false;

        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error !!! Cannot open save file: " << filename << endl;
            return false;
        }

        SaveData temp;
        temp.reset();

        // Load basic stats
        file >> temp.level >> temp.health >> temp.coins >> temp.score;
        file.ignore();
        getline(file, temp.playerName);
        file >> temp.towerCount >> temp.currentWave >> temp.enemiesRemaining;
        file >> temp.enemiesSpawned >> temp.spawnTimer >> temp.spawnDelay;

        if (file.fail()) {
            cerr << "Error!!! File is corrupted (basic stats)" << endl;
            file.close();
            return false;
        }

        // Load tower data
        if (temp.towerCount > 0 && temp.towerCount <= 50) {
            for (int i = 0; i < temp.towerCount; i++) {
                file >> temp.towerTypes[i];
                file >> temp.towerX[i];
                file >> temp.towerY[i];
                file >> temp.towerUpgradeLevels[i];
                file >> temp.towerDamages[i];
                file >> temp.towerRanges[i];
            }
        }

        // Load grid state
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 15; j++) {
                int val;
                file >> val;
                temp.towerGrid[i][j] = (val == 1);
            }
        }

        // Load remaining enemy types
        file >> temp.remainingEnemyCount;
        for (int i = 0; i < temp.remainingEnemyCount && i < 100; i++) {
            file >> temp.remainingEnemyTypes[i];
        }

        if (file.fail() && !file.eof()) {
            cerr << "Error!!! File is corrupted (tower/enemy data)" << endl;
            file.close();
            return false;
        }

        if (!temp.isValid()) {
            cerr << "Error!!! Loaded data is invalid" << endl;
            file.close();
            return false;
        }

        state = temp;
        file.close();
        cout << "Game loaded! " << state.towerCount << " towers restored. Wave: " << state.currentWave << endl;
        return true;
    }

    bool saveExists(const string& filename) {
        ifstream file(filename);
        return file.is_open();
    }
};

//pause manger
class PauseManager {
private:
    bool paused;

public:
    PauseManager() : paused(false) {}

    void togglePause() {
        paused = !paused;
    }

    void setPause(bool p) {
        paused = p;
    }

    bool isPaused() const {
        return paused;
    }
};