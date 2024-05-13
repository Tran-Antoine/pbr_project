#include <core/adaptive.h>
#include <core/camera.h>
#include <core/vector.h>
#include <core/color.h>
#include <gui/block.h>
#include <integrator/integrator.h>
#include <core/scene.h>
#include <stats/sampler.h>
#include <gui/bitmap.h>
#include <tbb/tbb.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>

int N_RADIUS = 10;
int P_RADIUS = 3;

NORI_NAMESPACE_BEGIN

struct PixelData {
    Color3f color;
    int totalSamplesUsed;
    int currentSampleCount;
    Color3f var;
};

using ImageData = Eigen::Array<PixelData, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

Color3f sqr(const Color3f& c) {
    return c*c;
}

std::vector<Point2i> neighbors(const Point2i& p) {
    std::vector<Point2i> pixels((2*N_RADIUS + 1) * (2*N_RADIUS + 1));

    for(int i = -N_RADIUS; i <= N_RADIUS; i++) {
        for(int j = -N_RADIUS; j <= N_RADIUS; j++) {
            Vector2i pos = p + Vector2i(i, j);
            pixels[i++] = pos;
        }
    }
    return pixels;
}


Color3f color(const Point2i& p, const ImageData& image) {
    return image(p.y(), p.x()).color;
}

Color3f variance(const Point2i &p, const ImageData& image) {
    return image(p.y(), p.x()).var;
}

Color3f variance(const Point2i& a, const Point2i& b, const ImageData& image) {
    Color3f va = variance(a, image);
    Color3f vb = variance(b, image);
    return Color3f(std::min(va.r(), vb.r()), std::min(va.g(), vb.g()), std::min(va.b(), vb.b()));
}

Color3f distance(const Point2i& p, const Point2i& q, float alpha, float k, const ImageData& image) {
    return sqr(color(p, image) - color(q, image)) - alpha * (variance(p, image) + variance(p, q, image))
           / (10e-10 + k*k * (variance(p, image) + variance(q, image)));
}

float d2(const Point2i& p, const Point2i& q, float alpha, float k, const ImageData& image) {

    float norm = 1.0 / (3*(2*P_RADIUS+1)*(2*P_RADIUS+1));

    float sum = 0;

    for(const auto& offset : neighbors(0)) {
        Color3f dis = distance(p + offset, q + offset, alpha, k, image);
        sum += dis.x() + dis.y() + dis.z();
    }

    return norm * sum;
}

float weight(const Point2i& p, const Point2i& q, float alpha, float k, const ImageData& image) {
    float exponent = std::max(0.0f, d2(p, q, alpha, k, image));
    return exp(exponent);
}

void split(const ImageData& in, ImageData& bufferA, ImageData& bufferB) {
    bufferA.resize(in.rows(), in.cols());
    bufferB.resize(in.rows(), in.cols());
    for(int i = 0; i < in.rows(); i++) {
        for(int j = 0; j < in.cols(); j++) {
            auto data = in(i, j);
            PixelData copy = {data.color, data.totalSamplesUsed, data.currentSampleCount / 2, data.var};
            bufferA(i, j) = copy;
        }
    }
}

Color3f nonLocalFilter(const Point2i& p, float alpha, float k, const ImageData& buffer, const ImageData& other) {

    Color3f out;
    float normalization = 0;

    for(const auto& neighbor : neighbors(p)) {
        float w = weight(p, neighbor, k, alpha, other);

        normalization += w;
        out += color(neighbor, buffer) * w;
    }

    return (1.0 / normalization) * out;
}

void applyFilter(float k, float alpha, ImageData& buffer, const ImageData& other) {
    for(int i = 0; i < buffer.rows(); i++) {
        for(int j = 0; j < buffer.cols(); j++) {
            Color3f newColor = nonLocalFilter(Point2i(j, i), alpha, k, buffer, other);
            buffer(i, j).color = newColor;
        }
    }
}

void merge(const ImageData& a, const ImageData& b, ImageData& out) {
    for(int i = 0; i < out.rows(); i++) {
        for(int j = 0; j < out.cols(); j++) {

            const PixelData& current = out(i, j);
            const PixelData& pa = a(i, j);
            const PixelData& pb = b(i, j);

            out(i, j) = {
						(pa.color + pb.color) / 2.0f,
                        current.totalSamplesUsed + pa.currentSampleCount + pb.currentSampleCount,
                        pa.currentSampleCount + pb.currentSampleCount,
                        (pa.var + pb.var) / 2.0f
                        };
        }
    }
}

int filterStep(float k, float alpha, ImageData& bufferA, ImageData& bufferB, ImageData& image) {

    applyFilter(k, alpha, bufferA, bufferB);
    applyFilter(k, alpha, bufferB, bufferA);

    merge(bufferA, bufferB, image);

    return 0;
}

Color3f estimateVariance(const Point2i& p, float k, float alpha, const ImageData& bufferA, const ImageData& bufferB) {
    return sqr(variance(p, bufferA) - variance(p, bufferB)) / 2.0f;
}

void estimateVariance(float k, float alpha, ImageData& bufferA, ImageData& bufferB) {
    for(int i = 0; i < bufferA.rows(); i++) {
        for(int j = 0; j < bufferA.cols(); j++) {
            Color3f estimated = estimateVariance(Point2i(j, i), k, alpha, bufferA, bufferB);
            bufferA(i, j).var = estimated;
            bufferB(i, j).var = estimated;
        }
    }
}

