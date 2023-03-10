
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;


void basic_model::draw(const glm::mat4 &view, const glm::mat4 proj) {
	mat4 modelview = view * modelTransform;
	
	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	drawCylinder();
    drawCone();
	//mesh.draw(); // draw
}


Application::Application(GLFWwindow *window) : m_window(window) {
	
	shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
	m_model.mesh = load_wavefront_data(CGRA_SRCDIR + std::string("/res//assets//teapot.obj")).build();
	m_model.color = vec3(1, 0, 0);
	m_cam_pos = vec2( 0, 0 );
    
    // assignment
    m_skeleton.shader = shader;
    m_skeleton_2.shader = shader;
    m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//priman.asf"));

}

int time1 = 0;
void Application::render() {
	
	// retrieve the window height
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height); 

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);
    
    // animetion
    float currentFrame = glfwGetTime();
    dt = currentFrame - lastFrame;
    lastFrame = currentFrame;
    
    
//    cout << f << endl;
    
	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// view matrix
	mat4 view = translate(mat4(1), vec3( m_cam_pos.x, m_cam_pos.y, -m_distance))
		* rotate(mat4(1), m_pitch, vec3(1, 0, 0))
		* rotate(mat4(1), m_yaw,   vec3(0, 1, 0));


	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

    // assignment
    m_skeleton.draw(view, proj, dt);
    if (m_skeleton_2.interaction)
        m_skeleton_2.draw(view, proj, dt);
    
}


void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>() / 2, pi<float>() / 2, "%.2f");
	ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2f");
	ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.2f", 2.0f);
    
    ImGui::Separator();
    ImGui::Text("Core");
    
    if (ImGui::Button("Priman")) {
        m_skeleton.doAnime = false;
        m_skeleton.doPose = false;
        m_skeleton_2.interaction = false;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//priman.asf"));
    }
    ImGui::SameLine();
    if (ImGui::Button("Test Skeleton 1")) {
        m_skeleton.doAnime = false;
        m_skeleton.doPose = false;
        m_skeleton_2.interaction = false;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//testskeleton1.asf"));
    }
    ImGui::SameLine();
    if (ImGui::Button("Test Skeleton 2")) {
        m_skeleton.doAnime = false;
        m_skeleton.doPose = false;
        m_skeleton_2.interaction = false;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//testskeleton2.asf"));
    }
    
    ImGui::Separator();
    
    ImGui::Text("Completion");
    if (ImGui::Button("Sit")) {
        m_skeleton.doAnime = false;
        m_skeleton.doPose = true;
        m_skeleton_2.interaction = false;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//priman.asf"));
        m_skeleton.anime = skeleton_animation(CGRA_SRCDIR + std::string("//res//assets//comp_sitting.amc"), m_skeleton.skel);
    }
    ImGui::SameLine();
    if (ImGui::Button("Walk")) {
        m_skeleton.doAnime = false;
        m_skeleton.doPose = true;
        m_skeleton_2.interaction = false;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//priman.asf"));
        m_skeleton.anime = skeleton_animation(CGRA_SRCDIR + std::string("//res//assets//comp_walking.amc"), m_skeleton.skel);
    }
    ImGui::SameLine();
    if (ImGui::Button("Fight")) {
        m_skeleton.doAnime = false;
        m_skeleton.doPose = true;
        m_skeleton_2.interaction = false;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//priman.asf"));
        m_skeleton.anime = skeleton_animation(CGRA_SRCDIR + std::string("//res//assets//comp_fighting.amc"), m_skeleton.skel);
    }
    ImGui::SameLine();
    if (ImGui::Button("Bend Elbow")) {
        m_skeleton.doAnime = false;
        m_skeleton.doPose = true;
        m_skeleton_2.interaction = false;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//priman.asf"));
        m_skeleton.anime = skeleton_animation(CGRA_SRCDIR + std::string("//res//assets//bend_elbow_priman.amc"), m_skeleton.skel);
        duration = m_skeleton.anime.poses.size();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        m_skeleton.doAnime = false;
        m_skeleton.doPose = false;
        m_skeleton_2.interaction = false;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//priman.asf"));
    }
    
    ImGui::Separator();
    
    ImGui::Text("Challenge");
    if (ImGui::Button("Interaction")) {
        m_skeleton.doPose = false;
        m_skeleton.doAnime = true;
        m_skeleton_2.interaction = true;
        m_skeleton_2.doPose = false;
        m_skeleton_2.doAnime = true;
        m_skeleton.frame = 0;
        m_skeleton_2.frame = 0;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//subject1.asf"));
        m_skeleton.anime = skeleton_animation(CGRA_SRCDIR + std::string("//res//assets//interaction_s1.amc"), m_skeleton.skel);
        m_skeleton_2.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//subject2.asf"));
        m_skeleton_2.anime = skeleton_animation(CGRA_SRCDIR + std::string("//res//assets//interaction_s2.amc"), m_skeleton_2.skel);
    }
    ImGui::SameLine();
    if (ImGui::Button("Walk Animetion")) {
        m_skeleton.doPose = false;
        m_skeleton.doAnime = true;
        m_skeleton_2.interaction = false;
        m_skeleton.frame = 0;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//priman.asf"));
        m_skeleton.anime = skeleton_animation(CGRA_SRCDIR + std::string("//res//assets//walking_priman.amc"), m_skeleton.skel);
    }
    ImGui::SameLine();
    if (ImGui::Button("Breakdance")) {
        m_skeleton.doPose = false;
        m_skeleton.doAnime = true;
        m_skeleton_2.interaction = false;
        m_skeleton.frame = 0;
        m_skeleton.skel = skeleton_data(CGRA_SRCDIR + std::string("//res//assets//priman.asf"));
        m_skeleton.anime = skeleton_animation(CGRA_SRCDIR + std::string("//res//assets//breakdance_priman.amc"), m_skeleton.skel);
    }
    
    ImGui::Separator();

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);
	
	ImGui::Separator();

	// example of how to use input boxes
	static float exampleInput;
	if (ImGui::InputFloat("example input", &exampleInput)) {
		cout << "example input changed to " << exampleInput << endl;
	}

	// finish creating window
	ImGui::End();
}


