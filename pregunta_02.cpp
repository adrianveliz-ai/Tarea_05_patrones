#include <iostream>
#include <string>
#include<map>
#include<memory>
#include<functional>
using namespace std;

class Window {
public:
    virtual void draw() = 0;
    virtual ~Window() = default;
};

class Button {
public:
    virtual void draw() = 0;
    virtual ~Button() = default;
};

class Shader {
public:
    virtual void compile() = 0;
    virtual ~Shader() = default;
};

class RenderFactory {
public:
    virtual ~RenderFactory() = default;
    virtual unique_ptr<Window> createWindow() = 0;
    virtual unique_ptr<Shader> createShader() = 0;
    virtual unique_ptr<Button> createButton() = 0;
    virtual string name() = 0;
};

// ------------------------------------ OpenGL ------------------------------------

class OpenGLWindow : public Window {
public:
    void draw() override {
        cout << "[OpenGL] Creando ventana...\n";
    }
};

class OpenGLButton : public Button {
public:
    void draw() override {
        cout << "[OpenGL] Boton: Quad dibujado\n";
    }
};

class OpenGLShader : public Shader {
public:
    void compile() override {
        cout << "[OpenGL] Shader: GLSL compilado\n";
    }
};

class OpenGLFactory : public RenderFactory {
public:
    unique_ptr<Window> createWindow() override {
        return make_unique<OpenGLWindow>();
    }

    unique_ptr<Button> createButton() override {
        return make_unique<OpenGLButton>();
    }

    unique_ptr<Shader> createShader() override {
        return make_unique<OpenGLShader>();
    }

    string name() override {
        return "OpenGL";
    }
};

// ------------------------------------ Vulkan ------------------------------------

class VulkanWindow : public Window {
public:
    void draw() override {
        cout << "[Vulkan] Creando ventana...\n";
    }
};

class VulkanButton : public Button {
public:
    void draw() override {
        cout << "[Vulkan] Boton: Comandos grabados\n";
    }
};

class VulkanShader : public Shader {
public:
    void compile() override {
        cout << "[OpenGL] Shader: SPIR-V cargado\n";
    }
};

class VulkanFactory : public RenderFactory {
public:
    unique_ptr<Window> createWindow() override {
        return make_unique<VulkanWindow>();
    }

    unique_ptr<Button> createButton() override {
        return make_unique<VulkanButton>();
    }

    unique_ptr<Shader> createShader() override {
        return make_unique<VulkanShader>();
    }

    string name() override {
        return "Vulkan";
    }
};

void renderScene(RenderFactory &factory) {
    cout << "|--- Escena con " << factory.name() << " ---|\n";
    auto window = factory.createWindow();
    auto button = factory.createButton();
    auto shader = factory.createShader();
    window->draw();
    button->draw();
    shader->compile();
}

int main() {
    unique_ptr<RenderFactory> factory = make_unique<OpenGLFactory>();
    auto window = factory->createWindow();
    auto button = factory->createButton();

    window->draw();
    button->draw();

    factory = make_unique<VulkanFactory>();

    auto shader = factory->createShader();
    shader->compile();

    map<string, function<unique_ptr<RenderFactory>()>> registry {
        {"OpenGL", []{return make_unique<OpenGLFactory>();}},
        {"Vulkan", []{return make_unique<VulkanFactory>();}},
    };

    cout << endl;
    for (const char* choice : {"OpenGL", "Vulkan"}) {
        factory = registry.at(choice)();
        renderScene(*factory);
        cout << endl;
    }

    return 0;
}
