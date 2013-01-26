<html>
<head>
	<title>Raspberry Telescope</title>
	<script>
	
		function updateImage()
		{
			var image = document.getElementById("previewImage");
			if(image.complete) {
				image.src="/preview#" + new Date().getTime();
			}
			setTimeout(updateImage, 500);
		}
	
	</script>
</head>
<body onload="setTimeout(updateImage, 1000);">
<div class="menu">
Menu woot <a href="/capture">Capture Now</a>
</div>
<div>
	<img id="previewImage" src="/preview" />
</div>
</body>
</html>