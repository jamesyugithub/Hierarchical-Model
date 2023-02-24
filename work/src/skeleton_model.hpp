#pragma once

// glm
#include <glm/glm.hpp>

// project
#include "opengl.hpp"
#include "skeleton.hpp"




class skeleton_model {
private:
    glm::vec3 jointColor{0.0, 0.7, 0.8};
    glm::vec3 boneColor{1.0};
    glm::vec3 xColor{1.0, 0.0, 0.0};
    glm::vec3 yColor{0.0, 1.0, 0.0};
    glm::vec3 zColor{0.0, 0.0, 1.0};
    
	void drawBone(const glm::mat4 &view, int boneid);
    void calBoneTrans(const glm::mat4 &parentTransform, int boneid, int frame);

public:
	GLuint shader = 0;
	skeleton_data skel; // holds the skeleton_data
	skeleton_pose pose; // holds skeleton_pose
    skeleton_animation anime;
    bool doAnime = false;
    bool doPose = false;
    bool interaction = false;
    float frame = 0;
    
	skeleton_model() { }
	void draw(const glm::mat4 &view, const glm::mat4 &proj, float dt);
    
    void drawJoint(const glm::mat4 &view);
    void drawLength(const glm::mat4 &view, skeleton_bone currentBone);
    void drawAxis(const glm::mat4 &view, skeleton_bone currentBone);
    
    void update(int timestep);

};
