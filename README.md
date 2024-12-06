# ConnectSix

## Introduction to Renju (Six-in-a-Row)

Renju, also known as Six-in-a-Row, is a strategy board game played on a grid. The objective of the game is to align six of your pieces in a straight line (horizontally, vertically, or diagonally) before your opponent does. The rules are as follows:  
1.    The game is played between two players who take turns placing pieces on the board.  
2.    The first player uses black pieces, and the second player uses white pieces.
3.    The goal is to align six pieces of the same color in a row without interruption.  
4.    The game ends when a player achieves six in a row, or when the board is full and no further moves are possible.  

This project implements a Renju-playing AI using evaluation functions and a game tree search algorithm. Below is a detailed explanation of the methodology, innovations, and observations during development.  

---

## Summary of the Project

### 2.1 Overall Approach
#### 1.    Global Greedy Evaluation Function
A global evaluation function was implemented based on referenced research.  
•    Black pieces are assigned a value of 1 and white pieces a value of 7.  
•    The function evaluates the sum of six consecutive cells to determine the presence of black-only, white-only, mixed, or empty lines.  
•    Evaluation is performed globally across four directions: horizontal, vertical, and the two diagonals.  
#### 2.    Game Tree with Localized Reevaluation
•    A game tree search was employed, enhanced with a localized evaluation method.  
•    After placing two pieces, only 7–8 newly affected lines are reevaluated, significantly improving efficiency.  
•    Points with the greatest changes are recursively searched using a minimax algorithm.  
#### 3.    Dual Parameters for Offensive and Defensive Strategies
•    Offensive and defensive parameters were defined for different situations.  
•    A unified evaluation function is used for final board assessments, ensuring consistent scoring.  

---

### 2.2 Novelty and Innovations
#### 1.    Optimized Point Selection
•    During point selection, the algorithm simultaneously evaluates pairs of points, which greatly increases computational requirements as the game progresses.  
•    Initial attempts involved selecting all potential pairs (~thousands) and sorting them, which was computationally expensive even with quicksort.  
•    Optimizations Implemented:  
•    Maintained an array of the top 30 points, using insertion sort to update it dynamically when new points surpassed the 30th. This reduced time complexity to approximately.  
•    Observed that the top value and the 50th value generally have a ratio > 0.7. Points with a value < 70% of the maximum are ignored, reducing candidates to ~500. This significantly reduced sorting overhead.  
#### 2.    Opening Moves with Predefined Playbooks
•    Early-game moves are guided by a playbook of winning strategies to lock in advantageous positions and improve overall success rates.  
#### 3.    Greedy Approach in Final Layers
•    The final layer of minimax search adopts a greedy heuristic to accelerate the process without significant loss of accuracy.  
#### 4.    Opponent Strategy Detection
•    A function was developed to identify whether the opponent is using an offensive or defensive strategy. Based on the detection, corresponding parameters are adjusted dynamically.  

---

### 3. Observations and Challenges
#### 1.    Debugging Greedy Functions and Game Trees
•    Debugging the greedy evaluation function was straightforward since incorrect placements could be traced to specific calculations.  
•    However, debugging the game tree required more effort due to its recursive nature. Setting breakpoints and analyzing recursion depths proved critical in identifying issues.  
#### 2.    Modular Design
•    Breaking the program into well-defined functions for specific tasks significantly reduced complexity and enhanced clarity.  

---

### 4. References
1.    Qi Yifei, Research and Application of Dual Evaluation Parameters Based on Lines in Renju.  
2.    Bilibili videos on game trees and alpha-beta pruning.  
3.    Course materials on Learning Pass.  
