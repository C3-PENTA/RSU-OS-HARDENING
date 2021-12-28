<?php

trait datatable
{
    public function dt_init($post)
    {
        $draw   = (isset($post["draw"]))?       $post["draw"]:"";

        $val = array(
            "draw"     => $draw
        );
        return $val;
    }
}