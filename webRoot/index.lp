<!doctype html><?
	function findParam(request, param, default)
		local start = string.find(request, param) + 2
		if not start
		then
			return default
		else
			local param_end = string.find(request, '&', start)
			if not param_end
			then 
				return string.sub(request, start) 
			else
				return string.sub(request, start, param_end - 1)
			end
		end
	end
?>
<html>
<head>
	<meta charset="utf-8">
	<title>Raspberry Telescope</title>
	<link href="style.css" rel="stylesheet" type="text/css" />
	<!-- note we keep a local copy of these incase we are running with out an internet connection -->
	<link href="jquery-ui/jquery-ui-1.10.0.custom.css" rel="stylesheet" type="text/css" />
	<script src="jquery-ui/jquery-1.9.0.js"></script>
	<script src="jquery-ui/jquery-ui-1.10.0.custom.js"></script>
	
	<script>
		tickTime = new Date();
	
		function updateImage()
		{
			var image = document.getElementById("previewImage");
			if(image.complete) {
				image.src="/preview#" + new Date().getTime();
				
				var endTime = new Date();
			
				$("#tick").text ("" + (endTime.getTime() - tickTime.getTime()) + "ms");
				tickTime = endTime;
			}
			setTimeout(updateImage, 500);

		}
		
		function captureButtonClick() {
			var returnValue = ""
			if($("#immediatSetting").is(':checked')) {
				returnValue = "r=1"
			}
			else {
				returnValue = "r=0"
			}
			var fileName = $.trim($("#captureName").val())
			if(fileName != "") {
				fileName = "&n=" + fileName
			}
			var deleteValue = "&d=0"
			if($("#deleteSetting").is(":checked")) {
				deleteValue = "&d=1"
			}
			var captureCountValue = $.trim($("#captureCount").val())
			if(captureCountValue != "") {
				captureCountValue = "&i=" + captureCountValue
			}
			
			window.location.href = "/capture?" + returnValue + fileName + deleteValue + captureCountValue;
		}
		
		function optionsButtonClick() {
			// hide every thing then show the one we want about.
			$("#previewDiv").hide();
			$("#captureOptions").hide();
			$("#cameraOptions").hide();
			
			if($("#previewOptionsButton").is(":checked")) {
				$("#previewDiv").show();
			}
			else if($("#captureOptionsButton").is(":checked")) {
				$("#captureOptions").show();
			}
			else if($("#cameraOptionsButton").is(":checked")) {
				$("#cameraOptions").show();
			}
			else {
				$("#previewDiv").show();
			}
		}
		
		function isoChange() {
			var newValue = $("#isoSelect").find(":selected").text();
			$.ajax("/setsetting?k=iso&v="+newValue);
		}
			
		function shutterspeedChange() {
			var newValue = $("#shutterSpeedSelect").find(":selected").text();
			$.ajax("/setsetting?k=shutterspeed&v="+newValue);
		}
			
		function apertureChange() {
			var newValue = $("#apertureSelect").find(":selected").text();
			$.ajax("/setsetting?k=aperture&v="+newValue);
		}
			
		function whitebalanceChange() {
			var newValue = $("#whiteBalanceSelect").find(":selected").text();
			$.ajax("/setsetting?k=whitebalance&v="+newValue);
		}
		
		function populateSelect(name, choices, selected) {
			$(name).empty();
			$.each(choices, function(index, choiceValue) {
				$(name).append($('<option>', { value : index })
					.text(choiceValue));
			});
			
			$(name + " option").filter(function() {
				return $(this).text() == selected;
			}).attr('selected', true);
		}
		
		function handleJSONSettings(data) {
			populateSelect('#isoSelect', data.main.imgsettings.iso.choices, data.main.imgsettings.iso.value);
			populateSelect('#shutterSpeedSelect', data.main.capturesettings.shutterspeed.choices, data.main.capturesettings.shutterspeed.value);
			populateSelect('#apertureSelect', data.main.capturesettings.aperture.choices, data.main.capturesettings.aperture.value);
			populateSelect('#whiteBalanceSelect', data.main.imgsettings.whitebalance.choices, data.main.imgsettings.whitebalance.value);
		
			$("#isoSelect").change(isoChange);
			$("#shutterSpeedSelect").change(shutterspeedChange);
			$("#apertureSelect").change(apertureChange);
			$("#whiteBalanceSelect").change(whitebalanceChange);
		}
		
		$(function() {
			$( "#captureButton" )
				.button()
				.click(captureButtonClick);
			$("#previewOptionsButton")
				.button()
				.click(optionsButtonClick);
			$("#captureOptionsButton")
				.button()
				.click(optionsButtonClick);
			$("#cameraOptionsButton")
				.button()
				.click(optionsButtonClick);
				
			$("#optionsButtonSet").buttonset();
			
			$("#previewDiv").show();
			$("#captureOptions").hide();
			$("#cameraOptions").hide();
			
			setTimeout(updateImage, 1000);
			
			$.getJSON("/settings", handleJSONSettings);
		});
	
	</script>
</head>
<body>
	<div class="menu">
		<button id="captureButton" onclick="captureButtonClick" >Capture</button>

		<input type="radio" name="optionsButtonSet" id="previewOptionsButton" checked="checked" /><label for="previewOptionsButton">Preview</label>
		Settings:<span id="optionsButtonSet">
			<input type="radio" name="optionsButtonSet" id="captureOptionsButton" /><label for="captureOptionsButton">Capture</label>
			<input type="radio" name="optionsButtonSet" id="cameraOptionsButton" /><label for="cameraOptionsButton">Camera</label>
		</span>
		<span id="tick">-50</span>
	</div>
	<div id="previewDiv">
		<img id="previewImage" src="/preview" />
	</div>
	<div id="captureOptions" class="optionsTab">
		<div class="setting">
			<label for="immediatSetting">Show image immediately</label>
			<input type="checkbox" id="immediatSetting" <? if string.find(request_info.query_string, 'r=1') then print('checked=\"checked\"') end ?>/>
		</div>
		<div class="setting">
			<label for="immediatSetting">Delete from camera</label>
			<input type="checkbox" id="deleteSetting" <? if string.find(request_info.query_string, 'd=1') then print('checked=\"checked\"') end ?>/>
		</div>
		<div class="setting">
			<label for="captureName">File Name:</label>
			<input type="text" <? print('value=\"' .. findParam(request_info.query_string, 'n=', '') .. '\"') ?> id="captureName" />
			Warning if the file already exists it will be overwritten. Leave blank for a date stamped file name.
		</div>
		<div class="setting">
			<label for="captureCount">Number of frames:</label>
			<input type="text" <? print('value=\"' .. findParam(request_info.query_string, 'i=', '1') .. '\"') ?> id="captureCount" />
			Instruct the camera to take this many pictures as fast as possible. A number will be appended to the file name along with the image file extention.
		</div>
	</div>
	<div id="cameraOptions" class="optionsTab">
		<div class="setting">
			<label for="isoSelect">ISO</label>
			<select id="isoSelect"></select>
		</div>
		<div class="setting">
			<label for="shutterSpeedSelect">Shutter Speed</label>
			<select id="shutterSpeedSelect"></select>
		</div>
		<div class="setting">
			<label for="apertureSelect">Aperture</label>
			<select id="apertureSelect"></select>
		</div>
		<div class="setting">
			<label for="whiteBalanceSelect">White Balance</label>
			<select id="whiteBalanceSelect"></select>
		</div>
	</div>
</body>
</html>