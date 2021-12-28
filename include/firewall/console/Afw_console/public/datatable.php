<?php

trait datatable
{
    public function dt_init($post)
    {
        $draw   = (isset($post["draw"]))?       $post["draw"]:"";
        $start  = (isset($post["start"]))?      $post["start"]:"";
        $length = (isset($post["length"]))?     $post["length"]:"";

        $val = array(
            "draw"     => $draw
            ,"start"    => $start
            ,"length"   => $length
        );
        return $val;
    }
}