from PIL import Image
import sys
import os

def convert_png_to_c_array(input_path):
    try:
        img = Image.open(input_path).convert('RGBA')
    except Exception as e:
        print(f"Błąd otwierania pliku: {e}")
        return

    width, height = img.size
    name = os.path.splitext(os.path.basename(input_path))[0]
    
    print(f"/* Ikona: {name} ({width}x{height}) */")
    print(f"unsigned int {name}_icon[{width * height}] = {{")
    
    pixels = list(img.getdata())
    
    for y in range(height):
        line = "    "
        for x in range(width):
            r, g, b, a = pixels[y * width + x]
            
            # Jeśli piksel jest przezroczysty (alpha < 128), dajemy 0
            if a < 128:
                color_hex = "0x000000"
            else:
                # Format 0xRRGGBB
                color_hex = f"0x{r:02X}{g:02X}{b:02X}"
            
            line += color_hex + (", " if (y * width + x) < (width * height - 1) else "")
        print(line)
        
    print("};")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Użycie: python png_to_vos.py ikona.png")
    else:
        convert_png_to_c_array(sys.argv[1])