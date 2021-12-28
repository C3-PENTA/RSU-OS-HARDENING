<?php

class fileStructure
{
    public function __construct()
	{
		$list = $this->getLs(ROOT_PATH);
		debug($list);
	}
}