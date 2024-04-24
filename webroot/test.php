<?php 
echo "hello world\n\n"; 
var_dump(headers_list());

foreach ($_SERVER as $key => $value) {
    echo "$key: $value <br>";
}

foreach ($_COOKIE as $key => $value) {
	echo "$key: $value <br>";
}

?>
