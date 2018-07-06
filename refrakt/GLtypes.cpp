#include "GLtypes.hpp"
#include "type_helpers.hpp"
#include <initializer_list>
#include <iostream>

namespace /*type_string*/ refrakt {
#define MAKE_TYPE_STRING_FUNCTION(name) auto type_string( const name& value ) -> const std::string { return #name; }

	auto type_string(const arg_t& arg) -> const std::string {
		return std::visit([](auto&& v) -> const std::string {
			return type_string(v);
		}, arg);
	}

	auto type_string(const refrakt::float_t&) -> const std::string { return "float"; }
	auto type_string(const refrakt::double_t&) -> const std::string { return "double"; }
	auto type_string(const refrakt::int32_t&) -> const std::string { return "int32"; }
	auto type_string(const refrakt::uint32_t&) -> const std::string { return "uint32"; }
	auto type_string(const refrakt::struct_t& val) -> const std::string { return val.type_string(); }

	MAKE_TYPE_STRING_FUNCTION(vec2);
	MAKE_TYPE_STRING_FUNCTION(dvec2);
	MAKE_TYPE_STRING_FUNCTION(ivec2);
	MAKE_TYPE_STRING_FUNCTION(uvec2);

	MAKE_TYPE_STRING_FUNCTION(vec3);
	MAKE_TYPE_STRING_FUNCTION(dvec3);
	MAKE_TYPE_STRING_FUNCTION(ivec3);
	MAKE_TYPE_STRING_FUNCTION(uvec3);

	MAKE_TYPE_STRING_FUNCTION(vec4);
	MAKE_TYPE_STRING_FUNCTION(dvec4);
	MAKE_TYPE_STRING_FUNCTION(ivec4);
	MAKE_TYPE_STRING_FUNCTION(uvec4);

	MAKE_TYPE_STRING_FUNCTION(mat2x2);
	MAKE_TYPE_STRING_FUNCTION(mat2x3);
	MAKE_TYPE_STRING_FUNCTION(mat2x4);
	MAKE_TYPE_STRING_FUNCTION(mat3x2);
	MAKE_TYPE_STRING_FUNCTION(mat3x3);
	MAKE_TYPE_STRING_FUNCTION(mat3x4);
	MAKE_TYPE_STRING_FUNCTION(mat4x2);
	MAKE_TYPE_STRING_FUNCTION(mat4x3);
	MAKE_TYPE_STRING_FUNCTION(mat4x4);

	MAKE_TYPE_STRING_FUNCTION(dmat2x2);
	MAKE_TYPE_STRING_FUNCTION(dmat2x3);
	MAKE_TYPE_STRING_FUNCTION(dmat2x4);
	MAKE_TYPE_STRING_FUNCTION(dmat3x2);
	MAKE_TYPE_STRING_FUNCTION(dmat3x3);
	MAKE_TYPE_STRING_FUNCTION(dmat3x4);
	MAKE_TYPE_STRING_FUNCTION(dmat4x2);
	MAKE_TYPE_STRING_FUNCTION(dmat4x3);
	MAKE_TYPE_STRING_FUNCTION(dmat4x4);
}

namespace /*struct_t*/ refrakt {

	struct_t::struct_t(std::initializer_list<std::pair<std::string, refrakt::arg_t>> l) {
		for (auto f : l) members_.insert(f);
	}

	auto struct_t::add(const std::string& name, const std::string& type) -> refrakt::arg_t& {
		return this->members_.insert({ name, refrakt::type_helpers::factory(type) }).first->second;
	}

	auto struct_t::get(const std::string& name) -> refrakt::arg_t& {
		return this->members_[name];
	}

	auto struct_t::get(const std::string& name) const -> const refrakt::arg_t& {
		return this->members_.at(name);
	}

	auto struct_t::list() const -> std::vector<std::string> {
		std::vector<std::string> ret;
		for (auto kv : members_) { ret.push_back(kv.first); }
		return ret;
	}

	auto struct_t::type_string() const -> const std::string {
		return "struct";
	}

}

namespace /*json*/ refrakt {

	void to_json(nlohmann::json& j, const refrakt::arg_t& a) {
		std::visit([&](auto&& v) { j = v; }, a);
	}

	void to_json(nlohmann::json& j, const refrakt::struct_t& s) {
		for (auto&& kv : s) j.emplace(kv.first + ":" + refrakt::type_string(kv.second), kv.second);
	}

	void from_json(const nlohmann::json& j, refrakt::struct_t& s) {
		for (nlohmann::json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
			const std::size_t colon_location = it.key().find(":");
			std::string name = it.key().substr(0, colon_location);
			std::string type = it.key().substr(colon_location + 1);

			std::visit([&](auto&& arg) {
				using arg_type = std::decay_t<decltype(arg)>;

				if constexpr(refrakt::is_static_array_v<arg_type>) {
					const std::size_t size = std::min(arg.size(), it.value().size());
					for (std::size_t i = 0; i < size; i++)
						arg[i] = it.value()[i].get<arg_type::value_type>();
				}
				else if constexpr (std::is_same_v<arg_type, refrakt::struct_t>) {
					from_json(it.value(), arg);
				}
			}, s.add(name, type));
		}
	}
}