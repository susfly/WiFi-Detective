var speedTest = require('speedtest-net');

console.log("start to test");

test = speedTest({maxTime: 5000});

test.on('data', function(data) {
  console.dir(data);
  });

test.on('error', function(err) {
    console.error(err);
});

test.on('downloadspeed', function(speed) {
  console.log('Download speed: ', speed.toFixed(2) + 'Mbps');
});

test.on('uploadspeed', function(speed) {
  console.log('Upload speed: ', speed.toFixed(2) + 'Mbps');
});

test.on('testserver', function(server) {
  console.log('Using server by ' + server.sponsor + ' in ' + server.name + ', ' + server.country + ' (' + (server.distMi * 0.621371).toFixed(0) + 'mi, ' + (server.bestPing).toFixed(0) + 'ms)');
});

test.on('downloadspeedprogress', function (speed) {
    console.log("downloadspeedprogress " + speed);
});

test.on('uploadspeedprogress', function (speed) {
    console.log("uploadspeedprogress " + speed);
});
test.on('downloadprogress', function(progress) {
    console.log('Download progress:', progress);
});
test.on('uploadprogress', function(progress) {
    console.log('Upload progress:', progress);
});
test.on('bestservers', function(servers) {
    console.log('Closest servers:');
    console.dir(servers);
});
test.on('done', function () {
    console.log("speedtest done");
});
