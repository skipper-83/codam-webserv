<?php
header("Cache-Control: no-cache, no-store, must-revalidate"); // HTTP 1.1
header("Pragma: no-cache"); // HTTP 1.0
header("Expires: 0"); // Proxies
// Function to set cookie for dark/night mode
function setDarkModeCookie($mode) {
    // Set cookie with name "dark_mode" and value $mode (either "dark" or "light")
    setcookie("dark_mode", $mode, time() + 120); // Lifetime: 2 minutes
}

// Check if user has submitted form
if ($_SERVER["REQUEST_METHOD"] == "POST" && isset($_POST["mode"])) {
    // Get mode from form submission
    $mode = $_POST["mode"];
    // Set dark mode cookie
    setDarkModeCookie($mode);
}

foreach ($_COOKIE as $key => $value) {
	echo "$key: $value <br>";
}

// Check if dark mode cookie is set
if (isset($_COOKIE["dark_mode"])) {
    $mode = $_COOKIE["dark_mode"];
} else {
    // If cookie is not set, prompt the user to choose a mode
    echo '<form method="post" action="">';
    echo '<label for="dark">Dark mode</label>';
    echo '<input type="radio" name="mode" value="dark" id="dark">';
    echo '<label for="light">Light mode</label>';
    echo '<input type="radio" name="mode" value="light" id="light">';
    echo '<input type="submit" value="Set Mode">';
    echo '</form>';
    // Exit script after displaying the form
    exit;
}

// Display content based on selected mode
if ($mode === "dark") {
    // Dark mode
    echo '<style>body { background-color: #222; color: #fff; }</style>';
    echo '<h1>Welcome to Dark Mode</h1>';
} else {
    // Light mode (default)
    echo '<style>body { background-color: #fff; color: #222; }</style>';
    echo '<h1>Welcome to Light Mode</h1>';
}

?>
