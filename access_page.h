#ifndef ACCESS_PAGE_H
#define ACCESS_PAGE_H
#include "Arduino.h"
const char http_head[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
  <title>Connect Gaming Access Portal</title>
  <style>
    body,
    html {

      font-family: 'Lato', sans-serif;
    }

    h1 {
      text-align: center;
      color: lightseagreen;
      padding: auto;
      font-size: 80px;
    }

    h2 {
      text-align: center;
      color: lightseagreen;
      font-size: 60px;
      border-bottom: 5px solid white;
      width: 100%;
      padding: auto;
    }

    div.round {
      border: 2px solid grey;
      border-radius: 20px;
      display: inline-block;
      font-size: 22px;
      width: 100%;
      height: 100%;
      min-height: 100%;
      max-width: 800px;
      background: #222222;
      box-shadow: 0 8px 16px 0 rgba(88, 78, 78, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
      text-align: center
    }

    /* Hide the browser's default radio button */
    .container input[type=radio] {
      position: absolute;
      opacity: 50;
      cursor: pointer;
    }

    /* Create a custom radio button */
    .checkmark {
      position: absolute;
      top: 0;
      left: 0;
      height: 25px;
      width: 25px;
      background-color: #eee;
      border-radius: 50%;
    }

    /* On mouse-over, add a grey background color */
    .container:hover input~.checkmark {
      background-color: #ccc;
    }

    /* When the radio button is checked, add a blue background */
    .container input:checked~.checkmark {
      background-color: #2196F3;
    }

    /* Create the indicator (the dot/circle - hidden when not checked) */
    .checkmark:after {
      position: absolute;
      display: none;
    }

    /* Show the indicator (dot/circle) when checked */
    .container input:checked~.checkmark:after {
      display: block;
    }

    /* Style the indicator (dot/circle) */
    .container .checkmark:after {
      top: 9px;
      left: 9px;
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: white;
    }

    .button {
      border: none;
      color: white;
      padding: 16px 32px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 64px;
      margin: 4px 2px;
      -webkit-transition-duration: 0.5s;

      /* Safari */
      transition-duration: 0.5s;
      cursor: pointer;
      border-radius: 8px;
    }

    .button6 {
      background-color: white;
      color: rgb(72, 255, 0);
      border: 4px rgb(72, 255, 0);
      padding: 10px;
      width: 98%;
      min-height: 100px;
      max-height: 300%;
    }

    .button6:hover {
      background-color: rgb(72, 255, 0);
      color: white;
    }

    .container {
      display: block;
      position: relative;
      margin: auto auto;
      height: 100%;
      min-height: max-content;
      width: 100%;
      padding: auto;

    }

    .container ul {
      list-style: none;
      margin: 0;
      padding: auto;
      overflow: auto;
    }

    ul li {
      color: #AAAAAA;
      display: block;
      position: relative;
      float: left;
      width: 100%;
      height: auto;
    }

    ul li input[type=radio] {
      position: absolute;
      visibility: hidden;
    }

    ul li label {
      display: block;
      position: relative;
      font-weight: 300;
      font-size: 40px;
      padding: 5px 5px 5px 80px;
      margin: 0px auto;
      height: auto;
      z-index: 9;
      cursor: pointer;
      -webkit-transition: all 0.25s linear;
    }

    ul li:hover label {
      color: #FFFFFF;
    }

    ul li .check {
      display: block;
      position: absolute;
      border: 5px solid #AAAAAA;
      border-radius: 100%;
      height: 25px;
      width: 25px;
      top: 2px;
      left: 20px;
      z-index: 5;
      transition: border .25s linear;
      -webkit-transition: border .25s linear;
    }

    ul li:hover .check {
      border: 5px solid #FFFFFF;
    }

    ul li .check::before {
      display: block;
      position: absolute;
      conten
    td {
      padding: 5px;
      text-align: left;
      min-width: 75px;
    }

    th.sdata {
      padding: 8px;
      text-align: center;
      color: white;
      border-bottom: 1px solid #ddd;
    }

    div.sdata {
      border: 2px solid grey;
      border-radius: 15px;
      display: inline-block;
      font-size: 22px;
      width: 95%;
      background: #222222;
      box-shadow: 0 8px 16px 0 rgba(88, 78, 78, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
      text-align: center;
      padding-bottom: 3px;

    }

    table.sdata {
      width: 100%;
    }

    #st1_tm,
    #st2_tm {
      color: white;
      font-size: 500%;
    }

    #RS1,
    #RS2 {
      width: 98%;
      min-height: 50px;
      min-width: 50px;
      border-radius: 10px;
      font-size: 300%;
      color: white;
      background-color: rgb(228, 30, 30);
      box-shadow: 0 4px #999;
    }

    #RS1:active,
    #RS2:active {
      background-color: rgb(228, 30, 30);
      box-shadow: 0 4px #666;
      transform: translateY(4px);
    }
  </style>
</head>

<body>
  <center>
    <div class='round'>
      <div class='container'>

        <h1>CONNECT GAMING </h1>

        <h2> Access Portal</h2>
        <div class='sdata'>
          <span style="color: white;font-size: 30px;" id="date_time">DD/MM/YY HH:MM</span>
          <table class='sdata'>
            <tr>
              <th class='sdata'>STATION 1 CONTROL </th>
              <th class='sdata'>STATION 2 CONTROL</th>
            </tr>
            <tr>
              <td style='text-align:center'>
                <span id='st1_tm'>000</span>
                <span style='font-size:30px; color:white;'>min</span>
              </td>
              <td style='text-align:center'>
                <span id='st2_tm'>000</span>
                <span style='font-size:30px; color:white;'>min</span>
              </td>
            </tr>
            <tr>
              <td><button id='RS1'>RESET</button></th>
              <td><button id='RS2'>RESET</button></th>
            </tr>
          </table>
        </div>
        <p></p>
        <form action='/data_submit' method='POST' onsubmit='return validateInput()'>
          <div style='font-size:200%;color:white;'>
            <label for='time'>Time(min) :</label>
            <input type='number' id='time' name='time' min='0' max='510' step='5' value='5'
              style='font-size:100%;border-radius:10px;max-width:50%;text-align: center;' required>
          </div>

          <ul>
            <h3 style='color:white;font-size: 35px'>
              Select Station
            </h3>
            <center>
              <table>
                <tr>
                  <td>
                    <li>
                      <input type='radio' id='Station_1' name='station' required>
                      <label for='Station_1'>Station 1</label>
                      <div class='check'></div>
                    </li>
                  </td>
                  <td>
                    <li>
                      <input type='radio' id='Station_2' name='station'>
                      <label for='Station_2'>Station 2</label>
                      <div class='check'></div>
                    </li>
                  </td>
                </tr>
              </table>
            </center>
            <table style="width:100%; padding-left:1%;">
              <h3 style='color:white;font-size: 35px'>Select Game
                <tr>
                  <td>
                    <li>
                      <input type='radio' id='fifa' name='games' required>
                      <label for='fifa'>FIFA 20</label>
                      <div class='check'></div>
                    </li>
                  </td>
                  <td>
                    <li>
                      <input type='radio' id='mk' name='games'>
                      <label for='mk'>MK X / Injustice </label>
                      <div class='check'>
                        <div class='inside'></div>
                      </div>
                    </li>
                  </td>
                </tr>
                <tr>
                  <td>
                    <li>
                      <input type='radio' id='gtav' name='games'>
                      <label for='gtav'>GTAV / COD</label>
                      <div class='check'>
                        <div class='inside'></div>
                      </div>
                    </li </td> <td>
                    <li>
                      <input type='radio' id='other' name='games'>
                      <label for='other'>Other</label>
                      <div class='check'>
                        <div class='inside'></div>
                      </div>
                    </li>
                  </td>
                </tr>
            </table>
          </ul>
          <input type='submit' value='S T A R T' class='button button6'>
        </form>
      </div>
    </div>
    <script>
      var ajaxRequest = null;
      if (window.XMLHttpRequest) {
        ajaxRequest = new XMLHttpRequest();
      }
      else {
        ajaxRequest = new ActiveXObject("Microsoft.XMLHTTP");
      }

      window.addEventListener('load', on_load_done);
      setInterval(on_load_done(), 20000);

      function on_load_done() {
        getData("st1_tm", "st1_tm", false);
        getData("st2_tm", "st2_tm", false);
        getData("date_time", "date_time", true);
      }

      function validateInput() {
        // Check station radio button input
        var temp = document.getElementsByName("station");
        var chkdStation;
        var res = false;
        for (var i = 0; i < temp.length; i++) {
          if (temp[i].checked) {
            chkdStation = i.toString();
            res = true;
          }
        }
        if (!res) alert("Please select a Station");

        var checkdGame;
        temp = document.getElementsByName("games");
        for (var i = 0; i < temp.length; i++) {
          if (temp[i].checked) {
            checkdGame = i.toString();
            res = true;
          }
        }
        if (!res) alert("Please select a game");
        ajaxRequest.open("POST", "gameData", false);
        ajaxRequest.onreadystatechange = function () {
          if (ajaxRequest.readyState == 4 && ajaxRequest.status == 200) {
            alert(ajaxRequest.responseText);
          }
        }
        var p_time = document.getElementById("time").value.toString();
        var data = "time=" + p_time.toString() + "&" + "game=" + checkdGame + "&" + "staxn="+chkdStation;
        ajaxRequest.send(data);
        return false;
      }

      function getData(data, html_id, async) {
        if (!ajaxRequest) { alert("AJAX is not supported."); return; }
        ajaxRequest.open("GET", data, async);
        ajaxRequest.onreadystatechange = function () {
          if (ajaxRequest.readyState == 4 && ajaxRequest.status == 200) {
            document.getElementById(html_id).innerHTML = ajaxRequest.responseText;
          }
        }
        ajaxRequest.send();
      }

    </script>
  </center>
</body>

</html>

)rawliteral";
#endif
