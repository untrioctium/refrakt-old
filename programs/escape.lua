return {
	meta = {
		name = "Escape",
		author = "Alex Riley"
	},
	
	parameters = {
		{
			name = "center",
			type = "vec2",
			description = "Camera center",
			tool_tip = "Center point of the viewing window",
			bounds = { -1, 1 },
			speed = .01
		},
		
		{
			name = "scale",
			type = "float",
			description = "Picture scale",
			tool_tip = "Picture scale, where 1 means that the vertical length is 1 unit",
			bounds = { 0, 2 },
			speed = .00001,
			power = .5
		},
		
		{
			name = "exponent",
			type = "vec2",
			description = "Exponent",
			tool_tip = "Complex exponent",
			bounds = { -4, 4 },
			speed = .01
		},
		
		{
			name = "escape_radius",
			type = "float",
			description = "Escape radius",
			tool_tip = "Value to consider iterated points too far away",
			bounds = { .5, 16 },
			speed = .01
		},
		
		{
			name = "max_iterations",
			type = "uint32",
			description = "Max iterations",
			tool_tip = "Number of iterations before considering a point in the set",
			bounds = {100, 1000}
		},
		
		{
			name = "julia",
			type = "vec2",
			tool_tip = "Julia weight (both 1.0 for Julia set fractal)",
			bounds = {0, 1},
			description = "Julia Mode",
			speed = .01
		},
		
		{
			name = "julia_c",
			type = "vec2",
			description = "Julia C",
			tool_tip = "C for the Julia set",
			bounds = { -2, 2 },
			depends_on = "julia_mode",
			speed = .01
		},
		
		{
			name = "burning_ship",
			type = "vec2",
			description = "Burning ship weights",
			bounds = {0, 1},
			tool_tip = "Abs() weight on components (both 1.0 for classic 'Burning Ship' fractal)",
			child_names = {"Re", "Im"},
			speed = .01
		}
	},

	code = {
		type = "fragment",
		file = "escape.frag"
	}
}