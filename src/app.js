var station1, station2; 
var requests = 1;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getTrainInfo() {
	// Construct URL
	station1 = station1.replace(' ', '%20');
	station2 = station2.replace(' ', '%20');
  	var URL = 'http://www3.septa.org/hackathon/NextToArrive/' + station1 + '/' + station2 + '/' + requests;
  	URL = URL.replace(/\s/g, '%20');
	//URL = encodeURIComponent(URL);
console.log(URL);
  // Send request to SEPTA
  	xhrRequest(URL, 'GET',
    	function(responseText) {
      		// responseText contains a JSON object with train info
			var json = JSON.parse(responseText);
			var dictionary;
      
      		// Show to user
      		if (json.length === 0){
        		// Assemble dictionary using our keys
        		dictionary = {
					"KEY_DEPART_TIME": 'No trains',
				  	"KEY_DELAY": '999',
				  	"KEY_ARRIVE_TIME": 'No trains'
        		};
      		} 
			else {
        		showCard(0);
      		}

      function showCard(trainNo){
        var delay = json[trainNo].orig_delay;
        var delayMins = delay.substr(0, delay.indexOf(' '));

        if (delayMins == 'On'){delayMins = '0';}
        else if (delayMins < 0){delayMins = '999';}

        dictionary = {
          "KEY_DEPART_TIME": json[trainNo].orig_departure_time,
          "KEY_DELAY": delayMins,
          "KEY_ARRIVE_TIME": json[trainNo].orig_arrival_time
        };
      }
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Train info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending train info to Pebble!");
        }
      );
    }  
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial train info
    // getTrainInfo();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    
    // Get the dictionary from the message
    var dict = e.payload;

    if(dict.KEY_STATION1) {
    // The AppKeyRequestData key is present, read the value
      station1 = dict.KEY_STATION1;
      console.log('station1: ' + station1);
    }
    
    if(dict.KEY_STATION2) {
    // The AppKeyRequestData key is present, read the value
      station2 = dict.KEY_STATION2;
      console.log('station2: ' + station2);
    }
    
    if (station1 && station2){
      getTrainInfo();
    }
  }
);

// Listen for when an AppMessage is received from the configuration page
Pebble.addEventListener('showConfiguration', function() {
	var url = 'http://1zentao1.wix.com/next2arrive';

	Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
	// Decode the user's preferences
	var configData = JSON.parse(decodeURIComponent(e.response));
	
	// Send to the watchapp via AppMessage
	var dict = {
	  'KEY_STATION1': configData.station1_input,
	  'KEY_STATION2': configData.station2_input
	};
	
	// Send to the watchapp
	Pebble.sendAppMessage(dict, function() {
	  console.log('Config data sent successfully!');
	}, function(e) {
	  console.log('Error sending config data!');
	});	
});
						
