<?php

Namespace classes\system\framework;

class moduleLoader
{
    public function_construct()

    $dir_list = $this->getDirList( ROOT_PATH . "module" );
    $this->module_name = $this->setModuleList( $dir_list );

    private function getDirList ( $path )
    {
        $module = getDir( $path );
        return $module;
    }

    private function setModuleList( $list )
    {
        return $list;
    }
}