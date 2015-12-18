Pebble.addEventListener('ready', function(e) {
});

Pebble.addEventListener('showConfiguration', function(e) {
  console.log("showConfiguration()");
  
  var settings = localStorage.getItem("settings");
  var dateformat = 0;
  if (typeof(settings) == "string" && settings != "CANCELLED") {
    console.log("parsing settings: " + settings);
    var values = JSON.parse(settings);
    dateformat = values['0'];
  }
  Pebble.openURL('http://data.lingen.me/pebble/retroclock/config-2.0.html?dateformat=' + dateformat);
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log("webviewclosed()"); 
  if ((typeof(e.response) == 'string') && (e.response.length > 0)) {
    console.log("settings changed: " + e.response);

    if (e.response == "CANCELLED") return;
    if (e.response == "{}" ) return;

    localStorage.setItem("settings", e.response);
    try {
      Pebble.sendAppMessage(JSON.parse(e.response));
    } catch(e) {
    }
  }
});
