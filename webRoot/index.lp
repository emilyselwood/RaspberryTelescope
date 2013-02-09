<!doctype html>
<?
	function findNameParam(request)
		print('hello')
		local start = string.find(request, 'n=')
		if not start
		then
			return '""'
		else
			local end = string.find(request, '&', start)
			if not end
			then 
				return string.sub(request, start)
			else
				return string.sub(request, start, end)
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
			
			window.location.href = "/capture?" + returnValue + fileName + deleteValue;
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
	<div><label for="immediatSetting">Show image immediately</label><input type="checkbox" id="immediatSetting" <? if string.find(request_info.query_string, 'r=1') then print('checked=\"checked\"') end ?>/></div>
	<div><label for="immediatSetting">Delete from camera</label><input type="checkbox" id="deleteSetting" <? if string.find(request_info.query_string, 'd=1') then print('checked=\"checked\"') end ?>/></div>
	<div><label for="captureName">File Name:</label><input type="text" <?
		local start = string.find(request_info.query_string, 'n=')
		if not start
		then
			print( 'value=\"\"' )
		else
			local param_end = string.find(request_info.query_string, '&', start + 1)
			if not param_end
			then 
				print( 'value=', string.sub(request_info.query_string, start + 2) )
			else
				print( 'value=', string.sub(request_info.query_string, start + 2, param_end - 1) )
			end
		end
	?> id="captureName" />Warning if the file already exists it will be overwritten. Leave blank for a date stamped file name.</div>
</div>
<div id="cameraOptions">
	<p>This is where the camera options will go. Things like ISO and exposure time.</p>
</div>
</body>
</html>