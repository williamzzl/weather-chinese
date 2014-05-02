function getAndShowWeather () {
  navigator.geolocation.getCurrentPosition(function (position) {
    getCurrentWeather(position.coords.longitude, position.coords.latitude);
  });

  navigator.geolocation.getCurrentPosition(function (position) {
    getForcastWeather(position.coords.longitude, position.coords.latitude);
  });

  setTimeout(getAndShowWeather, 3600000);
}

function getCurrentWeather(lon, lat) {
  var req = new XMLHttpRequest();
    req.open('GET',"http://api.openweathermap.org/data/2.5/weather?lat=" + lat + "&lon=" + lon);
    req.onload = function(e) {
      if (req.readyState == 4 && req.status == 200) {
        if (req.status == 200) {
          var response = JSON.parse(req.responseText);

          var send = {};
          var delta = 273.15;
          send.temp = Number(response.main.temp - delta).toFixed(0);
          Pebble.sendAppMessage(send);
        } else {
          console.log("Error");
        }
      }
    };
    req.send(null);
}

function getForcastWeather(lon, lat) {
  var req = new XMLHttpRequest();
    req.open('GET',"http://api.openweathermap.org/data/2.5/forecast/daily?lat=" + lat + "&lon=" + lon + "&lang=zh_cn&cnt=3&mode=json");
    req.onload = function(e) {
      if (req.readyState == 4 && req.status == 200) {
        if (req.status == 200) {
          var response = JSON.parse(req.responseText);

          var send = {};
          var delta = 273.15;
          send.temp_min_day1 = Number(response.list[0].temp.min - delta).toFixed(0);
          send.temp_max_day1 = Number(response.list[0].temp.max - delta).toFixed(0);
          send.icon_day1 = response.list[0].weather[0].icon;
          send.desc_day1 = response.list[0].weather[0].description;
          send.temp_min_day2 = Number(response.list[1].temp.min - delta).toFixed(0);
          send.temp_max_day2 = Number(response.list[1].temp.max - delta).toFixed(0);
          send.icon_day2 = response.list[1].weather[0].icon;
          send.desc_day2 = response.list[1].weather[0].description;

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