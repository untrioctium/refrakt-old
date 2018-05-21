#include <set>

class RefraktWidget
{
public:
  // load an initial state from a package.json from within /programs/program_name/
  static std::shared_ptr<RefraktWidget> load( std::string program_name );

  sol::state& getLuaState() { return lua_state_; };

  template<typename T> T& get(const std::string& parameter) {
	  return parameters_[parameter]["value"].get<T&>();
  }

  std::string serialize();

  bool deserialize(std::string s);

  void drawGui();

private:
  sol::state lua_state_;

  void loadBindings();

  static const std::set<std::string> simple_types_;

  std::set<std::string> registered_types_;
  std::vector<std::string> draw_order_;
  sol::table parameters_;
};
