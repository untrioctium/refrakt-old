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
			bounds = { {-1, 1}, {1, 1} }
		},
		
		{
			name = "scale",
			type = "float",
			description = "Picture scale",
			tool_tip = "Picture scale, where 1 means that the vertical length is 1 unit",
			bounds = { 0, 10000 },
			scale_type = "log"
		},
		
		{
			name = "exponent",
			type = "vec2",
			description = "Exponent",
			tool_tip = "Complex exponent",
			bounds = { {-4, -4}, {4, 4} }
		},
		
		{
			name = "escape_radius",
			type = "float",
			description = "Escape radius",
			tool_tip = "Value to consider iterated points too far away",
			bounds = { .5, 4 }
		},
		
		{
			name = "max_iterations",
			type = "integer",
			description = "Max iterations",
			tool_tip = "Number of iterations before considering a point in the set",
			bounds = {100, 1000}
		},
		
		{
			name = "julia_mode",
			type = "bool",
			tool_tip = "Generate Julia sets for a point",
			description = "Julia Mode"
		},
		
		{
			name = "julia_c",
			type = "vec2",
			description = "Julia C",
			tool_tip = "C for the Julia set",
			bounds = { {-2, -2}, {2, 2} },
			depends_on = "julia_mode"
		},
		
		{
			name = "burning_ship",
			type = "bvec2",
			description = "Burning ship mode",
			tool_tip = "Abs() components (both for classic 'Burning Ship' fractal)",
			child_names = {"Re", "Im"}
		}
	},
	
	code = {
		type = "compute",
		file = "escape.frag"
	}
}