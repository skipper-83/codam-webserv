<?php
// Check if the 'delete' parameter is present in the URL
if (isset($_GET['delete']) && $_GET['delete'] == 'true') {
    // If 'delete' parameter is true, delete the cookie
    setcookie('testCookie', '', time() - 3600); // set the expiration date to one hour ago
    echo "Cookie has been deleted.<br>";
} else {
    // If there is no 'delete' parameter, set a new cookie
    setcookie('testCookie', 'This is a test cookie', time() + 3600); // expire in 1 hour
    echo "Cookie has been set.<br>";
}

// Check if the cookie exists and display a message
if (isset($_COOKIE['testCookie'])) {
    echo "Cookie value: " . $_COOKIE['testCookie'] . "<br>";
} else {
    echo "Cookie is not set or has been deleted.<br>";
}
?>
