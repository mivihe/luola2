#include <glm/gtc/matrix_transform.hpp>

#include "world.h"
#include "renderer.h"
#include "res/font.h"

#include "ship/ship.h"

Renderer::Renderer(const World &world)
    : m_world(world)
{
    m_font = Resources::get<FontResource>("core.font.default");
}

void Renderer::render(double frametime) const
{
    glClear( GL_COLOR_BUFFER_BIT );

    glm::mat4 proj = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    glm::mat4 view = glm::lookAt(
        glm::vec3(0,0,20),
                                 glm::vec3(0,0,0),
                                 glm::vec3(0,1,0)
    );

    glm::mat4 PV = proj * view; 
    for(const terrain::Zone *zone : m_world.m_zones)
        zone->draw(PV);

    for(const terrain::Solid *solid : m_world.m_static_terrain)
        solid->draw(PV);

    for(const terrain::Solid *solid : m_world.m_dyn_terrain)
        solid->draw(PV);

    for(const Ship *ship : m_world.m_ships)
        ship->draw(PV);

#if 0
    for(int i=0;i<OBJS;++i) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(objects[i].position().x, objects[i].position().y, 0));
        ship1->render(PV * model);
    }
#endif

    m_font->text("FPS: %.1f", 1.0 / frametime)
    .scale(0.5).pos(1,1).align(TextRenderer::RIGHT).color(1,1,0)
    .render();

    glfwSwapBuffers();
}