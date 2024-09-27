from PIL import Image, ImageFont, ImageDraw


def ttf_to_bitmap_hex(font_path, char, size=8):
    # Load the font
    font = ImageFont.truetype(font_path, size)
    # Handle transparent bits at this resolution
    font.getmask(char).getbbox()

    # Create a blank image with 1-bit color mode ('1' is binary/monochrome)
    image = Image.new('1', (size, size), 1)  # 1 means white background
    draw = ImageDraw.Draw(image)


    # Draw the character at the calculated position
    draw.text((0, 0), char, font=font, fill=0)  # 0 means black text


    # Convert the image to a list of hex strings
    bitmap_hex = []
    for y in range(size):
        hex_string = ''
        for x in range(size):
            hex_string += '1' if image.getpixel((x, y)) == 0 else '0'
        bitmap_hex.append(hex_string)

    return bitmap_hex


import sys
ascii_chars = ' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~'
result = []

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 font_to_bits.py font.ttf")
        sys.exit(1)

    
    font_path = sys.argv[1]
    font_name = font_path.strip().split('.ttf')[0]

    print("Generating bitmaps for ASCII characters using font:", font_path)
    bitmap_hex = []
    for char in ascii_chars:
        bitmap = ttf_to_bitmap_hex(font_path, char, size=8)
        print(f"Hex bitmap for '{char}':")
        for row in bitmap:
            print(row)
        bitmap_hex.extend(bitmap)

    
    i = 0
    while i < len(bitmap_hex):
        binary_msb_str = bin(int(bitmap_hex[i], 2))[2:].zfill(8)
        binary_lsb_str = bin(int(bitmap_hex[i+1], 2))[2:].zfill(8)

        binary_string = binary_msb_str + binary_lsb_str
        # Convert from 16 bits binary string to hex string
        hex_string = hex(int(binary_string, 2))[2:].zfill(4)
        result.append(hex_string)
        i += 2


    # Save the bitmaps to a file as font_name.virt16font in array format hexadecimal strings
    with open(f'{font_name}.virt16font', 'w') as f:
        f.write("[")
        # Write the hex strings to the file every 8 results newline separated by commas
        for i in range(len(result)):
            if i % 8 == 0:
                f.write("\n")
            f.write("0x" + (result[i]))
            if i != len(result) - 1:
                f.write(", ")
        f.write("]")
            

