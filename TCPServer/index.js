const net = require('net');
const { MongoClient } = require('mongodb');
const moment = require('moment');

// 設定伺服器的 IP 和端口
const HOST = '0.0.0.0'; // 監聽所有可用的網絡接口
const PORT = 12345;

// MongoDB 連接設定
const mongoUrl = 'mongodb://localhost:27017';
const dbName = 'IOT';
const collectionName = 'GPS';

const convertDMToDD = (dm) => {
    const degrees = Math.floor(dm / 100);
    const minutes = dm % 100;
    return degrees + (minutes / 60);
  };
  
  let dbClient;
  let collection;
  
  const server = net.createServer((socket) => {
    console.log('Client connected:', socket.remoteAddress, socket.remotePort);
  
    socket.on('data', async (data) => {
      console.log('Received data:', data.toString());
  
      const dataString = data.toString();
      const match = dataString.match(/Latitude: (\d+\.\d+), Longitude: (\d+\.\d+), Speed: (\d+\.\d+) km\/h, Date: (\d+).*Time: (\d+\.\d+),.*Satellites: (\d+)/);
  
      if (match) {
        const [latitude, longitude, speed, date, time, satellites] = match.slice(1, 7);
  
        const latitudeDD = convertDMToDD(parseFloat(latitude));
        const longitudeDD = convertDMToDD(parseFloat(longitude));
        const timestamp = moment(`${date} ${time}`, 'DDMMYY HHmmss.SSS').add(8, 'hours').unix();
        const googleMapUrl = `https://www.google.com/maps?q=${latitudeDD},${longitudeDD}`;
  
        const document = {
          latitude: latitudeDD,
          longitude: longitudeDD,
          speed: parseFloat(speed),
          timestamp: timestamp,
          satellites: parseInt(satellites, 10),
          googleMap: googleMapUrl
        };
  
        try {
          await collection.insertOne(document);
          console.log('Data written to MongoDB');
        } catch (err) {
          console.error('MongoDB error:', err);
        }
      } else {
        console.error('Data format error: unable to parse received data');
      }
    });
  
    socket.on('end', () => {
      console.log('Client disconnected');
    });
  
    socket.on('error', (err) => {
      console.error('Socket error:', err);
    });
  });
  
  MongoClient.connect(mongoUrl)
    .then(client => {
      dbClient = client;
      const db = client.db(dbName);
      collection = db.collection(collectionName);
      server.listen(PORT, HOST, () => {
        console.log(`Server listening on ${HOST}:${PORT}`);
      });
    })
    .catch(err => {
      console.error('MongoDB connection error:', err);
    });
  
  server.on('error', (err) => {
    console.error('Server error:', err);
  });