return {
	meta = {
		name = "Escape",
		author = "Alex Riley"
	},
	
	input = {},
	output = {
		{
			name = "surface",
			type = "texture",
			size = "arbitrary",
			format = "RGBA8UI"
		},
	},

	parameters = {
		{
			name = "center",
			type = "vec2",
			description = "Camera center",
			tool_tip = "Center point of the viewing window",
			bounds = { -5, 5 },
			speed = .001
		},
		
		{
			name = "scale",
			type = "float",
			description = "Picture scale",
			tool_tip = "Picture scale, where 2 means that the vertical length is 1/2 units",
			bounds = { 1, 10000 },
			speed = .25,
			init = float(1)
		},
		{
			name = "hue_shift",
			type = "float",
			description = "Hue shift",
			tool_tip = "Modify color shading scale",
			bounds = { 0, 1},
			speed = .0001,
			init = float(0)
		},
		{
			name = "hue_stretch",
			type = "float",
			description = "Hue stretch",
			tool_tip = "How often colors repeat",
			bounds = { .25, 8},
			speed = .001,
			init = float(1)
		},
		
		{
			name = "exponent",
			type = "vec2",
			description = "Exponent",
			tool_tip = "Complex exponent",
			bounds = { -4, 4 },
			speed = .001,
			init = vec2(2.0, 0.0)
		},
		
		{
			name = "escape_radius",
			type = "float",
			description = "Escape radius",
			tool_tip = "Value to consider iterated points too far away",
			bounds = { .5, 16 },
			speed = .001,
			init = float(2.0)
		},
		
		{
			name = "max_iterations",
			type = "uint32",
			description = "Max iterations",
			tool_tip = "Number of iterations before considering a point in the set",
			bounds = {100, 1000},
			init = uint32(100)
		},
		
		{
			name = "julia",
			type = "vec2",
			tool_tip = "Julia weight (both 1.0 for Julia set fractal)",
			bounds = {0, 1},
			description = "Julia Mode",
			speed = .001
		},
		
		{
			name = "julia_c",
			type = "vec2",
			description = "Julia C",
			tool_tip = "C for the Julia set",
			bounds = { -2, 2 },
			depends_on = "julia_mode",
			speed = .001
		},
		
		{
			name = "burning_ship",
			type = "vec2",
			description = "Burning ship weights",
			bounds = {0, 1},
			tool_tip = "Abs() weight on components (both 1.0 for classic 'Burning Ship' fractal)",
			child_names = {"Re", "Im"},
			speed = .001
		},

		{
			name = "hq_mode",
			type = "uint32",
			description = "Enable HQ rendering",
			bounds = {0, 1},
			tool_tip = "Enables HQ mode which may include higher precisions and anti-aliasing"
		}
	},

	code = {
		type = "fragment",
		file = "escape.frag"
	}
}