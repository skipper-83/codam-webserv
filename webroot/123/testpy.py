#!/usr/bin/env python3

import os

# Print the Content-Type header
print("Content-Type: text/html\n\n")

# Print the HTML content
print("<html><head><title>Test Python</title></head><body>")
print("<h1>Hello, World!</h1>")
print("<p>Hello, World!</p>")
print("<p>Hello, World!</p>")
print("<p>Hello, World!</p>")

# Function to print all environment variables
def print_all_env_variables():
    for key, value in os.environ.items():
        print(f"<p>{key}: {value}</p>")
print_all_env_variables()

print("<p>Hello, env!</p>")
print("</body></html>")
