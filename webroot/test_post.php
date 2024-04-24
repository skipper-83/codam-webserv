<?php

// echo "<DOCTYPE html>\n\n";

// echo "test_post.php\n\n";	
foreach ($_SERVER as $key => $value) {
    echo "$key: $value <br>";
}

echo "<h1>post data:</h1>";
// Loop through all POST data and print key-value pairs
foreach ($_POST as $key => $value) {
    echo "$key: $value<br>";
}

?>