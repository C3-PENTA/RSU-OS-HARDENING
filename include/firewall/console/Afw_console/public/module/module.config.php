<?php
return array(
    "routes" => array(
        "route" => "/rule",

        "constraints" => array(
            "module",
            "controller",
            "action",
        ),

        "defaults" => array(
            "controller"    => "index",
            "action"        => "index",
        ),
    ),
);
