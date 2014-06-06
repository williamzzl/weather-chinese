function getAndShowWeather () {
  navigator.geolocation.getCurrentPosition(function (position) {
    getWeather(position.coords.longitude, position.coords.latitude);
  });

  setTimeout(getAndShowWeather, 300000);
}

function getWeather(lon, lat) {
  console.log("lon:",lon);
  console.log("lat:",lat);
  var req = new XMLHttpRequest();
    req.open('GET',"http://pebbleengine.sinaapp.com?location=" + lat + "," + lon);
    req.onload = function(e) {
      if (req.readyState == 4 && req.status == 200) {
        if (req.status == 200) {
          var response = JSON.parse(req.responseText);

          var send = {};
          send.temp = Number(response.curTemp).toFixed(0);
          send.temp_min_day1 = Number(response.today.tempFrom).toFixed(0);
          send.temp_max_day1 = Number(response.today.tempTo).toFixed(0);
          send.icon_day1 = response.today.icon;
          send.desc_day1 = response.today.weather;
          send.temp_min_day2 = Number(response.tomorrow.tempFrom).toFixed(0);
          send.temp_max_day2 = Number(response.tomorrow.tempTo).toFixed(0);
          send.icon_day2 = response.today.icon;
          send.desc_day2 = response.today.weather;
          send.pm25 = Number(response.pm25).toFixed(0)
          send.ts = response.ts;

          Pebble.sendAppMessage(send);
        } else {
          console.log("Error");
        }
      }
    };

    req.send(null);
}

Pebble.addEventListener("ready",
  function(e) {
    setTimeout(getAndShowWeather, 2000);
  }
);
