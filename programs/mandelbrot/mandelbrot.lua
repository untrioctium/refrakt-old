function RefraktProgram:processGui( )

  self.gui:beginWindow("ayy lmao")
    if self.gui:colorEdit3("Background Color", "bg_color") then
      print(self.data.bg_color.r)
    end

    self.gui:inputText("Window Title", "window_title")

    if self.gui:button("Update window title") then
        print(self.data.window_title)
    end
  self.gui:endWindow()

end

function RefraktProgram:step()
  --local dt = self.gfx:getDeltaTime()
  self.data.ayylmao = 42
  io.write("ayy lmao\n")
end

function RefraktProgram:init()

end

io.write("Hello from outside\n");
