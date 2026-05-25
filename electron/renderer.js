const { ipcRenderer } = require('electron');

const boardElement = document.getElementById('board');
const statusElement = document.getElementById('status');
const overlay = document.getElementById('overlay');
const winnerText = document.getElementById('winner-text');
const gameSelect = document.getElementById('game-select');

let SIZE = 15;
let board = [];
let gameOver = false;
let aiThinking = false;
const AI_LEVEL = 100;

function initBoard() {
  boardElement.innerHTML = '';
  boardElement.style.gridTemplateColumns = `repeat(${SIZE}, 30px)`;
  boardElement.style.gridTemplateRows = `repeat(${SIZE}, 30px)`;
  board = Array(SIZE).fill(null).map(() => Array(SIZE).fill(0));

  for (let r = 0; r < SIZE; r++) {
    for (let c = 0; c < SIZE; c++) {
      const cell = document.createElement('div');
      cell.className = 'cell';
      cell.dataset.row = r;
      cell.dataset.col = c;
      cell.onclick = () => handleCellClick(r, c);

      if (SIZE < 5) {
        cell.style.width = '100px';
        cell.style.height = '100px';
        boardElement.style.gridTemplateColumns = `repeat(${SIZE}, 100px)`;
        boardElement.style.gridTemplateRows = `repeat(${SIZE}, 100px)`;
      }

      boardElement.appendChild(cell);
    }
  }
}

function handleCellClick(r, c) {
  if (gameOver || aiThinking || board[r][c] !== 0) return;

  placePiece(r, c, 1);
  statusElement.innerText = 'AI is thinking...';
  aiThinking = true;

  ipcRenderer.send('send-to-ai', `move ${r} ${c} 1`);
  ipcRenderer.send('send-to-ai', `level ${AI_LEVEL}`);
}

function changeGame() {
  const game = gameSelect.value;
  ipcRenderer.send('send-to-ai', `switch ${game}`);
}

function placePiece(r, c, player) {
  board[r][c] = player;
  const cell = boardElement.children[r * SIZE + c];
  const piece = document.createElement('div');
  piece.className = `piece ${player === 1 ? 'black' : 'white'}`;

  if (SIZE < 5) {
    piece.style.width = '80px';
    piece.style.height = '80px';
  }

  const lastAIPiece = document.querySelector('.ai-last-move');
  if (lastAIPiece) lastAIPiece.classList.remove('ai-last-move');
  if (player === 2) piece.classList.add('ai-last-move');

  cell.appendChild(piece);
}

function resetGame() {
  gameOver = false;
  aiThinking = false;
  overlay.style.display = 'none';
  statusElement.innerText = 'Your Turn';
  initBoard();
  ipcRenderer.send('send-to-ai', 'reset');
}

ipcRenderer.on('ai-response', (event, response) => {
  const parts = response.trim().split(' ');
  const cmd = parts[0];

  if (cmd === 'switched') {
    SIZE = parseInt(parts[2], 10);
    resetGame();
  } else if (cmd === 'best') {
    const r = parseInt(parts[1], 10);
    const c = parseInt(parts[2], 10);
    placePiece(r, c, 2);
    aiThinking = false;
    statusElement.innerText = 'Your Turn';
  } else if (cmd === 'win') {
    const winner = parseInt(parts[1], 10);
    gameOver = true;
    aiThinking = false;
    winnerText.innerText = winner === 1 ? 'You Win!' : 'AI Wins!';
    overlay.style.display = 'flex';
  } else if (cmd === 'draw') {
    gameOver = true;
    aiThinking = false;
    winnerText.innerText = "It's a Draw!";
    overlay.style.display = 'flex';
  } else if (cmd === 'error') {
    aiThinking = false;
    statusElement.innerText = 'AI error';
  }
});

initBoard();
