$(document).ready(function(){

	// 등록
	$(".action_register").click(function(){
		$.register();
	});

    	// rows별 packet 전송
	$(".action_send_packet").click(function(){
		var idx = $(this).attr("idx");
		$.send_packet(idx);
	});

	// apply rule
	$(".action_apply_rule").click(function(){
		$.apply_rule();
	});

});