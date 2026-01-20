# Universal Game AI Framework (通用棋类 AI 框架)

演示了如何使用 C++ 构建一个通用的博弈引擎，并特化为五子棋和井字棋。

## 1. 架构设计 (Algorithm Architecture)

### 抽象接口：`GameEngine`
核心接口定义在 `cpp/engine.h` 中。任何棋类只要实现以下接口即可被 AI 驱动：
- `getPossibleMoves()`: 生成当前合法的落子点。
- `makeMove()` / `undoMove()`: 更新棋盘状态。
- `evaluate()`: 对当前局势进行启发式分值评估。
- `checkWin()`: 检查胜负。

### 核心算法：`MinimaxAI`
算法实现位于 `cpp/minimax.cpp`。它不再针对特定游戏，而是操作 `GameEngine` 接口：
- 使用 **递归树搜索** 到达指定深度。
- 使用 **Alpha-Beta 剪枝** 消除无效的分支。
- 搜索终点的分值由具体游戏的 `evaluate()` 函数提供。

## 2. 实现的游戏 (Current Games)

- **Gomoku (五子棋)**: 特化了 15x15 棋盘及复杂的线搜索评估逻辑。
- **Tic-Tac-Toe (井字棋)**: 特化了 3x3 棋盘，展示了框架对不同尺寸棋盘的适应性。

## 3. 实现方案 (Implementation)

- **C++**: 使用多态（Polymorphism）实现引擎切换。`main.cpp` 维护一个 `unique_ptr<GameEngine>`，可根据指令随时切换。
- **Electron**: UI 会根据后端推送的 `switched` 消息动态调整 Board 尺寸。

## 4. 运行方法

1. **编译 (Compile)**:
   ```bash
   cd cpp
   g++ -O3 -std=c++11 main.cpp minimax.cpp gomoku_engine.cpp ttt_engine.cpp -o universal_ai
   ```

2. **启动 (Run)**:
   ```bash
   cd ../electron
   npm start
   ```

## 5. 开发新的 AI
只需三步即可添加新游戏：
1. 继承 `GameEngine` 类。
2. 实现该游戏的落子规则和评分。
3. 在 `main.cpp` 的 `switch` 指令中注册该引擎。
