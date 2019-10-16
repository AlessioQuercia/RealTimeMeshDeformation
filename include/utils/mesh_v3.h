/*
Mesh class - v2
- allocation and initialization of VBO, VAO, and EBO buffers, and sets as OpenGL must consider the data in the buffers

VBO : Vertex Buffer Object - memory allocated on GPU memory to store the mesh data (vertices and their attributes, like e.g. normals, etc)
EBO : Element Buffer Object - a buffer maintaining the indices of vertices composing the mesh faces
VAO : Vertex Array Object - a buffer that helps to "manage" VBO and its inner structure. It stores pointers to the different vertex attributes stored in the VBO. When we need to render an object, we can just bind the corresponding VAO, and all the needed calls to set up the binding between vertex attributes and memory positions in the VBO are automatically configured.
See https://learnopengl.com/#!Getting-started/Hello-Triangle for details.

N.B. 1) in this version of the class, textures are loaded and applied

N.B. 2) adaptation of https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/mesh.h

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2018/2019
Master degree in Computer Science
Universita' degli Studi di Milano
*/

#pragma once

using namespace std;

// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

// GL Includes
#include <glad/glad.h> // Contains all the necessery OpenGL includes
// we use GLM data structures to write data in the VBO, VAO and EBO buffers
#include <glm/glm.hpp>

// data structure for vertices
struct Vertex {
    // vertex coordinates
    glm::vec3 Position;
    // Normal
    glm::vec3 Normal;
    // Texture coordinates
    glm::vec2 TexCoords;
    // Tangent
    glm::vec3 Tangent;
    // Bitangent
    glm::vec3 Bitangent;
};

// data structure for textures
struct Texture {
    GLuint id;
    string type;
    aiString path;
};

/////////////////// MESH class ///////////////////////
class Mesh {
public:
    // data structures for vertices, and indices of vertices (for faces)
    vector<Vertex> vertices;
    vector<GLuint> indices;
    // data structures for textures
    vector<Texture> textures;

    // VAO
    GLuint VAO;
    
    GLint inputAttrib;
    bool feedback;

