CS 330 Final Project- Reflection
Chrystal Ramirez
Development
When developing my 3D scene, I chose objects that would work together to create a realistic arrangement while allowing me to practice different computer graphics techniques. I used basic 3D shapes, such as cylinders, spheres, toruses, and planes, to build the objects in my scene. For example, I used cylinders and toruses to create the planter and scaled spheres to create the aloe vera leaves. I also used different textures to make objects look more realistic, including wood, stone, and plant materials.

One of the biggest challenges was getting the objects to line up correctly. I had to adjust the scale, rotation, and position of individual shapes to make them fit together. I also worked with textures, materials, and lighting to improve the appearance of my scene. This helped me understand how small changes to an object's properties can affect the overall result.

I used reusable functions and existing mesh-drawing tools to create objects and apply their transformations. I also worked with lighting and shaders to control how objects appeared in the scene. Through trial and error, I learned that creating a 3D scene requires problem-solving, attention to detail, and an understanding of how different elements work together.

Navigate the 3D Scene
I used a virtual camera to allow the user to view the 3D scene from different perspectives. The camera uses its position and viewing direction to determine what appears on the screen. I adjusted the camera settings to create a starting view that displayed the objects in my scene clearly.

I also implemented keyboard controls to move the camera and change the viewing perspective. The WASD keys control movement, while the Q and E keys allow additional vertical movement. I used the P and O keys to switch between perspective and orthographic projections. Perspective projection makes objects appear smaller as they get farther away, while orthographic projection allows objects to retain their apparent size regardless of distance. I also used the T key to access a top-down camera view so the scene could be viewed from above.

These controls helped me understand how the virtual camera affects the user's experience. I learned that camera placement and viewing angles are important because even a completed scene can be difficult to understand if the camera is not positioned correctly. Adjusting the camera allowed me to view my objects from different angles and better understand their placement in the 3D environment.

Custom Functions Used to Make the Code Modular and Organized
I used functions in my program to organize the code and avoid repeating the same steps for every object. For example, the SetTransformations() function allows me to provide an object's scale, rotation, and position before drawing it. I also used functions to manage textures and materials, including SetShaderTexture(), SetTextureUVScale(), and SetShaderMaterial(). These functions allow me to reuse the same process while applying different textures and material properties to different objects. For example, I can apply a wood texture to one object and a stone texture to another without rewriting the entire texture setup each time.

Other functions, such as SetShaderColor(), DefineObjectMaterials(), and SetupSceneLights(), help control object colors, material properties, and lighting. I also used PrepareScene() and RenderScene() to organize the setup and drawing of my objects. These functions divide the program into smaller tasks, making the code easier to understand and maintain.

One custom function I developed was called SetLeafAtStem(). I created this function to help position the leaves around the center stem of my plant. Since the leaves were different sizes and rotated at different angles, positioning each leaf manually and making them connect correctly to the stem was challenging. The function calculates where each leaf should be placed based on its size, rotation, and attachment point. This allowed me to reuse the same function for multiple leaves instead of repeating the same calculations for each one.

Developing this function helped me understand how transformations, rotation, and positioning work together to create an object in a 3D scene. Instead of only changing numbers until the leaves looked right, I used calculations to position them more accurately. Overall, this project showed me how breaking a program into reusable functions can reduce repetitive work and make code easier to read, troubleshoot, update, and expand as the project becomes more complex.
