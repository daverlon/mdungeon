import bpy
import os
import math
import bpy

def print(data):
    for window in bpy.context.window_manager.windows:
        screen = window.screen
        for area in screen.areas:
            if area.type == 'CONSOLE':
                with bpy.context.temp_override(window =window, screen=screen,area=area):
                    bpy.ops.console.scrollback_append(text=str(data), type="OUTPUT")     

def render_spin(path):

    rotate_circle = bpy.data.objects['Empty']
    for x in range(0, 8):
        rotate_circle.rotation_euler[2] = -math.radians(45) * x
        bpy.context.scene.render.filepath = (path + "\\" + str(x) + "\\" + str(x))
        bpy.ops.render.render( animation=True, write_still=True ) 
    rotate_circle.rotation_euler[2] += -math.radians(45)
    

render_spin('/tmp')
print("Done")
