var station1 = 'Wissahickon';
var station2 = 'Suburban%20Station';
var requests = 3;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getTrainInfo(direction) {
  // Determine the train direction
  var staStart, staEnd;
  if (direction === 0){
    staStart = station1; staEnd = station2;}
  else {
    staStart = station2; staEnd = station1;}
  console.log(direction +' ' + staStart + ' ' + staEnd);

  // Construct URL
  var URL = 'http://www3.septa.org/hackathon/NextToArrive/' + staStart + '/' + staEnd + '/' + requests;

  // Send request to OpenWeatherMap
  xhrRequest(URL, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      staStart = staStart.substr(0, 14).replace('%20',' ');
      staEnd = staEnd.substr(0, 14).replace('%20',' ');

      // Show to user
      if (json.length === 0){
        // Assemble dictionary using our keys
        var dictionary = {
          "KEY_TEMPERATURE": 'No trains are available at this time',
          "KEY_CONDITIONS": 'No delay'
        };
        
        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log("Weather info sent to Pebble successfully!");
          },
          function(e) {
            console.log("Error sending weather info to Pebble!");
          }
        );
      }
      else{
        showCard(0);
      }

      function showCard(trainNo){
        var delay = json[trainNo].orig_delay;
        var delayMins = delay.substr(0, delay.indexOf(' '));
        var bgColor;

        if (delayMins == 'On'){bgColor = 'green';}
          else if (delayMins > 0 && delayMins < 15){
            bgColor = 'yellow';
            delay = delay + ' late';}
          else if (delayMins > 0){
            bgColor = 'red';
            delay = delay + ' late';}
/*
        var card = new UI.Card({
          backgroundColor: bgColor,
          title: staStart,
          subtitle: '>> ' + json[trainNo].orig_departure_time + '  >>',
          body: 'Arrives: ' + json[trainNo].orig_arrival_time +
            '\n  at ' + staEnd +
            '\n' + delay
        });
        card.show();
*/
        var dictionary = {
          "KEY_TEMPERATURE": json[trainNo].orig_departure_time,
          "KEY_CONDITIONS": delay
        };
      
/*
      // Temperature in Kelvin requires adjustment
      var temperature = Math.round((json.main.temp - 273.15)*(9/5)+32);
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;
      console.log("Conditions are " + conditions);

      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions
      };
*/
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }
    }  
  );
}


// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getTrainInfo(0);
  }
);