ImageData initBuffer(int sizeX, int sizeY, int initialBudget) {
    ImageData buffer;
    buffer.resize(sizeY, sizeX);
    for(int i = 0; i < buffer.rows(); i++) {
        for(int j = 0; j < buffer.cols(); j++) {
            buffer(i, j) = {Color3f(0.f), 0,initialBudget, 0.0f};
        }
    }
    return buffer;
}

static void renderBlock(const Scene *scene, Sampler *sampler, const ImageBlock &block, ImageData& imageData) {
    const Camera *camera = scene->getCamera();
    const Integrator *integrator = scene->getIntegrator();

    Point2i offset = block.getOffset();
    Vector2i size  = block.getSize();

    /* For each pixel and pixel sample sample */
    for (int y=0; y<size.y(); ++y) {
        for (int x=0; x<size.x(); ++x) {

            Point2i absCoordinates = Point2i(x + offset.x(), y + offset.y());
            PixelData pixel = imageData(absCoordinates.y(), absCoordinates.x());

            Color3f output;

            for (uint32_t i=0; i< pixel.currentSampleCount; ++i) {
                Point2f pixelSample = Point2f((float) (x + offset.x()), (float) (y + offset.y())) + sampler->next2D();
                Point2f apertureSample = sampler->next2D();

                /* Sample a ray from the camera */
                Ray3f ray;
                Color3f value = camera->sampleRay(ray, pixelSample, apertureSample);

                /* Compute the incident radiance */
                value *= integrator->Li(scene, sampler, ray);

                output += value;
            }

            pixel.color = 1.0 / (pixel.totalSamplesUsed + pixel.currentSampleCount)
                        * (output + pixel.totalSamplesUsed * pixel.color);
            imageData(absCoordinates.y(), absCoordinates.x()) = pixel;
        }
    }
}

static void sampleStep(Scene *scene, ImageData& imageData) {

    const Camera *camera = scene->getCamera();
    Vector2i outputSize = camera->getOutputSize();

    /* Create a block generator (i.e. a work scheduler) */
    BlockGenerator blockGenerator(outputSize, NORI_BLOCK_SIZE);

    /* Allocate memory for the entire output image and clear it */
    ImageBlock result(outputSize, camera->getReconstructionFilter());
    result.clear();

    /* Do the following in parallel and asynchronously */
    std::thread render_thread([&] {
        tbb::task_scheduler_init init(tbb::task_scheduler_init::automatic);

        tbb::blocked_range<int> range(0, blockGenerator.getBlockCount());

        auto map = [&](const tbb::blocked_range<int> &range) {
            /* Allocate memory for a small image block to be rendered
               by the current thread */
            ImageBlock block(Vector2i(NORI_BLOCK_SIZE),
                             camera->getReconstructionFilter());

            /* Create a clone of the sampler for the current thread */
            std::unique_ptr<Sampler> sampler(scene->getSampler()->clone());

            for (int i=range.begin(); i<range.end(); ++i) {
                /* Request an image block from the block generator */
                blockGenerator.next(block);

                /* Inform the sampler about the block to be rendered */
                sampler->prepare(block);

                /* Render all contained pixels */
                renderBlock(scene, sampler.get(), block, imageData);

                /* The image block has been processed. Now add it to
                   the "big" block that represents the entire image */
                result.put(block);
            }
        };

        /// Default: parallel rendering
        tbb::parallel_for(range, map);

    });
    render_thread.join();
}

int cost(const ImageData& buffer) {
    int total = 0;
    for(int i = 0; i < buffer.rows(); i++) {
        for(int j = 0; j < buffer.cols(); j++) {
            total += buffer(i, j).currentSampleCount;
        }
    }
    return total;
}

static ImageData getInitialBuffer(Scene* scene, int samplingBudget) {
    const Camera *camera = scene->getCamera();
    Vector2i outputSize = camera->getOutputSize();
    scene->getIntegrator()->preprocess(scene);

    int initialBudget = samplingBudget / (20 * outputSize.x() * outputSize.y());

    return initBuffer(outputSize.x(), outputSize.y(), initialBudget);
}

void render(Scene *scene, const std::string &filename, int samplingBudget) {

    int usedBudget = 0;
    float k = 0, alpha = 0;

    ImageData current = getInitialBuffer(scene, samplingBudget);
    ImageData bufferA, bufferB;

    while(usedBudget < samplingBudget) {

        split(current, bufferA, bufferB);

        sampleStep(scene, bufferA);
        sampleStep(scene, bufferB);

        estimateVariance(k, alpha, bufferA, bufferB);

        filterStep(k, alpha, bufferA, bufferB, current);

        usedBudget += cost(current);
    }

    Bitmap result(Vector2i(current.cols(), current.rows()));
    for(int i = 0; i < current.rows(); i++) {
        for(int j = 0; j < current.cols(); j++) {
            result.coeffRef(i, j) = current(i, j).color;
        }
    }

    if(scene->saveResult()) {
        result.saveEXR(filename);
        result.savePNG(filename);
    }
}

NORI_NAMESPACE_END