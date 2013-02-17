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

		function formatSize(s) {
			var sizes = ["B", "KB", "MB", "GB", "TB"];
			for (var i = 0; i < sizes.length; i++) {
				if(s < 1024) {
					return (Math.round(s * 10000) / 10000) + sizes[i]; // rounding to four dp.
				}
				s = s / 1024;
			}
		}
	
	    function formatDate(d) {
			var da = new Date(d * 1000);
			return da.toISOString();
	    }
	
		function populateList(data) {
		
			var sortedList = data.files.sort(function(a,b) { return b.date - a.date; });
			$.each(sortedList, function(index, image) {
				$("#imageTable tbody").append("<tr><td><a href=\"/image?n=" + image.name + "\"><img src=\"/image?n=" + image.name + "\" height=\"100px\" width=\"100px\"/></a></td><td>" + image.name +
					"</td><td>" + formatDate(image.date) + 
					"</td><td>"+formatSize(image.size) + 
					"</td></tr>")
			});
		}
	
		$(function () {
			$.getJSON("/listimages", populateList);
		});
	</script>
</head>
<body>
	<div id="listContainer">
		<table id="imageTable">
			<thead>
				<th>
				</th>
				<th>
					Name
				</th>
				<th>
					Date
				</th>
				<th>
					Size
				</th>
			</thead>
			<tbody>
			</tbody>
		</table>
	</div>
</body>
</html>