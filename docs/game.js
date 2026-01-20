/**
 * AI Game Logic - Connects WASM module with Canvas UI
 */

class GameUI {
    constructor() {
        this.canvas = document.getElementById('game-board');
        this.ctx = this.canvas.getContext('2d');
        this.loadingOverlay = document.getElementById('loading');
        this.currentPlayerText = document.getElementById('current-player');
        this.statusText = document.getElementById('game-status');
        this.difficultySelect = document.getElementById('ai-difficulty');

        this.gameType = 'gomoku';
        this.boardSize = 15;
        this.cellSize = 0;
        this.padding = 30;

        this.module = null;
        this.game = null;
        this.isGameOver = false;
        this.isThinking = false;
        this.firstPlayer = 1; // 1 = Human (Black), 2 = AI (White)

        this.bindEvents();
        this.initTheme();
        this.loadWasm();
    }

    async loadWasm() {
        try {
            this.showLoading();
            // createGameModule is defined in game_module.js (exported by Emscripten)
            if (typeof createGameModule === 'undefined') {
                throw new Error('WebAssembly 模块加载脚本未找到，请先运行 build_wasm.sh');
            }

            this.module = await createGameModule();
            this.initGame('gomoku');
            this.hideLoading();
        } catch (error) {
            console.error('WASM Load Error:', error);
            this.statusText.textContent = '错误: 无法加载游戏引擎';
            this.hideLoading();
            alert('无法加载 WebAssembly 模块。请确保已经运行了 build_wasm.sh 并通过 Web 服务器访问。');
        }
    }

    initGame(type) {
        this.gameType = type;
        this.isGameOver = false;
        this.isThinking = false;

        if (this.game) {
            this.game.delete(); // Clean up previous instance if Emscripten class
        }

        this.game = new this.module.GameWrapper(type);
        this.boardSize = this.game.getBoardWidth();

        this.setupCanvas();
        this.updateUI();
        this.drawBoard();

        // If AI is first player, make move
        if (this.firstPlayer === 2) {
            this.getAIMove();
        }
    }

    setupCanvas() {
        // Handle high DPI screens
        const dpr = window.devicePixelRatio || 1;
        const size = Math.min(window.innerWidth - 80, 600);

        this.canvas.style.width = size + 'px';
        this.canvas.style.height = size + 'px';
        this.canvas.width = size * dpr;
        this.canvas.height = size * dpr;
        this.ctx.scale(dpr, dpr);

        if (this.gameType === 'gomoku') {
            this.cellSize = (size - this.padding * 2) / (this.boardSize - 1);
        } else {
            this.cellSize = (size - this.padding * 2) / this.boardSize;
        }
        this.displaySize = size;
    }

