import argparse
import subprocess
from PIL import Image
import tempfile
import os
import re
import sys

def get_image_dimensions(image_path):
    with Image.open(image_path) as img:
        width, height = img.size
    return width, height

def extract_first_match_between_braces(text):
    # Use regular expression to find the first match of content between { and }
    match = re.search(r'\{([^}]*)\}', text, re.DOTALL)
    return match.group(1) if match else None

def format_hex_values(hex_string):
    # Remove whitespace and split by commas
    hex_values = hex_string.replace('\n', '').replace(' ', '').split(',')
    # Remove '0x' prefix and concatenate
    formatted_string = ''.join([value[2:] for value in hex_values if value.startswith('0x')])
    return formatted_string

def main():
    # Set up argument parser
    parser = argparse.ArgumentParser(description='Get dimensions of a PNG image and run LVGLImage.py script.')
    parser.add_argument('image_path', type=str, help='Path to the PNG image')

    # Parse arguments
    args = parser.parse_args()   


    _, extension = os.path.splitext(args.image_path)
    if extension.lower() != '.png':
        print("Invalid file format, use only .png",file=sys.stderr)
        return

    # Get image dimensions
    width, height = get_image_dimensions(args.image_path)
    if width != 60 and height != 38:
        print("Invalid image dimensions, use only 60x38 size image",file=sys.stderr)
        return

    # Create a temporary directory
    with tempfile.TemporaryDirectory() as temp_dir:
        # Run the LVGLImage.py script
        result = subprocess.run(
            ['python3', 'LVGLImage.py', args.image_path, '--ofmt', 'C', '--cf', 'RGB565', '-o', temp_dir],
            capture_output=True,
            text=True
        )

        if result.stderr:
            print(result.stderr, file=sys.stderr)
            
        # List and print the first match of contents in files in the temporary directory
        for filename in os.listdir(temp_dir):
            file_path = os.path.join(temp_dir, filename)
            with open(file_path, 'r') as file:
                content = file.read()
                first_match = extract_first_match_between_braces(content)
                if first_match:
                    formatted_output = format_hex_values(first_match)
                    print(formatted_output,file=sys.stdout)
                else:
                    print(f"No match found in {filename}",file=sys.stderr)

if __name__ == "__main__":
    main()
