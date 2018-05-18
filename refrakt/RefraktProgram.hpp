#include <set>

class RefraktProgram
{
public:
  // load an initial state from a package.json from within /programs/program_name/
  static std::shared_ptr<RefraktProgram> load( std::string program_name );

  sol::state& getLuaState() { return lua_state_; };

  void drawGui();

private:
  sol::state lua_state_;

  void loadBindings();

  static const std::set<std::string> simple_types_;

  std::set<std::string> registered_types_;
  std::vector<std::string> draw_order_;
  sol::table parameters_;
};
