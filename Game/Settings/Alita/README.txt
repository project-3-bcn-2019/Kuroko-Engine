Kuroko Engine 1.0

This is a videogame engine in process, developed by Rodrigo de Pedro Lombao and Lucas García Mateu.

Features of the engine:

Lucas García Mateu
- Information about hardware and application.
- Application log and performance indicators as UI items.
- Rendering and window options.
- About tab with information about the engine and links to the used libraries.
- Primitive shapes (Sphere and Cylinder).
- File import through drag and drop.
- Load and Save of the editor configuration through JSON (open tabs and window options).
- UI docking.
- Own File Format for meshes and all textures.
- Quadtree.
- Save and Load of scene.
- Time control implemented, with serialization of scene reload when stopping time.
- Resource manager with online functionalities (add, modify and delete resources).

Rodrigo De Pedro Lombao
- Geometry storage and render in real time.
- Primitive shapes.
- Hierarchy viewer and game object inspector.
- Game object system, with parenting and components.
- Camera as component with frustum, frustum culling and rendering window.
- Gizmo implemented, with toolbar (does not work in ortographic mode).
- Mouse Picking.

INNOVATION: 
Rodrigo De Pedro Lombao
- File import through windows explorer
- Camera views are controllable by the user and can display their depth buffer.
- Notifications of memory leaks in console
- Skybox
- Viewports
- Transform with Global and Local modes and constraints.
- Camera menu with information about all game cameras and tweakable options.
- Asset viewer window with previews and info about the files in Asset folder.
- Color picker

Lucas García Mateu
- Using "map" container to store resources.
- When loading a scene, the engine will keep the name of the scene, and if "save" is selected it will be automatically saved
in the "scene folder" in assets, while if "save as" is clicked it will allow to rename it and save as a different scene.
- Menu with options on the quadtree, included how many objects are being discarted.
- "Duplicate" button

Libraries used:
- Assimp 	(http://www.assimp.org/)
- Glew		(http://glew.sourceforge.net/)
- DevIL		(http://openil.sourceforge.net/)
- MathGeoLib	(http://clb.demon.fi/MathGeoLib/)
- SDL		(https://wiki.libsdl.org/FrontPage)
- OpenGL, GLSL	(https://www.opengl.org/
- Parson	(https://github.com/kgabis/parson)
- PCG Random	(http://www.pcg-random.org)
- ImGuizmo      (https://github.com/CedricGuillemet/ImGuizmo)
- ImGuiColoredTextEditor (https://github.com/BalazsJako/ImGuiColorTextEdit)
- ImGui		(https://github.com/ocornut/imgui)
- Wren 		(http://wren.io/)
Extra information:

- Press "V" to see all the viewports at the same time.
- Right click on the empty hierarchy and click "Empty object" to create an empty object.
- Right click on a object to delete, rename, save to prefab or duplicate.
- You can add mesh components to an object, and assign them their material and mesh.
- Using "Import File" or "Drag and drop" on files will add them to the assets folder of the project. 
- Scenes can be loaded by double clicking them on asset window or using "Load Scene" in main menu.
- Double 3d objects in the asset window to add them to the scene.
- Camera views can be controled as the background camera by focusing their UI window. Mouse picking is also
  enabled and usable.
- The update ratio of the resource manager can be changed in the "Resources" window.


SUBSYSTEM: Scripting with wren

Features:

- Right click on asset window to add a new script. The name of the script will be the one
of the class defined in it. The script will automatically include the base libraries to interact
with the attached gameobject, as well as a constructor, Update() and Start() functions, and a comment with instructions
on how to use it.

- Attatch a script to a gameobject by pressing "Add script". This will display a window with all the scripts contained
in the asset window.

- The same script can be attached to multiple gameobjects, and the variables declared in its class will be displayed
in the editor. Choose a variable type and assign a value to them. You can force types, which will unable the user to change 
the variable type from the editor, as well as make variables public or private, which will enable/disable them being shown.
This is done through the asset inspector, select the script in the asset window and apply the desired changes. The script
component can be erased using the "Remove" button.

- Variables in the editor will display the value of the variable in the script, even if it is initialized in the "Start()"
function or modified at runtime.

- Scripts are executed according to the time in the world. "Start()" is executed in the first frame after pressing Play,
and Update() is executed every frame afterwards. The simulation can be paused as well.

- The system features hot reloading, meaning that scripts can be modified while the engine is running. This will recompile
all the scripts, but the values assigned to the variables in the editor will remain the same for variables which name's
are not changed.

- If a script fails to compile, a set of errors will display in the Log. When trying to access components with this script,
a message will appear warning the user. This can also be seen in the asset window and asset inspector.

Constraints:

- Can't modify scripts while the simulation is running.

Advices

- Remember to initialize variables in the "Start()" function of the script or the editor, not doing it will lead
to undetermined behaviours.

Demo:

- Tank moves with arrow keys
- Turret will point where the mouse is pointing to, if it is on top of the black plane
- Shoot with spacebar


Link to website: https://skyway666.github.io/Kuroko-Engine/
Link to github: https://github.com/Skyway666/Kuroko-Engine

Github profiles:
- Rodrigo De Pedro - (https://github.com/rodrigodpl)
- Lucas García Mateu - (https://github.com/Skyway666)



