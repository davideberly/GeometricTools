import os
import shutil
import sys

extensions = [".ipch", ".pch", ".sdf", ".vspx", ".aps", ".vsp", ".psess"]
directories = ["_output", "ipch"]
if "--all" in sys.argv:
    extensions += [".suo", ".user"]
    directories.append('.vs')   
       
def robust_rmtree(p):
    while 1:
        try:
            shutil.rmtree(p)
            break
        except WindowsError:
            continue
        
for dirpath, dirnames, filenames in os.walk(os.getcwd()):
    for dn in dirnames:
        p = os.path.join(dirpath, dn)
        if dn.lower() in directories:
            print(p)
            robust_rmtree(p)
            
for dirpath, dirnames, filenames in os.walk(os.getcwd()):
    for fn in filenames:
        base, ext = os.path.splitext(fn)
        if ext.lower() in extensions or (ext == '' and base in extensions):
            p = os.path.join(dirpath, fn)
            print(p)
            os.unlink(p)
            
    if "--all" in sys.argv:
        for dn in dirnames:
            if dn == '.vs':
                p = os.path.join(dirpath, dn)
                print(p)
                robust_rmtree(p)
