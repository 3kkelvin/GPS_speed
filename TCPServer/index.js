const express = require('express');
const { MongoClient } = require('mongodb');
const moment = require('moment');

const app = express();
app.use(express.json());

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

app.post('/gps', async (req, res) => {
  const { latitude, longitude, speed, date, time, satellites } = req.body;

  console.log('Received JSON:', req.body);
  if (!latitude || !longitude || !speed || !date || !time || !satellites) {
    return res.status(400).send('Invalid data format');
  }

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
    res.status(200).send('Data written to MongoDB');
  } catch (err) {
    console.error('MongoDB error:', err);
    res.status(500).send('Internal Server Error');
  }
});
app.get('/', (req, res) => {
    console.log('Some one GET');
    res.status(200).send('OK');
  });

MongoClient.connect(mongoUrl)
  .then(client => {
    dbClient = client;
    const db = client.db(dbName);
    collection = db.collection(collectionName);
    app.listen(12345, () => {
      console.log('Server listening on port 12345');
    });
  })
  .catch(err => {
    console.error('MongoDB connection error:', err);
  });