$(document).ready(function(){

	
	$(".action_register").click(function(){
		$.register();
	});

    $(".action_send_packet").click(function(){
		var idx = $(this).attr("idx");
		$.send_packet(idx);
	});

    $(".action_apply_rule").click(function(){
		$.apply_rule();
	});

});

$.extend({
    "send_packet": function(idx){
		$.ajax({
			url: "/filter/list/send_packet",
			dataType: "json",
			data: {"idx": idx},
			type: "POST",
			success: function(res){

				if(res.result == "send success"){
					alert("Success");	
				}else{
					alert("fail");	
				}
			},error: function(res){
				console.log(res);	
			}
		});
	},

    "apply_rule": function()
	{
		$.ajax({
			url: "/filter/list/apply_rule",
			dataType: "json",
			type: "POST",
			success: function(res){
				if(res.result == "success"){
					alert("Success");	
				}else{
					alert("fail");	
				}
			},error: function(res){
				console.log(res);	
			}
		});
	}
});