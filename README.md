Tower Defense Game
OOP Project 

A C++ Tower Defense game built with SFML. Defend your castle by placing towers strategically to stop waves of enemies!

Team Members:
Member 1 : Game Loop, Level System, Win/Loss Conditions 
Member 2 : Towers, Combat, Economy 
Member 3 : Enemies, Wave Generator, Difficulty 
Member 4 : UI, Graphics, Grid 
Member 5 : Save/Load, High Scores 

Features:
- Three tower types: Cannon (splash damage), Rapid (fast attacks), Ice (slow enemies)
- Three enemy types: Normal, Fast, Tank
- Upgrade system (Damage → Range → Special)
- 3 levels with increasing difficulty
- Coin system (earn coins by killing enemies)
- Save/Load game
- High score leaderboard

Prerequisites
Git 
CMake: 3.15+
C++ Compiler:  C++17 
Windows : Visual Studio 2022 

Building the Game:

No need to install SFML. CMake will download it automatically.

Step 1: Clone the Repository
```bash
git clone https://github.com/YOUR_USERNAME/TowerDefenseGame.git
cd TowerDefenseGame

Step 2: Build with CMake
Windows (Command Prompt):
mkdir build
cd build
cmake ..
cmake --build . --config Release

Step 3: Run the Game
Release\TowerDefenseGame.exe


How to Play:
1. Start Game → Enter your name and begin
2. Buy Towers → Click tower button, then click on grass
3. Upgrade Towers → Click a tower, then click Upgrade
4. Delete Towers → Click a tower, then click Delete (no refund)
5. Survive → Don't let enemies reach the castle!



