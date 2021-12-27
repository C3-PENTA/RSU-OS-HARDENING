<?php

# session start
session_start();
# enc type
header("Content-Type: text/html; charset=UTF-8");

#force setting timestemp
ini_set("date.timezone", "Asia/Seoul");


### Define Variable
define ('DS', DIRECTORY_SEPARATOR);
define ('ROOT_PATH', __DIR__ . DS);
//---
define ('CLASS_PATH', ROOT_PATH . "classes" . DS);
define ('SYSTEM_CLASS_PATH', ROOT_PATH . "classes" . DS . "system" . DS);
//---
define ('MODULE_PATH', ROOT_PATH . "module" . DS);


use classes\system\framework\dkFrameWork;