#!/usr/bin/env python3

import sys
import os

# Read the Content-Length header from environment variables
content_length = int(os.environ.get('CONTENT_LENGTH', 0))

# Read POST data from stdin
post_data = sys.stdin.read(content_length)

# Print the Content-Type header
print("Content-Type: text/html\n\n")

# Print HTML content
print("<html><body>")
print("<h1>POST Data:</h1>")
print("<ul>")

# Parse and print each key-value pair
if (post_data == ""):
    print("<li>No data</li>")
else:    
    pairs = post_data.split("&")
    for pair in pairs:
        key, value = pair.split("=")
        print(f"<li>{key}: {value}</li>")

print("</ul>")
print("</body></html>")