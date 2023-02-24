#define GLM_ENABLE_EXPERIMENTAL
// glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

//#include <glm/gtx/euler_angles.hpp>

// project
#include "cgra/cgra_geometry.hpp"
#include "skeleton_model.hpp"

#include <iostream>
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;
using namespace cgra;




void skeleton_model::draw(const mat4 &view, const mat4 &proj, float dt) {
	// set up the shader for every draw call
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));

    int duation = anime.poses.size();
    
    if (duation > 0)
        frame = fmodf(frame + 60*dt, duation);
//    cout << frame << endl;
    
	// if the skeleton is not empty, then draw
	if (!skel.bones.empty()) {
        if (doAnime)
            calBoneTrans(view, 0, int(frame));
        else if (doPose) {
            calBoneTrans(view, 0, 0); // show the still frame
        }
        else {
            drawBone(view, 0);
        }
	}
}

void skeleton_model::drawJoint(const mat4 &view) {
    mat4 modelTrans(view);
    modelTrans = glm::scale(modelTrans, vec3(0.02));
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelTrans));
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(jointColor));
        
    drawSphere();
}

void skeleton_model::drawLength(const glm::mat4 &view, skeleton_bone currentBone) {
    mat4 modelTrans(view);
    
    modelTrans *= glm::orientation(currentBone.direction, vec3(0.0, 0.0, 1.0));
    modelTrans = glm::scale(modelTrans, vec3(0.01, 0.01, currentBone.length));
        
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelTrans));
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(boneColor));

    drawCylinder();
}

void skeleton_model::drawAxis(const glm::mat4 &view, skeleton_bone currentBone) {
    mat4 modelTrans(view);
    
    modelTrans *= glm::rotate(glm::mat4(1), currentBone.basis.z, glm::vec3(0, 0, 1));
    modelTrans *= glm::rotate(glm::mat4(1), currentBone.basis.y, glm::vec3(0, 1, 0));
    modelTrans *= glm::rotate(glm::mat4(1), currentBone.basis.x, glm::vec3(1, 0, 0));
    
    mat4 axis_X = modelTrans;
    mat4 axis_Y = modelTrans;
    mat4 axis_Z = modelTrans;
    
    // X - axis
    axis_X *= glm::rotate(glm::mat4(1), glm::pi<float>() / 2.0f, glm::vec3(0, 1, 0));
    mat4 axis_X_arrow = glm::scale(axis_X, vec3(0.008, 0.008, 0.02));
    axis_X_arrow = glm::translate(axis_X_arrow, vec3(0.0, 0.0, 3.5));
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(axis_X_arrow));
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(xColor));
    drawCone();
    axis_X = glm::scale(axis_X, vec3(0.003, 0.003, 0.075));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(axis_X));
    drawCylinder();
    
    // Y - axis
    axis_Y *= glm::rotate(glm::mat4(1), glm::pi<float>() / 2.0f, glm::vec3(-1, 0, 0));
    mat4 axis_Y_arrow = glm::scale(axis_Y, vec3(0.008, 0.008, 0.02));
    axis_Y_arrow = glm::translate(axis_Y_arrow, vec3(0.0, 0.0, 3.5));
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(axis_Y_arrow));
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(yColor));
    drawCone();
    axis_Y = glm::scale(axis_Y, vec3(0.003, 0.003, 0.075));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(axis_Y));
    drawCylinder();
    
    // Z -axis
    axis_Z *= glm::rotate(glm::mat4(1), glm::pi<float>() / 2.0f, glm::vec3(0, 0, 1));
    mat4 axis_Z_arrow = glm::scale(axis_Z, vec3(0.008, 0.008, 0.02));
    axis_Z_arrow  = glm::translate(axis_Z_arrow , vec3(0.0, 0.0, 3.5));
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(axis_Z_arrow ));
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(zColor));
    drawCone();
    axis_Z = glm::scale(axis_Z, vec3(0.003, 0.003, 0.075));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(axis_Z));
    drawCylinder();
}


void skeleton_model::calBoneTrans(const mat4 &parentTransform, int boneid, int frame) {
//    if (boneid == 18) cout << "18";
    
    mat4 pt(parentTransform);
    
    skeleton_bone currentBone;
    for(int i = 0; i<(int)skel.bones.size(); i++){
        if(skel.findBone(skel.bones[i].name) == boneid){
            currentBone = skel.bones[i];
        }
    }

    skeleton_bone_pose currentBonePose = anime.poses.at(frame).boneTransforms.at(boneid);
    
    vec3 trans = currentBonePose.translation;
    vec3 rota = currentBonePose.rotation;
        
    mat4 global2local = mat4(1);
    global2local *= glm::rotate(glm::mat4(1), currentBone.basis.z, glm::vec3(0, 0, 1));
    global2local *= glm::rotate(glm::mat4(1), currentBone.basis.y, glm::vec3(0, 1, 0));
    global2local *= glm::rotate(glm::mat4(1), currentBone.basis.x, glm::vec3(1, 0, 0));
    mat4 local2global = inverse(global2local);
    pt = pt * global2local;
    pt = glm::translate(pt, trans) * rotate(mat4(1), rota.z, vec3(0, 0, 1)) * rotate(mat4(1), rota.y, vec3(0, 1, 0)) * rotate(mat4(1), rota.x, vec3(1, 0, 0));
    pt = pt * local2global;
    
    mat4 globalModelTrans = pt;

    drawJoint(globalModelTrans);
    drawLength(globalModelTrans, currentBone);
    
    if(boneid != 0) drawAxis(globalModelTrans, currentBone);
    
    globalModelTrans = glm::translate(globalModelTrans, currentBone.direction * currentBone.length);
    
    for (int child : currentBone.children){
        int newID = skel.findBone(skel.bones[child].name);
        calBoneTrans(globalModelTrans, newID, frame);
    }
}

void skeleton_model::drawBone(const mat4 &parentTransform, int boneid) {
	// TODO
    mat4 modelTrans(parentTransform);
    
    skeleton_bone currentBone;
    
    for(int i = 0; i<(int)skel.bones.size(); i++){
        if(skel.findBone(skel.bones[i].name) == boneid){
            currentBone = skel.bones[i];
        }
    }
    
    drawJoint(modelTrans);
    drawLength(modelTrans, currentBone);
    
    if(boneid != 0) drawAxis(modelTrans, currentBone);

//    if (1) {
//        if ( boneid == 0 )
//                cout << "-------------------------" << endl;
//        cout << "id: " << boneid << ", " << currentBone.name << endl;
//        cout << "    basis: " << glm::to_string(glm::degrees(currentBone.basis)) << endl;
//        //cout << "    rot: " << glm::to_string(glm::degrees(bonePose.rotation)) << endl;
//    }
    
    // Offset to T-pose
    modelTrans = glm::translate(modelTrans, currentBone.direction * currentBone.length);
    
    //recursively do the same for the children
    for (int child : currentBone.children){
        int newID = skel.findBone(skel.bones[child].name);
        drawBone(modelTrans, newID);
    }
}
