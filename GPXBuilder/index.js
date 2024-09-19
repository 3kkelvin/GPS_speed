const { MongoClient } = require('mongodb');
const fs = require('fs');

async function fetchDataAndConvertToGPX() {
  const uri = 'mongodb://localhost:27017';
  const client = new MongoClient(uri, { useNewUrlParser: true, useUnifiedTopology: true });

  try {
    await client.connect();
    const database = client.db('IOT');
    const collection = database.collection('GPS');
    
    const data = await collection.find({}).toArray();
    
    const gpxData = `
<gpx version="1.1" creator="3kkelvin's GPS">
  ${data.map(item => `
  <wpt lat="${item.latitude}" lon="${item.longitude}">
    <time>${new Date(item.timestamp * 1000).toISOString()}</time>
    <speed>${item.speed}</speed>
    <sat>${item.satellites}</sat>
  </wpt>`).join('')}
</gpx>`;

    fs.writeFileSync('output.gpx', gpxData.trim());
    console.log('GPX file has been created.');
  } finally {
    await client.close();
  }
}

fetchDataAndConvertToGPX().catch(console.error);