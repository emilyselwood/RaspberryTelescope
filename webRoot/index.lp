<!doctype html>
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
			
			window.location.href = "/capture?" + returnValue;
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
			
			//$("#previewOptionsButton").set(":checked");
			
		});
	
	</script>
</head>
<body onload="setTimeout(updateImage, 1000);">
<div class="menu">
	<button id="captureButton" onclick="captureButtonClick" >Capture</button>
	Settings:<span id="optionsButtonSet">
		<input type="radio" name="optionsButtonSet" id="previewOptionsButton" checked="checked" /><label for="previewOptionsButton">Preview</label>
		<input type="radio" name="optionsButtonSet" id="captureOptionsButton" /><label for="captureOptionsButton">Capture</label>
		<input type="radio" name="optionsButtonSet" id="cameraOptionsButton" /><label for="cameraOptionsButton">Camera</label>
	</span>
	<span id="tick">-50</span>
</div>
<div id="previewDiv">
	<img id="previewImage" src="/preview" />
</div>
<div id="captureOptions">
	<input type="checkbox" id="immediatSetting" /><label for="immediatSetting">Show image immediately</label>
</div>
<div id="cameraOptions">
	<p>This is where the camera options will go. Things like ISO and exposure time.</p>
</div>
</body>
</html>