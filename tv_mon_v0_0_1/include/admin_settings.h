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
	</style>
</head>

<body>
	<center>
		Date:<input type="datetime-local" id="dtm" style="font-size: 25px;">
		<button id="dtm_btn" onclick="dtm_send()">SET</button>
		<br>
		<br>
		<button onclick="get_usage_data()">GET USAGE DATA</button>
		<br>
		<br>
		<button onclick="clear_eeprom()" style="background-color: red;">CLEAR EEPROM</button>
	</center>
	</center>
	<script>
	 var ajaxRequest = null;
		if (window.XMLHttpRequest) {
			ajaxRequest = new XMLHttpRequest();
		}
		else {
			ajaxRequest = new ActiveXObject("Microsoft.XMLHTTP");
		}

		function get_usage_data() {
			alert("Function ot yet supported.");
		}

		function clear_eeprom() {
			ajax_post("", "clear_eeprom", false);
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
	</script>
</body>

</html>
)rawliteral";
#endif