    bindEvents() {
        this.canvas.addEventListener('click', (e) => this.handleCanvasClick(e));

        document.querySelectorAll('.game-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const type = e.currentTarget.dataset.game;
                if (type === this.gameType) return;

                document.querySelectorAll('.game-btn').forEach(b => b.classList.remove('active'));
                e.currentTarget.classList.add('active');
                this.initGame(type);
            });
        });

        document.getElementById('reset-btn').addEventListener('click', () => {
            this.initGame(this.gameType);
        });

        document.getElementById('first-player-btn').addEventListener('click', () => {
            this.firstPlayer = (this.firstPlayer === 1) ? 2 : 1;
            this.initGame(this.gameType);
            const playerStr = this.firstPlayer === 1 ? '你' : 'AI';
            alert(`下局开始将由 ${playerStr} 先手`);
        });

        document.getElementById('theme-btn').addEventListener('click', () => {
            const theme = document.body.dataset.theme === 'light' ? 'dark' : 'light';
            document.body.dataset.theme = theme;
            localStorage.setItem('game-theme', theme);
            this.drawBoard();
        });

        window.addEventListener('resize', () => {
            this.setupCanvas();
            this.drawBoard();
        });
    }

    initTheme() {
        const savedTheme = localStorage.getItem('game-theme') || 'dark';
        document.body.dataset.theme = savedTheme;
    }

    handleCanvasClick(e) {
        if (this.isGameOver || this.isThinking) return;

        const rect = this.canvas.getBoundingClientRect();
        const x = e.clientX - rect.left - this.padding;
        const y = e.clientY - rect.top - this.padding;

        let gridX, gridY;
        if (this.gameType === 'gomoku') {
            gridX = Math.round(x / this.cellSize);
            gridY = Math.round(y / this.cellSize);
        } else {
            gridX = Math.floor(x / this.cellSize);
            gridY = Math.floor(y / this.cellSize);
        }

        if (gridX >= 0 && gridX < this.boardSize && gridY >= 0 && gridY < this.boardSize) {
            this.makeMove(gridX, gridY);
        }
    }

    makeMove(x, y) {
        const player = this.game.getCurrentPlayer();
        const success = this.game.makeMove(x, y);

        if (success) {
            this.drawBoard();
            if (this.game.checkWin(x, y, player)) {
                this.endGame(player === 1 ? '你赢了！' : 'AI 赢了！');
                return;
            }
            if (this.game.isFull()) {
                this.endGame('平局！');
                return;
            }

            this.updateUI();

            // If it's AI's turn now
            if (this.game.getCurrentPlayer() === this.game.getAIPlayer()) {
                this.getAIMove();
            }
        }
    }

    async getAIMove() {
        this.isThinking = true;

        // Only show loading for Gomoku or if it takes time
        if (this.gameType === 'gomoku') {
            this.showLoading();
        }
        this.statusText.textContent = 'AI 思考中...';

        // Use a small timeout to allow UI update
        await new Promise(r => setTimeout(r, 50));

        const depth = parseInt(this.difficultySelect.value);

        // Get AI move from WASM
        const bestMove = this.game.getAIMove(depth);

        this.isThinking = false;
        this.hideLoading();

        if (bestMove.x !== -1 && bestMove.y !== -1) {
            this.makeMove(bestMove.x, bestMove.y);
        } else {
            this.statusText.textContent = 'AI 认输或无法移动';
        }
    }

    drawBoard() {
        const isLight = document.body.dataset.theme === 'light';
        const boardColor = isLight ? '#f3e5ab' : '#2a2a4a';
        const lineColor = isLight ? '#8b4513' : '#4a4a6a';

        const size = this.displaySize;
        this.ctx.clearRect(0, 0, size, size);

        // Draw background
        this.ctx.fillStyle = boardColor;
        this.ctx.fillRect(0, 0, size, size);

        // Draw grid lines
        this.ctx.beginPath();
        this.ctx.strokeStyle = lineColor;
        this.ctx.lineWidth = 1;

        const lines = this.gameType === 'gomoku' ? this.boardSize : this.boardSize + 1;
        for (let i = 0; i < lines; i++) {
            // Vertical
            this.ctx.moveTo(this.padding + i * this.cellSize, this.padding);
            this.ctx.lineTo(this.padding + i * this.cellSize, size - this.padding);

            // Horizontal
            this.ctx.moveTo(this.padding, this.padding + i * this.cellSize);
            this.ctx.lineTo(size - this.padding, this.padding + i * this.cellSize);
        }
        this.ctx.stroke();

        // Draw special points for Gomoku
        if (this.gameType === 'gomoku') {
            const points = [[3, 3], [11, 3], [7, 7], [3, 11], [11, 11]];
            this.ctx.fillStyle = lineColor;
            points.forEach(p => {
                this.ctx.beginPath();
                this.ctx.arc(this.padding + p[0] * this.cellSize, this.padding + p[1] * this.cellSize, 4, 0, Math.PI * 2);
                this.ctx.fill();
            });
        }

        // Draw pieces
        const board = this.game.getBoard();
        for (let i = 0; i < this.boardSize; i++) {
            for (let j = 0; j < this.boardSize; j++) {
                const val = board[i * this.boardSize + j];
                if (val !== 0) {
                    this.drawPiece(j, i, val);
                }
            }
        }
    }

    drawPiece(x, y, player) {
        let centerX, centerY;
        if (this.gameType === 'gomoku') {
            centerX = this.padding + x * this.cellSize;
            centerY = this.padding + y * this.cellSize;
        } else {
            centerX = this.padding + (x + 0.5) * this.cellSize;
            centerY = this.padding + (y + 0.5) * this.cellSize;
        }
        const radius = this.cellSize * 0.4;

        this.ctx.beginPath();
        const gradient = this.ctx.createRadialGradient(
            centerX - radius * 0.3, centerY - radius * 0.3, radius * 0.1,
            centerX, centerY, radius
        );

        if (player === 1) { // Black
            gradient.addColorStop(0, '#555');
            gradient.addColorStop(1, '#000');
        } else { // White
            gradient.addColorStop(0, '#fff');
            gradient.addColorStop(1, '#ccc');
        }

        this.ctx.fillStyle = gradient;
        this.ctx.shadowBlur = 5;
        this.ctx.shadowColor = 'rgba(0,0,0,0.3)';
        this.ctx.shadowOffsetY = 2;

        this.ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
        this.ctx.fill();

        this.ctx.shadowBlur = 0;
        this.ctx.shadowOffsetY = 0;
    }

    updateUI() {
        const player = this.game.getCurrentPlayer();
        this.currentPlayerText.textContent = player === 1 ? '⚫ 黑方 (你)' : '⚪ 白方 (AI)';
        this.statusText.textContent = '进行中';
    }

    endGame(msg) {
        this.isGameOver = true;
        this.statusText.textContent = msg;
        this.statusText.style.color = 'var(--accent-secondary)';
        setTimeout(() => alert(msg), 100);
    }

    showLoading() {
        this.loadingOverlay.classList.add('active');
    }

    hideLoading() {
        this.loadingOverlay.classList.remove('active');
    }
}

// Initialize when DOM loaded
window.addEventListener('DOMContentLoaded', () => {
    window.gameUI = new GameUI();
});
