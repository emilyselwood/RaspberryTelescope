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
		$(function() {
			$( "#retry" ).button();
			$("#gallery").button();
		});
	</script>
</head>
<body>
	<div class="noCamMessage">Could not detect a camera. Make sure the camera is connected and turned on.</div>
	<div class="retryButton"><a href="/" id="retry">Retry</a><a id="gallery" href="/gallery.lp">Gallery</a></div>
</body>
</html>
 