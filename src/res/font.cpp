//
// This file is part of Luola2.
// Copyright (C) 2012 Calle Laakkonen
//
// Luola2 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Luola2 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Luola2.  If not, see <http://www.gnu.org/licenses/>.
//
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <cstring>
#include <cstdarg>

#include <boost/algorithm/string.hpp>

#include "../util/tinyxml2.h"
#include "../fs/datafile.h"

#include "font.h"
#include "texture.h"
#include "shader.h"

namespace resource {

namespace {
    struct CharDescription {
        // The glyph rectangle UV coordinates
        int left, top, right, bottom;

        // The glyph width (i.e. how much space to reserve)
        float width;

        // Glyph rectangle offset
        int offx, offy;

        // Glyph vertex offset
        int index;
    };

    // TODO unicode support
    typedef std::unordered_map<char, CharDescription> CharMap;
    typedef std::unordered_map<char, int> GlyphMap;

    /**
     * Parse a single character description element.
     */
    CharDescription parseCharElement(const tinyxml2::XMLElement *el)
    {
        std::vector<string> rect;
        string rectstr = el->Attribute("rect");
        boost::split(rect, rectstr, boost::is_any_of(" "));
        if(rect.size() != 4)
            throw ResourceException("", "", "Expected char element to have 4 rect points!");

        std::vector<string> offset;
        string offsetstr = el->Attribute("offset");
        boost::split(offset, offsetstr, boost::is_any_of(" "));
        if(offset.size() != 2)
            throw ResourceException("", "", "Expected char element to have 2 offset points!");

        CharDescription cd;

        cd.left = atoi(rect[0].c_str());
        cd.top = atoi(rect[1].c_str());
        cd.right = cd.left + atoi(rect[2].c_str());
        cd.bottom = cd.top + atoi(rect[3].c_str());

        cd.width = atof(el->Attribute("width"));
        cd.offx = atoi(offset[0].c_str());
        cd.offy = atoi(offset[1].c_str());

        // TODO kerning

        return cd;
    }

