<?php

Namespace classes\system\framework;

class routeLoader
{
    public function __construct(  )
    {
        # 디렉토리 리스트 가져오기.
        $dir_list = getDir( ROOT_PATH . "module" );
        # 라우트 등록하기
        $this->route = $this->setRouting( $dir_list );
    }
}
