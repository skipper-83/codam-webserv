#!/bin/bash

# Define an array of funny lines
funny_lines=("Why don't scientists trust atoms? Because they make up everything!"
             "How does a computer get drunk? It takes screenshots!"
             "Why was the math book sad? It had too many problems."
             "If you have 10 apples in one hand and 14 oranges in the other, what do you have? Very large hands!"
             "Parallel lines have so much in common. It’s a shame they’ll never meet.")

# Get a random line from the array
rand_index=$(($RANDOM % ${#funny_lines[@]}))

# Output HTTP headers
echo "Content-Type: text/html"
echo ""  # Blank line is necessary to separate headers from content

# Output HTML content
cat <<EOF
<!DOCTYPE html>
<html>
<head>
    <title>Funny CGI Page</title>
</head>
<body>
    <h1>Hello from Bash CGI!</h1>
    <p>${funny_lines[$rand_index]}</p>
</body>
</html>
EOF