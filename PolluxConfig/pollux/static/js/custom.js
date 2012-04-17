function close_alerts() {
	$(".alert").fadeOut(1000);
}

$(document).ready(function() {
  $(".alert").alert();
  setTimeout('close_alerts()',3000);
});
