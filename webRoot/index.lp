<html>
<head>
	<title>Raspberry Telescope</title>
	<link href="style.css" rel="stylesheet" type="text/css" />
	<script>
		tickTime = new Date();
	
		function updateImage()
		{
			var image = document.getElementById("previewImage");
			if(image.complete) {
				image.src="/preview#" + new Date().getTime();
				
				var endTime = new Date();
			
				document.getElementById("tick").innerHTML = "" + (endTime.getTime() - tickTime.getTime())
				tickTime = endTime;
			}
			setTimeout(updateImage, 100);

		}
	
	</script>
</head>
<body onload="setTimeout(updateImage, 1000);">
<div class="menu">
<a href="/capture">Capture Now</a><p id="tick"></p>
</div>
<div>
	<img id="previewImage" src="/preview" />
</div>
</body>
</html>