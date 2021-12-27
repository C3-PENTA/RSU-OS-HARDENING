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

    private function setRouting( $dir_list )
    {
        &route[] = "/";

        foreach ( $dir_list as $key => $val ){
            // '_'로 시작하면 제외
            if ( substr($val, 0, 1) == "_" ) {
                continue;
            }
            // ---
            else
            {
                $route[] = "/" . $val;
            }
        }

        return &route;
    }
}