void Application::cursorPosCallback(double xpos, double ypos) {
	vec2 whsize = m_windowsize / 2.0f;

	double y0 = glm::clamp((m_mousePosition.y - whsize.y) / whsize.y, -1.0f, 1.0f);
	double y = glm::clamp((float(ypos) - whsize.y) / whsize.y, -1.0f, 1.0f);
	double dy = -( y - y0 );

	double x0 = glm::clamp((m_mousePosition.x - whsize.x) / whsize.x, -1.0f, 1.0f);
	double x = glm::clamp((float(xpos) - whsize.x) / whsize.x, -1.0f, 1.0f);
	double dx = x - x0;

	if (m_leftMouseDown) {
		// clamp the pitch to [-pi/2, pi/2]
		m_pitch += float( acos(y0) - acos(y) );
		m_pitch = float(glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2));

		// wrap the yaw to [-pi, pi]
		m_yaw += float( acos(x0) - acos(x) );
		if (m_yaw > pi<float>()) 
			m_yaw -= float(2 * pi<float>());
		else if (m_yaw < -pi<float>()) 
			m_yaw += float(2 * pi<float>());
	} else if ( m_rightMouseDown ) {
		m_distance += dy * 10;
	} else if ( m_middleMouseDown ) {
		m_cam_pos += vec2( dx, dy ) * 10.f;
	}

	// updated mouse position
	m_mousePosition = vec2(xpos, ypos);
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
		m_rightMouseDown = (action == GLFW_PRESS);
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		m_middleMouseDown = (action == GLFW_PRESS);
}


void Application::scrollCallback(double xoffset, double yoffset) {
	(void)xoffset; // currently un-used
	m_distance *= pow(1.1f, -yoffset);
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}
