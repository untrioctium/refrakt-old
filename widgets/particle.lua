widget.name = "particle"
widget.author = "Alex Riley"
widget.requires = {"gl", "glm"}

widget.input = {
	pos = {
		type = "texture",
		name = "positions",
		desc = "a texture consisting of particle positions"
	},
	col = {
		type = "texture",
		name = "colors",
		desc = "a texture consisting of particle colors"
	},
	scale = {
		type = "float",
		name = "particle scale",
		desc = "scale output in all directions by this value"
		default = 1.0,
		bounds = {.1, 10},
		speed = .01,
	},
	rotation = {
		type = "vec3",
		name = "rotation",
		desc = "pitch/yaw/roll for the output (in degrees)",
		default = 0.0,
		bounds = {-180, 180},
		speed = .1
	},
	clear = {
		type = "uint32",
		name = "output clear"
		desc = "clear output before drawing (0: no, 1: yes)",
		default = 1,
		bounds = {0,1},
		speed = 1
	}
}

widget.output.result = {
	type = "texture",
	name = "Result",
	desc = "The rendered particles"
}

function widget.setup(data, args)
	local vp = gl.make_shader(gl.VERTEX_SHADER, [[
		#version 430
		out vec4 frag_color;
	
		uniform ivec2 dim;
		uniform mat4 view;

		uniform sampler2D pos;
		uniform sampler2D col;

		void main() {
			vec2 vertex_pos = vec2( (gl_VertexID % dim.x) / float(dim.x), (gl_VertexID / dim.x) / float(dim.y));
			vec3 v = texture(pos, vertex_pos).xyz;
			frag_color = texture(col, vertex_pos);

			if( isnan( dot(frag_color, vec4(1.0))) ) frag_color = vec4(0.0, 0.0, 0.0, 0.0);

			gl_Position = view * vec4(v.xyz, 1.0);
		}
	]])

	local fp = gl.make_shader(gl.FRAGMENT_SHADER, [[
		#version 430
		in vec4 frag_color;
		layout(location=0) out vec4 color;

		void main() { color = frag_color; }
	]])

	data.program = gl.make_program(vp, fp)
	data.fbo = gl.gen_framebuffer()
end

function widget.run(data, input, output)
	gl.enable(gl.BLEND)
	gl.blend_func(gl.ONE, gl.ONE)

	gl.use_program(data.program)
	gl.bind_framebuffer(data.framebuffer)

	local total_elements = input.pos.w * input.pos.h
	gl.uniform(data.program, "dim", ivec2(input.pos.w, input.pos.h))
	
	gl.uniform(data.program, "pos", uint32(0))
	gl.active_texture(gl.TEXTURE0)
	gl.bind_texture(GL_TEXTURE_2D, input.pos)

	gl.uniform(data.program, "pos", uint32(1))
	gl.active_texture(gl.TEXTURE1)
	gl.bind_texture(GL_TEXTURE_2D, input.col)

	local projection = glm.perspective(glm.radians(70), output.result.w/output.result.h, 0.1, 100.0)
	projection = projection * glm.look_at(vec3(0, 0, -1), vec3(0, 0, 0), vec3(0, 1, 0))
	projection = projection * glm.euler_angle_yzx(glm.radians(input.rot.x), glm.radians(input.rot.y), glm.radians(input.rot.z))

	gl.uniform(data.program, "view", projection)

	gl.framebuffer_texture_2d(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT_0, gl.TEXTURE_2D, output.result)
	gl.draw_buffer(gl.COLOR_ATTACHMENT_0)
	if input.clear.x == 1 then
		gl.clear_color(vec4(0.0))
		gl.clear(gl.COLOR_BUFFER_BIT)
	end

	gl.viewport(0.0, 0.0, output.result.w, output.result.h)

	gl.draw_arrays(gl.POINTS, 0, total_elements)

	gl.clear_program()
	gl.unbind_framebuffer(gl.FRAMEBUFFER)
	gl.disable(gl.BLEND)
end


