#include <GL/glew.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include "GLtypes.hpp"
#include "type_helpers.hpp"
#include "lua_modules.hpp"

struct mod_gl_buffers : refrakt::lua::modules::registrar<mod_gl_buffers> {
	mod_gl_buffers() {}

	static inline std::string name = "buffers";
	static inline std::string parent = "gl";
	static inline std::string description = "OpenGL Buffers (SSBO, FBO, VBO, etc.)";

	struct fbo_wrapper : public refrakt::events::gl_was_reset {
		GLuint id;

		fbo_wrapper() { glGenFramebuffers(1, &id); }
		~fbo_wrapper() { glDeleteFramebuffers(1, &id); }

		void on_notify(refrakt::events::gl_was_reset::tag){
			glGenFramebuffers(1, &id);
		}
	};

	using fbo_handle = std::shared_ptr<fbo_wrapper>;

	static void apply(sol::table mod) {

		mod["FRAMEBUFFER"] = GL_FRAMEBUFFER;
		mod["COLOR_ATTACHMENT0"] = GL_COLOR_ATTACHMENT0;

		mod.new_usertype<fbo_wrapper>("fbo");

		mod["gen_framebuffer"] = []() {
			return fbo_handle(new fbo_wrapper);
		};

		mod["bind_framebuffer"] = [](GLenum target, fbo_handle fbo) {
			glBindFramebuffer(target, fbo->id);
		};

		mod["framebuffer_texture_2d"] = 
			sol::overload(
				[](GLenum attachment, GLenum tex_target, refrakt::texture tex) {
					glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex_target, tex.handle(), 0);
				},
				[](GLenum attachment, GLenum tex_target, GLuint tex) {
					glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex_target, tex, 0);
				}
			);

		mod["unbind_framebuffer"] = []() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		};

		mod["draw_buffers"] = [](sol::table buffers) {
			std::vector<GLenum> b;
			for (int i = 0; buffers[i].valid(); i++) {
				b.push_back(buffers[i]);
			}
			glDrawBuffers(static_cast<GLsizei>(b.size()), b.data());
		};
	}
};