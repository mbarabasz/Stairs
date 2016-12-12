var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);
var url = 'http://stairs.objectivity.co.uk/api/';

function showStats(username){
  var req = new XMLHttpRequest();
  req.onreadystatechange = function() {
    if (req.readyState == XMLHttpRequest.DONE) {
      Pebble.showSimpleNotificationOnPebble('Success', req.responseText);
    }
  }
  req.open('GET', url + 'User/' + username + '/all', true);
  req.send(null);
}


function sendRequest(stairs, username) {
  var req = new XMLHttpRequest();
  req.open('POST', url + 'stairs', true);
  req.setRequestHeader('Content-Type', 'application/json');
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        //console.log(req.responseText);
        console.log('Call success');
      } else if (req.status === 201){
        console.log('Created');
        // Show the notification
        // Pebble.showSimpleNotificationOnPebble('Success', 'Saved');
        showStats(username);
      } else {
        console.log('Response code: ' + req.status);
        Pebble.showSimpleNotificationOnPebble('Error', 'Response code: ' + req.status);
      }
    }else{
      console.log('Bad status');
      Pebble.showSimpleNotificationOnPebble('Error', 'Bad status');
    }
  };
  var payload = '{\"UserName\": \"' + username + '\",\"Count\": ' + stairs + '}';
  console.log('payload sent to api: ' + payload);
  req.send(payload);
}

Pebble.addEventListener('ready', function (e) {
  console.log('connect!' + e.ready);
  console.log(e.type);
});

Pebble.addEventListener('appmessage', function (e) {
  console.log('message comming from C:' + e.type + ',' + e.payload[1] + ',' + e.payload[2]);
  if ( e.payload[2] === 'undefined' ||  e.payload[2] === ''){
    Pebble.showSimpleNotificationOnPebble('Error', 'Please provide username in application settings');
  }else{
    sendRequest(e.payload[1], e.payload[2]);
  }
});