    //////////////////////////////////////////
    // Constructor
    Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, bool feedback = false)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->feedback = feedback;
        
        if (feedback)
            printf("Feedback MESH\n");
        else
            printf("NoFeedback MESH\n");
        
        // initialization of OpenGL buffers
        this->setupMesh();
    }

    //////////////////////////////////////////

    // rendering of mesh
    void Draw(Shader shader)
    {
        if (feedback)
        {
            // VAO is made "active"
//            glBindVertexArray(this->VAO);
//            // we copy data in the VBO - we must set the data dimension, and the pointer to the structure cointaining the data
//            glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
//            glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);
//
//            glBindBuffer(GL_ARRAY_BUFFER, this->TBO);
//            glBufferData(GL_ARRAY_BUFFER, this->vertices.size(), nullptr, GL_STATIC_READ);
//
//            // we copy data in the EBO - we must set the data dimension, and the pointer to the structure cointaining the data
//            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
//            glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);
//
//            // we set in the VAO the pointers to the different vertex attributes (with the relative offsets inside the data structure)
//            // vertex positions
//            glEnableVertexAttribArray(0);
//            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
//            // Normals
//            glEnableVertexAttribArray(1);
//            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
//            // Texture Coordinates
//            glEnableVertexAttribArray(2);
//            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
//            // Tangent
//            glEnableVertexAttribArray(3);
//            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Tangent));
//            // Bitangent
//            glEnableVertexAttribArray(4);
//            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Bitangent));

            GLuint vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            GLuint vbo;
            GLuint tbo;

            glm::vec3 data[vertices.size()];
            for(int i=0; i<vertices.size(); i++)
                data[i] = vertices[i].Position;
                
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

            GLint inputAttrib = glGetAttribLocation(shader.ID, "inValue");
            glEnableVertexAttribArray(inputAttrib);
            glVertexAttribPointer(inputAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

            // Create transform feedback buffer
            glGenBuffers(1, &tbo);
            glBindBuffer(GL_ARRAY_BUFFER, tbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(data), nullptr, GL_STATIC_READ);

            // Perform feedback transform
            glEnable(GL_RASTERIZER_DISCARD);

            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

            glBeginTransformFeedback(GL_POINTS);
            glDrawArrays(GL_POINTS, 0, sizeof(data));
            glEndTransformFeedback();

            glDisable(GL_RASTERIZER_DISCARD);

            glFlush();

            // Fetch and print results
            glm::vec3 feed[sizeof(data)];
            glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feed), feed);

//            printf("%f %f %f %f %f %f\n", feed[0].x, feed[0].y, feed[0].z, feed[1].x, feed[1].y, feed[1].z);

            int size = sizeof(data)/sizeof(data[0]);

//            for (int i = 0; i<size; i++)
//            {
//                data[i] = feed[i];
//            }

            // Perform feedback transform
//            glEnable(GL_RASTERIZER_DISCARD);
//
//            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, TBO);
//
//            glBeginTransformFeedback(GL_POINTS);
//                glDrawArrays(GL_POINTS, 0, this->vertices.size());
//            glEndTransformFeedback();
//
//            glDisable(GL_RASTERIZER_DISCARD);
//
//            glFlush();
//            
//            // Fetch and print results
//            glm::vec3 feed[vertices.size()];
//            glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feed), feed);
//            
//            int size = sizeof(feed)/sizeof(feed[0]);
//            
////            printf("%f %f %f %f %f %f\n", feedback[0].x, feedback[0].y, feedback[0].z, feedback[1].x, feedback[1].y, feedback[1].z);
//            
////            printf("HIT\n");
//            
//            for (int i = 0; i<size; i++)
//            {
//                printf("PRIMA: %f %f %f\n", vertices[i].Position.x, vertices[i].Position.y, vertices[i].Position.z);
////                vertices[i].Position = feedback[i];
////                printf("DOPO: %f %f %f\n", vertices[i].Position.x, vertices[i].Position.y, vertices[i].Position.z);
//                printf("DOPO: %f %f %f\n", feed[i].x, feed[i].y, feed[i].z);
//            }
            
//            setupMesh();

//            // VAO is made "active"
//            glBindVertexArray(this->VAO);
//            // rendering of data in the VAO
//            glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
//            // VAO is "detached"
//            glBindVertexArray(0);
//
//            // Always good practice to set everything back to defaults once configured.
//            for (GLuint i = 0; i < this->textures.size(); i++)
//            {
//                glActiveTexture(GL_TEXTURE0 + i);
//                glBindTexture(GL_TEXTURE_2D, 0);
//            }
        }
        else
        {
            // Bind appropriate textures
            GLuint diffuseNr = 1;
            GLuint specularNr = 1;
            GLuint normalNr = 1;
            GLuint heightNr = 1;
            for(GLuint i = 0; i < this->textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
                // Retrieve texture number (the N in diffuse_textureN)
                stringstream ss;
                string number;
                string name = this->textures[i].type;
                if(name == "texture_diffuse")
                    ss << diffuseNr++; // Transfer GLuint to stream
                else if(name == "texture_specular")
                    ss << specularNr++; // Transfer GLuint to stream
                else if(name == "texture_normal")
                    ss << normalNr++; // Transfer GLuint to stream
                 else if(name == "texture_height")
                    ss << heightNr++; // Transfer GLuint to stream
                number = ss.str();
                // Now set the sampler to the correct texture unit
                glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
                // And finally bind the texture
                glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
            }

            // VAO is made "active"
            glBindVertexArray(this->VAO);
            // rendering of data in the VAO
            glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
            // VAO is "detached"
            glBindVertexArray(0);

            // Always good practice to set everything back to defaults once configured.
            for (GLuint i = 0; i < this->textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }

    //////////////////////////////////////////

    // buffers are deallocated when application ends
    void Delete()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

private:
  // VBO and EBO
  GLuint vbo, VBO, EBO;
  GLuint TBO;

  //////////////////////////////////////////
  // buffer objects\arrays are initialized
  // a brief description of their role and how they are binded can be found at:
  // https://learnopengl.com/#!Getting-started/Hello-Triangle
  // (in different parts of the page), or here:
  // http://www.informit.com/articles/article.aspx?p=1377833&seqNum=8
  void setupMesh()
  {
      // we create the buffers
      glGenVertexArrays(1, &this->VAO);
      glGenBuffers(1, &this->VBO);
      glGenBuffers(1, &this->EBO);
      
      if (feedback)
      {
           // Create transform feedback buffer
           glGenBuffers(1, &this->TBO);
      }

      // VAO is made "active"
      glBindVertexArray(this->VAO);
      // we copy data in the VBO - we must set the data dimension, and the pointer to the structure cointaining the data
      glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
      glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);
      
      if (feedback)
      {
          glBindBuffer(GL_ARRAY_BUFFER, this->TBO);
          glBufferData(GL_ARRAY_BUFFER, this->vertices.size(), nullptr, GL_STATIC_READ);
      }
      
      // we copy data in the EBO - we must set the data dimension, and the pointer to the structure cointaining the data
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

      // we set in the VAO the pointers to the different vertex attributes (with the relative offsets inside the data structure)
      // vertex positions
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
      // Normals
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
      // Texture Coordinates
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
      // Tangent
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Tangent));
      // Bitangent
      glEnableVertexAttribArray(4);
      glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Bitangent));

      glBindVertexArray(0);
  }
};
