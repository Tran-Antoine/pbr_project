/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <nori/parser.h>
#include <nori/scene.h>
#include <nori/camera.h>
#include <nori/block.h>
#include <nori/timer.h>
#include <nori/bitmap.h>
#include <nori/sampler.h>
#include <nori/integrator.h>
#include <nori/gui.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>
#include <filesystem/resolver.h>
#include <thread>
#include <numeric>
#include <nori/octree.h>
#include <string>

using namespace nori;

static int threadCount = -1;
static bool gui = true;

void test(Scene* scene);

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Syntax: " << argv[0] << " <scene.xml> [--no-gui] [--threads N]" <<  endl;
        return -1;
    }

    std::string sceneName = "";
    std::string exrName = "";

    for (int i = 1; i < argc; ++i) {
        std::string token(argv[i]);
        if (token == "-t" || token == "--threads") {
            if (i+1 >= argc) {
                cerr << "\"--threads\" argument expects a positive integer following it." << endl;
                return -1;
            }
            threadCount = atoi(argv[i+1]);
            i++;
            if (threadCount <= 0) {
                cerr << "\"--threads\" argument expects a positive integer following it." << endl;
                return -1;
            }

            continue;
        }
        else if (token == "--no-gui") {
            gui = false;
            continue;
        }

        filesystem::path path(argv[i]);

        try {
            if (path.extension() == "xml") {
                sceneName = argv[i];

                /* Add the parent directory of the scene file to the
                   file resolver. That way, the XML file can reference
                   resources (OBJ files, textures) using relative paths */
                getFileResolver()->prepend(path.parent_path());
            } else if (path.extension() == "exr") {
                /* Alternatively, provide a basic OpenEXR image viewer */
                exrName = argv[i];
            } else {
                cerr << "Fatal error: unknown file \"" << argv[i]
                     << "\", expected an extension of type .xml or .exr" << endl;
            }
        } catch (const std::exception &e) {
            cerr << "Fatal error: " << e.what() << endl;
            return -1;
        }
    }

    if (exrName !="" && sceneName !="") {
        cerr << "Both .xml and .exr files were provided. Please only provide one of them." << endl;
        return -1;
    }
    else if (exrName == "" && sceneName == "") {
        cerr << "Please provide the path to a .xml (or .exr) file." << endl;
        return -1;
    }
    else if (exrName != "") {
        if (!gui) {
            cerr << "Flag --no-gui was set. Please remove it to display the EXR file." << endl;
            return -1;
        }
        try {
            Bitmap bitmap(exrName);
            ImageBlock block(Vector2i((int) bitmap.cols(), (int) bitmap.rows()), nullptr);
            block.fromBitmap(bitmap);
            nanogui::init();
            NoriScreen *screen = new NoriScreen(block);
            nanogui::mainloop(50.f);
            delete screen;
            nanogui::shutdown();
        } catch (const std::exception &e) {
            cerr << e.what() << endl;
            return -1;
        }
    }
    else { // sceneName != ""
        if (threadCount < 0) {
            threadCount = tbb::task_scheduler_init::automatic;
        }
        try {
            std::unique_ptr<NoriObject> root(loadFromXML(sceneName));
            /* When the XML root object is a scene, start rendering it .. */
            if (root->getClassType() == NoriObject::EScene) {
                test(static_cast<Scene *>(root.get()));
            }
        } catch (const std::exception &e) {
            cerr << e.what() << endl;
            return -1;
        }
    }

    return 0;
}

void test(Scene* scene) {
    SetConsoleCP(437);
    SetConsoleOutputCP(437);

    Mesh* mesh = scene->getMeshes()[0];
    std::cout << "Triangles:" + std::to_string(mesh->getTriangleCount()) << std::endl;

    std::vector<uint32_t> indices(50);
    std::iota(indices.begin(), indices.end(), 0);

    Octree* tree = Octree::build(scene->getBoundingBox(), mesh, indices);

    std::cout << "Tree with first 50 triangles:\n" + tree->pretty_print() << std::endl;
}