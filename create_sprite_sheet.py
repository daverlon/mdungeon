from PIL import Image
import os

FOLDER_NAMES = [str(i) for i in range(8)]

SPRITE_SIZE = 256
# SPRITE_SIZE = 128

FRAME_COUNT = 10

def create_sprite_sheet(input_folder, output_path):
    # Get a list of all subfolders in the input folder
    # subfolders = sorted([f for f in os.listdir(input_folder) if os.path.isdir(os.path.join(input_folder, f))])
    subfolders = sorted([f for f in os.listdir(input_folder) if f in FOLDER_NAMES])
    print(subfolders)

    # Create a new image with dimensions for all frames
    sprite_width = SPRITE_SIZE
    sprite_height = SPRITE_SIZE
    sprite_sheet = Image.new('RGBA', (sprite_width * FRAME_COUNT, sprite_height * len(subfolders)))

    # Paste each folder's frames onto the sprite sheet
    current_y = 0
    for folder in subfolders:
        folder_path = os.path.join(input_folder, folder)
        print("Folder path:", folder_path)
        files = sorted([f for f in os.listdir(folder_path) if f.endswith(".png")])
        print("\t> Frames:", len(files))

        # Paste each image in the current folder
        current_x = 0
        for file_name in files:
            print("Opened:", file_name)
            img_path = os.path.join(folder_path, file_name)
            frame = Image.open(img_path)
            sprite_sheet.paste(frame, (current_x, current_y))
            current_x += SPRITE_SIZE

        current_y += sprite_height

    # Save the sprite sheet
    sprite_sheet.save(output_path)

if __name__ == "__main__":
    print(FOLDER_NAMES)
    # input_folder = "path/to/your/tmp/folder"
    # output_path = "path/to/your/output/sprite_sheet.png"
    input_folder = "/tmp"
    output_path = "./bin/res/sprite_dumps/output.png"

    # use aseprite to concatenate sheets vertically
    create_sprite_sheet(input_folder, output_path)
