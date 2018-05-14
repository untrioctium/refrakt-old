class RefraktProgram
{
public:
  // load an initial state from a package.json from within /programs/program_name/
  static std::shared_ptr<RefraktProgram> load( std::string program_name );

  sol::state& getLuaState() { return lua_state; };

//private:
  sol::state lua_state;

  static void loadBindings( sol::state& state );
};
