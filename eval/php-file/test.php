<?php 
header("Content-Type: text/html");
echo "hello world\n\n"; 
var_dump(headers_list());

echo "<h3>Server Variables</h3>";
foreach ($_SERVER as $key => $value) {
    echo "$key: $value <br>";
}

echo "<h3>Cookies</h3>";
foreach ($_COOKIE as $key => $value) {
	echo "$key: $value <br>";
}

echo "<h3>Get variables</h3>";
foreach ($_GET as $key => $value) {
	echo "$key: $value <br>";
}
print_r($_GET);

echo "<h3>Post variables</h3>";
foreach ($_POST as $key => $value){
	echo "{$key} = {$value}\r\n";
  }

  print_r($_POST)
?>
