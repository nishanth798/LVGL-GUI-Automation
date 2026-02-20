## Processing PNG Images with nkdConverter.py

## Overview

The nkdConverter.py script processes PNG images by validating their dimensions and then using another script (LVGLImage.py) for further processing. This guide provides step-by-step instructions on how to set up and run.

## Usage

### Setup 
Install the pip in the system.

```
sudo apt install python3-pip
```

You can install the required packages using pip with the following command.
>Note: Run the following command from the location where requirements.txt file is located:

```
pip install -r requirements.txt
```
### Running the Script

To process a PNG image, execute nkdConverter.py:
```
python3 nkdConverter.py path/to/your/image.png
```
#### Note
- Checks if the file extension is .png or .PNG.
- Verifies if the image dimensions are exactly 60x38 pixels.

Replace path/to/your/image.png with the actual path to your PNG image.


## Functionality
- Argument Parsing: Parses the command-line arguments to get the path to the PNG image.
- Image Validation: Checks if the file extension is .png or .PNG and verifies if the image dimensions are exactly 60x38 pixels.
- Temporary Directory: Creates a temporary directory to store processed files.
- Subprocess Execution: Executes LVGLImage.py script with specified parameters (--ofmt C --cf RGB565) to process the image.

## Output
- Standard Output: Prints the formatted hexadecimal values extracted from the processed files.
- Error Output: Prints error messages if the image dimensions are incorrect or if no match is found in the processed files.
	- If invalid image format
	> Invalid file format, use only .png
	- If invalid image dimensions
	> Invalid image dimensions, use only 60x38 size image

### Notes
- Ensure Python 3.x is installed and accessible from the command line.
- Verify that LVGLImage.py is correctly located and accessible from the script's directory.
