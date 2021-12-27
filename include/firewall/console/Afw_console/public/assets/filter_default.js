$(document).ready(function(){

	
	$(".action_register").click(function(){
		$.register();
	});

    $(".action_send_packet").click(function(){
		var idx = $(this).attr("idx");
		$.send_packet(idx);
	});

});