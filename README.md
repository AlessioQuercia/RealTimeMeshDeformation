# Real-Time Polygon Mesh Deformation

## Overview
The repository is intended to keep track of the progress of the project related to the Real-Time Graphics Programming course at Università degli Studi di Milano. 

## Info
The goal of this project was to create an application that shows 3D real-time polygon mesh deformation on collision between two objects. The objective was achieved by using two different approaches: the first one uses arrays of uniforms in the vertex shader to store the hit points and directions, whereas the second one uses the feedback transform technique in a first render pass, updates the meshes and then renders normally at a second pass. The second method resulted to be more efficient and allowed an infinite number of deformations in the scene.

For more info, read the report inside the repository.
