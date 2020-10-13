 #ifndef ADMIN_SETTINGS_H
 #define ADMIN_SETTINGS_H
 #include "pgmspace.h"
const char* html_admin_page PROGMEM =R"rawliteral(
<!DOCTYPE html>
<html>

<head>
	<title>Connect Gaming Access Portal</title>
	<style>
		body {
			font-size: 25px;
		}

		button {
			font-size: 25px;
			padding-bottom: 5px;
		}

		#udtable {
			font-family: "Trebuchet MS", Arial, Helvetica, sans-serif;
			border-collapse: collapse;
			width: 100%;
			padding-top: 10px;
		}

		#udtable td,
		#udtable th {
			border: 1px solid #ddd;
			padding: 8px;
			text-align: center;
			min-width: 200px;
		}

		#udtable tr:nth-child(even) {
			background-color: #f2f2f2;
		}

		#udtable tr:hover {
			background-color: #ddd;
		}

		#udtable th {
			padding-top: 12px;
			padding-bottom: 12px;
			text-align: center;
			background-color: #4CAF50;
			color: white;
		}

		#myProgress {
			width: 90%;
			background-color: #ddd;
			text-align: left;
			border-radius: 20px;
		}

		#myBar {
			width: 0%;
			height: 30px;
			background-color: #4CAF50;
			border-radius: 20px;
		}
	</style>
</head>

<body>
	<center>
		Date:<input type="datetime-local" id="dtm" style="font-size: 25px;">
		<button id="dtm_btn" onclick="dtm_send()">SET</button>
		<br>
		<br>
		<br>
		<br>
		<table style="border-collapse: separate;
		border-spacing: 50px 0;">
			<th></th>
			<th></th>
			<tr>
				<td>
					<button onclick="get_usage_data()"
						style="background-color: green; min-height: 200; min-width: 200px;">GET DATA</button>
				</td>
				<td>
					<button onclick="clear_eeprom()"
						style="background-color: red; min-height: 200px; min-width: 200px;">CLEAR DATA</button>
				</td>
			</tr>
		</table>
		<br>
		<div id="myProgress"  >
			<div id="myBar"></div>
		</div>
	</center>
	<div id="udtable"></div>
	<script>
		var ajaxRequest;
		var WSocket;
		var WSConnected = false;
		if (window.XMLHttpRequest) {
			ajaxRequest = new XMLHttpRequest();
		}
		else {
			ajaxRequest = new ActiveXObject("Microsoft.XMLHTTP");
		}

		window.addEventListener('load', on_load_done);
		function on_load_done() {
			WS_msg_recvd(1);
			WSocket = new WebSocket('ws://' + window.location.hostname + ':81/');

			WSocket.binaryType = "arraybuffer";
			WSocket.onopen = function (e) {
				WSConnected = true;
			}
			WSocket.onclose = function (e) {
				WSConnected = false;
			}
			WSocket.onmessage = function (event) {
				WS_msg_recvd(event);
			}

			//get_station_data();
		}

		function WS_msg_recvd(event) {
			var arr = new Uint8Array(event.data);
			var arr_size = arr.byteLength;
			var td = '<td>';
			var tdc = '</td>';
			var ctr = 0;
			if (arr[0] == 6) //Get usage data command
			{
				document.getElementById("udtable").innerHTML = '';
				document.getElementById("udtable").innerHTML +=
					"<center><table ><tbody id='udata'><tr>" +
					"<th>Entry No.</th>" +
					"<th>Date</th>" +
					"<th>Station</th>" +
					"<th>Game</th>" +
					"<th>Time on</th>" +
					"<th>Paid $</th>" +
					"<th>Total $</th>" +
					"<th>Total Time</tr>";
				// Iterate incrementing by 10 == size of usage data stored in eeprom
				var curr_dt;
				var curr_mnth;
				var curr_sum = 0;
				var curr_ttl_time = 0;
				for (var i = 1; i + 10 < arr_size; i += 10) {
					// Check if we're on next day
					date_s = get_date_time_String(arr[i], arr[i + 1], arr[i + 2], arr[i + 3]);
					if (curr_dt != arr[i]) {
						curr_dt = arr[i] ;
						curr_mnth=arr[i+1];
						curr_sum = 0;
						curr_ttl_time=0;
					}
					curr_sum += arr[i + 7];
					curr_ttl_time += arr[i + 6] * 2;
					// Fill table with data
					document.getElementById("udata").innerHTML += '<tr>' +
						td + ctr++ + tdc +
						td + date_s + tdc +
						td + arr[i + 4].toString() + tdc +
						"<td class='game'>" + getGameName(arr[i + 5]) + tdc +		// Game
						td + (arr[i + 6] * 2).toString() + tdc + 					// Time
						td + arr[i + 7].toString() + tdc +							// Amount
						td + curr_sum.toString() + tdc + // Total $
						td + curr_ttl_time.toString() + tdc + // total time
						'</tr>';
			
				}
				document.getElementById("udata").innerHTML +=
					"</tbody></table></center>";
			}
			else if(arr[0] == 7) // Update progress bar
			{
				updateProgBar(arr[1]);
			}
		}
		function get_date_time_String(day, month, hr, min) {
			var retval = (day < 10) ? ("0" + day.toString()) : day.toString();
			retval += "/";
			retval += (month < 10) ? ("0" + month.toString()) : month.toString();
			retval += " ";
			retval += (hr < 10) ? ("0" + hr.toString()) : hr.toString();
			retval += ":";
			retval += (min < 10) ? ("0" + min.toString()) : min.toString();
			return retval;
		}

		function getGameName(game) {
			var retval = "Unknown";
			switch (game) {
				case 0:
					retval = "FIFA20";
					break;
				case 1:
					retval = "MK_X / Fighting";
					break;
				case 2:
					retval = "GTA V";
					break;
				case 3:
					retval = "CoD / War";
					break;
				case 4:
					retval = "Forza / Racing";
					break;
				case 5:
					retval = "Other";
					break;
				case 15:
					retval = "RESET"
					break;
			}
			return retval;
		}
		function WS_send(data) {
			WSocket.send(data);
		}

		function get_usage_data() {
			var data = new ArrayBuffer(1);
			var arr = new Uint8Array(data);
			arr[0] = 6;
			WS_send(data);
		}

		function clear_eeprom() {
			var password = prompt("Enter password:", "");
			if (password == "5yk@") {
				ajax_post("", "clear_eeprom", false);
			}
			else {
				navigator.vibrate([500]);
				alert("Wrong password!");
			}
		}

		function dtm_send() {
			var date_time = document.getElementById("dtm").value;
			if (date_time == '') {
				alert("Please pick a date");
				return;
			}

			ajax_post(date_time, "set_date_time", false);
		}

		function ajax_post(data, html_id, async) {
			if (!ajaxRequest) { alert("AJAX is not supported."); return; }
			ajaxRequest.open("POST", html_id, async);
			ajaxRequest.onreadystatechange = function () {
				if (ajaxRequest.readyState == 4 && ajaxRequest.status == 200) {
					alert(ajaxRequest.responseText);
				}
			}
			ajaxRequest.send(data);
		}
		function updateProgBar(width) {
			var elem = document.getElementById("myBar");
			elem.style.width = width + "%";
			elem.innerText=width.toString() + "%";
		}
	</script>
</body>

</html>

)rawliteral";
#endif