    /**
     * Parse font description file (Divo compatible XML, as generated by
     * FontBuilder)
     */
    CharMap parseFontDescription(const string &xmlstring)
    {
        using namespace tinyxml2;

        XMLDocument doc;
        int error = doc.Parse(xmlstring.c_str());
        if(error != XML_NO_ERROR)
            throw ResourceException("", "", doc.GetErrorStr1());

        const XMLElement *root = doc.RootElement();
        CharMap charmap;

        const XMLElement *charEl = root->FirstChildElement();
        while(charEl) {
            if(strcmp(charEl->Name(), "Char"))
                throw ResourceException("", "", string("Unhandled font description element: ") + charEl->Name());

            charmap[charEl->Attribute("code")[0]] = parseCharElement(charEl);

            charEl = charEl->NextSiblingElement();
        }

        return charmap;
    }
}

// Private implementation so we don't leak all the messy details outside
// this module.
class FontImpl {
public:
    FontImpl(const CharMap &charmap, Texture *texture, Program *program)
        : m_charmap(charmap)
    {
        // Create vertices and their UV coordinates.
        std::vector<glm::vec2> vertex;
        std::vector<glm::vec2> uv;

        const glm::vec2 scale(1.0f / texture->width(), 1.0f / texture->height());

        int ind = 0;
        for(auto &c : m_charmap) {
            c.second.index = ind;
            ind += 4;

            float x0 = c.second.offx * scale.x;
            float y0 = c.second.offy * scale.y;
            float w = (c.second.right - c.second.left) * scale.x;
            float h = (c.second.bottom - c.second.top) * scale.y;

            c.second.width *= scale.x;

            // Vertices
            vertex.push_back(glm::vec2(x0, -y0));
            vertex.push_back(glm::vec2(x0 + w, -y0));
            vertex.push_back(glm::vec2(x0 + w, -y0 - h));
            vertex.push_back(glm::vec2(x0, -y0 - h));

            // Texture coordinates
            uv.push_back(glm::vec2(c.second.left, c.second.top) * scale);
            uv.push_back(glm::vec2(c.second.right, c.second.top) * scale);
            uv.push_back(glm::vec2(c.second.right, c.second.bottom) * scale);
            uv.push_back(glm::vec2(c.second.left, c.second.bottom) * scale);

        }

        // Create the vertex array object for the font
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        // Get uniform locations
        m_offset_uniform = glGetUniformLocation(program->id(), "offset");
        m_texture_uniform = glGetUniformLocation(program->id(), "fontSampler");
        m_color_uniform = glGetUniformLocation(program->id(), "color");
        m_scale_uniform = glGetUniformLocation(program->id(), "scale");

        glGenBuffers(3, m_buffers);

        // Bind vertex buffer (0)
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(glm::vec2) * vertex.size(),
            vertex.data(),
            GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Bind UV buffer (1)
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[1]);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(glm::vec2) * uv.size(),
            uv.data(),
            GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Bind element index buffer
        // Only two triangles are need to draw each glyph.
        // They can be represented simply as a triangle strip.
        static const GLushort elements[] = { 0, 1, 3, 2 };

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[2]);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            sizeof(elements),
            elements,
            GL_STATIC_DRAW);

        glBindVertexArray(0);

        m_program_id = program->id();
        m_texture_id = texture->id();
    }

    ~FontImpl()
    {
        glDeleteBuffers(3, m_buffers);
        glDeleteVertexArrays(1, &m_vao);
    }

    void renderText(
        const char *text,
        float scale,
        glm::vec2 pos,
        const glm::vec4 &color,
        TextRenderer::Alignment align)
    {
        glBindVertexArray(m_vao);

        glUseProgram(m_program_id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
        glUniform1i(m_texture_uniform, 0);

        glUniform4fv(m_color_uniform, 1, &color[0]);
        glUniform1f(m_scale_uniform, scale);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if(align == TextRenderer::RIGHT) {
            float xoff = 0;
            const char *s = text;
            while(*s) {
                const CharDescription &chr = m_charmap[*s];
                xoff += chr.width;
                ++s;
            }
            pos.x -= xoff * scale;
        }

        const char *s = text;
        while(*s) {
            const CharDescription &chr = m_charmap[*s];

            glUniform2fv(m_offset_uniform, 1, &pos[0]);

            glDrawElementsBaseVertex(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0, chr.index);
            pos.x += chr.width * scale;
            ++s;
        }
        glDisable(GL_BLEND);
    }

private:
    CharMap m_charmap;

    // Our vertex, UV and element index buffers
    GLuint m_buffers[3];

    // Our vertex array object
    GLuint m_vao;

    // ID references
    GLuint m_program_id;
    GLuint m_texture_uniform;
    GLuint m_texture_id;
    GLuint m_offset_uniform;
    GLuint m_color_uniform;
    GLuint m_scale_uniform;
};

Font *Font::load(
    const string& name,
    fs::DataFile &datafile,
    const string &descfile,
    Texture *texture,
    Program *program)
{
    // Load font description
    string fontdesc;
    {
        fs::DataStream ds(datafile, descfile);
        fontdesc = string(
            (std::istreambuf_iterator<char>(ds)),
            std::istreambuf_iterator<char>());
    }
    CharMap charmap;
    try {
        charmap = parseFontDescription(fontdesc);
    } catch(const ResourceException &ex) {
        throw ResourceException(datafile.name(), descfile, ex.error());
    }

    // Private implementation class handles the rest
    FontImpl *impl = new FontImpl(charmap, texture, program);

    Font *res = new Font(name, impl);
    Resources::getInstance().registerResource(res);
    return res;
}

Font::Font(const string& name, FontImpl *impl)
    : Resource(name, FONT), m_priv(impl)
{
}

Font::~Font()
{
    delete m_priv;
}

TextRenderer Font::text(const string &text)
{
    return TextRenderer(this, text.c_str());
}

TextRenderer Font::text(const char *text, ...)
{
    static char str[512];
    va_list arguments;
    va_start(arguments, text);
    vsnprintf(str, sizeof(str), text, arguments);
    va_end(arguments);
    return TextRenderer(this, str);
}

TextRenderer::TextRenderer(Font *font, const char *text)
    : m_font(font), m_text(text), m_scale(1.0f), m_color(1.0f), m_pos(0), m_align(LEFT)
{
}

void TextRenderer::render()
{
    m_font->m_priv->renderText(m_text, m_scale, m_pos, m_color, m_align);
}

}
