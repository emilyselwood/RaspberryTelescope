<html>
<head>
	<title>Raspberry Telescope</title>
	<link href="style.css" rel="stylesheet" type="text/css" />
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
<a href="/capture">Capture Now</a>
</div>
<div>
	<img id="previewImage" src="/preview" />
</div>
</body>
</html>