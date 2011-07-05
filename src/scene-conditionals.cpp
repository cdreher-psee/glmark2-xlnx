/*
 * Copyright © 2010-2011 Linaro Limited
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Alexandros Frantzis (glmark2)
 */
#include "scene.h"
#include "mat.h"
#include "stack.h"
#include "vec.h"
#include "log.h"

#include <cmath>
#include <sstream>

static const std::string shader_file_base(GLMARK_DATA_PATH"/shaders/conditionals-");

static const std::string vtx_file_prologue(shader_file_base + "prologue.vert");
static const std::string vtx_file_step_conditional(shader_file_base + "step-conditional.vert");
static const std::string vtx_file_step_simple(shader_file_base + "step-simple.vert");
static const std::string vtx_file_epilogue(shader_file_base + "epilogue.vert");

static const std::string frg_file_prologue(shader_file_base + "prologue.frag");
static const std::string frg_file_step_conditional(shader_file_base + "step-conditional.frag");
static const std::string frg_file_step_simple(shader_file_base + "step-simple.frag");
static const std::string frg_file_epilogue(shader_file_base + "epilogue.frag");

SceneConditionals::SceneConditionals(Canvas &pCanvas) :
    Scene(pCanvas, "conditionals")
{
    mOptions["fragment-steps"] = Scene::Option("fragment-steps", "1",
            "The number of computational steps in the fragment shader");
    mOptions["fragment-conditionals"] = Scene::Option("fragment-conditionals", "true",
            "Whether each computational step includes an if-else clause");
    mOptions["vertex-steps"] = Scene::Option("vertex-steps", "1",
            "The number of computational steps in the vertex shader");
    mOptions["vertex-conditionals"] = Scene::Option("vertex-conditionals", "true",
            "Whether each computational step includes an if-else clause");
}

SceneConditionals::~SceneConditionals()
{
}

static std::string
get_vertex_shader_source(int steps, bool conditionals)
{
    std::string vtx_prologue, vtx_step_conditional, vtx_step_simple, vtx_epilogue;

    if (!gotSource(vtx_file_prologue, vtx_prologue) ||
        !gotSource(vtx_file_step_conditional, vtx_step_conditional) ||
        !gotSource(vtx_file_step_simple, vtx_step_simple) ||
        !gotSource(vtx_file_epilogue, vtx_epilogue))
    {
        return "";
    }

    std::stringstream ss;

    ss << vtx_prologue;
    for (int i = 0; i < steps; i++) {
        if (conditionals)
            ss << vtx_step_conditional;
        else
            ss << vtx_step_simple;
    }
    ss << vtx_epilogue;

    return ss.str();
}

static std::string
get_fragment_shader_source(int steps, bool conditionals)
{
    std::string frg_prologue, frg_step_conditional, frg_step_simple, frg_epilogue;

    if (!gotSource(frg_file_prologue, frg_prologue) ||
        !gotSource(frg_file_step_conditional, frg_step_conditional) ||
        !gotSource(frg_file_step_simple, frg_step_simple) ||
        !gotSource(frg_file_epilogue, frg_epilogue))
    {
        return "";
    }

    std::stringstream ss;

    ss << frg_prologue;
    for (int i = 0; i < steps; i++) {
        if (conditionals)
            ss << frg_step_conditional;
        else
            ss << frg_step_simple;
    }
    ss << frg_epilogue;

    return ss.str();
}

int SceneConditionals::load()
{
    std::vector<int> vertex_format;
    vertex_format.push_back(3);
    mMesh.set_vertex_format(vertex_format);

    mMesh.make_grid(32, 32, 5.0, 5.0, 0.02);
    mMesh.build_vbo();

    mRotationSpeed = 36.0f;

    mRunning = false;

    return 1;
}

void SceneConditionals::unload()
{
    mMesh.reset();
}

void SceneConditionals::setup()
{
    Scene::setup();

    bool vtx_conditionals = mOptions["vertex-conditionals"].value == "true";
    bool frg_conditionals = mOptions["fragment-conditionals"].value == "true";
    int vtx_steps = 0;
    int frg_steps = 0;

    std::stringstream ss;

    ss << mOptions["vertex-steps"].value;
    ss >> vtx_steps;
    ss.clear();
    ss << mOptions["fragment-steps"].value;
    ss >> frg_steps;

    std::string vtx_shader(get_vertex_shader_source(vtx_steps, vtx_conditionals));
    std::string frg_shader(get_fragment_shader_source(frg_steps, frg_conditionals));

    if (!Scene::load_shaders_from_strings(mProgram, vtx_shader, frg_shader))
        return;

    mProgram.start();

    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(mProgram.getAttribIndex("position"));
    mMesh.set_attrib_locations(attrib_locations);

    mCurrentFrame = 0;
    mRotation = 0.0f;
    mRunning = true;
    mStartTime = Scene::get_timestamp_us() / 1000000.0;
    mLastUpdateTime = mStartTime;
}

void SceneConditionals::teardown()
{
    mProgram.stop();
    mProgram.release();

    Scene::teardown();
}

void SceneConditionals::update()
{
    double current_time = Scene::get_timestamp_us() / 1000000.0;
    double dt = current_time - mLastUpdateTime;
    double elapsed_time = current_time - mStartTime;

    mLastUpdateTime = current_time;

    if (elapsed_time >= mDuration) {
        mAverageFPS = mCurrentFrame / elapsed_time;
        mRunning = false;
    }

    mRotation += mRotationSpeed * dt;

    mCurrentFrame++;
}

void SceneConditionals::draw()
{
    // Load the ModelViewProjectionMatrix uniform in the shader
    LibMatrix::Stack4 model_view;
    LibMatrix::mat4 model_view_proj(mCanvas.projection());

    model_view.translate(0.0f, 0.0f, -5.0f);
    model_view.rotate(mRotation, 0.0f, 0.0f, 1.0f);
    model_view_proj *= model_view.getCurrent();

    mProgram.loadUniformMatrix(model_view_proj, "ModelViewProjectionMatrix");

    mMesh.render_vbo();
}

Scene::ValidationResult
SceneConditionals::validate()
{
    return Scene::ValidationUnknown;
}
