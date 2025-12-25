import sys
from PIL import Image
import numpy as np

def main():
    if len(sys.argv) < 3:
        print("Usage: python display_monitor.py <input.yuv> <output.png>")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]
    width = 256
    height = 256

    try:
        with open(input_path, 'rb') as f:
            data = f.read()
            
        if len(data) != width * height:
            print(f"Warning: File size {len(data)} does not match expected {width*height}. Truncating or padding.")
            data = data[:width*height] + b'\x00' * (width*height - len(data))

        # Create image from bytes (L mode for grayscale)
        img = Image.frombytes('L', (width, height), data)
        img.save(output_path)
        print(f"Successfully saved monitor image to {output_path}")

    except Exception as e:
        print(f"Error processing image: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
