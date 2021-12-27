<?php

Namespace classes\system\framework;

class moduleLoader
{
    public function_construct()

    $dir_list = $this->getDirList( ROOT_PATH . "module" );
    $this->module_name = $this->setModuleList( $dir_list );
}