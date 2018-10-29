widget = {}
data = {}

widget.name = "glsl"
widget.author = "Alex Riley"
widget.requires = {"gl"}
widget.is_abstract = true

widget.arguments = {}
widget.arguments.source = {
	type = "string",
	name = "source code",
	desc = "GLSL fragment shader code"
}

function widget.setup(args)
	local vp, vp_compiled = gl.make_shader(gl.VERTEX_SHADER, [[
			#version 430
			out vec2 pos;

			const vec2 verts[4] = vec2[](
				vec2(-1.0f, -1.0f),
				vec2(-1.0f, 1.0f),
				vec2(1.0f, -1.0f),
				vec2(1.0f, 1.0f)
			);

			void main() {
				pos = verts[gl_VertexID] * 0.5 + 0.5;
				gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
			}
	]])

	local fp, fp_compiled = gl.make_shader(gl.FRAGMENT_SHADER, args.source)
	data.program, program_linked = gl.make_program(vp, fp)
	data.fbo = gl.gen_framebuffer()
end

function widget.run(input, output)
	--local old_viewport = gl.get(gl.VIEWPORT)

	gl.use_program(data.program)
	gl.bind_framebuffer(gl.FRAMEBUFFER, data.fbo)

	local bound_input_textures = 0

	for name, arg in pairs(input) do
		if arg.type == "texture" then
			gl.uniform(data.program, name, uint32(bound_input_textures))
			gl.active_texture(gl.TEXTURE_0 + bound_input_textures)
			gl.bind_texture(gl.TEXTURE_2D, arg)
			bound_input_textures = bound_input_textures + 1
		else
			gl.uniform(data.program, name, arg)
		end
	end

	local smallest = uvec2(100000)
	local draw_buffers = {}

	for name, arg in pairs(output) do
		if arg.type == "texture" then
			if arg.w < smallest.x then smallest.x = arg.w end
			if arg.h < smallest.y then smallest.y = arg.h end

			local location = gl.get_frag_data_location(data.program, name)
			gl.framebuffer_texture_2d(gl.COLOR_ATTACHMENT0 + location, gl.TEXTURE_2D, arg)
			draw_buffers[location] = gl.COLOR_ATTACHMENT0 + location
		end
	end

	gl.viewport(0, 0, smallest.x, smallest.y)
	gl.draw_buffers(draw_buffers)

	gl.draw_arrays(gl.TRIANGLE_STRIP, 0, 4)
	gl.clear_program()
	gl.unbind_framebuffer()
	--gl.viewport(old_viewport)
end