const net = require('net');

// 設定伺服器的 IP 和端口
const HOST = '0.0.0.0'; // 監聽所有可用的網絡接口
const PORT = 12345;

const server = net.createServer((socket) => {
  console.log('Client connected:', socket.remoteAddress, socket.remotePort);

  // 當接收到資料時，打印到控制台
  socket.on('data', (data) => {
    console.log('Received data:', data.toString());
  });

  // 當客戶端斷開連接時，打印信息
  socket.on('end', () => {
    console.log('Client disconnected');
  });

  // 處理錯誤
  socket.on('error', (err) => {
    console.error('Socket error:', err);
  });
});

// 啟動伺服器
server.listen(PORT, HOST, () => {
  console.log(`Server listening on ${HOST}:${PORT}`);
});

// 處理伺服器錯誤
server.on('error', (err) => {
  console.error('Server error:', err);
});