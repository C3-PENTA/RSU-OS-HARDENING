<?php

chdir(dirname(__DIR__));

require "init_autoloader.php";

$func->debug( get_defined_vars() );
$func->debug( get_defined_functions() );
$func->debug( get_defined_constants() );